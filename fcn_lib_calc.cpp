#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <limits>   // per std::numeric_limits
#include <matplot/matplot.h>
#include "fcn_lib_calc.h"
#include "json.hpp"
#include <fstream>
#include <stdexcept>

using std::cout;
using std::endl;
using Vec = std::vector<double>;
using Mat = std::vector<Vec>;
using namespace matplot;
using namespace std;
using json = nlohmann::json;

std::string itostr(const int nn) {
    return std::to_string(nn);
} 

double deg2rad(const int alpha) {
    return (((double) alpha) * PI / 180.0);
}

void cin_clear() {
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
    };
    std::streamsize max_ignore = std::cin.rdbuf()->in_avail(); // numero di caratteri disponibili in ingresso
    if (max_ignore > 0) {
        std::cin.ignore(max_ignore, '\n'); // ignora fino a max_ignore caratteri o fino a newline
    };
};

// -----------------------------------------------------------------------------
// Funzione f(t) nota nel problema di Cauchy.
// Cambiare definizione per provare altri casi (es. cos(t), t, ...).
// -----------------------------------------------------------------------------
double f_x(double t) {
    return  t;
}
double f_sin(double t) {
    return  std::sin(t); // esempio: f(t) = sin(t)
}
double f_cos(double t) {
    return  std::cos(t); // esempio: f(t) = cos(t)
}
double f_atan(double t) {
    return  std::atan(t); // esempio: f(t) = atan(t)
}
double f_atan_d(double t) {
    return 1.0 / (1.0 + t * t); // derivata di atan(t), giuro che mi ha letto nel pensiero
}

double fcallb(
    double t,  
    double (*f)(double)) {
    return f(t);
}

double h_ticks(
    const double a_start, 
    const double a_stop, 
    const int a_steps){
    if (a_stop>a_start) {
        return (a_stop-a_start)/double(a_steps);
    } else {
        cout << "Intervallo da " << a_start << " a " << a_stop << " non valido!\n";
        exit(1);
    }
}


// ----------------------------------------------------------------------------
// Funzione di utilità: Display  
// ----------------------------------------------------------------------------

string color_bool(
    const bool val) {
    string s = "";
    if (!val) {
        s =  "\033[92m";  // verde chiaro sta migliorando 
    } else {
        s = "\033[91m";  // rosso chiaro e' gia' al fondo della prec
    }
    return s;

/*
    codici ansi per colori (ricordarsi setw dopo cout del codice)
    30 nero      + 10 bg e + 60 versioni chiare
    31 rosso     
    32 verde
    33 giallo
    34 blu
    35 magenta
    36 cyan
    37 bianco
*/    
}


string color_dbl(
    const double val) {

    const double precision1 = 1e-10;
    const double precision2 = 1e-16;


    string s = "";
    if (val > precision1) 
        s =  "\033[92m";  // verde chiaro positivo 
    else if (val > precision2) 
        s =  "\033[37m";  // azzurro scuro zero+
    else if (val < -precision1)
        s = "\033[91m";  // rosso chiaro negativo
    else if (val < -precision2)
        s = "\033[90m";  // giallo scuro zero-
    else
        s = "\033[97m";  // bianco per zero zero nullo;
    return s;

/*
    codici ansi per colori (ricordarsi setw dopo cout del codice)
    30 nero      + 10 bg e + 60 versioni chiare
    31 rosso     
    32 verde
    33 giallo
    34 blu
    35 magenta
    36 cyan
    37 bianco
*/    
}

void color_rst(){
    std::cout << "\033[0m";       // reset colore - importante!
}

/*
//
// questa e' la versione di bordo
//
void stampa_matrice(const Mat &A, const std::string &nome) {
    cout << "\n-- " << nome << " --\n";
    for (const auto &row : A) {
        for (double v : row) {
            cout << v << " ";
        }
        cout << "\n";
    }
}
*/

//
// questa e' la versione di campionamento
//

void stampa_matrice(
    const Mat& G, 
    const double NR, 
    const std::string &nome) 
{
    std::cout << "\n── " << nome << " " << NR << "(" <<  G.size() << ")" <<" ──\n";
    for (int i = 0; i < (int)G.size(); ++i) {
        for (int j = 0; j < (int)G[i].size(); ++j)
            std::cout << std::fixed
		      << std::setprecision(8)
              << color_dbl(G[i][j]) 
              << std::setw(12) 
              << std::setfill(' ')
		      << G[i][j] << " ";
		      // << G[i][j] << " ";
        color_rst();
        std::cout << "\n";
    }
}

