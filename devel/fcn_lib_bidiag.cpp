#include <cstdio>
#include <cstddef>
#include <cmath>
#include <cassert>

// ... dichiarazioni householder_* e bidiag_householder_inplace ...

static void print_matrix(const char* name,
                         const double* A,
                         std::size_t m,
                         std::size_t n);

static double frobenius_norm(const double* A,
                             std::size_t m,
                             std::size_t n);

static void matmul(const double* A,
                   const double* B,
                   double*       C,
                   std::size_t   m,
                   std::size_t   k,
                   std::size_t   n);

static void matmul_AT_B(const double* A,
                        const double* B,
                        double*       C,
                        std::size_t   m,
                        std::size_t   k,
                        std::size_t   n);

static void matmul_B_VT(const double* A,
                        const double* B,
                        double*       C,
                        std::size_t   m,
                        std::size_t   k,
                        std::size_t   n);

static void mat_subtract(const double* A,
                         const double* B,
                         double*       C,
                         std::size_t   m,
                         std::size_t   n);


void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v);

void householder_apply_left_to_matrix(double*       A,
                                      std::size_t   m,
                                      std::size_t   n,
                                      std::size_t   row_offset,
                                      std::size_t   len,
                                      const double* v);

void householder_apply_right_to_matrix(double*       A,
                                       std::size_t   m,
                                       std::size_t   n,
                                       std::size_t   col_offset,
                                       std::size_t   len,
                                       const double* v);

// Riduce A (m x n, m>=n, column-major) a forma bidiagonale superiore:
// U^T * A * V = B, con B bidiagonale. In-place in A:
// - diag e superdiag contengono B
// - sotto diag: code di u_k
// - sopra superdiag: code di v_k
void bidiag_householder_inplace(double*     A,
                                std::size_t m,
                                std::size_t n)
{
    assert(A != nullptr);
    std::size_t k, i;

    const std::size_t K = n; // numero passi (per m>=n)

    for (k = 0; k < K; ++k) {

        // --- Riflettore sinistro U_k (colonna k) ---

        // Sottospazio attivo: righe k..m-1, colonne k..n-1
        const std::size_t len_col = m - k;
        double* col_k = A + k * m;
        double* x_col = col_k + k; // A(k:m-1, k)

        double ubuf[1024];
        assert(len_col <= 1024);

        householder_construct_segment(x_col, len_col, ubuf);

        // Applica U_k a sinistra: A(k:m-1, k:n-1) <- (I - 2 u u^T) * ...
        householder_apply_left_to_matrix(A + k,    // inizio riga k
                                         m - k,   // m locale
                                         n - k,   // n locale
                                         0,
                                         len_col,
                                         ubuf);

        // Memorizza coda di u_k sotto la diagonale nella colonna k
        for (i = 1; i < len_col; ++i)
            col_k[k + i] = ubuf[i];

        // --- Riflettore destro V_k (riga k, da colonna k+1 in poi) ---

        if (k + 1 < n) {
            const std::size_t len_row = n - (k + 1);

            // Costruiamo w_k da x = A(k, k+1:n-1).
            // Poiché A è column-major, estraiamo la riga pezzo per pezzo.
            double wbuf[1024];
            double x_row[1024];
            assert(len_row <= 1024);

            std::size_t j;

            for (j = 0; j < len_row; ++j) {
                double* col_j = A + (k + 1 + j) * m;
                x_row[j] = col_j[k]; // A(k, k+1+j)
            }

            householder_construct_segment(x_row, len_row, wbuf);

            // Ora applichiamo V_k a destra: A(k:m-1, k+1:n-1) <- ... * (I - 2 w w^T)
            // Possiamo usare householder_apply_right_to_matrix sul sottoblocco.
            householder_apply_right_to_matrix(A + k * 1, // attenzione: servono offset di colonna
                                              m,
                                              n,
                                              k + 1,
                                              len_row,
                                              wbuf);

            // Memorizza coda di w_k sopra la superdiagonale nella riga k:
            // la componente 0 di w_k è implicita (=1), le altre in A(k, k+2..n-1).
            for (j = 1; j < len_row; ++j) {
                double* col_jp = A + (k + 1 + j) * m;
                col_jp[k] = wbuf[j];
            }
        }
    }
}

