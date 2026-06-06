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
#include <sstream>
#include <stdexcept>
#include <algorithm>

using namespace std;
using Vec = std::vector<double>;
using VecN = std::vector<int>;
using Mat = std::vector<Vec>;

using namespace matplot;

using json = nlohmann::json;

// 
// gestione interfaccia utente 
// 

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

void clear_screen() 
{int dummy = system("clear");}

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
        s =  "\033[92m";  // verde chiaro positivo oltre +eps_f
    else if (val > precision2) 
        s =  "\033[90m";  // grigio zero+ tra +eps_d e +eps_f
    else if (val < -precision1)
        s = "\033[91m";  // rosso chiaro negativo  oltre -eps_f
    else if (val < -precision2)
        s = "\033[90m";  // grigio zero- tra -eps_d e -eps_f
    else
        s = "\033[97m";  // bianco per zero zero nullo, entro +/- eps_d
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

//
// funzioni di conversione formato ed equivalenza unita' di misura
//


std::string itostr(const int nn) {
    return std::to_string(nn);
} 

double deg2rad(const int alpha) {
    return (((double) alpha) * PI / 180.0);
}

std::string format_numstr(double value) {
    std::stringstream ss;
    ss << std::scientific << std::setprecision(2) << value;
    return ss.str();
}


//
// funzioni di base per intervalli 
//

