#ifndef FCN_LIB_FORM_H
#define FCN_LIB_FORM_H

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

// Ritorno dal rapporto stdio alla form dei parametri.
namespace fcn_form
{
    void wait_report();
}
#endif
