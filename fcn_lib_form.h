#ifndef FCN_LIB_FORM_H
#define FCN_LIB_FORM_H

#include <array>

/*
 * Layer UI specifico FCN.
 *
 * I prompt delle action espongono verso fcn_all.cpp soltanto la semantica
 * applicativa: true = parametri confermati, false = prompt abbandonato.
 * I dettagli GnFormRunResult restano confinati nell'implementazione.
 */

// 1. Individua i parametri
// 2. Definisci il contratto
struct F1SampleParams {
    int K = 40;        // Numero funzioni base
    int N = 100; // Numero nodi    
};

namespace f1_sample_form {
    bool prompt(F1SampleParams& params);
}

struct F1TaylorParams {
    int n_terms = 20;
    int target_arg = -2;
};

namespace f1_taylor_exp_form {
    bool prompt(F1TaylorParams& params);
}

namespace f1_taylor_cos_form {
    bool prompt(F1TaylorParams& params);
}

struct F1PrecParams {
    double increment = 0.01;
    int repetitions = 100;
};

namespace f1_prec_form {
    bool prompt(F1PrecParams& params);
}

struct F2DerivParams {
    int n = 100;
    double domain_span = 1.0;

};

namespace f2_deriv_form
{
    bool prompt(F2DerivParams& params);
}

struct F2CauchyParams {
    int n = 50;
    double t0 = 0.0;
    double T = 5.0;
    double x0 = 1.0;
};
namespace f2_cauchy_form {
    bool prompt(F2CauchyParams& params);
}

struct F2BordoParams {
    int n = 200;
    double t0 = 0.0;
    double T = 3.14159;
    double x0 = 0.0;
    double x1 = 0.0;
};
namespace f2_bordo_form {
    bool prompt(F2BordoParams& params);
}

struct F2IntegralParams {
    double a = -7.0;
    double b = 7.0;
    int N = 40;
};
namespace f2_integral_form {
    bool prompt(F2IntegralParams& params);
}


struct F2NormeParams {
    int n_max = 640;          // default 640
    int timing_reps = 1;    // default 1
};


namespace f2_norme_form
{
    bool prompt(F2NormeParams& params);
};

struct F2DeconvParams {
    double a = 0.0;
    double b = 1.0;
    int N = 100;
    double sigma = 2.0;  // moltiplicatore del passo h
    bool KG_Normal = true;
};

namespace f2_deconv_form {
    bool prompt(F2DeconvParams& params);
}


namespace f3_svd_test_form
{
    bool prompt(int& n, int& d);
};

namespace f3_svd_form
{
    bool prompt(int& n, int& d);
};

struct F3SvdPcaParams {
    int n = 100;
    int d = 10;
    float factor_n = 2.0f;
    float factor_d = 2.0f;
};

namespace f3_svd_pca_form
{
    bool prompt(F3SvdPcaParams& params);
};

struct F3SvdAnParams {
    int n = 200;
    std::array<int, 4> d{{4, 8, 40, 80}};
    int seed = 42;
};

namespace f3_svd_an_form {
    bool prompt(F3SvdAnParams& params);
}

/*
 * Simulazione d'esame - problema di Cauchy scalare
 *
 *   z'(t) = a(t) z(t) + f(t),   t in [t0,T],   n nodi equidistanti.
 *
 * I sei campi descrivono la griglia e i coefficienti delle tre prove
 * attualmente selezionate in sym_cauchy(); la scelta di a(t) e f(t)
 * tramite menu NON e' ancora implementata.
 *
 * Esperimento 1 (pannelli 0,1):
 *   a(t)=0, f(t)=1/(1+sin(t)^2), schema backward, z(T)=x0.
 *   NOTA: qui x0 e' il valore all'estremo FINALE T.
 * Esperimento 2 (pannello 2):
 *   a(t)=cos(t), f(t)=1/(1+sin(t)^2), forward, z(t0)=x0.
 * Esperimento 3 (pannello 3):
 *   a(t)=a_arg, f(t)=f_arg, forward, z(t0)=1 (fisso).
 *   NOTA: x0 NON modifica l'esperimento 3.
 *
 * t0 e T sono indipendenti (non si impone t0=-T); T>t0.
 * n conta i NODI, quindi gli intervalli sono n-1.
 * I puntatori a funzione e il comportamento di f_const appartengono
 * a matrix_build_cauchy(), dichiarata in fcn_lib_calc.h.
 *
 * prompt(params): true se confermato con F10, false se annullato.
 * La struct rimane nel chiamante; Esc non ne modifica i valori.
 */
