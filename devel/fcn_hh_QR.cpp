#include <cstdio>
#include <cstddef>
#include <cmath>
#include <cassert>

// Dichiarazioni delle funzioni (implementazioni in altri file/precedenti)
void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v);

void householder_apply_left_to_matrix(double*       A,
                                      std::size_t   m,
                                      std::size_t   n,
                                      std::size_t   row_offset,
                                      std::size_t   len,
                                      const double* v);

void qr_householder_inplace(double*     A,
                            std::size_t m,
                            std::size_t n);

void qr_form_Q_from_householder(const double* A,
                                std::size_t   m,
                                std::size_t   n,
                                double*       Q);

// Funzioni di utilità per stampare e calcolare norme Frobenius

static void print_matrix(const char* name,
                         const double* A,
                         std::size_t m,
                         std::size_t n)
{
    std::printf("%s =\n", name);
    for (std::size_t i = 0; i < m; ++i) {
        std::printf("  [");
        for (std::size_t j = 0; j < n; ++j) {
            std::printf(" % .10f", A[i + j * m]);
            if (j + 1 < n) std::printf(",");
        }
        std::printf(" ]\n");
    }
    std::printf("\n");
}

static double frobenius_norm(const double* A,
                             std::size_t m,
                             std::size_t n)
{
    double sum = 0.0;
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            double a = A[i + j * m];
            sum += a * a;
        }
    }
    return std::sqrt(sum);
}

// C = A * B   (A: m×k, B: k×n), column-major
static void matmul(const double* A,
                   const double* B,
                   double*       C,
                   std::size_t   m,
                   std::size_t   k,
                   std::size_t   n)
{
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            double sum = 0.0;
            for (std::size_t p = 0; p < k; ++p)
                sum += A[i + p * m] * B[p + j * k];
            C[i + j * m] = sum;
        }
    }
}

// C = A^T * B   (A: m×k, B: m×n), result C: k×n
static void matmul_AT_B(const double* A,
                        const double* B,
                        double*       C,
                        std::size_t   m,
                        std::size_t   k,
                        std::size_t   n)
{
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t p = 0; p < k; ++p) {
            double sum = 0.0;
            for (std::size_t i = 0; i < m; ++i)
                sum += A[i + p * m] * B[i + j * m];
            C[p + j * k] = sum;
        }
    }
}

// C = A - B   (stessa dimensione)
static void mat_subtract(const double* A,
                         const double* B,
                         double*       C,
                         std::size_t   m,
                         std::size_t   n)
{
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            C[i + j * m] = A[i + j * m] - B[i + j * m];
        }
    }
}

int main()
{
    // Matrice di test 4x3 (column-major):
    // A =
    // [ 12   -51    4 ]
    // [  6    167  -68]
    // [ -4    24   -41]
    // [ -1     1    0 ]
    //
    // (è un classico esempio didattico di QR via Householder)
    const std::size_t m = 4;
    const std::size_t n = 3;

    double A[m * n] = {
        // colonna 0
        12.0,  6.0, -4.0, -1.0,
        // colonna 1
        -51.0, 167.0, 24.0, 1.0,
        // colonna 2
        4.0,  -68.0, -41.0, 0.0
    };

    // Copia di A per controllo residuo A - Q*R
    double A_orig[m * n];
    for (std::size_t j = 0; j < n; ++j)
        for (std::size_t i = 0; i < m; ++i)
            A_orig[i + j * m] = A[i + j * m];

    std::printf("=== QR Householder (esempio 4x3) ===\n\n");
    print_matrix("A (originale)", A_orig, m, n);

    // Fattorizzazione in-place: A -> [R; vettori di Householder sotto la diag]
    qr_householder_inplace(A, m, n);

    // Estrai R: parte triangolare superiore di A
    double R[m * n];
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            if (i <= j)
                R[i + j * m] = A[i + j * m];
            else
                R[i + j * m] = 0.0;
        }
    }

    // Costruisci Q esplicito (m x m)
    double Q[m * m];
    qr_form_Q_from_householder(A, m, n, Q);

    print_matrix("R (triangolare sup.)", R, m, n);
    print_matrix("Q (ortogonale)", Q, m, m);

    // Calcola residuo A_orig - Q*R
    double QR[m * n];
    matmul(Q, R, QR, m, m, n);

    double RES[m * n];
    mat_subtract(A_orig, QR, RES, m, n);

    double norm_A   = frobenius_norm(A_orig, m, n);
    double norm_RES = frobenius_norm(RES, m, n);

    std::printf("||A||_F         = %.12e\n", norm_A);
    std::printf("||A - Q*R||_F   = %.12e\n", norm_RES);
    std::printf("rel. residuo    = %.12e\n\n", norm_RES / norm_A);

    // Verifica ortogonalità: Q^T Q ≈ I
    double QTQ[m * m];
    matmul_AT_B(Q, Q, QTQ, m, m, m);

    // Costruisci identità I
    double I[m * m];
    for (std::size_t j = 0; j < m; ++j)
        for (std::size_t i = 0; i < m; ++i)
            I[i + j * m] = (i == j) ? 1.0 : 0.0;

    double QTQmI[m * m];
    mat_subtract(QTQ, I, QTQmI, m, m);

    double norm_QTQmI = frobenius_norm(QTQmI, m, m);

    std::printf("||Q^T*Q - I||_F = %.12e\n", norm_QTQmI);

    return 0;
}