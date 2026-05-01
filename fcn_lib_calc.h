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
using Mat = std::vector<Vec>;
using namespace matplot;
using namespace std;

using KM = matplot::keyword_manual_type ;
using KA = matplot::keyword_automatic_type ;


const double PI = std::acos(-1.0);


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
    const int a_steps);

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

string color_bool(
    const bool val);

string color_dbl(
    const double val);

void color_rst();

void stampa_matrice(
    const Mat &A, 
    const double NR, 
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

Mat costruisci_triangolare(
    int n);
Mat costruisci_inv_triangolare_sint(
    int n);

Vec risolvi_colonna(
    const Mat& L,
    const Mat& U,
    int j);

double prodotto_scalare(
    const Vec &u,
    const Vec &v);

Mat prodotto_matrici(
    const Mat& A, 
    const Mat& B);

Vec prodotto_matrice_vettore(
    const Mat& A, 
    const Vec& v);

Mat prodotto_matrice_coeff(
    const Mat& A, 
    const double coeff) ;

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
    
Mat calcola_inversa_LU (
    const Mat& A);

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