double h_ticks(
    const double a_start, 
    const double a_stop, 
    const int a_points)
{
        //  ora si chiama a_points per disambiguazione.
        // i points vanno passati come numero di separatori, 
        // quindi se voglio 10 nodi equidistanti allora points = 10 
        // e h = (b-a)/9 perche' gli intervalli sono points-1
        //
        // h e' la lunghezza dell' intervallo e gli intervalli sono points - 1
        //
    if (a_stop>a_start) {
        return (a_stop-a_start)/double(a_points - 1);
    } else {
        cout << "Intervallo da " << a_start << " a " << a_stop << " non valido!\n";
        exit(1);
    }
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

//
// funzioni matematiche ad uso callback 
//

double f_x(double t) {
    return  t;
}
double f_x1(double t) {
    const double eps = 1e-12;
    return 1.0 / (std::abs(t) < eps ? std::copysign(eps, t) : t);}
    
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
double at_zero(double t) {
    return 0.0;
}   
double at_one(double t) {
    return 1.0;
}  
double ft_zero(double t) {
    return 0.0;
}    
double f_sin2plus1(double t){
    double s = std::sin(t);
    return 1.0 / (1.0 + s*s);
}

double fcallb(
    double t,  
    double (*f)(double)) {
    return f(t);
}

//
// funzioni macro di algebra lineare
//

Vec linear_subst_BW(
    const Mat &U, 
    const Vec &y) {
    int m = static_cast<int>(U.size());
    Vec x(m, 0.0);

    // x[0] = y[0] / U[0][0]; // vecchia riga 
    x[m - 1] = y[m - 1] / U[m - 1][m - 1];  // nuova riga

    // for (int i = m-1; i > 0; --i) {   //vecchia riga
    for (int i = m - 2; i >= 0; --i) {  // nuova riga
        double s = y[i];
        for (int j = i + 1; j < m; ++j) { // corretto, era j-1
            s -= U[i][j] * x[j];
        }
        x[i] = s / U[i][i];
    };
    return x;
}

Vec linear_subst_FW(
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

void linear_jacobi_autoval_simmetrica(
    const Mat& D, 
    Vec& lambda, 
    Mat& V)
{
    Mat A = D; 
    int n = A.size();
    V = matrix_build_Id(n);

    const int max_iter = 100 * n * n;
    const double tol = 1e-12;

    for (int iter = 0; iter < max_iter; ++iter)
    {
        int p = 0, q = 1;
        double max_off = 0.0;

        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                if (std::abs(A[i][j]) > max_off)
                {
                    max_off = std::abs(A[i][j]);
                    p = i;
                    q = j;
                }

        if (max_off < tol)
            break;

        double app = A[p][p];
        double aqq = A[q][q];
        double apq = A[p][q];

        double tau = (aqq - app) / (2.0 * apq);
        double t = (tau >= 0.0)
                 ? 1.0 / (tau + std::sqrt(1.0 + tau * tau))
                 : -1.0 / (-tau + std::sqrt(1.0 + tau * tau));

        double c = 1.0 / std::sqrt(1.0 + t * t);
        double s = t * c;

        for (int k = 0; k < n; ++k)
        {
            if (k != p && k != q)
            {
                double aik = A[k][p];
                double akq = A[k][q];

                A[k][p] = c * aik - s * akq;
                A[p][k] = A[k][p];

                A[k][q] = s * aik + c * akq;
                A[q][k] = A[k][q];
            }
        }

        A[p][p] = c*c*app - 2.0*s*c*apq + s*s*aqq;
        A[q][q] = s*s*app + 2.0*s*c*apq + c*c*aqq;
        A[p][q] = 0.0;
        A[q][p] = 0.0;

        for (int k = 0; k < n; ++k)
        {
            double vip = V[k][p];
            double viq = V[k][q];

            V[k][p] = c * vip - s * viq;
            V[k][q] = s * vip + c * viq;
        }
    }

    lambda.resize(n);
    for (int i = 0; i < n; ++i)
        lambda[i] = A[i][i];
}


Vec linear_LU_calcola_autovalori (
    const Mat& A) 
{
    // Calcola gli autovalori di A usando la fattorizzazione LU
    // Non e' detto che funzioni perche' A potrebbe essere singolare ma proviamoci lo stesso
    // da usare solo su matrici simmetriche e positive semidefinite come Tt*T,
    // altrimenti la LU potrebbe essere instabile o non definita.
    Mat L, U;
    linear_LU_dec(A, L, U);
    Vec autovalori(U.size(), 0.0);
    for (std::size_t i=0;i<U.size();i++) {
        autovalori[i] = U[i][i]; // gli autovalori di A sono approssimativamente i valori diagonali di U
    };
    return autovalori;
};  

void linear_LU_dec(
    const Mat &A, 
    Mat &L, 
    Mat &U) 
{
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

Mat linear_LU_inversa (
    const Mat& T) {          
    // Calcola l' inversa di A usando la fattorizzazione LUx = e_i

    Mat  L, U , T_inv;
    Vec T_inv_j;

    T_inv=matrix_build_zero(T.size(), T[0].size()); // alloca l' inversa della stessa dim di T

    linear_LU_dec(T, L, U); 
    
    // Se T fosse triangolare unitaria (caso dell' integrale se  h=1)
    // L = T e U = I ma lasciamoglielo calcolare perche' h e' funzione dell' intervallo e del numero di nodi e quindi non sappiamo se e' unitaria o no.
    // Quindi se Tx = e_i allora x è la i-esima colonna di T^-1
    // cosa succede qui:
    // Tx = e_i ==> LUx = e_i => Ly = e_i => Ux = y 

    for (std::size_t j=0;j<T[0].size();j++) {
        T_inv_j = linear_LU_risolve_colonna(L, U, vector_build_versore_canonico(j, T.size())); // risolvi per e_j e ottieni la colonna j di T^-1
        for (std::size_t i=0;i<T_inv.size();i++) {
            T_inv[i][j] = T_inv_j[i];
        };
    };
    return T_inv;
}

Vec linear_LU_risolve_colonna(
    const Mat& L,
    const Mat& U,
    Vec e){
        // Risolveva  T x = e usando LU
        // Risolve L U x = e_j usando sostituzione in avanti e indietro
        // ora risolve un vettore generico e 

    int n = L.size();
    Vec y = linear_subst_FW(L, e);
    // vector_dump(y, 10, y.size(), "y");
    Vec x = linear_subst_BW(U, y);
    // vector_dump(x, 10, x.size(), "x");
    return x;
    };

Vec linear_LU_risolve_sistema(
    const Mat& T,
    const Vec& x)
{
    Mat L, U;
    int N = T.size(); 
    linear_LU_dec(T, L, U); 
    Vec g = linear_LU_risolve_colonna(L, U, x);
    return g;
}

double linear_max_autoval_pwr_any(
    const Mat& A, 
    int max_iter, 
    double tol) 
{
    int seed = 1;
    // questa e' la versione per matrici qualsiasi, entra A ed usa AtA senza calcolarlo
    int N = A[0].size(); // n. di colonne di A = dim del vettore q[]
    std::mt19937 rng(seed);
    std::normal_distribution<double> gauss(0.0, 1.0);

    Vec q(N);
    Vec z(N);
    for (double& v : q) v = gauss(rng); 
    double nq = vector_calcola_norma(2,q);
    for (double& v : q) v /= nq;

    double mu = 0.0;
    double mu_old = 0.0;
    double nz;
    Mat At = matrix_trasposta(A);
    // TODO: iterazione del metodo delle potenze su A^T*A
    // Suggerimento: mat_trasposta = trasposta(A);
    //               vec1 = matvec(A, q);
    //               z    = matvec(mat_trasposta, vec1);
    for (int iter=0;iter<max_iter;iter++) {
        z = matrix_prodotto_vector(A, q);
        z = matrix_prodotto_vector(At,z);
        nz = vector_calcola_norma(2,z);
        for (int j=0;j<N;j++) q[j]=z[j]/nz; // q = z normalizzato
        mu = vector_prodotto_scalare(q, matrix_prodotto_vector(At, matrix_prodotto_vector(A, q)));  
        //
        // forse i due passaggi si potevano invertire nel quoziente di Rayleigh
        //
        if (std::abs(mu-mu_old)<tol) break;
        mu_old=mu;
    }
    return mu;
}

double linear_max_autoval_pwr_any_res(
    const Mat& A, 
    int max_iter, 
    double tol) 
{
    int seed = 1;
    // questa e' la versione per matrici qualsiasi, entra A ed usa AtA senza calcolarlo
    int N = A[0].size(); // n. di colonne di A = dim del vettore q[]
    int M = A.size(); //n. di righe di A = dim del vettore Aq[]

    std::mt19937 rng(seed);
    std::normal_distribution<double> gauss(0.0, 1.0);

    Vec q(N);

    for (double& v : q) v = gauss(rng); 
 
    double nq = vector_calcola_norma(2,q);
    for (double& v : q) v /= nq;
 
    Mat At = matrix_trasposta(A);

    // double mu = 0.0;
    // double mu_old = 0.0;
    //
    //iterazione su AtA senza calcolarla esplicitamente
    //
    for (int iter=0;iter<max_iter;iter++) {   // iter k.simo
        Vec Aq = matrix_prodotto_vector(A, q);  // A q(k)
        Vec z = matrix_prodotto_vector(At, Aq); // z = At A q(k)

        // nuovo vettore normalizzato per q(k+1)
        double nz = vector_calcola_norma(2,z);
        if (nz == 0.0) return 0.0; // matrice AtA singolare?
        
        for (int j=0;j<N;j++) q[j]=z[j]/nz; // q = z normalizzato
        //
        // uscita anticipata con controllo su norma2(z) invece di mu
        static double nz_old = 0.0;
        if (iter > 0) {
            double rel = std::abs(nz - nz_old) / std::max(1.0, std::abs(nz));
            if (rel < tol) break;
        }
        nz_old = nz;        
    }
    // valutazione finale sull'ultimo q
    Vec Aq = matrix_prodotto_vector(A, q);  // A q(n)
    Vec z = matrix_prodotto_vector(At, Aq); // z = At A q(n)
    double mu = vector_prodotto_scalare(Aq, Aq);  // (Aq)^t Aq = q^t A^t A q

    // residuo relativo, per chi non si fida....

    Vec r(q.size()); 
    for (int j = 0; j<N; j++) r[j] = z[j] - mu * q[j];
    double res = vector_calcola_norma(2, r) / std::max(1.0, std::abs(mu));
    //
    // poi vediamo cosa possiamo fare con res, per ora lascialo li'

    return mu;  // questo e' ancora l'approssimazione dell' autovalore
}


double linear_max_autoval_pwr_AtA(
    const Mat& M, 
    int max_iter, 
    double tol) 
{
    // questa versione e' per matrici quadrate, quindi funziona per AtA gia' formata 
    std::size_t N = M.size();
    std::vector<double> x(N, 1.0 / std::sqrt(N));  // vettore iniziale normalizzato
    
    double lambda_old = 0.0;
    double lambda = 0.0;
    
    for (int iter = 0; iter < max_iter; ++iter) {
        // y = M * x
        std::vector<double> y(N, 0.0);
        for (std::size_t i = 0; i < N; ++i) {
            for (std::size_t j = 0; j < N; ++j) {
                y[i] += M[i][j] * x[j];
            }
        }
        
        // lambda = y^T * x (prodotto scalare Rayleigh quotient)
        lambda = 0.0;
        for (std::size_t i = 0; i < N; ++i) {
            lambda += y[i] * x[i];
        }
        
        // normalizza x = y / ||y||
        double norm = 0.0;
        for (std::size_t i = 0; i < N; ++i) {
            norm += y[i] * y[i];
        }
        norm = std::sqrt(norm);
        for (std::size_t i = 0; i < N; ++i) {
            x[i] = y[i] / norm;
        }
        
        if (std::abs(lambda - lambda_old) < tol) {
            break;
        }
        lambda_old = lambda;
    }
    
    return lambda;
}


//
// funzioni per la manipolazione di matrici
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
    double (*at)(double),   // default  at_one, se at == nullptr allora at(t) = 0.0
    double (*ft)(double),   // default  ft_zero, se ft == nullptr allora ft(t) = 0.0    
    bool backw)            // default false = forward, se true allora schema backward

        // -----------------------------------------------------------------------------
        // versione generalizzata con coefficiente at(t) davanti a z(t) e con f(t) generica, 
        // da passare come puntatori a funzione con la convenzione che: 
        // 1)   at(t) e ft(t) sono funzioni di t che restituiscono rispettivamente 
        //      il coefficiente di z(t) e il termine noto f(t)
        // 2)   esistono funzioni "costanti" 
        //      at_zero(t) = 0.0 
        //      at_one(t) = 1.0 
        //      ft_zero(t) = 0.0 
        //      at == nullptr come at_zero(t)
        //      ft == nullptr come ft_zero(t)
    
        // continuiamo a farci passare il vettori dei nodi
    {
    t = Vec(n);                 // lo calcoliamo dentro insieme ai valori a(t) e f(t)
    avals = Vec(n);
    fvals = Vec(n);
    L = Mat(n, Vec(n, 0.0));
    b = Vec(n, 0.0);
    using FnPtr = double(*)(double);
    FnPtr ata;
    FnPtr fta;    
    ata = (at != nullptr) ? at : at_zero; // se at è nullptr, usa at_zero
    fta = (ft != nullptr) ? ft : ft_zero; // se ft è nullptr, usa ft_zero
    double h = h_ticks(t0, T, n);

    for (int i = 0; i < n; ++i) {
        t[i] = t0 + i * h;
        avals[i] = fcallb(t[i], ata);
        fvals[i] = fcallb(t[i], fta);
    };

    if (backw) {

    // Schema backward: x_i - (1+h)*x_{i+1} = h*f(t_i), i = 0..n-2
    // Condizione finale: x_{n-1} = z_bc
    // Produce matrice triangolare superiore, risolta da linear_subst_BW

        for (int i = 0; i < n - 1; ++i) {
            L[i][i] = -(1.0 + h * avals[i + 1]);   
            L[i][i + 1] = 1.0;
            b[i] = h * fvals[i + 1]; // prima era i
        }
        L[n - 1][n - 1] = 1.0;
        b[n - 1] = z_bc;

    }
    else {
        // forward: lower triangolare, con condizione iniziale

        // Riga 0: x_0 = x0  => L[0][0] = 1, b[0] = x0
        // Righe i>=1: - (1+h) x_{i-1} + x_i = h f(t_{i-1})
        // X[0][0] sta sulla diagonale, potrei assegnargli 1 come agli altri Xii
        // ma i termini sotto diagonale sono uno in meno: condiziono o li conto in riga)
        L[0][0] = 1.0;
        b[0] = z_bc;
        for (int i = 1; i < n; ++i) {
            L[i][i - 1] = -(1.0 + h * avals[i - 1]);  // versione per z' = z + f(t) ok
            L[i][i] = 1.0;
            b[i] = h * fvals[i - 1];
        }
    }
}

// FCN – Laboratorio 2, Esercizio 1A
// Problema di Cauchy: x'(t) = x(t) + f(t), x(t0) = c
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
    bool backw) 
    {
    t = Vec(n);
    fvals = Vec(n);
    L = Mat(n, Vec(n, 0.0));
    b = Vec(n, 0.0);

    double h = (T - t0) / (n - 1);

    // TODO 1: riempire il vettore dei nodi t_i e dei valori f(t_i)
    
    // migliorata: passare puntatore a funzione invece di indice di f
    for (int i = 0; i < n; ++i) {
        t[i] = t0 + i * h;
        fvals[i] = fcallb(t[i], ft); 
    };

    // TODO 2: costruire L e b
    if (backw) {
    // Schema backward: x_i - (1+h)*x_{i+1} = h*f(t_i), i = 0..n-2
    // Condizione finale: x_{n-1} = x0
    // Produce matrice triangolare superiore, risolta da linear_subst_BW

        // qui abbiamo -(1+h) sulla diagonale e 1 a destra (buono per z' = z + f(t))
        // test invece con -1 per integrale puro z' = f(t) 
        for (int i = 0; i < n - 1; ++i) {
            L[i][i] = -1.0;             //versione per z' = f(t) ok
            L[i][i + 1] = 1.0;
            b[i] = h * fvals[i+1]; // prima era i
        }
        L[n - 1][n - 1] = 1.0;
        b[n - 1] = x0;

    }
    else {
        // forward: lower triangolare, con condizione iniziale

        // Riga 0: x_0 = x0  => L[0][0] = 1, b[0] = x0
        // Righe i>=1: - (1+h) x_{i-1} + x_i = h f(t_{i-1})
        // X[0][0] sta sulla diagonale, potrei assegnargli 1 come agli altri Xii
        // ma i termini sotto diagonale sono uno in meno: condiziono o li conto in riga)
        L[0][0] = 1.0;
        b[0] = x0;
        for (int i = 1; i < n; ++i) {
            L[i][i - 1] = -1.0;             //versione per z' = f(t) da testare 
            L[i][i] = 1.0;
            b[i] = h * fvals[i - 1];
        }
    }
}

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
    bool backw) 
    {
    t = Vec(n);
    fvals = Vec(n);
    L = Mat(n, Vec(n, 0.0));
    b = Vec(n, 0.0);

    double h = (T - t0) / (n - 1);

    // TODO 1: riempire il vettore dei nodi t_i e dei valori f(t_i)
    
    // migliorata: passare puntatore a funzione invece di indice di f
    for (int i = 0; i < n; ++i) {
        t[i] = t0 + i * h;
        fvals[i] = fcallb(t[i], ft); 
    };

    // TODO 2: costruire L e b
    if (backw) {
    // Schema backward: x_i - (1+h)*x_{i+1} = h*f(t_i), i = 0..n-2
    // Condizione finale: x_{n-1} = x0
    // Produce matrice triangolare superiore, risolta da linear_subst_BW

        // qui abbiamo -(1+h) sulla diagonale e 1 a destra (buono per z' = z + f(t))
        // test invece con -1 per integrale puro z' = f(t) 
        for (int i = 0; i < n - 1; ++i) {
            L[i][i] = -(1.0 + h);   // versione per z' = z + f(t) da testare
            L[i][i + 1] = 1.0;
            b[i] = h * fvals[i+1]; // prima era i
        }
        L[n - 1][n - 1] = 1.0;
        b[n - 1] = x0;

    }
    else {
        // forward: lower triangolare, con condizione iniziale

        // Riga 0: x_0 = x0  => L[0][0] = 1, b[0] = x0
        // Righe i>=1: - (1+h) x_{i-1} + x_i = h f(t_{i-1})
        // X[0][0] sta sulla diagonale, potrei assegnargli 1 come agli altri Xii
        // ma i termini sotto diagonale sono uno in meno: condiziono o li conto in riga)
        L[0][0] = 1.0;
        b[0] = x0;
        for (int i = 1; i < n; ++i) {
            L[i][i - 1] = -(1.0 + h);  // versione per z' = z + f(t) ok
            L[i][i] = 1.0;
            b[i] = h * fvals[i - 1];
        }
    }
}
// FCN – Laboratorio 2, Esercizio 1B
// Problema differenziale con condizioni al bordo: x''(t) = f(t), x(0) = 0, x(1) = 0

