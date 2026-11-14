#ifndef FCN_SVD_QR_H
#define FCN_SVD_QR_H

#include <vector>

using Vec = std::vector<double>;
using Mat = std::vector<Vec>;

// B e' gia' bidiagonale. Il solutore restituisce Ub, Vb trasposta e sigma.
int svd_bidiagonale(const Mat& B_in, Mat& Ub, Mat& VTb, Vec& sigma,
                    bool verbose = true);

#endif
