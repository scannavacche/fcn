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


// =============================================================================
//  Estensione sperimentale JL: separazione tra i centroidi controllabile.
//  La funzione genera_cluster(n,d,seed) originale sopra e' INALTERATA.
// =============================================================================
#include <cmath>
#include <stdexcept>

enum class JLScalaCentroidi {
    Federico,            // Routine originale: centroidi U[-1,1]^d (baseline esatta)
    LeggeDiPotenza       // ||c1-c0|| = D_ref * (d / d_ref)^alfa
};

struct JLConfigCentroidi {
    JLScalaCentroidi modo = JLScalaCentroidi::LeggeDiPotenza;
    // JLScalaCentroidi modo = JLScalaCentroidi::Federico;
    double distanza_ref = 4.0; 
    // double distanza_ref = std::sqrt(2000.0 / 3.0); // ~25.82 a d_ref=1000
    int d_ref = 1000;
    double alfa = 0.0;  // 0: separazione fissa; 0.5: proporzionale a sqrt(d); 1: a d
};

// Overload a quattro parametri: NON cambia il comportamento delle chiamate
// esistenti genera_cluster(n,d,seed). I label restano gli stessi.
// In modo controllato mantiene gli stessi campioni di rumore gaussiano
// dell'originale, modificando soltanto la posizione dei due centroidi.
inline std::pair<Mat, std::vector<int>>
genera_cluster(int n, int d, int seed, const JLConfigCentroidi& cfg) {
    if (n <= 0 || d <= 0)
        throw std::invalid_argument("genera_cluster: n e d devono essere positivi");

    // Percorso IDENTICO al generatore consegnato da Federico.
    if (cfg.modo == JLScalaCentroidi::Federico)
        return genera_cluster(n, d, seed);

    if (cfg.modo != JLScalaCentroidi::LeggeDiPotenza)
        throw std::invalid_argument("genera_cluster: modo centroidi sconosciuto");
    if (cfg.d_ref <= 0 || !std::isfinite(cfg.distanza_ref) ||
        cfg.distanza_ref < 0.0 || !std::isfinite(cfg.alfa))
        throw std::invalid_argument("genera_cluster: D_ref, d_ref o alfa non validi");

    // 1) Genera i punti con la STESSA routine e lo STESSO seed.
    //    Questo ci permette di mantenere identici i residui gaussiani,
    //    variando soltanto la separazione dei centri a n,d,seed fissati.
    auto risultato = genera_cluster(n, d, seed);
    Mat& X = risultato.first;
    const std::vector<int>& label = risultato.second;

    // 2) Ricostruisce i due centri originali. Le prime 2*d estrazioni
    //    di mt19937/uniform_real_distribution sono proprio quelle del
    //    generatore fornito (il rumore viene estratto soltanto dopo).
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> unif(-1.0, 1.0);
    Vec c0(d), c1(d);
    long double norma2 = 0.0L;
    for (int j = 0; j < d; ++j) {
        c0[j] = unif(rng);
        c1[j] = unif(rng);
        const long double delta = static_cast<long double>(c1[j]) - c0[j];
        norma2 += delta * delta;
    }
    const double distanza_orig = std::sqrt(static_cast<double>(norma2));
    const double distanza_target = cfg.distanza_ref *
        std::pow(static_cast<double>(d) / cfg.d_ref, cfg.alfa);
    if (!std::isfinite(distanza_target))
        throw std::invalid_argument("genera_cluster: distanza obiettivo non finita");
    if (distanza_orig == 0.0 && distanza_target > 0.0)
        throw std::runtime_error("genera_cluster: centroidi casuali coincidenti");

    const double fattore = (distanza_orig > 0.0)
                         ? distanza_target / distanza_orig : 0.0;

    // 3) Centro comune invariato, stessa direzione della separazione.
    //    Se delta_j = c1_j-c0_j, ciascun centro si sposta di
    //    +/- (fattore-1)*delta_j/2. Nessun nuovo campionamento di rumore.
    for (int i = 0; i < n; ++i) {
        const double segno = (label[i] == 0 ? -1.0 : 1.0);
        for (int j = 0; j < d; ++j) {
            const double delta = c1[j] - c0[j];
            X[i][j] += segno * (fattore - 1.0) * 0.5 * delta;
        }
    }
    return risultato;
}