Mat matrix_build_derivata1(
    int n, 
    double a, 
    double b) {
    int m = n - 1;               // numero di righe
    Mat D(m, Vec(n, 0.0));
    // ----------------------------------------------------------------------------
    // TODO 1 --- Matrice D (differenze finite in avanti)
    // Matrice D ∈ R^{(n-1)×n} che approssima la derivata prima su [0,1]:
    // (Du)_i = (u_{i+1} - u_i) / h, con h = 1/(n-1), i = 0,...,n-2.
    // ----------------------------------------------------------------------------


    double h = h_ticks(a, b, n );
    for (int i = 0; i < m; ++i) {
        D[i][i]     = -1.0 / h;
        D[i][i + 1] =  1.0 / h;
    };
    return D;
}


double matrix_build_gausskernel_item(  // forse non vale neanche la pena esportarla
    const double t, 
    const double s) 
{
    if (s!= 0) {
        return std::exp( - (t*t)/(2*s*s));
    }else {
        throw std::invalid_argument("Sigma nullo in calcolo Kernel Gaussiano per t = " + std::to_string(t));
    }
}

Mat matrix_build_gausskernel(
    const int N, 
    const double sigma,
    const double h, 
    bool norm_flag, 
    Vec& Indicatori) 
    {
        // indicatori deve restituire il numero di cond, la norma della matrice e della sua inversa
        Mat K(N, Vec(N, 0.0));
        for (int i = 0; i < N; ++i) {
            double xi = i * h;
            for (int j = 0; j < N; ++j) {
                double xj = j * h;
                K[i][j] = matrix_build_gausskernel_item((xi-xj), sigma);
            }
        }
        if (norm_flag) K = matrix_normalize_byrow(K);
        double n2K = matrix_calcola_norma(12, K);
        // double n12K = matrix_calcola_norma(12, K);
        // cout << "Confronto norme: nuova 2 " << n2K << " contro vecchia ora 12 " << n12K <<endl;
        //
        // test norma spettrale
        //
        double n2K_inv = matrix_calcola_norma(12, linear_LU_inversa(K));
        // double n12K_inv = matrix_calcola_norma(12, linear_LU_inversa(K));
        // cout << "Confronto norme: nuova 2 " << n2K_inv << " contro vecchia ora 12 " << n12K_inv <<endl<<endl;

        Indicatori = {n2K*n2K_inv, n2K, n2K_inv};
    return K;
}       

Mat matrix_build_gram(
        const Vec& x, 
        const int K) 
    {
        // gram computes a $K \times K$ Gram matrix from a vector x by sampling K subvectors with campiona, 
        // taking pairwise dot products via vector_prodotto_scalare, and filling a symmetric matrix G.

        Mat G = matrix_build_zero(K, K);
        Vec sample_u, sample_v;
        double Gij;     // usiamo uno scalare per accelerare senza lookup doppio
        for (int i=0;i<K;i++)
            for (int j=i;j<K;j++)
            {
                sample_u = vector_campiona_f_k(i, x, f_cos);
                sample_v= vector_campiona_f_k(j, x, f_cos);
                Gij = vector_prodotto_scalare(sample_u, sample_v);
                G[i][j] = Gij;
                G[j][i] = Gij;
            }

        // DONE: doppio ciclo su i e j
        return G;
    }

