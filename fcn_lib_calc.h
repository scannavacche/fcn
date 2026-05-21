#ifndef LIB_H
#define LIB_H

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <matplot/matplot.h>

using std::cout;
using std::endl;
using Vec = std::vector<double>;
using VecN = std::vector<int>;
using Mat = std::vector<Vec>;
using namespace matplot;
using namespace std;

using KM = matplot::keyword_manual_type ;
using KA = matplot::keyword_automatic_type ;


const double PI = std::acos(-1.0);

enum class SortOrder { Asc, Desc };


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

void cin_clear();

double h_ticks(
    const double a_start, 
    const double a_stop, 
    const int a_points);

double f_x(
    double t);

double f_sin(
    double t);

double f_cos(
    double t);

double f_atan(
    double t);

double f_atan_d(
    double t);

double fcallb(
    double t, 
    double (*f)(double));   

Mat kernel_gaussiano_matrice(
    const int N, 
    const double sigma,
    const double h, 
    bool norm_flag,
    Vec& Indicatori) ;
    
Vec add_rumore(
    Vec& v, 
    double e);

string color_bool(
    const bool val);

string color_dbl(
    const double val);

void color_rst();

void stampa_matrice(
    const Mat &A, 
    const std::string &nome);

Vec vector_shift(
    const Vec &v, 
    const double shift);

void vector_dump (
    Vec x, 
    int colspan, 
    int totnum, 
    const std::string s);

std::string itostr(
    const int nn);

double deg2rad(
    const int alpha);

string format_numstr(
    double v);

Vec nodi_equidistanti(
    const double amin,
    const double amax,
    const int NPoints);

Vec nodi_random(
    const double amin, 
    const double amax,
    const int NPoints);

Vec nodi_bubblesort (
    const Vec x_uns,
    const int totnum);
    
Mat crea_matrice(
    int righe, 
    int colonne);

Mat identita(
    int n);

Mat costruisci_triangolare(
    int n);
Mat costruisci_inv_triangolare_sint(
    int n);

double prodotto_scalare(
    const Vec &u,
    const Vec &v);

Mat matrice_ridotta(
    const Mat& A, 
    int n, 
    bool bycol);

Mat differenza_matrici(
    const Mat& A, 
    const Mat& B);

Mat prodotto_matrici(
    const Mat& A, 
    const Mat& B);

Vec prodotto_matrice_vettore(
    const Mat& A, 
    const Vec& v);

Mat prodotto_matrice_coeff(
    const Mat& A, 
    const double coeff) ;

double max_autoval_power_A_res(
    const Mat& M, 
    int max_iter, 
    double tol);

double max_autoval_power_A(
    const Mat& M, 
    int max_iter, 
    double tol);

double max_autoval_power_AtA(
    const Mat& M, 
    int max_iter, 
    double tol);

double norma_vettore(
    int norma, 
    const Vec& V);

double norma_matrice(
    int norma,
    const Mat& A);

Vec forward_substitution(
    const Mat &L, 
    const Vec &b);

Vec backward_substitution(
    const Mat &U, 
    const Vec &y);

void LU(
    const Mat &A, 
    Mat &L, 
    Mat &U);

Mat matrice_righe_normalizzate(
    Mat& K);

Mat converti_vettore_a_matrice(
    const Vec& v,
    bool transp);

Mat calcola_trasposta(
    const Mat& A);

Vec versore_canonico(
    int j, 
    int N);

Vec risolvi_colonna(
    const Mat& L,
    const Mat& U,
    Vec x);

Vec risolvi_sistema_LU(
    const Mat& T,
    const Vec& x);

Mat calcola_inversa_LU (
    const Mat& A);

Vec calcola_autovalori_LU (
    const Mat& A);

Vec segnale_finestra(
    int N,
    double a, 
    double b, 
    double t);

Vec householder_colonna(
    const Vec& v);

Vec householder_riga(
    const Vec& v);

void bidiagonalizza_underdet(
    const Mat& X, 
    Mat& U0, 
    Mat& B, 
    Mat& V0,
    bool sup_diag,
    bool dump_flag = false);

void bidiagonalizza(
    const Mat& A, 
    Mat& U0, 
    Mat& B, 
    Mat& V0, 
    bool sup_diag = false,  
    bool dump_flag = false);

Mat matrice_At_A (
    const Mat& A,
    bool A_right = true
);

Mat matrice_diagonale(
    const Vec& s, 
    int m, 
    int n) ;


double errore_F(
    const Mat& A, 
    const Mat& B);

void calcola_medie_matrice(
    const Mat& A, 
    Vec& avg_col, 
    Vec& avg_row);

Mat centra_matrice(
    const Mat& A, 
    const Vec& avg_vec,
    bool by_col = true);

void test_ortogonalita(
    const Mat& A,
    string s);

void svd_bidiagonale_ridotta(
    const Mat& B,      // p x d, p <= d, bidiagonale superiore
    Mat& Ub,           // p x p
    Vec& sigma,        // p
    Mat& Vb_red,       // d x p
    double ev_tol = 1e-12  // soglia per autovalori quasi nulli
);

void svd_bidiagonale_compat(
    const Mat& B,
    Mat& Ub,
    Mat& Vb,
    Vec& sigma
);

void jacobi_simmetrica(
    const Mat& A, 
    Vec& lambda, 
    Mat& V);

void ordina_autocoppie(
    Vec& lambda,
    Mat& V,
    double zero_tol = 0.0,
    SortOrder order = SortOrder::Desc);   // default: discendente

void gram_schmidt_modificato(
    Mat& Q, 
    int j0 = 0);

void check_sv_vs_lambda(
    Vec lambda,
    const Vec& sigma);

figure_handle TableInit (
    const bool ahold, 
    const std::string &nome, 
    const std::string &titolo, 
    const int xlab, 
    const int ylab) ;

void legend_align(
    legend_handle lg, 
    int pos_enum, 
    float xscale,
    float yscale) ;

#endif
