#ifndef FCN_LIB_FORM_H
#define FCN_LIB_FORM_H

/*
 * Layer UI specifico FCN.
 *
 * I prompt delle action espongono verso fcn_all.cpp soltanto la semantica
 * applicativa: true = parametri confermati, false = prompt abbandonato.
 * I dettagli GnFormRunResult restano confinati nell'implementazione.
 */
bool f3_svd_test_prompt(int& n, int& d);
bool f3_svd_prompt(int& n, int& d);

struct F3SvdPcaParams {
    int n;
    int d;
    float factor_n;
    float factor_d;
};

bool f3_svd_pca_prompt(F3SvdPcaParams& params);

// Ritorno dal rapporto stdio alla form dei parametri.
void fcn_form_wait_report();

#endif
