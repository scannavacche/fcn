#include <cstddef>
#include <cmath>
#include <cassert>

void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v)
{
    assert(x   != nullptr);
    assert(v   != nullptr);
    assert(len > 0);

    std::size_t i;

    for (i = 0; i < len; ++i)
        v[i] = x[i];

    double norm_x2 = 0.0;
    for (i = 0; i < len; ++i)
        norm_x2 += v[i] * v[i];

    if (norm_x2 == 0.0) {
        v[0] = 1.0;
        for (i = 1; i < len; ++i)
            v[i] = 0.0;
        return;
    }

    const double norm_x = std::sqrt(norm_x2);
    const double x0     = v[0];
    const double sign   = (x0 >= 0.0) ? 1.0 : -1.0;
    const double alpha  = sign * norm_x;

    v[0] += alpha;

    double norm_u2 = 0.0;
    for (i = 0; i < len; ++i)
        norm_u2 += v[i] * v[i];

    if (norm_u2 == 0.0) {
        v[0] = 1.0;
        for (i = 1; i < len; ++i)
            v[i] = 0.0;
        return;
    }

    const double norm_u = std::sqrt(norm_u2);
    const double inv_norm_u = 1.0 / norm_u;

    for (i = 0; i < len; ++i)
        v[i] *= inv_norm_u;
}

void householder_apply_to_segment(double*       y,
                                  std::size_t   len,
                                  const double* v)
{
    assert(y   != nullptr);
    assert(v   != nullptr);
    assert(len > 0);

    std::size_t i;
    double beta = 0.0;

    for (i = 0; i < len; ++i)
        beta += v[i] * y[i];

    const double factor = 2.0 * beta;

    for (i = 0; i < len; ++i)
        y[i] -= factor * v[i];
}

void householder_apply_left_to_matrix(double*       A,
                                      std::size_t   m,
                                      std::size_t   n,
                                      std::size_t   row_offset,
                                      std::size_t   len,
                                      const double* v)
{
    assert(A != nullptr);
    assert(v != nullptr);
    assert(row_offset + len <= m);

    std::size_t j, i;

    for (j = 0; j < n; ++j) {
        double* col_j = A + j * m;

        double beta = 0.0;
        for (i = 0; i < len; ++i)
            beta += v[i] * col_j[row_offset + i];

        const double factor = 2.0 * beta;

        for (i = 0; i < len; ++i)
            col_j[row_offset + i] -= factor * v[i];
    }
}

void householder_apply_right_to_matrix(double*       A,
                                       std::size_t   m,
                                       std::size_t   n,
                                       std::size_t   col_offset,
                                       std::size_t   len,
                                       const double* v)
{
    assert(A != nullptr);
    assert(v != nullptr);
    assert(col_offset + len <= n);

    std::size_t i, k;

    for (i = 0; i < m; ++i) {
        double beta = 0.0;

        for (k = 0; k < len; ++k)
            beta += A[i + (col_offset + k) * m] * v[k];

        const double factor = 2.0 * beta;

        for (k = 0; k < len; ++k)
            A[i + (col_offset + k) * m] -= factor * v[k];
    }
}

#include <cstdio>
#include <cstddef>
#include <cmath>
#include <cassert>

void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v);

void householder_apply_to_segment(double*       y,
                                  std::size_t   len,
                                  const double* v);

static void print_vector(const char* name,
                         const double* x,
                         std::size_t n)
{
    std::size_t i;
    std::printf("%s = [", name);
    for (i = 0; i < n; ++i) {
        std::printf("% .12f", x[i]);
        if (i + 1 < n) std::printf(", ");
    }
    std::printf("]\n");
}

int main()
{
    const std::size_t n = 3;
    double x[n] = {4.0, 3.0, 0.0};
    double v[n];
    double y[n];

    std::size_t i;
    for (i = 0; i < n; ++i)
        y[i] = x[i];

    householder_construct_segment(x, n, v);
    householder_apply_to_segment(y, n, v);

    print_vector("x", x, n);
    print_vector("v", v, n);
    print_vector("H*x", y, n);

    const double err_tail = std::sqrt(y[1]*y[1] + y[2]*y[2]);
    const double err_norm = std::fabs(std::sqrt(y[0]*y[0] + y[1]*y[1] + y[2]*y[2]) - 5.0);

    std::printf("errore coda     = %.12e\n", err_tail);
    std::printf("errore norma    = %.12e\n", err_norm);

    return 0;
}

// x   = [ 4.000000000000,  3.000000000000,  0.000000000000]
// v   = [ 0.948683298051,  0.316227766017,  0.000000000000]
// H*x = [-5.000000000000,  0.000000000000,  0.000000000000]

// g++ -O2 -std=c++17 householder_test.cpp -o householder_test ./householder_test