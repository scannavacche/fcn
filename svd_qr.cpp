#include <iostream>
#include <cmath>
#include "ap.h"
#include "alglibinternal.h"
#include "linalg.h"

using namespace alglib;

using Vec = std::vector<double>;
using Mat = std::vector<Vec>;

void set_identity(real_2d_array &A, int n)
{
    A.setlength(n, n);

    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            A[i][j] = (i == j ? 1.0 : 0.0);
}


void extract_bidiag(const real_2d_array& B,
                    real_1d_array& D,
                    real_1d_array& E,
                    bool isupper = true)
{
    int m = B.rows();
    int n = B.cols();

    int p = std::min(m, n);

    D.setlength(p);

    if(p > 1)
        E.setlength(p - 1);
    else
        E.setlength(0);

    // -------------------------
    // diagonale principale
    // -------------------------
    for(int i = 0; i < p; i++)
        D[i] = B[i][i];

    // -------------------------
    // upper bidiagonal
    // -------------------------
    if(isupper)
    {
        for(int i = 0; i < p - 1; i++)
            E[i] = B[i][i + 1];
    }
    else
    {
        // lower bidiagonal
        for(int i = 0; i < p - 1; i++)
            E[i] = B[i + 1][i];
    }
}

real_2d_array to_alglib(const Mat& A)
{
    printf("to alglib: inizio \n");

    int m = A.size();
    printf("to alglib: m = %d\n", m);
    int n = A[0].size();
    printf("to alglib: n = %d\n", n);

    real_2d_array B;
    B.setlength(m, n);

    for(int i = 0; i < m; i++)
        for(int j = 0; j < n; j++)
            B[i][j] = A[i][j];

    return B;
}

Mat from_alglib(const real_2d_array& A)
{
    int m = A.rows();
    int n = A.cols();

    Mat B(m, Vec(n));

    for(int i = 0; i < m; i++)
        for(int j = 0; j < n; j++)
            B[i][j] = A[i][j];

    return B;
}

Vec from_alglib(const real_1d_array& A)
{
    int n = A.length();

    Vec v(n);
    for(int i = 0; i < n; i++)
        v[i] = A[i];

    return v;
}


int svd_bidiagonale(const Mat& B_in, Mat& Ub, Mat& VTb, Vec& sigma){

    real_2d_array B;
    printf("svd_bidiagonale: inizio\n");
    B = to_alglib(B_in);
    printf("svd_bidiagonale: B convertita\n");

    ae_int_t n = B.cols(); //colonne
    ae_int_t m = B.rows(); //righe

    bool isupper = true;

    if (m<n) isupper = false;

    int p = std::min(m, n);

    // -------------------------
    // bidiagonale (d, e)
    // -------------------------
    real_1d_array d, e;
    d.setlength(p);
    e.setlength(p-1);

    printf("svd_bidiagonale: estrai diagonali da B \n");

    extract_bidiag(B, d, e, isupper);
    
    // -------------------------
    // output
    // -------------------------
    real_2d_array U, VT, C;
    C.setlength(p,p);

    set_identity(U, m);
    set_identity(VT, n);

    ae_int_t nru = m;
    ae_int_t ncvt = n;
    ae_int_t ncc = 0;

    bool ok = rmatrixbdsvd(
        d, e,
        p,
        isupper,   // upper bidiagonal
        true,      // high accuracy
        U, nru,
        C, ncc,
        VT, ncvt
    );

    if(!ok)
    {
        std::cout << "SVD failed\n";
        return 1;
    }

    Ub = from_alglib(U);
    VTb = from_alglib(VT);
    sigma = from_alglib(d);

    return 0;
}
