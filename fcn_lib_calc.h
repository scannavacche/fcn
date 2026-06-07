#ifndef LIB_H
#define LIB_H

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <matplot/matplot.h>

using namespace std;
using Vec = std::vector<double>;
using VecN = std::vector<int>;
using Mat = std::vector<Vec>;

using namespace matplot;
using KM = matplot::keyword_manual_type ;
using KA = matplot::keyword_automatic_type ;

const double PI = std::acos(-1.0);

enum class SortOrder { Asc, Desc };

// 
// funzioni di gestione interfaccia utente 
// 

void cin_clear();

void clear_screen() ;

string color_bool(
    const bool val);

string color_dbl(
    const double val);

void color_rst();

//
// funzioni di conversione formato ed equivalenza unita' di misura
//

std::string itostr(
    const int nn);

double deg2rad(
    const int alpha);

string format_numstr(
    double v);


//
// funzioni di base per intervalli 
//

double h_ticks(
    const double a_start, 
    const double a_stop, 
    const int a_points);

Vec nodi_bubblesort (
    const Vec x_uns,
    const int totnum);  

Vec nodi_equidistanti(
    const double amin,
    const double amax,
    const int NPoints);

Vec nodi_random(
    const double amin, 
    const double amax,
    const int NPoints);

//
// funzioni matematiche ad uso callback 
//

double f_x(
    double t);
    
double f_x1(
    double t) ;

double f_sin(
    double t);

double f_cos(
    double t);

double f_atan(
    double t);

double f_atan_d(
    double t);

double f_const(double t) ;
double f_zero(double t);
double f_one(double t) ; 
double f_zero(double t) ;

double f_sin2plus1(
    double t);

double fcallb(
    double t, 
    double (*f)(double));   

//
// Funzioni per soluzioni di ODE con metodi iterativi
//

static double ode_oscillator_omega = 1.0;

void ode_set_oscillator_omega(
    double omega);

double ode_scalar_rhs_decay(   
    double t, 
    double y);   // scalare: y' = lambda * y


void ode_scalar_step_euler(
    double t, 
    double h, 
    double* y,
    double (*f)(double, double));   // Passo di Eulero scalare 

void ode_scalar_step_heun(
    double t, 
    double h, 
    double* y,
    double (*f)(double, double)); // Passo di Heun scalare (Eulero migliorato, ordine 2)

void ode_vec2_rhs_oscillator(
    double t,
    const double* y,
    double* dydt, 
    int n);

void ode_vecN_step_euler(
    double t, 
    double h,
    double* y, 
    int n,
    void (*f)(
        double, 
        const double*, 
        double*, 
        int
    )
);
    
int ode_scalar_test_euler_decay(); // test di eulero sul decadimento (in R)
int ode_scalar_test_heun_decay();  // test di heun sul decadimento (in R)
//
// funzioni macro algebra lineare
// 

Vec linear_subst_BW(
    const Mat &U, 
    const Vec &y);

Vec linear_subst_FW(
    const Mat &L, 
    const Vec &b);

void linear_jacobi_autoval_simmetrica(
    const Mat& A, 
    Vec& lambda, 
    Mat& V);

Vec linear_LU_calcola_autovalori (
    const Mat& A);

void linear_LU_dec(
    const Mat &A, 
    Mat &L, 
    Mat &U);

Mat linear_LU_inversa (
    const Mat& A);

Vec linear_LU_risolve_colonna(
    const Mat& L,
    const Mat& U,
    Vec x);

Vec linear_LU_risolve_sistema(
    const Mat& T,
    const Vec& x);

double linear_max_autoval_pwr_any_res(
    const Mat& M, 
    int max_iter, 
    double tol);

double linear_max_autoval_pwr_any(
    const Mat& M, 
    int max_iter, 
    double tol);

double linear_max_autoval_pwr_AtA(
    const Mat& M, 
    int max_iter, 
    double tol);

//
// funzioni per la gestione di vettori e matrici
//
void matrix_build_cauchy(
    int n, 
    double t0,      // tempo iniziale t0
    double T,       // tempo finale T
    double z_bc,    // condizione al contorno z(t0) = z_bc (era x0 prima)
    Vec &t,         // nodelist t_i 
    Vec &avals,     // coefficiente di z(t) valutati nei nodi t_i
    Vec &fvals,     // funzioni f(t) valutate nei nodi t_i
    Mat &L, 
    Vec &b, 
    double (*at)(double) = f_one,
    double a_arg = 1,           // argomento (se at == &f_const) della funzione at
    double (*ft)(double) = f_zero,
    double f_arg = 0,           // argomento (se at == &f_const) della funzione ft
    bool backw = false) ;


void matrix_build_cauchy_int(
    int n, 
    double t0, 
    double T, 
    double x0,
    Vec &t, 
    Vec &fvals, 
    Mat &L, 
    Vec &b, 
    double (*ft)(double),
    bool backw) ;

void matrix_build_cauchy_sep(
    int n, 
    double t0, 
    double T, 
    double x0,
    Vec &t, 
    Vec &fvals, 
    Mat &L, 
    Vec &b, 
    double (*ft)(double),
    bool backw) ;

Mat matrix_build_derivata1(
        int n, 
        double a, 
        double b) ;

Mat matrix_build_gausskernel(
    const int N, 
    const double sigma,
    const double h, 
    bool norm_flag,
    Vec& Indicatori) ;

Mat matrix_build_gram(
        const Vec& x, 
        const int K) ;

Mat matrix_build_Id(
    int n);

Mat matrix_build_triang(
    int n);
Mat matrix_build_triang_inv(
    int n);