Vec vector_shift(
    const Vec &v, 
    const double shift) {
    Vec v_shifted = v;
    for (int i = 0; i < (int)v.size(); ++i) {
        v_shifted[i] += shift;
    }
    return v_shifted;
}

void vector_dump (
    Vec x, 
    int colspan, 
    int totnum, 
    const std::string s) {
    int icol = 0, ipos = 0;
//    cout << "entro con " << totnum;
	std::cout << "Inspect vector: " << s << endl;
    do
    {
        while (icol<colspan && ipos<totnum)
        {
            cout << std::fixed
                << std::setprecision(7)
                << std::setw(10)       
                << std::setfill('0')            
                << x[ipos] << "  ";
            icol++;
            ipos++;
        };
        icol = 0;
        cout <<endl;
//        cout << "Fine riga ad " << ipos;
//        exit(1);
    } while (ipos<totnum);
    cout  << endl;
//    cout << "Uscito con "  << ipos;   
}

//
// Funzioni algebriche 
//

Mat crea_matrice(
    int righe, 
    int colonne) {
    // Crea una matrice N x N inizializzata a zero

    return Mat(righe, Vec(colonne, 0.0));
}

Mat costruisci_triangolare(
    int n) 
{
    Mat T(n, Vec(n, 0.0)); // vettore anonimo
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
            T[i][j] = 1.0;
    return T;
};

Mat costruisci_inv_triangolare_sint(
    int n)
{
    Mat T(n, Vec(n, 0.0)); // vettore anonimo
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
            if (i==j){
                T[i][j] = 1.0; 
            } else {
                if (i==j+1) T[i][j] = -1.0;
            } ;
    return T;
}

Vec risolvi_colonna(
    const Mat& L,
    const Mat& U,
    int j){
        // Risolve T x = e_j usando LU
        // Risolve L U x = e_j usando sostituzione in avanti e indietro

    int n = L.size();
    Vec e(n, 0.0);
    e[j] = 1.0;
    Vec y = forward_substitution(L, e);
    // vector_dump(y, 10, y.size(), "y");
    Vec x = backward_substitution(U, y);
    // vector_dump(x, 10, x.size(), "x");
    return x;
    };

Mat calcola_inversa_LU (
    const Mat& T) {          
    // Calcola l' inversa di A usando la fattorizzazione LUx = e_i

    Mat  L, U , T_inv;
    Vec T_inv_j;

    T_inv=crea_matrice(T.size(), T[0].size()); // alloca l' inversa della stessa dim di T

    LU(T, L, U); 
    
    // Se T fosse triangolare unitaria (caso dell' integrale se  h=1)
    // L = T e U = I ma lasciamoglielo calcolare perche' h e' funzione dell' intervallo e del numero di nodi e quindi non sappiamo se e' unitaria o no.
    // Quindi se Tx = e_i allora x è la i-esima colonna di T^-1
    // cosa succede qui:
    // Tx = e_i ==> LUx = e_i => Ly = e_i => Ux = y 

    for (std::size_t j=0;j<T[0].size();j++) {
        T_inv_j = risolvi_colonna(L, U, j); // risolvi per e_j e ottieni la colonna j di T^-1
        for (std::size_t i=0;i<T_inv.size();i++) {
            T_inv[i][j] = T_inv_j[i];
        };
    };
    return T_inv;
}

Vec nodi_equidistanti(
    const double amin, 
    const double amax, 
    const int NPoints) {
    //
    // probabilmente linspace di matplot++ potrebbe essere usato per generare i nodi
    //
    Vec x(NPoints);
    double NRange = amax - amin;
    double h = h_ticks(amin, amax, (double) NPoints);
    for (int i=0; i < NPoints; i++)
    {
        x[i]= ((double) i * h) + amin ;
    }
    return x;
    // DONE : riempire x
}