struct SymCauchyParams {
    int n = 100;                                  // nodi, NON intervalli
    double t0 = -3.14159265358979323846;         // istante iniziale
    double T = 3.14159265358979323846;           // istante finale, T>t0
    double x0 = -1.0;                             // z(T) prova 1; z(t0) prova 2
    double a_arg = 1.0;                           // a costante, solo prova 3
    double f_arg = 0.0;                           // f costante, solo prova 3
};
namespace sym_cauchy_form {
    bool prompt(SymCauchyParams& params);
}

/*
 * Simulazione E1c - monitoraggio del riflettore di Householder.
 *
 * n e' la dimensione del VETTORE DI OUTPUT del monitoraggio, non la
 * dimensione di un singolo riflettore. Per ogni i = 2,...,n-1:
 *   v^(i) = (1,-1,1,-1,...) in R^i;
 *   w^(i) = H(v^(i)) * v^(i), con H riflettore di Householder;
 *   si registra w^(i)[0] e lo si rappresenta rispetto ad i.
 *   Poiche' v_0=1, il valore teorico e' w^(i)[0] = -sqrt(i).
 *
 * Se n >= 7 viene anche costruito e verificato H(v^(6)), stampato
 * su console. I primi due campioni degli array del grafico sono
 * segnaposto inizializzati a zero; non sono esperimenti i=0,1.
 *
 * Il form modifica soltanto n, e lascia intatte le routine numeriche.
 * prompt(params) restituisce true su F10, false su ESC.
 */
struct SymHHAlphaParams {
    int n = 100; // campioni allocati; i test reali vanno da 2 a n-1
};

namespace sym_hhalpha_form {
    bool prompt(SymHHAlphaParams& params);
}

struct OdeEulerParams {
    std::array<int, 4> N{10, 20, 40, 80}; // intervalli, NON nodi
    double b = 5.0;
    double y0 = 1.0;
};
namespace ode_euler_form {
    bool prompt(OdeEulerParams& p);
}

struct OdeOsc2DParams {
    std::array<int, 4> N{10, 20, 40, 80};
    double b = 5.0;
    double q0 = 1.0;
    double p0 = 1.0;
    double omega = 1.0;
    double gamma = 0.2;
};
namespace ode_osc2d_form {
    bool prompt(OdeOsc2DParams& p);
}

struct OdeLotkaVParams {
    std::array<int, 4> N{50, 100, 200, 400};
    double b = 10.0;
    double q0 = 1.2; // prede
    double p0 = 0.8; // predatori
    double alpha = 1.0;
    double beta  = 1.0;
    double gamma = 1.0;
    double delta = 1.0;
};
namespace ode_lotkav_form {
    bool prompt(OdeLotkaVParams& p);
}

/*
 * al_qr - fattorizzazione QR e minimi quadrati su una matrice tall.
 * Convenzione LOCALE di al_qr(): A in R^(n x d)
 *   n = numero di righe, osservazioni/equazioni
 *   d = numero di colonne, feature/incognite
 * Il risolutore linear_QR_least_squares richiede n >= d.
 * GNPORT verifica questo vincolo PRIMA dell'avvio dell'esperimento.
 * Il parametro del form non rinomina le variabili degli algoritmi:
 * altre routine possono usare la convenzione classica m x n.
 */
struct AlQRParams {
    int n = 12; // righe / osservazioni
    int d = 4;  // colonne / feature, con d <= n
};
namespace al_qr_form {
    bool prompt(AlQRParams& params);
}

struct SandboxParams {
    double i_ini = 1.0;
    double i_max = 5.0;
    double i_val = 1.0;  // valore iniziale ammesso nel range (0,10]
};

namespace sandbox_form {
    bool prompt(SandboxParams& params);
}

struct JLDistanceParams {
    double dx = 3.0;
    double dy = 4.0;
    double t = 2.0;
};

namespace JL_distanze_form {
    bool prompt(JLDistanceParams& params);
}

struct JLDistanzeDatasetParams {
    int n = 10;
    int d = 80;
    int k = 20;
    int seed = 42;      // generatore dataset X
    int seed_A = 12345; // generatore matrice JL A
};

namespace JL_distanze_dataset_form {
    bool prompt(JLDistanzeDatasetParams& params);
}


// Ritorno dal rapporto stdio alla form dei parametri.
namespace fcn_form
{
    void wait_report();
}
#endif