Mat matrix_build_zero(
    int righe, 
    int colonne);

Vec matrix_calcola_deriv_byiter(
    const Vec &u , 
    const double a, 
    const double b) ;

Vec matrix_calcola_deriv_bymatr(
    const Mat &D, 
    const Vec &u) ;

Mat matrix_calcola_diff(
    const Mat& A, 
    const Mat& B);

Mat matrix_calcola_subfact(
    const Mat& A, 
    const Mat& B,
    const double fact);

double matrix_calcola_errore_Fr(
    const Mat& A, 
    const Mat& B);

void matrix_calcola_media(
    const Mat& A, 
    Vec& avg_col, 
    Vec& avg_row);

double matrix_calcola_norma(
    int norma,
    const Mat& A);

Mat matrix_centra_su_media(
    const Mat& A, 
    const Vec& avg_vec,
    bool by_col = true);

void matrix_dump(
    const Mat &A, 
    const std::string &nome);

Mat matrix_estende_ridotta(
    const Mat& A, 
    int n, 
    bool bycol);

Mat matrix_householder_reflector(
    const Vec& v);

Mat matrix_normalize_byrow(
    Mat& K);
    
void matrix_ordina_diagonale(
    Vec& lambda,
    Mat& V,
    double zero_tol = 0.0,
    SortOrder order = SortOrder::Desc);   // default: discendente

void matrix_ortogonalizza_GSmod(
    Mat& Q, 
    int j0 = 0);

Mat matrix_prodotto_AtA (
    const Mat& A,
    bool A_right = true
);

Mat matrix_prodotto_coeff(
    const Mat& A, 
    const double coeff) ;

Mat matrix_prodotto_matrix(
    const Mat& A, 
    const Mat& B);

Vec matrix_prodotto_vector(
    const Mat& A, 
    const Vec& v);

void matrix_test_ortogonale(
    const Mat& A,
    string s);

Mat matrix_trasposta(
    const Mat& A);

Vec vector_add_noise(
    Vec& v, 
    double e);

Vec vector_build_householder_bycol(
    const Vec& v);

Vec vector_build_householder_byrow(
    const Vec& v);

Vec vector_build_one_minus_one(
    const int n) ;

Vec vector_build_segnale_finestra(
    int N,
    double a, 
    double b, 
    double t);

Vec vector_build_versore_canonico(
    int j, 
    int N);

double vector_calcola_norma(
    int norma, 
    const Vec& V);

Vec vector_campiona_f(
    const Vec &x,           // vettore dei nodi di campionamento
    double (*ft)(double));  // funzione da  campionare 

Vec vector_campiona_f_k(
    int k,                   // acceleratore di frequenza, es. k=1 per sin(t), k=2 per sin(2t), ecc.
    const Vec &x,            // vettore dei nodi di campionamento
    double (*ft)(double));   // funzione da  campionare 

void vector_dump (
    Vec x, 
    int colspan, 
    int totnum, 
    const std::string s);

Vec vector_householder_reflected(
    const Vec v);

Vec vector_prodotto_coeff(
    const Vec v, 
    double mu);

double vector_prodotto_scalare(
    const Vec &u,
    const Vec &v);

Vec vector_reverse(
    const Vec& v);

Vec vector_shift(
    const Vec &v, 
    const double shift);

Vec vector_somma(
    const Vec u, 
    const Vec v);

Mat vector_to_matrix(
    const Vec& v,
    bool transp);

Mat vector_to_matrix_diag(
    const Vec& s, 
    int m, 
    int n) ;

//
// funzioni specifiche di trasformazione matrici 
//

void trmatrix_bidiagonalizza(
    const Mat& A, 
    Mat& U0, 
    Mat& B, 
    Mat& V0, 
    bool sup_diag = false,  
    bool dump_flag = false);

void trmatrix_bidiag_wide_to_lower(
    const Mat& X, 
    Mat& U0, 
    Mat& B, 
    Mat& V0,
    bool dump_flag) ;

void trmatrix_bidiag_wide_to_upper(
    const Mat& X, 
    Mat& U0, 
    Mat& B, 
    Mat& V0,
    bool dump_flag) ;


void trmatrix_SVDQR(
    const Mat& B,
    Mat& Ub,
    Mat& Vb,
    Vec& sigma
);

void trmatrix_SVDQR_ridotta(
    const Mat& B,      // p x d, p <= d, bidiagonale superiore
    Mat& Ub,           // p x p
    Vec& sigma,        // p
    Mat& Vb_red,       // d x p
    double ev_tol = 1e-12  // soglia per autovalori quasi nulli
);

void trmatrix_test_sv_autoval(
    Vec lambda,
    const Vec& sigma);

// 
// Funzioni per matplot++
//

enum LeAl : int {
    Auto    = -1,
    Left    = 0,
    HCenter = 1,
    Right   = 2,
    Bottom  = 4,
    VCenter = 8,
    Top     = 12
};

void matplot_legend_align(
    legend_handle lg, 
    int pos_enum, 
    float xscale,
    float yscale) ;

figure_handle matplot_table_init (
    const bool ahold, 
    const std::string &nome, 
    const std::string &titolo, 
    const int xlab, 
    const int ylab) ;

//
// gestione menu principale
// 

struct MenuItem {
    int key;
    std::string label;
    std::string action;
    bool enabled;
};

struct MenuConfig {
    std::string title;
    std::vector<MenuItem> items;
};

MenuConfig load_menu_config(
    const std::string& filename);

const MenuItem* find_menu_item(
    const MenuConfig& menu, 
    int key);

void wait_return_to_menu(bool bypass_waitakey); 


#endif