Vec nodi_random(
    const double amin, 
    const double amax, 
    const int NPoints) {

    std::random_device rd ; // obtain a random number from hardware
    std::mt19937 e2(rd()); // estrattore di interi tra e2.min() ed e2.max()
	double rndmin = e2.min();
	double rndmax = e2.max();
	double rndrange = rndmax - rndmin; // range, nel caso l' implentazione non setti min a 0
    double rndthis; // estratto da scalare
    double rndfrac; // estratto scalato
    cout << "min: "<<rndmin<<"\t max: " <<rndmax<< "\t range: " << rndrange <<endl;
    Vec x(NPoints);
    for (int n = 0; n < NPoints; ++n) {
        rndthis = e2() - rndmin;
        rndfrac=  rndthis /  rndrange;
        x[n]=rndfrac;
        
    };
   
    return x;

}

Vec nodi_bubblesort (
    const Vec x_uns, 
    const int totnum){
    double buff;
    Vec x = x_uns;
    for (int i=0;i<totnum;i++)
    {
        for (int j=i;j<totnum;j++){
            if (x[i] > x[j]) {
                buff=x[i];
                x[i]=x[j];
                x[j]=buff;
            }
        }
    }
    return x;
}

double prodotto_scalare(
    const Vec &u, 
    const Vec &v) {
    double s = 0.0;
    int usize = (int) u.size();
    int vsize = (int) v.size();
    if (usize==vsize) 
    { 
        for (int i = 0; i < usize; i++)  s += u[i]*v[i];
    }
    else 
    {
        cout << "Prodotto scalare impossibile tra dim " << usize << " e " << vsize << endl;
        exit(1);
    };
    return s;
    // DONE: prodotto scalare
}


Mat prodotto_matrici(
    const Mat& A, 
    const Mat& B) 
{
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    if (colsA != rowsB) {
        throw std::runtime_error(
            "Dimensioni incompatibili per il prodotto matrici: A è " \
            + std::to_string(rowsA) + "x" + std::to_string(colsA) \
            + " e B è " + std::to_string(rowsB) + "x" + std::to_string(colsB)
        );
    }

    Mat C(rowsA, Vec(colsB, 0.0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return C;
};

Vec prodotto_matrice_vettore(
    const Mat& A, 
    const Vec& v) 
{
    // u = Av

    int rowsA = A.size();
    int colsA = A[0].size();
    int sizeV = v.size();

    if (colsA != sizeV) {
        throw std::runtime_error(
            "Dimensioni incompatibili per il prodotto matrice-vettore: A è " \
            + std::to_string(rowsA) + "x" + std::to_string(colsA) \
            + " e v è di dimensione " + std::to_string(sizeV)
        );
    }

    Vec u(rowsA, 0.0);

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsA; ++j) {
            u[i] += A[i][j] * v[j];
        }
    }

    return u;
};  

Mat prodotto_matrice_coeff(
    const Mat& A, 
    const double coeff) 
{
    Mat C = A; // copia di A
    for (std::size_t i = 0; i < C.size(); ++i) {
        for (std::size_t j = 0; j < C[i].size(); ++j) {
            C[i][j] *= coeff;
        }
    }
    return C;
};

Vec forward_substitution(
    const Mat &L, 
    const Vec &b) {
        // -----------------------------------------------------------------------------
        // TODO: implementare la sostituzione in avanti per L x = b
        // L triangolare inferiore con L[i][i] != 0.
        // -----------------------------------------------------------------------------
    int n = static_cast<int>(L.size());
    Vec x(n, 0.0);
    x[0] = b[0] / L[0][0];
    for (int i = 1; i < n; ++i) {
        double s = b[i];
        for (int j = 0; j < i; ++j) {
            s -= L[i][j] * x[j];
        }
        x[i] = s / L[i][i];
    };
    return x;
}

// e rovesciata                                                                                                                                                                                                                                                                                                                                                     

Vec backward_substitution(
    const Mat &U, 
    const Vec &y) {
    int m = static_cast<int>(U.size());
    Vec x(m, 0.0);

    // TODO 3: implementare la sostituzione in avanti
    x[0] = y[0] / U[0][0];
    for (int i = m-1; i > 0; --i) {
        double s = y[i];
        for (int j = i+1; j < m - 1; ++j) {
            s -= U[i][j] * x[j];
        }
        x[i] = s / U[i][i];
    };
    return x;
}