// V: n x n, column-major
void bidiag_form_V_from_right(const double* A,
                              std::size_t   m,
                              std::size_t   n,
                              double*       V)
{
    assert(A != nullptr);
    assert(V != nullptr);

    std::size_t i, j, k;

    // V = I_n
    for (j = 0; j < n; ++j)
        for (i = 0; i < n; ++i)
            V[i + j * n] = (i == j) ? 1.0 : 0.0;

    // Riflettori destri esistono per k = 0 .. n-2
    if (n < 2) return;

    const std::size_t K = n - 1;

    // Applica V_k in ordine inverso: k = K-1, ..., 0
    for (std::size_t kk = 0; kk < K; ++kk) {
        k = K - 1 - kk;  // k scende da n-2 a 0

        const std::size_t len = n - (k + 1);
        if (len == 0) continue;

        double wbuf[1024];
        assert(len <= 1024);

        // Ricostruisci w_k (dimensione len) dalla riga k di A
        // w_k[0] = 1, w_k[j] = A(k, k+1+ j) per j>=1
        wbuf[0] = 1.0;

        for (std::size_t jloc = 1; jloc < len; ++jloc) {
            std::size_t col = k + 1 + jloc;
            const double* col_ptr = A + col * m;
            wbuf[jloc] = col_ptr[k];
        }

        // Applica H_k = I - 2 w w^T a destra di V(:, k+1:n-1)
        householder_apply_right_to_matrix(V,
                                          n,    // m locale (V è n x n)
                                          n,    // n locale
                                          k + 1,
                                          len,
                                          wbuf);
    }
}
// U: m x m, column-major
void bidiag_form_U_from_left(const double* A,
                             std::size_t   m,
                             std::size_t   n,
                             double*       U)
{
    assert(A != nullptr);
    assert(U != nullptr);

    std::size_t i, j, k;
    const std::size_t K = n; // numero riflettori sinistri (m>=n)

    // U = I_m
    for (j = 0; j < m; ++j)
        for (i = 0; i < m; ++i)
            U[i + j * m] = (i == j) ? 1.0 : 0.0;

    // Applica i riflettori U_k in ordine inverso
    for (std::size_t kk = 0; kk < K; ++kk) {
        k = K - 1 - kk;  // k = K-1, ..., 0

        const std::size_t len = m - k;
        double ubuf[1024];
        assert(len <= 1024);

        // Ricostruisci u_k di dimensione len
        ubuf[0] = 1.0;
        const double* col_k = A + k * m;

        for (i = 1; i < len; ++i)
            ubuf[i] = col_k[k + i];

        // Applica H_k = I - 2 u u^T a sinistra su U(k:m-1, :)
        householder_apply_left_to_matrix(U + k,
                                         m - k, // m locale
                                         m,     // tutte le colonne di U
                                         0,
                                         len,
                                         ubuf);
    }
}                              
int main()
{
    const std::size_t m = 4;
    const std::size_t n = 3;

    double A[m * n] = {
        12.0,  6.0, -4.0, -1.0,
        -51.0, 167.0, 24.0, 1.0,
        4.0,  -68.0, -41.0, 0.0
    };

    double A_orig[m * n];
    for (std::size_t j = 0; j < n; ++j)
        for (std::size_t i = 0; i < m; ++i)
            A_orig[i + j * m] = A[i + j * m];

    std::printf("=== Bidiagonalizzazione Householder (esempio 4x3) ===\n\n");
    print_matrix("A (originale)", A_orig, m, n);

    bidiag_householder_inplace(A, m, n);

    // Estrai la bidiagonale B (diag e superdiag)
    double B[m * n];
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            if (i == j || i == j + 1)
                B[i + j * m] = A[i + j * m];
            else
                B[i + j * m] = 0.0;
        }
    }

    print_matrix("B (bidiagonale)", B, m, n);

    double U[m * m];
    double V[n * n];

    bidiag_form_U_from_left(A, m, n, U);
    bidiag_form_V_from_right(A, m, n, V);

    print_matrix("U (ortogonale sinistra)", U, m, m);
    print_matrix("V (ortogonale destra)", V, n, n);

    // Calcola residuo A_orig - U * B * V^T
    double UB[m * n];
    matmul(U, B, UB, m, m, n);

    double UBVT[m * n];
    matmul_B_VT(UB, V, UBVT, m, n, n); // U B V^T

    double RES[m * n];
    mat_subtract(A_orig, UBVT, RES, m, n);

    double norm_A   = frobenius_norm(A_orig, m, n);
    double norm_RES = frobenius_norm(RES, m, n);

    std::printf("||A||_F             = %.12e\n", norm_A);
    std::printf("||A - U*B*V^T||_F   = %.12e\n", norm_RES);
    std::printf("rel. residuo        = %.12e\n", norm_RES / norm_A);

    return 0;
}

