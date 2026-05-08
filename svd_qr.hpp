// =============================================================================
//  svd_qr_fornita.hpp
//  SVD di una matrice bidiagonale superiore tramite QR iterativa con shift
//  Algoritmo di Golub–Reinsch
//  Routine FORNITA — non modificare
// =============================================================================
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert> // usato per assert/verifica su dimensioni matrici

using Vec = std::vector<double>;
using Mat = std::vector<Vec>;

// ── utilità ──────────────────────────────────────────────────────────────────

static Mat crea_mat(int r, int c) { return Mat(r, Vec(c, 0.0)); }

static void applica_givens(double c, double s,
                           Vec& a, Vec& b, int i, int j) {
    double ai = a[i], bi = b[i];
    double aj = (j < (int)a.size()) ? a[j] : 0.0;
    double bj = (j < (int)b.size()) ? b[j] : 0.0;
    a[i]= c*ai + s*(i<(int)b.size()? b[i]:0);(void)bi;(void)bj;(void)aj;
}

// Applica una rotazione di Givens: elimina l'elemento (i,j) usando (i,k)
// Versione su colonne di una matrice
static void givens_cols(Mat& M, int p, int q,
                        double c, double s) {
    int n = M.size();
    for (int i = 0; i < n; ++i) {
        double mp = M[i][p], mq = M[i][q];
        M[i][p] =  c*mp + s*mq;
        M[i][q] = -s*mp + c*mq;
    }
}

// Applica una rotazione di Givens: su righe di una matrice
static void givens_rows(Mat& M, int p, int q,
                        double c, double s) {
    int n = (int)M[0].size();
    for (int j = 0; j < n; ++j) {
        double mp = M[p][j], mq = M[q][j];
        M[p][j] =  c*mp + s*mq;
        M[q][j] = -s*mp + c*mq;
    }
}

static void compute_givens(double a, double b, double& c, double& s) {
    if (b == 0.0) { c = 1.0; s = 0.0; return; }
    double r = std::hypot(a, b);
    c = a / r; s = b / r;
}

// ── SVD bidiagonale ───────────────────────────────────────────────────────────

// Calcola la SVD di B bidiagonale superiore p x d:
//     B = Ub * diag(sigma) * Vb^T
//
// con Ub (p x p) e Vb (d x d) ortogonali, sigma vettore di p valori singolari
// in ordine decrescente.
//
// Algoritmo: QR iterativa con shift di Wilkinson sulla matrice T = B^T B.
// Implementazione via rotazioni di Givens sulla forma bidiagonale (Golub-Reinsch).

inline void svd_bidiagonale(const Mat& B_in,
                             Mat& Ub, Mat& Vb, Vec& sigma) {
    int p = B_in.size();
    int d = B_in[0].size();
    std::cout << "svd_bidiagonale: SVD di matrice bidiagonale " << p << " x " << d << "...\n";
    assert(p <= d);

    // Estrai diagonale e sopradiagonale
    Vec diag(p), sup(p - 1);
    for (int i = 0; i < p; ++i) diag[i] = B_in[i][i];
    for (int i = 0; i < p-1; ++i) sup[i] = B_in[i][i+1];

    // Inizializza Ub = I_p,  Vb = I_d
    Ub = crea_mat(p, p);
    Vb = crea_mat(d, d);
    for (int i = 0; i < p; ++i) Ub[i][i] = 1.0;
    for (int i = 0; i < d; ++i) Vb[i][i] = 1.0;

    const int MAX_ITER = 200 * p;
    const double eps   = 1e-14;

    for (int iter = 0; iter < MAX_ITER; ++iter) {
        // Porta a zero le entrate sopradiagonali piccole
        for (int i = 0; i < p-1; ++i)
            if (std::abs(sup[i]) <= eps * (std::abs(diag[i]) + std::abs(diag[i+1])))
                sup[i] = 0.0;

        // Trova il blocco attivo [q1, q2]
        int q2 = p - 1;
        while (q2 > 0 && sup[q2-1] == 0.0) --q2;
        if (q2 == 0) break;   // converso

        int q1 = q2 - 1;
        while (q1 > 0 && sup[q1-1] != 0.0) --q1;

        // Shift di Wilkinson: autovalore di [[d^2+s^2, d*e],[d*e, e^2+d2^2]]
        // più vicino a diag[q2]^2
        double d1 = diag[q2-1], e = (q2 < p) ? sup[q2-1] : 0.0, d2 = diag[q2];
        double t11 = d1*d1 + (q2 >= 2 ? sup[q2-2]*sup[q2-2] : 0.0);
        double t22 = d2*d2 + e*e;
        double t12 = d1 * e;
        double diff = (t11 - t22) / 2.0;
        double mu = t22 - t12*t12 / (diff + std::copysign(std::hypot(diff, t12), diff));

        // Passo QR implicito sul blocco [q1, q2]
        double x = diag[q1]*diag[q1] - mu;
        double z = diag[q1] * (q1 < p-1 ? sup[q1] : 0.0);

        for (int k = q1; k < q2; ++k) {
            // Rotazione a destra (su Vb)
            double c, s;
            compute_givens(x, z, c, s);

            // Aggiorna la bidiagonale
            if (k > q1) sup[k-1] = c*sup[k-1] + s*diag[k];
            double dk = diag[k], sup_k = (k < p-1 ? sup[k] : 0.0);
            double dk1 = (k+1 < p ? diag[k+1] : 0.0);
            diag[k]   =  c*dk   + s*sup_k;
            sup[k]    = -s*dk   + c*sup_k;   if(k>=p-1) sup[k]=0;
            z         =           s*dk1;
            diag[k+1] =           c*dk1;     if(k+1>=p) diag[k+1]=0;

            givens_cols(Vb, k, k+1, c, s);

            // Rotazione a sinistra (su Ub)
            compute_givens(diag[k], z, c, s);
            diag[k] = c*diag[k] + s*z;
            if (k < p-1) {
                double sup_k2 = sup[k];
                double dk2    = diag[k+1];
                sup[k]    =  c*sup_k2 + s*dk2;
                diag[k+1] = -s*sup_k2 + c*dk2;
                if (k+1 < p-1) {
                    x = sup[k]; z = s * sup[k+1];
                    sup[k+1] = c * sup[k+1];
                }
            }
            givens_rows(Ub, k, k+1, c, s);
        }
    }

    // Assicura valori singolari non negativi
    for (int i = 0; i < p; ++i) {
        if (diag[i] < 0) {
            diag[i] = -diag[i];
            for (int j = 0; j < p; ++j) Ub[j][i] = -Ub[j][i];
        }
    }

    // Ordina in senso decrescente
    std::vector<int> idx(p);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b){ return diag[a] > diag[b]; });

    sigma.resize(p);
    Mat Ub2 = crea_mat(p, p);
    Mat Vb2 = crea_mat(d, d);
    for (int i = 0; i < d; ++i) Vb2[i][i] = 1.0;  // lascia le righe extra invariate

    for (int i = 0; i < p; ++i) {
        sigma[i] = diag[idx[i]];
        for (int j = 0; j < p; ++j) Ub2[j][i] = Ub[j][idx[i]];
        for (int j = 0; j < d; ++j) Vb2[j][i] = Vb[j][idx[i]];
    }
    // Copia le colonne extra di Vb (da p a d-1)
    for (int i = p; i < d; ++i)
        for (int j = 0; j < d; ++j)
            Vb2[j][i] = Vb[j][idx.size() > i ? i : i];  // identità per le rimanenti

    Ub = Ub2;
    Vb = Vb2;
}
