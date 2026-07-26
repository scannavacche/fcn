#ifndef FCN_LIB_FORM_H
#define FCN_LIB_FORM_H

/*
 * Layer UI specifico FCN.
 *
 * I prompt delle action espongono verso fcn_all.cpp soltanto la semantica
 * applicativa: true = parametri confermati, false = prompt abbandonato.
 * I dettagli GnFormRunResult restano confinati nell'implementazione.
 */
namespace f3_svd_test_form
{
    bool prompt(int& n, int& d);
};

namespace f3_svd_form
{
    bool prompt(int& n, int& d);
};

struct F3SvdPcaParams {
    int n;
    int d;
    float factor_n;
    float factor_d;
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