void LU(
    const Mat &A, 
    Mat &L, 
    Mat &U) {
        // -----------------------------------------------------------------------------
        // implementare la fattorizzazione LU per L
        // L matrice tridiagonale con 2 sulla diagonale, -1 su sopra e sotto-diagonale
        // -----------------------------------------------------------------------------

    int m = A.size();
    L = Mat(m, Vec(m, 0.0));
    U = A; // U inizia come A e viene modificata in-place

    // inizializza L con 1 sulla diagonale, proviamo con il loop a parte.

    for (int i = 0; i < m; ++i) {
        L[i][i] = 1.0;
    } 
    // L[m-1][m-1] = 1.0; // o loop a parte o cosi' perche non ci arriva
    for (int k = 0; k < m-1; ++k) {
        L[k][k] = 1.0;
        for (int i = k+1; i < m; ++i) {
            if (U[k][k] == 0) {
                throw std::runtime_error("Pivoting necessario, ma non implementato");
            }
            double multiplier = U[i][k] / U[k][k];
            L[i][k] = multiplier;
            for (int j = k; j < m; ++j) {
                U[i][j] -= multiplier * U[k][j];
            }
        }
    }
}

// 
// Funzioni per matplot++

figure_handle TableInit (
    const bool ahold, 
    const std::string &nome, 
    const std::string &titolo, 
    const int xlab, 
    const int ylab) 
{
    figure_handle fig = matplot::figure(ahold);
    fig->size(1600,800);
    fig->position(10,10);
    fig->name(nome);
    fig->number_title(false);
    fig->title_color({0., 1., 0., 1.});   //alpha, red, green, blue 0. ~ 1.
    fig->tiledlayout(xlab, ylab); // stabilisce panel xlab righe x xlab colonne
    fig->title(titolo);
    fig->title_font_size_multiplier(3);
    return fig;
}


void legend_align(
    legend_handle lg, 
    int pos_enum, 
    float xscale,
    float yscale) {

    // alla fine si e' scoperto che la posizione del centro della legenda dipende dai punti sulla scala X
    // e quindi da K / 2 con un offset di 1 perche' il primo punto e' a 0 e non a 1 (bravo Copilot con gli option base 0)

    using GA = matplot::legend::general_alignment;
    using HA = matplot::legend::horizontal_alignment;
    using VA = matplot::legend::vertical_alignment;
    lg->box(false);

    if (pos_enum == 0) { // up and out, centered
        float xcenter = (xscale + 1.0) / 2.0;
        lg->position({ xcenter , yscale}); 
        lg->horizontal_location(HA::center);
        lg->vertical_location(VA::bottom);
        lg->box(true);
    } else if (pos_enum == 1) { // bottom right    
        lg->horizontal_location(HA::right);
        lg->vertical_location(VA::bottom);
    } else if (pos_enum == 2) { // center left
        lg->horizontal_location(HA::left);
        lg->vertical_location(VA::center);
    };
};

//
// gestione del menu principale e dei suoi item
//  

MenuConfig load_menu_config(const std::string& filename) {
    // int dummy = system("clear"); 

    std::cout << "Working directory: " << std::filesystem::current_path() << '\n';
    std::cout << "Provo ad aprire: " << filename << '\n';

    std::ifstream f(filename);
    if (!f) {
        throw std::runtime_error("Impossibile aprire il file di menu: " + filename);
    }

    // per debug: controlla se c'è contenuto
    f.seekg(0, std::ios::end);
    std::streampos size = f.tellg();
    std::cout << "Dimensione file: " << size << " byte\n";
    f.seekg(0, std::ios::beg);

    nlohmann::json j;
    f >> j;
    // ... 

    MenuConfig cfg;
    cfg.title = j.value("title", "Menu");

    if (!j.contains("items") || !j["items"].is_array()) {
        throw std::runtime_error("Il file JSON non contiene un array 'items' valido.");
    }

    for (const auto& item : j["items"]) {
        MenuItem m;
        m.key     = item.at("key").get<int>();
        m.label   = item.at("label").get<std::string>();
        m.action  = item.at("action").get<std::string>();
        m.enabled = item.value("enabled", true);
        cfg.items.push_back(m);
    }

    return cfg;
}

const MenuItem* find_menu_item(const MenuConfig& menu, int key) {
    for (const auto& item : menu.items) {
        if (item.key == key) {
            return &item;
        }
    }
    return nullptr;
}

void wait_return_to_menu(bool bypass_waitakey) {
    if (!bypass_waitakey) {
        std::cout << "\nPremi INVIO per tornare al menu...";
        cin_clear();
        std::cin.get();
    };
}