Mat matrix_build_Id(
    int n) 
{
    //
    // crea matrice identita' n x n
    //

    Mat I = matrix_build_zero(n, n);
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

Mat matrix_build_triang(
    int n) 
{
    Mat T(n, Vec(n, 0.0)); // vettore anonimo
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
            T[i][j] = 1.0;
    return T;
};

Mat matrix_build_triang_inv(
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


Mat matrix_build_zero(
    int righe, 
    int colonne)
{
    // Crea una matrice N x N inizializzata a zero

    return Mat(righe, Vec(colonne, 0.0));
}

Vec matrix_calcola_deriv_byiter(
    const Vec &u , 
    const double a, 
    const double b) {
        // ----------------------------------------------------------------------------
        // Calcolo della derivata discreta senza costruire D esplicitamente
        // per comprendere che un'applicazione lineare non necessariamente 
        // deve essere calcolata tramite una matrice.
        // u ∈ R^n, Du ∈ R^{n-1}.
        // ----------------------------------------------------------------------------
    int n = static_cast<int>(u.size());
    Vec Du(n-1, 0.0);
    double h = h_ticks(a, b, n);
    // TODO: calcolare Du[i] con un ciclo for soltanto usando u[i+1] - u[i].
    for (int j=0; j<n-1; ++j) {
        Du[j] = (u[j+1]-u[j]) / h;
    }
    return Du;
}

Vec matrix_calcola_deriv_bymatr(
    const Mat &D, 
    const Vec &u) { 

    return matrix_prodotto_vector(D, u);
}

Mat matrix_calcola_diff(
    const Mat& A, 
    const Mat& B)
{
    return matrix_calcola_subfact(A,B,1);
}

double matrix_calcola_errore_Fr(
    const Mat& A, 
    const Mat& B) 
{
// Norma di Frobenius di A - B
    double s = 0;
    for (int i = 0; i < (int)A.size(); ++i)
        for (int j = 0; j < (int)A[0].size(); ++j) {
            double d = A[i][j] - B[i][j];
            s += d*d;
        }
    return std::sqrt(s);
}
void matrix_calcola_media(
    const Mat& A, 
    Vec& somma_col, 
    Vec& somma_row) 
{   
    int m = A.size();
    int n = A[0].size();
    for (int i = 0; i < m; i++) {

        for (int j = 0; j < n; j++) {
            double Aij = A[i][j];
            somma_row[i] += Aij;
            somma_col[j] += Aij;
        }
        somma_row[i] /= (double) m;
        for (int j = 0; j < n; j++) 
            somma_col[j] /= (double) n;
    }
}

double matrix_calcola_norma(
    int norma,
    const Mat& A) 
{
    switch (norma) {
        case 3: 
            // norma 2: versione limitata LU
            {
                    //  vediamo se funziona con la LU di Tt * T, 
                    // ma non e' detto che funzioni perche' Tt*T e' simmetrica e positiva semidefinita, 
                    // quindi la sua LU potrebbe essere instabile o non definita. 
                    // nel frattempo sono implementati altri 3 metodi delle potenze per calcolare la norma 2 in modo piu' robusto,
                    //  questo e' solo un test e la risposta e' dentro di te...ma e' sbagliata!
                Mat At = matrix_trasposta(A);
                Mat AtA = matrix_prodotto_matrix(At, A);
                Vec AutoAtA = linear_LU_calcola_autovalori(AtA); // ci riesce.... se non e' singolare ;)  
                    // fosse SVD avremmo gia' il massimo valore singolare come primo autovalore di AtA, 
                    // ma con la LU non e' detto che sia ordinato o che sia stabile, 
                    // quindi cerchiamo il massimo tra tutti gli autovalori di AtA.
                double max_autovalore = 0.0;
                for (double val : AutoAtA) {
                    if (std::abs(val) > max_autovalore) {
                        max_autovalore = std::abs(val);
                    }
                }   
                // vector_dump(AutoAtA, 10, AutoAtA.size(), "Autovalori");
                return std::sqrt(max_autovalore); // la norma 2 è la radice quadrata del massimo autovalore di A^T A    
            }
        case 2: // versione metodo delle potenze per A con uscita a res (non calcola AtA esplicita)
            {
                return std::sqrt(linear_max_autoval_pwr_any_res(A, 1000, 1e-12));
            }
           
        case 12: // versione metodo delle potenze per A con uscita abs senza economia (non calcola AtA esplicita)
            {
                return std::sqrt(linear_max_autoval_pwr_any(A, 1000, 1e-12));
            }

        case 22: // versione metodo delle potenze per AtA
            {
                Mat At = matrix_trasposta(A);
                Mat AtA = matrix_prodotto_matrix(At, A);
                return std::sqrt(linear_max_autoval_pwr_AtA(AtA, 1000, 1e-12));
            }

        case 1: // norma 1: massimo assoluto della somma delle colonne
            {
                double max_col_sum = 0.0;
                for (std::size_t j = 0; j < A[0].size(); ++j) {
                    double col_sum = 0.0;
                    for (std::size_t i = 0; i < A.size(); ++i) {
                        col_sum += std::abs(A[i][j]);
                    }
                    if (col_sum > max_col_sum) {
                        max_col_sum = col_sum;
                    }
                }
                return max_col_sum;
            }
        case 0: // norma infinito: massimo assoluto della somma delle righe
            {
                double max_row_sum = 0.0;
                for (std::size_t i = 0; i < A.size(); ++i) {
                    double row_sum = 0.0;
                    for (std::size_t j = 0; j < A[0].size(); ++j) {
                        row_sum += std::abs(A[i][j]);
                    }
                    if (row_sum > max_row_sum) {
                        max_row_sum = row_sum;
                    }
                }
                return max_row_sum;
            }
        case -1: // norma Frobenius: radice quadrata della somma dei quadrati di tutti gli elementi
            {
                double sum_squares = 0.0;
                for (const auto& row : A) { // sintassi for row in A tipo python, 
                                            // ma in C++ con auto e reference per evitare copie inutili
                    for (auto& val : row) { // sintassi for val in row tipo python,                              
                        sum_squares += val * val; 
                    }
                }
                return std::sqrt(sum_squares);
            }
        default:
            throw std::invalid_argument("Norma non supportata");
    }
};

Mat matrix_calcola_subfact(
    const Mat& A, 
    const Mat& B, 
    const double fact)
{
    int ra = A.size(); 
    int ca = A[0].size();
    int rb = B.size();
    int cb = B[0].size();
    if ((ra == rb) && (ca == cb)) {
        Mat R = matrix_build_zero(ra, ca);
        for (int i = 0; i < ra; i++) {
            for (int j = 0; j < ca; j++) {
                R[i][j] = A[i][j] - fact * B[i][j];
            }
        };
        return R;
    } else {
        cout << "Matrici di dimensioni incompatibili per sottrazione A: "
            << ra << "x" << ca << " e B: "
            << rb << "x" << cb << endl; 
        exit(-1);
    }
       
}

Mat matrix_centra_su_media(
    const Mat& A, 
    const Vec& avg_vec,
    bool by_col)
{
    int m = A.size();
    int n = A[0].size();
    Mat Res = matrix_build_zero(m,n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            if (by_col) {
                Res[i][j]=A[i][j] - avg_vec[j];
            }else{
                Res[i][j]=A[i][j] - avg_vec[i];
            }
    return Res;
}

Mat matrix_completa_ridotta(
    const Mat& A)
// completa una matrice V ridotta da dXn a dXd aggiungendo (n-d) versori di Rd e poi la ri ortonorma
{
    int d = A.size();
    int n = A[0].size();
    //
    // debug ortonormalizzazione
    //
    // matrix_dump(A, "Verifica ridotta prima di completamento a dXd");
    //
    // fine debug
    //
    Mat Vb(d, Vec(d, 0.0));
    if (n>=d) return A;

    for (int j = 0; j < n; j++)
        for (int i = 0; i< d; i++) Vb[i][j] = A[i][j];
    for (int k = n; k < d; k++)
        Vb[k][k]=1.0; // andiamo in ordine con gli ultimi d-n versori della base canonica
    matrix_ortogonalizza_GSmod(Vb, n);
    return Vb;
}

void matrix_dump(
    const Mat& G, 
    const std::string &nome) 
{
    int NR = G.size();
    std::cout << "\n── " << nome << " " << NR << "(" <<  G.size() << ")" <<" ──\n";
    for (int i = 0; i < (int)NR; ++i) {
        for (int j = 0; j < (int)G[i].size(); ++j)
            std::cout << std::fixed
		      << std::setprecision(8)
              << color_dbl(G[i][j]) 
              << std::setw(12) 
              << std::setfill(' ')
		      << G[i][j] << " ";
		      // << G[i][j] << " ";
        color_rst();
        std::cout << "\n\n";
    }
}

Mat matrix_estende_ridotta(
    const Mat& A, 
    int n, 
    bool bycol)
{
    int cols = A[0].size();
    int rows = A.size();
    
    cout << "Entriamo con n = " << n << ( bycol ? "Colonne" : "Righe") << endl ;

    if (bycol) {
        if (cols > n) {
            Mat B = matrix_build_zero(rows, n);
            for (int i = 0; i < rows; i++) { 
                for (int j = 0; j < n; j++ ) {
                    B[i][j] = A[i][j];
                };
            };
            cout << "Rendo B\n";
            return B;
        } else {
            cout << "Rendo A\n";
            return A;
        }
    } else {
        if (rows>n) {
            Mat B = matrix_build_zero(n, cols);
            for (int i = 0; i < n; i++) {
                for (int j = 0; j <  cols; j++) {
                    B[i][j] = A[i][j];
                }
            }
            cout << "Rendo B\n";
            return B;
        } else {
            cout << "Rendo A\n";
            return A;
        };
    }

}

Mat matrix_householder_reflector(
    const Vec v)
{
    int n = v.size();
    assert(n>0);
    Vec w = vector_build_householder_bycol(v);
    Mat P = matrix_calcola_subfact(
                matrix_build_Id(n), 
                matrix_prodotto_matrix(
                    vector_to_matrix( w, false), 
                    vector_to_matrix( w, true)), 
                2);
    return P;
 }

Mat matrix_normalize_byrow(
    Mat& K)
{   
    // proviamo con un pass by ref e matrice locale

    int M = K.size(); // numero di righe da processare
    int N = K[0].size(); // prima riga buona per tutte
    Mat Kn(M, Vec(N, 0.0));

    for (int i=0; i<M; i++) {
        N = K[i].size(); // numero di elementi per riga
        double somma = 0;
        for (int j=0; j<N; j++) {
            somma+=K[i][j];
        }
        if (somma != 0) {
            for (int j=0; j<N; j++) {
                Kn[i][j] = K[i][j] / somma;
            }
        }
    }
    return Kn;
}

void matrix_ordina_diagonale(
    Vec& lambda,
    Mat& V,
    double zero_tol,
    SortOrder order)   // default: discendente
{
    //
    // sort dei valori singolari (o autovalori) con trascinamento colonne di V
    //

    const int n = static_cast<int>(lambda.size());
    // vettore indici 0..n-1
    std::vector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;

    auto key = [&](int i) {
        double a = lambda[i];
        if (std::abs(a) < zero_tol) a = 0.0;  // filtro nullità
        return a;
    };

    std::sort(idx.begin(), idx.end(),
              [&](int i, int j) {
                  double ai = key(i);
                  double aj = key(j);
                  return (order == SortOrder::Asc) ? (ai < aj) : (ai > aj);
              });

    // applica permutazione
    Vec lambda_sorted(n);
    Mat V_sorted = V;   // stessa dimensione

    for (int k = 0; k < n; ++k)
    {
        int old = idx[k];
        lambda_sorted[k] = lambda[old];
        for (int r = 0; r < static_cast<int>(V.size()); ++r)
            V_sorted[r][k] = V[r][old];
    }

    lambda.swap(lambda_sorted);
    V.swap(V_sorted);
}

void matrix_ortogonalizza_GSmod(
    Mat& Q, 
    int j0)
{
    if (Q.empty() || Q[0].empty()) return;
    const int m = static_cast<int>(Q[0].size()); // colonne
    const int n = static_cast<int>(Q.size());    // righe
    if (j0 < 0) j0 = 0;
    if (j0 >= m) return;

    for (int j = j0; j < m; ++j)
    {
        // v_j = colonna j
        // ortogonalizza rispetto a q_0, ..., q_{j-1}
        for (int k = 0; k < j; ++k) // ortogonalizza rispetto a TUTTE le precedenti
        {
            // prodotto scalare q_k^T v_j
            double dot = 0.0;
            for (int i = 0; i < n; ++i)
                dot += Q[i][k] * Q[i][j];

            // v_j -= dot * q_k
            for (int i = 0; i < n; ++i)
                Q[i][j] -= dot * Q[i][k];
        }

        // normalizza v_j -> q_j
        double norm2 = 0.0;
        for (int i = 0; i < n; ++i)
            norm2 += Q[i][j] * Q[i][j];

        double norm = std::sqrt(norm2);
        if (norm > 0.0)
        {
            double inv = 1.0 / norm;
            for (int i = 0; i < n; ++i)
                Q[i][j] *= inv;
        } else {
            // colonna quasi nulla: lasciala zero, se capita
        }
    }
}

Mat matrix_prodotto_AtA (
    const Mat& A, 
    bool A_right)  // default true. Come da nome fa At A (con A destra) se no fa A At
{   Mat B;
    if (A_right) {
        int n = A[0].size(); // A = mXn => At A = nXm mXn = nXn
        B = matrix_prodotto_matrix(matrix_trasposta(A), A);
    } else {
        int n = A.size(); // A = mXn => A At = mXn nXm = mXm
        Mat B(n, Vec(n, 0.0));
        B = matrix_prodotto_matrix(A, matrix_trasposta(A));
    };
    return B;
}

Mat matrix_prodotto_coeff(
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

Mat matrix_prodotto_matrix(
    const Mat& A, 
    const Mat& B) 
{
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    //
    // nella versione di foglio 3 usa assert  per il controllo di compatibilita'
    //     assert(colsA == rowsB);
    // questa era ancora tipo pedestre, poi proveremo con assert visto che la importiamo
    // 

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

Vec matrix_prodotto_vector(
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

void matrix_test_ortogonale(
    const Mat& A,
    string s)
{
    matrix_dump(matrix_prodotto_matrix(matrix_trasposta(A), A), "Test di ortogonalita' di "+s);
}

Mat matrix_trasposta(
    const Mat& A) 
{
    int rows = A.size();
    int cols = A[0].size();
    Mat At(cols, Vec(rows, 0.0));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            At[j][i] = A[i][j];
        }
    }
    return At;
};

//
// funzioni di manipolazione vettori
//

Vec vector_add_noise(
    Vec& v, 
    double e)
{   
    // v e' il vettore da perturbare, e l'aliquota in millesimi rispetto alla norma_inf del vettore 
    int N = v.size();
    Vec r(N,0.0);
    double fscala = vector_calcola_norma(2, v);
    double ampiezza = (e*fscala)/1000;
    for (int i = 0; i<N; i++) { 
        r[i]  = v[i] + ampiezza * ((double)rand() / RAND_MAX - 0.5);
    }
    return r;
}

Vec vector_build_householder_bycol(
    const Vec& v) 
{
    // =============================================================================
//  TODO 1 – Riflessione di Householder su una colonna
//
//  Dato v ∈ R^m, restituisce w normalizzato tale che
//     (I - 2ww^T) v = -sgn(v[0]) * ||v||_2 * e_1
//
//  Costruzione:
//     u = v;
//     u[0] += sgn(v[0]) * ||v||_2;    // sgn(0) = +1 per convenzione
//     w = u / ||u||_2;
//
//  ATTENZIONE: se ||v||_2 == 0, restituisci w = e_1 (nessuna trasformazione).
//
//  Verifica nella funzione main: per v = {3, 1, -2},
//  w risultante deve dare H*v = [-sqrt(14), 0, 0].
// =============================================================================

    int m = v.size();
    Vec w(m, 0.0);
    double n2 = vector_calcola_norma(2,v);
    if (n2 == 0) 
    {
        w[0]=1;
        return w; // restituisce e1
    } else {
        int sgn_v0 = 1;
        for (int i=0;i<m;i++) w[i]=v[i];
        if (w[0]<0) sgn_v0 = -1;
        w[0] += sgn_v0 * n2;
        n2 = vector_calcola_norma(2, w);   // ricicliamo, non ci serviva piu' norma di v
        for (int i=0;i<m;i++) w[i] /= n2;
        return w;
    }
}

Vec vector_build_householder_byrow(
    const Vec& v) 
{
// =============================================================================
//  TODO 2 – Riflessione di Householder su una riga
//
//  Matematicamente identico a vector_build_householder_bycol.
//  La distinzione è solo nell'uso: verrà applicato a destra (su righe).
//  Puoi semplicemente delegare a vector_build_householder_bycol.
// =============================================================================

    return vector_build_householder_bycol(v);
}

Vec vector_build_one_minus_one(
    const int n) 
{
    // crea un vettore di valori 1 e -1 alternati, di dimensione n
    int m = n;
    if (n < 1) m = 1; else m = n;
    Vec v(m, 0.0);
    // la facciamo col flip del segno per risparmiare 
    v[0] = 1;
    for (int i = 1; i < m; i++) {
        v[i] = -v[i-1];
    }
    return v;
}

Vec vector_build_segnale_finestra(
    int N,
    double a, 
    double b, 
    double t)
    {
        // a e b in % su N, sono frazioni in [0,1]
        Vec ws = Vec(N,0.0);
        double temp;
        for (int i=0; i<N; i++) {
            temp = (double) i / (double) N;
            if ((temp >=a) && (temp <= b)) ws[i]=t;
        }
        return ws;
    }

Vec vector_build_versore_canonico(
    int j, 
    int N)
{
    Vec e(N, 0.0);
    e[j] = 1.0;
    return e;
}

double vector_calcola_norma(
    int norma, 
    const Vec& V)
{
    int N = V.size(); 
    switch (norma) {
        case 2: // norma euclidea
        {
          double somma = 0;
            for (int i = 0; i<N;  i++) {
                double el = V[i];
                somma += el*el;     // possiamo fidarci che sia >= 0 ?
            }
            return std::sqrt(somma);
        }
        case 0: // norma infinito (max modulo degli el)
        {
            double max_el =0;
            for (int i = 0; i<N;  i++) {
                double el = std::abs(V[i]);
                if (el > max_el) max_el = el;
            }
            return max_el;
        }
        default:
            return 0;
    }
}

Vec vector_campiona_f(
    const Vec &x, // vettore dei nodi di campionamento
    double (*ft)(double))   // funzione da  campionare
{
        Vec phi(x.size());
        int totnum = (int) x.size();
        for (int i=0; i<totnum; i++) phi[i] = fcallb(x[i], ft);
        return phi;
}
Vec vector_campiona_f_k(
    int k, 
    const Vec &x,           // vettore dei nodi di campionamento
    double (*ft)(double))   // funzione da  campionare 
{
        // =============================================================================
        //  Campionamento di una funzione base generica
        //  Restituisce il vettore dei valori di phi_k nei nodi x:
        //      phi_k[n] = function (k * x[n])
        //  La funzione e' passata come puntatore a funzione ft, che accetta un double 
        //  e restituisce un double.

        Vec phi(x.size());
        int totnum = (int) x.size();
        for (int i=0; i<totnum; i++) phi[i] = fcallb(k*x[i], ft);
        
        return phi;
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

Vec vector_householder_reflected(
    const Vec v) 
{
    int n = v.size();
    if (n<5) cout << endl;
    if (n<5) vector_dump(v, 10, n, "Vettore da riflettere");
    Vec w = vector_build_householder_bycol(v);
    if (n<5) vector_dump(w, 10, n, "Vettore hh a n = " + itostr(n));
    double wt = vector_prodotto_scalare(w, v);
    if (n<5) cout << "coefficiente della proiezione di v su w = " << wt << endl << endl;
    Vec wwt = vector_prodotto_coeff(w, -2 * wt);
    if (n<5) vector_dump(wwt, 10, n, "Vettore wwt");
    Vec x = vector_somma(v, wwt);
    if (n<5) vector_dump(x, 10, n, "Vettore x riflesso");

    return x;
}


Vec vector_prodotto_coeff(
    const Vec v, 
    double mu)
{
    Vec w = v;
    int n = v.size();
    for (int i = 0; i<n; i++) w[i] *= mu;
    return w;
}

double vector_prodotto_scalare(
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

Vec vector_reverse(
    const Vec& v) 
{
    Vec r = v;
    std::reverse(r.begin(), r.end());
    return r;
}

Vec vector_shift(
    const Vec &v, 
    const double shift) {
        //
        // aggiunge un valore shift a tutto il vettore, un rappezzo per testare problemi di steccato, valutare se tenere 
        //
    Vec v_shifted = v;
    for (int i = 0; i < (int)v.size(); ++i) {
        v_shifted[i] += shift;
    }
    return v_shifted;
}

Vec vector_somma(
    const Vec u, 
    const Vec v)
{
    int nu = u.size();
    int nv = v.size();
    assert(nu == nv);
    Vec s(nu, 0.0);
    for (int i = 0; i<nu; i++) s[i] = u[i] + v[i];
    return s;
}
    
Mat vector_to_matrix(
    const Vec& v,
    bool transp)
{
    int n = v.size();
    if (transp) {   // vettore va in riga di M 1xn
        Mat M = matrix_build_zero(1, n);
        for (int i=0;i<n;i++) {
            M[0][i] = v[i];
        }
        return M;
    } else {        // vettore va in colonna di M nx1
        Mat M = matrix_build_zero(n,1);
        for (int i=0;i<n;i++) M[i][0] = v[i];
        return M;
    }
}

Mat vector_to_matrix_diag(
    const Vec& s, 
    int m, 
    int n) 
{
    //
    // crea matrice diagonale con i primi r = min(m,n,s.size()) elementi del veettore s 
    //

    Mat S = matrix_build_zero(m, n);
    int r = std::min<int>(s.size(), std::min(m, n));
    for (int i = 0; i < r; ++i)
        S[i][i] = s[i];
    return S;
}


//
// funzioni specifiche di trasformazione matrici 
//

// =============================================================================
//  TODO 3 – Bidiagonalizzazione di Golub–Kahan
//
//  Input:  X  ∈ R^{n×d}
//  Output: U0 ∈ R^{n×n} ortogonale,
//          B  ∈ R^{n×d} bidiagonale superiore,
//          V0 ∈ R^{d×d} ortogonale
//          tali che  X = U0 * B * V0^T
//
//  Segui l'Algoritmo 1 nel foglio.
//  Ricorda la formula efficiente:  H*A = A - 2*w*(w^T*A)
//
//  Aggiornamento di U0: U0 <- U0 * H_k  (H_k si applica a destra)
//    U0[:, k:] <- U0[:, k:] - 2 * (U0[:, k:] * w) * w^T
//
//  Aggiornamento di V0: V0 <- V0 * G_k  (G_k si applica a destra)
//    V0[:, k+1:] <- V0[:, k+1:] - 2 * (V0[:, k+1:] * w) * w^T
// =============================================================================

// 
// wrapper generale
//

void trmatrix_bidiagonalizza(
    const Mat& A, 
    Mat& U0, 
    Mat& B, 
    Mat& V0, 
    bool sup_diag,
    bool dump_flag) 
{
    // A = U0 * B * V0^T

    int m = A.size();
    int n = A[0].size();

    if (m < n) {
        if (sup_diag) 
            trmatrix_bidiag_wide_to_upper(A, U0, B, V0,  dump_flag);
        else
            trmatrix_bidiag_wide_to_lower(A, U0, B, V0,  dump_flag);
        cout << "Bidiagonalizzazione diretta" << endl;
    } else {
        Mat At = matrix_trasposta(A);

        Mat Ut, Bt, Vt;
        if (sup_diag)
            trmatrix_bidiag_wide_to_upper(At, Ut, Bt, Vt,  dump_flag);   // At = Ut * Bt * Vt^T
        else
            trmatrix_bidiag_wide_to_lower(At, Ut, Bt, Vt, dump_flag);   // At = Ut * Bt * Vt^T
        cout << "Bidiagonalizzazione inversa" << endl;
        U0 = Vt;           // A = Vt * Bt^T * Ut^T
        B  = matrix_trasposta(Bt);
        V0 = Ut;
    }
}

//
// versione per matrice wide (n <= d) con bidiagonale superiore
//

void trmatrix_bidiag_wide_to_upper(
    const Mat& X, 
    Mat& U0, 
    Mat& B, 
    Mat& V0,
    bool dump_flag) 
{

    Mat A = X;                 // copia di lavoro

    int n = A.size();
    int d = A[0].size();
    
    if (dump_flag) matrix_dump(A, "Originale"); 

    int p = std::min(n, d);

    U0 = matrix_build_Id(n);          // n x n
    V0 = matrix_build_Id(d);          // d x d

    Vec v, w;

    for (int k = 0; k < p; k++) {
        //
        // occhio a queste due che vengono reimpostate ad ogni loop
        // modalita' originale gkb e jacobi
        // a sinistra m = n-k righe, q = d-k colonne
        // a destra   m = n-k righe, q = q-k-1 colonne
        //
        // nuova modalita' con alglib
        // a sinistra m = n-k-1 righe, q = d-k colonne
        // a destra   m = n-k righe, q = q-k colonne
        //
        if (k < n-1) {
            int m = n - k;
            int q = d - k;

            v.assign(m, 0.0);

            // ── Householder colonna: azzera A[k+1:, k] ──────────────────────────
            // 1. Estrai la sotto-colonna: v = A[k:, k]  (lunghezza n-k)
            for (int i = 0; i<m; i++) {
                v[i] = A[k+i][k];
            }
            if (dump_flag) vector_dump(v, 10, v.size(), "Colonna di A sotto diag con k="+std::to_string(k));

            // 2. Calcola w = vector_build_householder_bycol(vv)
            w = vector_build_householder_bycol(v);
             if (dump_flag) vector_dump(w, 10, w.size(), "W di householder colonna sinistra");

            // 3. Aggiorna A[k:, :] <- A[k:, :] - 2*w*(w^T * A[k:, :])
            Mat A_k = matrix_build_zero(m, q);

            for (int i = 0; i < m; i++){
                for (int j = 0; j < q; j++) {
                    A_k[i][j] = A[k+i][k+j];
                }
            }
            if (dump_flag) matrix_dump(A_k, "A_k a giro "+std::to_string(k));
            Mat Wt = vector_to_matrix(w,true);
            if (dump_flag)matrix_dump(Wt, "Wt come matrice 1 x n");
            Mat Wt_Ak = matrix_prodotto_matrix(Wt, A_k); // w va direttamente trasposto con true
            if (dump_flag) matrix_dump(Wt_Ak, "Wt A_k" + std::to_string(k));
            Mat W_Wt_Ak = matrix_prodotto_matrix(vector_to_matrix(w, false), Wt_Ak);
            if (dump_flag) matrix_dump(W_Wt_Ak, "W Wt A_k" + std::to_string(k));
            Mat Two_W_Wt_Ak = matrix_prodotto_coeff(W_Wt_Ak, 2.0);
            if (dump_flag)matrix_dump(Two_W_Wt_Ak, "2 W Wt A_k" + std::to_string(k));

            for (int i=0; i<m; i++){
                for (int j = 0; j < q; j++) {
                    A[k+i][k+j] -= Two_W_Wt_Ak[i][j];
                }
            }
            if (dump_flag) matrix_dump(A, "A al passo sx " + std::to_string(k));

            // 4. Aggiorna U0[:, k:] <- U0[:, k:] - 2*(U0[:,k:]*w)*w^T

            Mat U0_k = matrix_build_zero(n,m); //tutte le righe e tante colonne quanto e' lungo w                                                                           

            for (int i = 0; i < n; i++){
                for (int j = 0; j < m; j++) {
                    U0_k[i][j] = U0[i][k+j];
                }
            }
            Mat U0_W =  vector_to_matrix(matrix_prodotto_vector(U0_k, w), false);
            Mat U0_W_Wt = matrix_prodotto_matrix(U0_W, vector_to_matrix(w, true));
            if (dump_flag)matrix_dump(U0_W_Wt, "U0_k W W_t " + std::to_string(k));
            Mat Two_U0_W_Wt = matrix_prodotto_coeff(U0_W_Wt, 2.0);

            for (int i = 0; i < n; i++){
                for (int j = 0; j < m; j++) {
                    U0[i][k+j] -= Two_U0_W_Wt[i][j];
                }
            }
            if (dump_flag) matrix_dump(U0, "U0 al passo " + std::to_string(k));
        }

        if (k < d - 2) {
            // TODO
            // ── Householder riga: azzera A[k, k+2:] ─────────────────────────────
            // Solo se k < d-2
            // 1. Estrai la sotto-riga: v = A[k, k+1:]  (lunghezza d-k-1)
            int m = n-k;
            int q = d - k - 1;
            v.assign(q, 0.0);

            for (int j = 0; j < q; j++) {
                v[j] = A[k][k+j+1];
            }
            if (dump_flag) vector_dump(v, 10, v.size(), "Riga di A a destra con k="+std::to_string(k));
            // 2. Calcola w = vector_build_householder_byrow(v)
            w = vector_build_householder_byrow(v);
            if (dump_flag) vector_dump(w, 10, w.size(), "W di householder riga destra");
            // 3. Aggiorna A[k:, k+1:] <- A[k:, k+1:] - 2*(A[k:,k+1:]*w)*w^T
            Mat A_k = matrix_build_zero(m, q);
            for (int i = 0; i < m; i++){
                for (int j = 0; j < q; j++) {
                    A_k[i][j] = A[k+i][k+j+1];
                }
            }
            Mat Ak_W =  vector_to_matrix(matrix_prodotto_vector(A_k, w), false);
            Mat Ak_W_Wt = matrix_prodotto_matrix(Ak_W, vector_to_matrix(w, true));
            if (dump_flag) matrix_dump(Ak_W_Wt, "A_k W W_t " + std::to_string(k));
            Mat Two_Ak_W_Wt = matrix_prodotto_coeff(Ak_W_Wt, 2.0);
            for (int i = 0; i < m; i++){
                for (int j = 0; j < q; j++) {
                    A[k+i][k+j+1] -=  Two_Ak_W_Wt[i][j];
                }
            }
            if (dump_flag) matrix_dump(A, "A al passo dx " + std::to_string(k));

            // 4. Aggiorna V0[:, k+1:] <- V0[:, k+1:] - 2*(V0[:,k+1:]*w)*w^T

            Mat V0_k = matrix_build_zero(d,q); //tutte le righe e tante colonne quanto e' lungo w                                                                           

            for (int i = 0; i < d; i++){
                for (int j = 0; j < q; j++) {
                    V0_k[i][j] = V0[i][k+j+1];
                }
            }
            Mat V0_W =  vector_to_matrix(matrix_prodotto_vector(V0_k, w), false);
            Mat V0_W_Wt = matrix_prodotto_matrix(V0_W, vector_to_matrix(w, true));
            if (dump_flag)  matrix_dump(V0_W_Wt, "V0_k W W_t " + std::to_string(k));
            Mat Two_V0_W_Wt = matrix_prodotto_coeff(V0_W_Wt, 2.0);

            for (int i = 0; i < d; i++){
                for (int j = 0; j < q; j++) V0[i][k+j+1] -= Two_V0_W_Wt[i][j];
            }
            if (dump_flag)  matrix_dump(V0, "V0 al passo " + std::to_string(k));
            //
            //  Verifica di U0 e V0 ortogonali
            //
            Mat V0t_V0 = matrix_prodotto_matrix(matrix_trasposta(V0), V0);
            Mat U0t_U0 = matrix_prodotto_matrix(matrix_trasposta(U0), U0);
            // matrix_dump(U0t_U0, "U0_t U0");
            // matrix_dump(V0t_V0, "V0_t V0");

            
            
        }
    }
    B = A;
}

//
// versione per matrice wide (n <= d) con bidiagonale inferiore
//

void trmatrix_bidiag_wide_to_lower(
    const Mat& X, 
    Mat& U0, 
    Mat& B, 
    Mat& V0,
    bool dump_flag) 
{

    Mat A = X;                 // copia di lavoro

    int n = A.size();
    int d = A[0].size();
    
    if (dump_flag) matrix_dump(A, "Originale"); 

    int p = std::min(n, d);

    U0 = matrix_build_Id(n);          // n x n
    V0 = matrix_build_Id(d);          // d x d

    Vec v, w;


    for (int k = 0; k < p; k++) {
        //
        // occhio a queste due che vengono reimpostate ad ogni loop
        // modalita' originale gkb e jacobi
        // a sinistra m = n-k righe, q = d-k colonne
        // a destra   m = n-k righe, q = q-k-1 colonne
        //
        // nuova modalita' con alglib
        // a sinistra m = n-k-1 righe, q = d-k colonne
        // a destra   m = n-k righe, q = q-k colonne
        //
        if (k < d - 1) {
            // TODO
            // ── Householder riga: azzera A[k, k+2:] ─────────────────────────────
            // Solo se k < d-1
            // 1. Estrai la sotto-riga: v = A[k, k+1:]  (lunghezza d-k-1)
            // int m = n - k;
            // int q = d - k;

            int m = n - k;      // righe k..n-1
            int q =  d - k;      // colonne k..d-1
            v.assign(q, 0.0);
            for (int j = 0; j < q; j++) {
                v[j] = A[k][k+j];
            }
            // 2. Calcola w = vector_build_householder_byrow(v)
            w = vector_build_householder_byrow(v);
            //vector_dump(w, 10, w.size(), "W di householder riga destra");
            // 3. Aggiorna A[k:, k+1:] <- A[k:, k+1:] - 2*(A[k:,k+1:]*w)*w^T
            // blocco A_k: righe k..n-1, colonne k..d-1
            Mat A_k = matrix_build_zero(m, q);

            for (int i = 0; i < m; i++){
                for (int j = 0; j < q; j++) {
                    A_k[i][j] = A[k+i][k+j];
                }
            }
            Mat Ak_W =  vector_to_matrix(matrix_prodotto_vector(A_k, w), false);
            Mat Ak_W_Wt = matrix_prodotto_matrix(Ak_W, vector_to_matrix(w, true));
            //matrix_dump(Ak_W_Wt, "A_k W W_t " + std::to_string(k));           
            Mat Two_Ak_W_Wt = matrix_prodotto_coeff(Ak_W_Wt, 2.0);

            for (int i = 0; i < m; i++){
                for (int j = 0; j < q; j++) {
                    A[k+i][k+j] -=  Two_Ak_W_Wt[i][j];
                }
            }
            if (dump_flag) matrix_dump(A, "A al passo dx " + std::to_string(k));

            // 4. Aggiorna V0[:, k+1:] <- V0[:, k+1:] - 2*(V0[:,k+1:]*w)*w^T
            Mat V0_k = matrix_build_zero(d,q); //tutte le righe e tante colonne quanto e' lungo w                                                                           

            for (int i = 0; i < d; i++){
                for (int j = 0; j < q; j++) V0_k[i][j] = V0[i][k+j];
            }
            Mat V0_W =  vector_to_matrix(matrix_prodotto_vector(V0_k, w), false);
            Mat V0_W_Wt = matrix_prodotto_matrix(V0_W, vector_to_matrix(w, true));

            //matrix_dump(V0_W_Wt, "V0_k W W_t " + std::to_string(k));
            
            Mat Two_V0_W_Wt = matrix_prodotto_coeff(V0_W_Wt, 2.0);
            for (int i = 0; i < d; i++){
                for (int j = 0; j < q; j++) 
                    V0[i][k+j] -= Two_V0_W_Wt[i][j];
            }
            if (dump_flag) matrix_dump(V0, "V0 al passo " + std::to_string(k));
            //
            //  Verifica di U0 e V0 ortogonali
            //

            //cout << "Trasposta V0" << endl;
            Mat V0t_V0 = matrix_prodotto_matrix(matrix_trasposta(V0), V0);
            //cout << "Trasposta U0" << endl;
            Mat U0t_U0 = matrix_prodotto_matrix(matrix_trasposta(U0), U0);
            if (dump_flag) cout << "Fatto" << endl;
            //matrix_dump(U0t_U0, "U0_t U0");
            //matrix_dump(V0t_V0, "V0_t V0");

            
            
        }

        if (k < n - 1) {
            int m = n - (k + 1);    // righe k+1..n-1
            int q = d - k;    // colonne k+1..d-1
            v.assign(m, 0.0);
            // ── Householder colonna: azzera A[k+1:, k] ──────────────────────────
            // 1. Estrai la sotto-colonna: v = A[k:, k]  (lunghezza n-k)
            for (int i = 0; i<m; i++) {
                v[i] = A[k+i+1][k];
            }
            if (dump_flag) vector_dump(v, 10, v.size(), "Colonna di A sotto diag con k="+std::to_string(k));
            // 2. Calcola w = vector_build_householder_bycol(vv)
            w = vector_build_householder_bycol(v);
            //vector_dump(w, 10, w.size(), "W di householder colonna sinsitra");
            // 3. Aggiorna A[k:, :] <- A[k:, :] - 2*w*(w^T * A[k:, :])
            Mat A_k = matrix_build_zero(m, q);
            for (int i = 0; i < m; i++){
                for (int j = 0; j < q; j++) {
                    A_k[i][j] = A[k+i+1][k+j];
                }
            }
            // matrix_dump(A_k, "A_k a giro "+std::to_string(k));
            Mat Wt = vector_to_matrix(w,true);
            // matrix_dump(Wt, "Wt come matrice 1 x n");
            Mat Wt_Ak = matrix_prodotto_matrix(Wt, A_k); // w va direttamente trasposto con true
            if (dump_flag) matrix_dump(Wt_Ak, "Wt A_k" + std::to_string(k));
            Mat W_Wt_Ak = matrix_prodotto_matrix(vector_to_matrix(w, false), Wt_Ak);
            if (dump_flag) matrix_dump(W_Wt_Ak, "W Wt A_k" + std::to_string(k));
            Mat Two_W_Wt_Ak = matrix_prodotto_coeff(W_Wt_Ak, 2.0);
            if (dump_flag)  matrix_dump(Two_W_Wt_Ak, "2 W Wt A_k" + std::to_string(k));
            for (int i=0; i<m; i++){
                for (int j = 0; j < q; j++) {
                    A[k+i+1][k+j] -= Two_W_Wt_Ak[i][j];
                }
            }
            if (dump_flag) matrix_dump(A, "A al passo sx " + std::to_string(k));

            // 4. Aggiorna U0[:, k:] <- U0[:, k:] - 2*(U0[:,k:]*w)*w^T

            Mat U0_k = matrix_build_zero(n,m); //tutte le righe e tante colonne quanto e' lungo w                                                                           
            for (int i = 0; i < n; i++){
                for (int j = 0; j < m; j++) {
                    U0_k[i][j] = U0[i][k+j+1];
                }
            }
            Mat U0_W =  vector_to_matrix(matrix_prodotto_vector(U0_k, w), false);
            Mat U0_W_Wt = matrix_prodotto_matrix(U0_W, vector_to_matrix(w, true));
            // matrix_dump(U0_W_Wt, "U0_k W W_t " + std::to_string(k));
            Mat Two_U0_W_Wt = matrix_prodotto_coeff(U0_W_Wt, 2.0);
            for (int i = 0; i < n; i++){
                for (int j = 0; j < m; j++) {
                    U0[i][k+j+1] -= Two_U0_W_Wt[i][j];
                }
            }
            // matrix_dump(U0, "U0 al passo " + std::to_string(k));
        }

    }
    B = A;
}


void trmatrix_SVDQR(
    const Mat& B,
    Mat& Ub,
    Mat& Vb,
    Vec& sigma)
{
    //
    // wrapper esterno per riordinare i parametri formali come quelli della svd_bidiagonale fornita
    // e per far completare correttamente Vbred -> Vb (non tocca Ub perche' lavoriamo sempre underdet)
    //

    Mat Vbred;
    double ev_tol = 1e-12;

    int m = B.size();
    int n = B[0].size();

    if (m <= n) {
        trmatrix_SVDQR_ridotta(B, Ub, sigma, Vbred, ev_tol);
        Vb = matrix_completa_ridotta(Vbred);
    } else {
        Mat Bt = matrix_trasposta(B);
        Mat U_bt, V_bt, V_btred;

        trmatrix_SVDQR_ridotta(Bt, U_bt, sigma, V_btred, ev_tol);
        V_bt = matrix_completa_ridotta(V_btred);

        Ub = V_bt;           
        Vb = U_bt;
    }
    //
    // lascio sigma() come vettore per compatibilita' con la svd fornita
    // bisogna ricordare sempre che viene completata all'esterno e 
    // le dimensioni devono accordarsi sempre con Ub[0].size() e Vb[0].size 
    // perche' le colonne di Vb cono le righe di Vb_t
    // questo dovrebbe essere corretto sia per la versione ridotta che per la integrale
    //

};

/*
    SVD ridotta di una matrice bidiagonale superiore B (p x d, p <= d).

    IDEA MATEMATICA
    ----------------
    1) Calcolo di BtB = B^T B (d x d), simmetrica, semidefinita positiva.
       Gli autovalori di BtB sono i quadrati dei valori singolari di B:
           BtB v_i = lambda_i v_i   =>   sigma_i = sqrt(lambda_i)  (lambda_i >= 0).

    2) Problema agli autovalori simmetrico:
           linear_jacobi_autoval_simmetrica(BtB, lambda, V)
       dove:
           - lambda[i]  = autovalori (non necessariamente ordinati);
           - V[:,i]     = autovettori ortonormali corrispondenti.

    3) Ordinamento autovalori/autovettori:
       - applichiamo una soglia ev_tol:
           se |lambda_i| < ev_tol => lambda_i := 0 (schiacciamo i quasi-null).
       - ordiniamo lambda in ordine decrescente, trascinando le colonne di V
         con la stessa permutazione:
           matrix_ordina_diagonale(lambda, V, ev_tol, SortOrder::Desc).

    4) Costruzione dei valori singolari e di V_ridotta:
       - per k = 0..p-1:
             sigma[k] = sqrt(max(lambda[k], 0));
             Vbred[:,k] = V[:,k];
       => Vbred (d x p) contiene i vettori singolari destri ridotti.

    5) Costruzione di Ub (vettori singolari sinistri ridotti):
       - per ogni k:
             v_k = Vbred[:,k];
             w_k = B * v_k;
             se sigma[k] > 0:
                 u_k = (1/sigma[k]) * w_k;
             altrimenti:
                 u_k = 0 (colonna nel kernel).
       - le colonne u_k formano Ub (p x p).

       Questa è la relazione classica:
           u_i = (1 / sigma_i) * B v_i,
       valida per tutti i sigma_i > 0.

    6) (Opzionale) Rifinitura dell'ortonormalità:
       - matrix_ortogonalizza_GSmod(Ub);
       in pratica Ub^T Ub ≈ I_p e Vbred^T Vbred ≈ I_p.

    7) Identità finale (SVD ridotta di B):
           B = Ub * Sigma * Vbred^T,
       con Sigma = diag(sigma[0..p-1]) (p x p).

    USO NELLA PIPELINE COMPLETA:
    ----------------------------
       X = U0 * B * V0^T
       B = Ub * Sigma * Vbred^T

       => X = (U0 * Ub) * Sigma * (V0 * Vbred)^T

       dove:
         U  = U0 * Ub   (vettori singolari sinistri di X, forma ridotta),
         Vr = V0 * Vbred (vettori singolari destri di X, forma ridotta).
*/

void trmatrix_SVDQR_ridotta(
    const Mat& B,
    Mat& Ub,
    Vec& sigma,
    Mat& Vb_red,
    double ev_tol)
{
    //
    // continuo a chiamnarla bidiagonale ma in realta' lavora su matrici qualsiasi
    // e' solo per sottolineare che sostituisce la svd_bidiagonale fornita 
    //

    const int p = static_cast<int>(B.size());          // righe
    const int d = static_cast<int>(B[0].size());       // colonne

    // 1) Costruisci BtB = B^T B (d x d)
    Mat Bt = matrix_trasposta(B);                             // d x p
    Mat BtB = matrix_prodotto_matrix(Bt, B);                 // d x d

    // 2) Autovalori/autovettori di BtB (d x d)
    Vec lambda;
    Mat V;                                             // d x d
    linear_jacobi_autoval_simmetrica(BtB, lambda, V);

    // 3) Ordina autovalori (desc) + colonne di V, con filtro nullità
    matrix_ordina_diagonale(lambda, V, ev_tol, SortOrder::Desc);

    // 4) Prepara sigma e Vb_red: tieni solo i primi p autovalori > 0
    sigma.assign(p, 0.0);
    Vb_red.assign(d, Vec(p, 0.0));                     // d x p

    for (int k = 0; k < p; ++k)
    {
        double lam = lambda[k];
        if (lam < 0.0 && std::abs(lam) < ev_tol)
            lam = 0.0;                                 // schiaccia negatività numeriche

        sigma[k] = (lam > 0.0) ? std::sqrt(lam) : 0.0;

        for (int i = 0; i < d; ++i)
            Vb_red[i][k] = V[i][k];                    // colonna k
    }

    // 5) Calcola Ub: per ogni i, u_i = (1/sigma_i) * B * v_i
    Ub.assign(p, Vec(p, 0.0));                         // p x p

    for (int k = 0; k < p; ++k)
    {
        // prendi v_k (colonna k di Vb_red, dimensione d)
        Vec vk(d);
        for (int j = 0; j < d; ++j)
            vk[j] = Vb_red[j][k];

        // w = B * v_k  (p)
        Vec wk = matrix_prodotto_vector(B, vk);              // p

        if (sigma[k] > 0.0)
        {
            double inv_sigma = 1.0 / sigma[k];
            for (int i = 0; i < p; ++i)
                Ub[i][k] = wk[i] * inv_sigma;
        }
        else
        {
            // sigma[k] ~ 0: vettore nel kernel, puoi lasciare colonna a zero
            for (int i = 0; i < p; ++i)
                Ub[i][k] = 0.0;
        }
    }

    //
    // a quanto pare U esce gia' ortonormale ....  senza esplicita MGS
    // non so se tenerla fuori per questioni di economia o se 
    // passarla a priori per pettinare errori di arrotondamento.
    //
    // invece serve nel completamento di V da ridotta ad integrale della svd_bidiagonmale_compat
    // alcuni versori di base canonica potrebbero non essere indipendenti
    // dalle colonne di Vb ridotta (se hanno piu' componenti che elementi della base)
    // 
}

void trmatrix_tridiag_wide(
    const Mat& X,
    Mat& U0,
    Mat& B,
    Mat& V0,
    bool dump_flag)
{
    Mat A = X;

    int n = A.size();
    int d = A[0].size();
    int p = std::min(n, d);

    U0 = matrix_build_Id(n);
    V0 = matrix_build_Id(d);

    Vec v, w;

    for (int k = 0; k < p; ++k) {

        // =========================================================
        // STEP SX: colonna k -> preserva diag, crea subdiag, zeri sotto
        // blocco naturale: A[k+1:, k:]
        // =========================================================
        if (k < n - 2) {
            int m = n - (k + 1);   // numero righe attive
            int q = d - k;         // numero colonne attive

            v.assign(m, 0.0);

            // v = A[k+1:, k]
            for (int i = 0; i < m; ++i)
                v[i] = A[k + 1 + i][k];

            w = vector_build_householder_bycol(v);

            Mat A_k = matrix_build_zero(m, q);
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < q; ++j)
                    A_k[i][j] = A[k + 1 + i][k + j];

            Mat Wt = vector_to_matrix(w, true);
            Mat W  = vector_to_matrix(w, false);
            Mat Wt_Ak = matrix_prodotto_matrix(Wt, A_k);
            Mat corr  = matrix_prodotto_coeff(matrix_prodotto_matrix(W, Wt_Ak), 2.0);

            for (int i = 0; i < m; ++i)
                for (int j = 0; j < q; ++j)
                    A[k + 1 + i][k + j] -= corr[i][j];

            // U0[:, k+1:] = U0[:, k+1:] * (I - 2ww^T)
            Mat U0_k = matrix_build_zero(n, m);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < m; ++j)
                    U0_k[i][j] = U0[i][k + 1 + j];

            Mat U0_W    = vector_to_matrix(matrix_prodotto_vector(U0_k, w), false);
            Mat U0_corr = matrix_prodotto_coeff(
                              matrix_prodotto_matrix(U0_W, vector_to_matrix(w, true)),
                              2.0);

            for (int i = 0; i < n; ++i)
                for (int j = 0; j < m; ++j)
                    U0[i][k + 1 + j] -= U0_corr[i][j];
        }

        // =========================================================
        // STEP DX: riga k -> azzera tutto a destra della diag
        // blocco naturale: A[k:, k+1:]
        // =========================================================
        if (k < d - 2) {
            int m = n - k;         // righe attive
            int q = d - (k + 1);   // colonne attive a destra

            v.assign(q, 0.0);

            // v = A[k, k+1:]
            for (int j = 0; j < q; ++j)
                v[j] = A[k][k + 1 + j];

            w = vector_build_householder_byrow(v);

            Mat A_k = matrix_build_zero(m, q);
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < q; ++j)
                    A_k[i][j] = A[k + i][k + 1 + j];

            Mat Ak_W    = vector_to_matrix(matrix_prodotto_vector(A_k, w), false);
            Mat Ak_corr = matrix_prodotto_coeff(
                              matrix_prodotto_matrix(Ak_W, vector_to_matrix(w, true)),
                              2.0);

            for (int i = 0; i < m; ++i)
                for (int j = 0; j < q; ++j)
                    A[k + i][k + 1 + j] -= Ak_corr[i][j];

            // V0[:, k+1:] = V0[:, k+1:] * (I - 2ww^T)
            Mat V0_k = matrix_build_zero(d, q);
            for (int i = 0; i < d; ++i)
                for (int j = 0; j < q; ++j)
                    V0_k[i][j] = V0[i][k + 1 + j];

            Mat V0_W    = vector_to_matrix(matrix_prodotto_vector(V0_k, w), false);
            Mat V0_corr = matrix_prodotto_coeff(
                              matrix_prodotto_matrix(V0_W, vector_to_matrix(w, true)),
                              2.0);

            for (int i = 0; i < d; ++i)
                for (int j = 0; j < q; ++j)
                    V0[i][k + 1 + j] -= V0_corr[i][j];
        }

        if (dump_flag) {
            matrix_dump(A, "A al passo " + std::to_string(k));
            matrix_dump(matrix_prodotto_matrix(matrix_trasposta(U0), U0), "U0_t U0");
            matrix_dump(matrix_prodotto_matrix(matrix_trasposta(V0), V0), "V0_t V0");
        }
    }

    B = A;
}

void trmatrix_test_sv_autoval(
    Vec lambda,
    const Vec& sigma)
{
    const int n = (int)sigma.size();

    // ordina gli autovalori in senso decrescente
    std::sort(lambda.begin(), lambda.end(), std::greater<double>());

    std::cout << std::setprecision(10) << std::scientific;

    double max_abs_diff_sq  = 0.0;
    double max_abs_diff_val = 0.0;

    for (int i = 0; i < n; ++i) {
        double lam  = lambda[i];
        double sv   = sigma[i];
        double sv2  = sv * sv;
        double root = std::sqrt(lam);

        double diff_sq  = sv2 - lam;
        double diff_val = sv - root;

        max_abs_diff_sq  = std::max(max_abs_diff_sq,  std::abs(diff_sq));
        max_abs_diff_val = std::max(max_abs_diff_val, std::abs(diff_val));

        std::cout << "i = " << i
                  << "  lambda = " << lam
                  << "  sigma = "  << sv
                  << "  sigma^2-lambda = " << diff_sq
                  << "  sigma-sqrt(lambda) = " << diff_val
                  << "\n";
    }

    std::cout << "\nMax |sigma^2 - lambda| = " << max_abs_diff_sq
              << "\nMax |sigma - sqrt(lambda)| = " << max_abs_diff_val
              << "\n";
}


// 
// Funzioni per matplot++
//

void matplot_legend_align(
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
    } else if (pos_enum == 3) { // top left
        lg->horizontal_location(HA::left);
        lg->vertical_location(VA::top);
    };
};

figure_handle matplot_table_init (
    const bool ahold, 
    const std::string &nome, 
    const std::string &titolo, 
    const int xlab, 
    const int ylab) 
{
    figure_handle fig = matplot::figure(ahold);
    fig->size(1800,900);
    fig->position(10,10);
    fig->name(nome);
    fig->number_title(false);
    fig->title_color({0., 1., 0., 1.});   //alpha, red, green, blue 0. ~ 1.
    fig->tiledlayout(xlab, ylab); // stabilisce panel xlab righe x xlab colonne
    fig->title(titolo);
    fig->title_font_size_multiplier(3);
    return fig;
}

//
// funzioni di gestione del menu principale e dei suoi item
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