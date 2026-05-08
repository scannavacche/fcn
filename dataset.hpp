// =============================================================================
//  dataset_fornito.hpp
//  Generatore di dataset a due cluster in R^d
//  Routine FORNITA — non modificare
// =============================================================================
#pragma once
#include <vector>
#include <random>
#include <utility>

using Vec = std::vector<double>;
using Mat = std::vector<Vec>;

// Genera un dataset di DUE cluster in R^d con n punti totali.
//
//  - Centri c1, c2 scelti casualmente in [-1,1]^d
//  - n/2 punti attorno a c1, n/2 punti attorno a c2
//  - Rumore gaussiano con sigma = 0.3 su ogni componente
//
// Restituisce {X, etichette}:
//   X          : matrice n x d  (ogni riga è un punto)
//   etichette  : vettore di 0/1 (0 = cluster A, 1 = cluster B)
inline std::pair<Mat, std::vector<int>>
genera_cluster(int n, int d, int seed = 0) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> unif(-1.0, 1.0);
    std::normal_distribution<double>       gauss(0.0, 0.3);

    // Centri dei due cluster
    Vec c1(d), c2(d);
    for (int j = 0; j < d; ++j) { c1[j] = unif(rng); c2[j] = unif(rng); }

    int n1 = n / 2, n2 = n - n1;
    Mat X(n, Vec(d));
    std::vector<int> label(n);

    for (int i = 0; i < n1; ++i) {
        label[i] = 0;
        for (int j = 0; j < d; ++j)
            X[i][j] = c1[j] + gauss(rng);
    }
    for (int i = 0; i < n2; ++i) {
        label[n1 + i] = 1;
        for (int j = 0; j < d; ++j)
            X[n1 + i][j] = c2[j] + gauss(rng);
    }
    return {X, label};
}
