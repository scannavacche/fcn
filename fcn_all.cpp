#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <map>
#include <cfloat>
#include <limits>   // per std::numeric_limits
#include <matplot/matplot.h>
#include "fcn_lib_calc.h"
#include <functional>
#include <string>
#include <unordered_map>


using std::cout;
using std::endl;
using Vec = std::vector<double>;
using Mat = std::vector<Vec>;
using namespace matplot;
using namespace std;

using matrix_ptr = std::shared_ptr<matplot::matrix>; 

using Vecd = std::vector<double>;
using Vecf = std::vector<float>;
using Vecn = std::vector<int>;

namespace {     
    //
    // namespace anonimo per funzioni di utilità locali al file, non visibili all'esterno
    //

using Action = std::function<void()>;
using ActionRegistry = std::unordered_map<std::string, Action>;

    //  FCN – Lezione 1, Esercizio 1
    //  Base di funzioni coseno e matrice di Gram

    Vec campiona(
        int k, 
        const Vec &x)
        {
            // =============================================================================
            //  Campionamento di una funzione base
            //  Restituisce il vettore dei valori di phi_k nei nodi x:
            //      phi_k[n] = cos(k * x[n])
            // =============================================================================

        Vec phi(x.size());
        int totnum = (int) x.size();
        for (int i=0; i<totnum; i++)
        {
            phi[i]=cos(k*x[i]);
        }
        // DONE: riempire phi
        return phi;
    }

    Mat gram(
        const Vec& x, 
        const int K) 
    {
        // gram computes a $K \times K$ Gram matrix from a vector x by sampling K subvectors with campiona, 
        // taking pairwise dot products via prodotto_scalare, and filling a symmetric matrix G.

        Mat G = crea_matrice(K, K);
        Vec sample_u, sample_v;
        double Gij;     // usiamo uno scalare per accelerare senza lookup doppio
        for (int i=0;i<K;i++)
            for (int j=i;j<K;j++)
            {
                sample_u = campiona(i,x);
                sample_v= campiona(j,x);
                Gij = prodotto_scalare(sample_u, sample_v);
                G[i][j] = Gij;
                G[j][i] = Gij;
            }

        // DONE: doppio ciclo su i e j
        return G;
    }

    //  FCN – Foglio 1, Esercizio 2
    //  Polinomi di Taylor per eˣ e cos(x)

    template <typename T>
    T taylor0_exp(
        T x, 
        int n) {
            // centrata in x=0
        bool reverse_x = false;
        if (x < T(0)) {
            x = -x;
            reverse_x = true;
        }

        T somma = T(1);
        T termine = T(1);

        for (int k = 1; k <= n; ++k) {
            termine = (termine / static_cast<T>(k)) * x;
            somma += termine;
        }

        if (somma == T(0) || !reverse_x) return somma;
        return T(1) / somma;
    }

    template <typename T>
    T taylor1_exp(
        T x, 
        int n, 
        int TCenter, 
        bool cheat_mode) {
            // centrata in TCenter = x-1 se x>0 e x+1 se x<0 
        bool reverse_x = false;
        if (x < T(0)) {
            x = -x;
            TCenter = -TCenter;
            reverse_x = true;
        }

        T e_calc = cheat_mode
            ? static_cast<T>(std::exp(static_cast<T>(TCenter)))
            : taylor0_exp(static_cast<T>(TCenter), 40);

        T somma = T(1);
        T termine = T(1);
        T dx = x - static_cast<T>(TCenter);

        for (int k = 1; k <= n; ++k) {
            termine = termine * dx / static_cast<T>(k);
            somma += termine;
        }

        T totale = somma * e_calc;
        if (totale == T(0) || !reverse_x) return totale;
        return T(1) / totale;
    }

    template <typename T>
    T taylor0_cos(
        int x, 
        int n) {
        T x_rad = static_cast<T>(deg2rad(x));
        bool reverse_x = false;
        if (x_rad < T(0)) {
            x_rad = -x_rad;
            // reverse_x = true; non serve, cos(x) = cos(-x)  e posso perderne traccia
        }

        T somma = T(1);
        T termine = T(1);
        bool segno = false;

        for (int k = 1; k <= n; ++k) {
            T two_k = static_cast<T>(2 * k);
            termine = (termine * x_rad) / (two_k - T(1));
            termine = (termine * x_rad) / two_k;

            if (segno) somma += termine;
            else somma -= termine;

            segno = !segno;
        }

        // if (somma == T(0) || !reverse_x) return somma;  // e' il cos e non serve reversare
        return somma;
    }    

    void TablePlot_exp(
        axes_handle ax, 
        const Vecd n_vald, 
        const Vecf n_valf, 
        const Vecn n_val, 
        const int target_arg, 
        const int curr_tile, 
        const int TCenter,
        bool cheat_mode) 
    {
        string e0 = (cheat_mode) ? "esatto" : "calcolato";
        title("Stima su e^" + itostr(target_arg) + " centrato in " + itostr(TCenter) +" con e^"+itostr(TCenter)+" " + e0);
        ax->clear();
        ax->legend();
        ax->grid(true);
        ax->axis(matplot::automatic);
        // inutile tentare in manuale di scendere sotto 10^-10 
        // perche' y-origin e' una float :(  quindi tolgo n_terms dai param formali
        // ax->axis(matplot::manual);
        // x->axis({ (double) 0 , (double) n_terms, (double) DBL_MIN , 100.});

        hold(on);
        semilogy(n_val, n_vald)->display_name("exp_d" + itostr(target_arg));
        semilogy(n_val, n_valf)->display_name("exp_f" + itostr(target_arg));

        matplot::xlabel("Taylor Terms #");
        matplot::ylabel("Abs Error (log)");

    }

    void TablePlot_cos(
        axes_handle ax, 
        const Vecd n_vald, 
        const Vecf n_valf, 
        const Vecn n_val, 
        const int target_arg, 
        const int curr_tile, 
        bool cheat_mode) 
    {
        string e0 = (cheat_mode) ? "esatto" : "calcolato";
        title("Stima su cos(" + itostr(target_arg) + ") centrato in 0 con cos(0) " + e0);
        ax->clear();
        ax->legend();
        ax->grid(true);
        ax->axis(matplot::automatic);
        hold(on);
        semilogy(n_val, n_vald)->display_name("cos_d" + itostr(target_arg));
        hold(on);
        semilogy(n_val, n_valf)->display_name("cos_f" + itostr(target_arg));

        matplot::xlabel("Taylor Terms #");
        matplot::ylabel("Abs Error (log)");

    }

    void TablePrint_exp(
        Vecd &n_vald, 
        Vecf &n_valf, 
        Vecn &n_val, 
        const int target_arg, 
        const double e_esatto, 
        const int n_terms, 
        const bool zerocentered, 
        int &TCenter, 
        bool cheat_mode, 
        double &LastD, 
        double &LastF ) {
        double tn;
        float tns;
        LastD=0;  // sono solo valori di ritorno
        LastF=0;  // se escono a zero l'errore non e' ancora flat
        double PrevD = 0;
        double PrevF = 0;
        string color_rowD;
        string color_rowF;

        n_vald.clear();
        n_valf.clear();
        n_val.clear();
        std::cout << "\n── Taylor exp(" << itostr(target_arg) << ") ────────────────────────────────────────────────────────────────────────\n";
        std::cout << std::setw(4) << "n"
                << std::setw(18) << "Tₙ(1) dbl"
                << std::setw(18) << "|errore dbl|" 
                << std::setw(26)  << "Tₙ(1) flt"
                << std::setw(26)  << "|errore flt|" << "\n" ;
        std::cout << std::string(90, '-') << "\n";

        double errd;
        float errf;
        for (int n = 1; n <= n_terms; ++n) {
            if (zerocentered) {
                TCenter = 0;
                tn  = taylor0_exp<double>(target_arg, n);
                tns = taylor0_exp<float>(target_arg, n);
            } else {
                if (target_arg < 0 ) TCenter = target_arg + 1; else TCenter = target_arg-1;
                tn  = taylor1_exp<double>(target_arg, n, TCenter, cheat_mode);
                tns = taylor1_exp<float>(target_arg, n, TCenter, cheat_mode);
            }
            double errd = std::abs(tn - e_esatto);
            double errf = std::abs(tns - e_esatto);
            n_vald.push_back(errd > 0 ? errd : 1e-20);
            n_valf.push_back(errf > 0 ? errf : 1e-20);
            color_rowD = color_bool((PrevD == errd));
            color_rowF = color_bool((PrevF == errf));
            std::cout << std::setw(4) << n
                    << std::setw(18) << std::fixed << std::setprecision(12) << tn <<"\t" 
                    << color_rowD 
                    << std::setw(18) << std::scientific << errd << "\t"
                    << "\033[0m"
                    << std::setw(18) << std::fixed << std::setprecision(12) << tns <<"\t" 
                    << color_rowF 
                    << std::setw(18) << std::scientific << errf << "\n"
                    << "\033[0m";
            n_val.push_back(n);
            PrevD = errd;
            PrevF = errf;

        
            // errori.push_back(err > 0 ? err : 1e-20);  // evita log(0)
        }

    }

    void TablePrint_cos(
        Vecd &n_vald, 
        Vecf &n_valf, 
        Vecn &n_val, 
        const int target_arg, 
        const double e_esatto, 
        const int n_terms, 
        bool cheat_mode, 
        double &LastD, 
        double &LastF ) {
        double tn;
        float tns;
        LastD=0;  // sono solo valori di ritorno
        LastF=0;  // se escono a zero l'errore non e' ancora flat
        double PrevD = 0;
        double PrevF = 0;
        string color_rowD;
        string color_rowF;

        n_vald.clear();
        n_valf.clear();
        n_val.clear();
        std::cout << "\n── Taylor cos(" << itostr(target_arg) << ") ────────────────────────────────────────────────────────────────────────\n";
        std::cout << std::setw(4) << "n"
                << std::setw(18) << "Tₙ() dbl"
                << std::setw(18) << "|errore dbl|" 
                << std::setw(26)  << "Tₙ() flt"
                << std::setw(26)  << "|errore flt|" << "\n" ;
        std::cout << std::string(90, '-') << "\n";

        double errd;
        float errf;
        for (int n = 1; n <= n_terms; ++n) {

            tn  = taylor0_cos<double>(target_arg, n);
            tns = taylor0_cos<float>(target_arg, n);

            errd = std::abs(tn - e_esatto);
            errf = std::abs(tns - e_esatto);
            n_vald.push_back(errd > 0 ? errd : 1e-20);
            n_valf.push_back(errf > 0 ? errf : 1e-20);
            color_rowD = color_bool((PrevD == errd));
            color_rowF = color_bool((PrevF == errf));
            std::cout << std::setw(4) << n
                    << std::setw(18) << std::fixed << std::setprecision(12) << tn <<"\t" 
                    << color_rowD 
                    << std::setw(18) << std::scientific << errd << "\t"
                    << "\033[0m"
                    << std::setw(18) << std::fixed << std::setprecision(12) << tns <<"\t" 
                    << color_rowF 
                    << std::setw(18) << std::scientific << errf << "\n"
                    << "\033[0m";
            n_val.push_back(n);
            PrevD = errd;
            PrevF = errf;

        
            // errori.push_back(err > 0 ? err : 1e-20);  // evita log(0)
        }

    }

    //  FCN – Foglio 1, Esercizio 3
    //  Precisione di macchina e aritmetica IEEE 754

    template <typename T>
    void checksum (
        T add1, 
        T add2, 
        T sum12) {
        double rounded_sum = std::ceil(sum12 * 1000.0) / 1000.0;
        string found_eq = "\n === somma potenze di 2?";
        if ((add1 + add2) !=  sum12) {
            found_eq = "\n !!! : l'arrotondamento della somma e' diverso dalla somma degli arrotondamenti";    // e' proprio diverso nel senso che c'e' errore di arrotondamento
        } else {
            if (sum12 != rounded_sum) {
                found_eq = "\n ~ ~ : la somma coincide con la rappresentazione degli addendi ma non con il valore atteso"; // in questo caso e' uguale alla somma ma al valore nominale? lo tronco a tre decimali e vediamo
            };
        };

        std::cout << endl << typeid(T).name() << endl << std::setprecision(17);
        std::cout << sum12  <<  " = " << sum12 << " Atteso \n";
        std::cout << add1 << " + "<<  add2 << " = " << sum12 << found_eq << "\n";
    }

    template <typename T>
    void loop_sum(
        T s, 
        string n){
        T steps = 0.01;
        int loops = 100;
        std::cout << "\n── Parte B2: somma di "<< std::setprecision(4)<< steps << " x " << loops << " "<< typeid(T).name() << ":"<< n <<" ──\n";
        for (int i = 0; i < loops; ++i) s += steps;
        std::cout << std::setprecision(20);
        std::cout << "Somma:   " << s << "\n";
        std::cout << "Atteso:  " << 1.0 << "\n";
        std::cout << "Errore:  " << std::abs(s - 1.0) << "\n";
    }

    // FCN – Laboratorio 2, Esercizio 1
    // Derivata numerica con differenze finite in avanti

    Mat derivata_prima(
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


        double h = h_ticks(a, b, n - 1);
        for (int i = 0; i < m; ++i) {
            D[i][i]     = -1.0 / h;
            D[i][i + 1] =  1.0 / h;
        };
        return D;
    }

    Vec campiona_arctan(
        int n, 
        double a, 
        double b) {
            // ----------------------------------------------------------------------------
            // TODO 3 --- Campionare arctan(t)
            // Campiona u(t) = arctan(t) su n nodi equidistanti in [a,b]:
            // t_i = a + i*h, h = (b-a)/(n-1), i = 0,...,n-1.
            // ----------------------------------------------------------------------------

        Vec u(n);
            // TODO: calcolare h, poi per ogni i, t = a + i*h, u[i] = std::atan(t).
        double t;
        double h = h_ticks(a, b, n - 1); // uso n-1 per avere gli stessi punti 
        for (int i = 0; i<n; ++i) {      // ma il loop sul vettore ha una comp in piu' 
            t = a + h*i;
            u[i] = std::atan(t);
        } ;

        return u;
    }

    Vec derivata_discreta(
        const Mat &D, 
        const Vec &u) {
        // ----------------------------------------------------------------------------
        // TODO 3 --- Applicazione di D al vettore u
        // Du = D * u, dove D ∈ R^{(n-1)×n} e u ∈ R^n.
        // ----------------------------------------------------------------------------

        int m = static_cast<int>(D.size());
        int n = static_cast<int>(u.size());
        Vec Du(m, 0.0);

        // TODO: prodotto matrice-vettore Du[i] = sum_j D[i][j] * u[j].

        for (int i = 0; i<m; ++i) {
            double sum_j = 0;
            for (int j =0; j<n; ++j) {
                sum_j += D[i][j]*u[j];
            }
            Du[i] = sum_j;
        }
        return Du;
    }

    Vec derivata_discreta_senza_matrice(
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
        double h = h_ticks(a, b, n-1);
        // TODO: calcolare Du[i] con un ciclo for soltanto usando u[i+1] - u[i].
        for (int j=0; j<n-1; ++j) {
            Du[j] = (u[j+1]-u[j]) / h;
        }
        return Du;
    }


    // FCN – Laboratorio 2, Esercizio 1A
    // Problema di Cauchy: x'(t) = x(t) + f(t), x(t0) = c

    void costruisci_sistema_cauchy(
        int n, 
        double t0, 
        double T, 
        double x0,
        Vec &t, 
        Vec &fvals, 
        Mat &L, 
        Vec &b, 
        double (*ft)(double)) 
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
        // Riga 0: x_0 = x0  => L[0][0] = 1, b[0] = x0
        // Righe i>=1: - (1+h) x_{i-1} + x_i = h f(t_{i-1})
        // X[0][0] sta sulla diagonale, potrei assegnargli 1 come agli altri Xii
        // ma i termini sotto diagonale sono uno in meno: condiziono o li conto in riga)
        L[0][0] = 1;
        b[0] = x0;
        for (int i = 1; i < n; i++) {
            L[i][i] = 1;
            L[i][i-1] = -(1+h);
            b[i] = h*fvals[i-1];
        }
    }


    // FCN – Laboratorio 2, Esercizio 1B
    // Problema differenziale con condizioni al bordo: x''(t) = f(t), x(0) = 0, x(1) = 0

    void costruisci_sistema_bordo(
        int n, 
        double t0, 
        double T, 
        double x0, 
        double x1,
        Vec &t, 
        Vec &fvals, 
        Mat &A, 
        Vec &b) 
        {
        t = Vec(n);
        fvals = Vec(n);
        double h = (T - t0) / (n - 1);
        double h_quad = 1.0 / pow(h,2);
        double diag_m2 = -2 * h_quad;
        int m = n - 2; // numero di punti interni
        A = Mat(m, Vec(m, 0.0));
        b = Vec(m, 0.0);

        // TODO 1: riempire il vettore dei nodi t_i e dei valori f_cos(t_i)
        for (int i = 0; i < n - 1; ++i) {
            t[i] = t0 + i * h;
            fvals[i] = f_cos(t[i]);
        }

        // TODO 2: costruire A e b
        b[0]=x0;
        for (int i = 0; i<m; i++) {
            A[i][i] = diag_m2; 
            if (i>0) {
                A[i][i-1] = h_quad;
                b[i] = h * fvals[i-1];
            };
            if (i<(m-1)) A[i][i+1] = h_quad;
        };
        b[m-1]=x1;

        // tutto restituito tramite args
    }


    void f1_sample() {

        // ── costanti ─────────────────────────────────────────────────────────────────
        int    K = 40 ;                  // numero di funzioni base (frequenze: 0..N-1)
        const int    N = 100;                // numero di nodi di campionamento
        const int   Cols = 10;               // N di colonne per il dump a righe dei vettori 

        // alloco tutto prima per poter usare in un loop

        double NR;
        int curr_tile = 0;
        vector<matrix_ptr> mtpr(4);
        Vec x_eq;
        Vec x_asym1;
        Vec x_asym2;
        Vec x_rnd;
        Mat G_eq;
        Mat G_asym1;
        Mat G_asym2;
        Mat G_rnd;
        bool leave = false;
        bool redo = false;

        int stderr_handle = dup(fileno(stderr)); // salva il vecchio stderr descriptor
        FILE* fres = freopen("/dev/null", "w", stderr);       // evita le menate di gnuplot :)

        while (!leave) {
            int dummy = system("clear"); 
        // ── Parte A: nodi equidistanti ────────────────────────────────────────────
            NR = 2*PI;
            x_eq = nodi_equidistanti(0, NR, N);
            G_eq  = gram(x_eq, K);
            //vector_dump(x_eq, Cols, N);
            //stampa_matrice(G_eq, NR, "Gram nodi eq  da 0 a ");
            
            // ── Parte B: nodi NON equidistanti ────────────────────────────────────────
            //    Creare N nodi a piacere (es. concentrati in [0, pi]) e ricalcolare G.
            //    Cosa cambia?


            NR = PI;
            x_asym1 = nodi_equidistanti(0, NR, N);
            G_asym1 = gram(x_asym1, K);

            NR = PI/2;
            x_asym2 = nodi_equidistanti(0, NR, N);
            G_asym2 = gram(x_asym2, K);
            //vector_dump(x_asym, Cols, N);
            //stampa_matrice(G_asym, NR, "Gram nodi eq da 0 a ");

            // veramente casuali

            NR = 100;
            x_rnd = nodi_random(0, NR, N);
            G_rnd  = gram(x_rnd, K);
            // vector_dump(x_rnd, Cols, N);
            // stampa_matrice(G_rnd, NR, "Gram nodi random da 0 a ");
            
            // riordinati con nodi crescenti
            // NR = 100;
            // Vec x_srt = nodi_bubblesort(x_rnd, N);
            // Mat G_srt  = gram(x_srt, K);
            // vector_dump(x_srt, Cols, N);
            // stampa_matrice(G_srt, NR, "Gram nodi random ord da 0 a "); // inutile 


            // ── Visualizzazione con matplot++ ─────────────────────────────────────────

            // 0) Plot di una delle funzioni base sui nodi equidistanti
        /*
            {
                auto fig = figure(true);
                matplot::title("Una delle funzioni base: phi_2(x) = cos(2*x)");
                Vec phi = campiona(2, x_eq);
                matplot::bar(x_eq, phi);
                matplot::xlabel("x");
                matplot::ylabel("phi_2(x)");
                matplot::show();
            }
        */   
        /*
            // 1) Plot delle funzioni base sui nodi equidistanti
            {
                auto fig = figure(true);
                title("Funzioni base: phi_k(x) = cos(k*x)");
                hold(on);
                for (int k = 0; k < K; ++k) {
                    Vec phi = campiona(k, x_eq);
                    plot(x_eq, phi)->display_name("k=" + itostr(k));
                }
                matplot::legend();
                matplot::xlabel("x");
                matplot::ylabel("phi_k(x)");
                matplot::show();
            }
        */
            string atitle = "Gram matrix con " + itostr(N) + " campioni su "+itostr(K) + " funzioni";
            figure_handle fig = TableInit(true, "Test su basi di coseni", atitle, 2, 2);
        
            
            // TODO 5: convertire G_eq in formato adatto a imagesc
            // imagesc accetta un vector<vector<double>>
            // G_eq è già in quel formato — visualizzarla e commentare il risultato
        
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_eq);
            legend_align(matplot::legend(), 0, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi equidistanti in 0..2PI");
            matplot::colorbar();
            curr_tile++;

            // 3) TODO 6: visualizzare anche G_nonEq come immagine e confrontare
            //    con G_eq. Cosa vedi? Riesci a spiegare la differenza?
            
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_rnd);
            legend_align(matplot::legend(), 0, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi random");
            matplot::colorbar();
            curr_tile++;
            
            // 3) TODO 5bis: visualizzare anche G_Eq asimmetrico come immagine 
            //    e confrontare
            //    con G_eq. Cosa vedi? Riesci a spiegare la differenza?

            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_asym1);
            legend_align(matplot::legend(), 0, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi equidistanti in 0..PI");
            matplot::colorbar();
            curr_tile++;
        

        
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_asym2);
            legend_align(matplot::legend(), 0, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi equidistanti in 0..PI/2");
            matplot::colorbar();
            
            fig->draw();
            curr_tile = 0 ;

            redo = false;
            while (!redo && !leave) {
                cout << "Segnale a freq K = "<< itostr(K) << " campionato su " << itostr(N) << " punti \n" ;
                cout << "Inserire nuovo valore di K (dim base dello spazio del segnale) 1 .. 256 (<q> per uscire) > ";
                cin_clear();
                if (cin >> K) { if (K>0 && K<256) redo=true;} else {cout << "Key: " << K <<endl; leave=true;} ; 
                cin_clear();
            };



        };
        fflush(stderr);  // tira lo sciacquone
        dup2(stderr_handle, fileno(stderr));  //ripristina lo stderr su console
        close(stderr_handle);  //liberaci dal duplicato tanto non ci induci in errore
    }

    void f1_taylor_exp() {

        int n_terms = 20;
        Vecn n_val;
        Vecd n_vald;
        Vecf n_valf;
        int curr_tile = 0;
        figure_handle fig;

        bool leave = false;
        bool redo = false;
        int target_arg = -2;
        bool Cheat_e0 = true;
        double LastD = 0; 
        double LastF = 0;
    
        
        while (!leave) {
            int dummy = system("clear");
            auto fig = TableInit (false, "Taylor exp", "Convergenza di Taylor per exp(x)", 2, 2);
            int TCenter = 0;
            // ── Parte A2: tabella errori per eˣ con x = 1 ────────────────────────   
            double e_esatto = std::exp(1);
            TablePrint_exp (n_vald, n_valf, n_val, 1, e_esatto, n_terms, true, TCenter, Cheat_e0, LastD, LastF);
            // ── Parte A3: plot errore in scala log ────────────────────────────────────
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_exp(fig->current_axes(), n_vald, n_valf, n_val, 1, curr_tile, TCenter,Cheat_e0);
            curr_tile++;

            // ── Parte A4: ripeti per x = -2 e poi user input ─────────────────────
            // TODO A4: costruisci tabella analoga per x utente
            //          L'errore converge più lentamente? Perché?
            //          Perche' siamo piu' lontani dalla centratura dello sviluppo di T
            //          In realta' stiamo sviluppando secondo McLaurin
            //          terzo panel con il parametro zerocentered a false in TablePrint lo mostra
    
            e_esatto = std::exp(target_arg);
            TablePrint_exp(n_vald, n_valf, n_val, target_arg, e_esatto, n_terms, true, TCenter, Cheat_e0, LastD, LastF);
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_exp(fig->current_axes(), n_vald, n_valf, n_val, target_arg, curr_tile, TCenter, Cheat_e0);
            curr_tile++;

            e_esatto = std::exp(target_arg);
            TablePrint_exp(n_vald, n_valf, n_val, target_arg, e_esatto, n_terms, false, TCenter, !Cheat_e0, LastD, LastF);
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_exp(fig->current_axes(), n_vald, n_valf, n_val, target_arg, curr_tile, TCenter, !Cheat_e0);
            curr_tile++;

            e_esatto = std::exp(target_arg);
            TablePrint_exp(n_vald, n_valf, n_val, target_arg, e_esatto, n_terms, false, TCenter, Cheat_e0, LastD, LastF);
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_exp(fig->current_axes(), n_vald, n_valf, n_val, target_arg, curr_tile, TCenter, Cheat_e0);
            curr_tile++;

            fig->draw();
            // getc(stdin);

            redo = false;
            while (!redo && !leave) {
                cout << "Inserire nuovo valore di x di cui calcolare exp(x) < x - eXit>:  " ;

                cin_clear();
                if (cin >> target_arg) { if (target_arg != 0) redo=true;} else {cout << "Key: " << target_arg <<endl; leave=true;} ; 
                cout << "Inserire numero di termini dello sviluppo di Taylor 1 .. 40 (<q> per uscire) > ";
                if (cin >> n_terms) { if (n_terms>0 && n_terms<=40) redo=true;} else {cout << "Key: " << n_terms <<endl; leave=true;} ; 
                cin_clear();
            };
        }
    }

    void f1_taylor_cos() {

        int n_terms = 20;
        Vecn n_val;
        Vecd n_vald;
        Vecf n_valf;
        int curr_tile = 0;
        figure_handle fig;

        bool leave = false;
        bool redo = false;
        int target_arg = 1;    // angolo in gradi 
        bool Cheat_e0 = true;
        double LastD = 0; 
        double LastF = 0;
        double e_esatto;
        int stderr_handle = dup(fileno(stderr)); // salva il vecchio stderr descriptor
        FILE* fres = freopen("/dev/null", "w", stderr);       // evita le menate di gnuplot :)
        
        while (!leave) {
            int dummy = system("clear"); 
            auto fig = TableInit (true, "Taylor cos", "Stima di cos(x) con Taylor centrato in 0", 2,2 );
            // ── Parte A2: tabella errori per cos(x) con x = 1 ────────────────────────   
            e_esatto = std::cos(deg2rad(target_arg)); // valore dal quale si calcola l'errore
            TablePrint_cos (n_vald, n_valf, n_val, target_arg, e_esatto, n_terms, Cheat_e0, LastD, LastF);
            // ── Parte A3: plot errore in scala log ────────────────────────────────────
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_cos(fig->current_axes(), n_vald, n_valf, n_val, target_arg, curr_tile, Cheat_e0);

            curr_tile++;
            target_arg = 30; // secondo grafico, di riferimento a 30 gradi
            e_esatto = std::cos(deg2rad(target_arg));
            TablePrint_cos (n_vald, n_valf, n_val, target_arg, e_esatto, n_terms, Cheat_e0, LastD, LastF);
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_cos(fig->current_axes(), n_vald, n_valf, n_val, target_arg, curr_tile, Cheat_e0);
            curr_tile++;

            fig->draw();
            // getc(stdin);

            redo = false;
            while (!redo && !leave) {
                cout << "Inserire nuovo valore di x di cui calcolare cos(x) [deg 1..360] < x - eXit>:  " ;
                cin_clear();
                if (cin >> target_arg) { if ((target_arg >= -360) && (target_arg<=360)) redo=true;} else {cout << "Key: " << target_arg <<endl; leave=true;} ; 
                cout << "Inserire numero di termini dello sviluppo di Taylor 1 .. 40 (<q> per uscire) > ";
                if (cin >> n_terms) { if (n_terms>0 && n_terms<=40) redo=true;} else {cout << "Key: " << n_terms <<endl; leave=true;} ; 
                cin_clear();
                curr_tile=0;
            };
        }
        fflush(stderr);  // tira lo sciacquone
        dup2(stderr_handle, fileno(stderr));  //ripristina lo stderr su console
        close(stderr_handle);  //liberaci dal duplicato tanto non ci induci in errore

        // ── Parte C2: cancellazione per x grande ─────────────────────────────────
        // TODO C1: trova sperimentalmente il valore di n oltre cui l'errore
        //          per taylor_exp(1.0, n) smette di diminuire
        {
            // questo e' gia' fatto sulla TablePrint con i codici colore ANSI 
        }
    }

    void f1_prec() {

        // =========================================================================
        //  PARTE A1 — Trovare ε_M sperimentalmente
        // =========================================================================
        std::cout << "── Parte A1: machine epsilon (double) ──\n";

        double eps = 1.0;
        // TODO: dimezza eps finché 1.0 + eps == 1.0 (cioè finché 1+eps > 1 è vero)
        // poi moltiplica per 2 per ottenere l'ultimo valore "visibile"
        // ...
        bool found_e = false;
        double div_2 = 2.0;
        int dummy = system("clear"); 

        while (!found_e) {
            if ((1.0 + eps) == 1.0) found_e = true; else eps /= div_2;
            cout << "#" ;
        }
        cout << endl << endl;

        std::cout << "eps_d (double)        :  " << std::scientific << eps << "\n";
        std::cout << "numeric_limits double :  " << std::numeric_limits<double>::epsilon() << "\n";
        std::cout << "rapporto  eps_d/eps_M :  " << eps / (double)std::numeric_limits<double>::epsilon() << "\n";
        std::cout << endl;

        // Sono uguali? Di quanto differiscono?

        // =========================================================================
        //  PARTE A2 — Confronto float error
        // =========================================================================
        std::cout << "\n── Parte A2s: machine epsilon (float) ──\n";

        float eps_f = 1.0f;
        // TODO: stesso algoritmo con float
        // ...
        found_e = false;
        div_2 = 2.0f;
        while (!found_e) {
            if ((1.0f + eps_f) == 1.0f) found_e = true; else eps_f /= div_2;
            cout << "!" ;
        }
        cout << endl << endl;

        std::cout << "eps_f (float)         :  " << std::scientific << eps_f << "\n";
        std::cout << "numeric_limits float  :  " << std::numeric_limits<float>::epsilon() << "\n";
        std::cout << "rapporto  eps_f/eps_M :  " << eps_f / (float)std::numeric_limits<float>::epsilon() << "\n";
        std::cout << endl;

        // =========================================================================
        //  PARTE A3 — Confronto float vs double 
        // =========================================================================
        std::cout << "\n── Parte A3s: float vs double ──\n";


        std::cout << "rapporto flt/dbl calc :  " << eps_f / eps << "\n";
        std::cout << "rapporto flt/dbl mach :  " <<  (float)std::numeric_limits<float>::epsilon() / (double)std::numeric_limits<double>::epsilon() << "\n";
        // Di quanti ordini di grandezza differiscono?

        // =========================================================================
        //  PARTE B1 — 0.1 + 0.2 == 0.3 ?
        // =========================================================================
        std::cout << "\n── Parte B1: 0.1 + 0.2 ──\n";
        // TODO: stampa se sono uguali o diversi
        // Spiega: 0.1 non è rappresentabile esattamente in binario

        checksum<double>(0.1, 0.2, 0.3);
        checksum<double>(0.5, 0.25, 0.75);

        checksum<float>(0.1, 0.2, 0.3);
        checksum<float>(0.5, 0.25, 0.75);

        
        
        // =========================================================================
        //  PARTE B2 — Somma di 0.1 per 10 volte
        // =========================================================================
        {
            float s = 0.0;
            loop_sum(s, "float"); // bene, riconosce il tipo dal primo param 
        };
        {
            double s = 0.0;
            loop_sum(s, "double"); // bene, riconosce il tipo dal primo param 
        };
        {
            long double s = 0.0;
            loop_sum(s, "long double"); // bene, riconosce il tipo dal primo param 
        };

        // =========================================================================
        //  PARTE B3 — Più piccolo N tale che fl(1 + 1/N) == 1
        // =========================================================================
        std::cout << "\n── Parte B3: soglia visibilita' ──\n";
        // TODO: ciclo su N crescenti, trova il primo N tale che
        //       1.0 + 1.0/N == 1.0  (il computer non vede la differenza)
        // Poi esprimilo in termini di eps_M: N ≈ 1/eps_M ?
        // ...
        double eps_m = std::numeric_limits<double>::epsilon();
        double N = 2.0;
        double N_target = 1.0;
        double N_term = 1.0;
        bool not_equal = true;
        while (not_equal) {
            if ((N_term + (N_term / N)) == N_target) {
                not_equal = false;
            } else{
                N *= 2;
            };
        }
        cout << "Ora andiamo a vedere tra " << (1 + N/2) << " e " << N << " ? " << endl;
        cout << "N/2 * eps_m = " <<  eps_m * N /2  <<  " Ultimo distinguibile" << endl;
        cout << "  N * eps_m = " <<  eps_m * N << " Primo indistinguibile" << endl;

        // =========================================================================
        //  PARTE C1 — Collegamento con Taylor (Esercizio 2)
        // =========================================================================
        std::cout << "\n── Parte C1: limite di Taylor collegato a eps_M ──\n";
        // Taylor exp(1): calcola l'errore per n = 1..25
        // Trova il valore n* dove l'errore è minimo
        // Verifica che il termine n*-esimo della serie sia circa eps_M * e
        // Stampa:
        //   n*  |  errore minimo  |  termine n*  |  eps_M * e
        // ...
        cout << " Questo lo abbiamo visto bene nelle tavole di taylor colorate \n"
            << " mancava solo il confronto con eps_M ma era gia' stimabile ad occhio attorno  10^-16 \n";
        cout << "Premere un tasto per continuare" << endl;   

        // cin_clear();
        wait_return_to_menu(false);
        wait_return_to_menu(false);

    }

    void f2_deriv() {
        int n = 100;          // numero di punti di campionamento
        double domain_span = 1;
        double a;      // estremi dell'intervallo per arctan(t)
        double b;
        int curr_tile = 0;
        bool leave = false;
        bool redo = false;
        while (!leave) {
            int dummy = system("clear");
            redo = false;
            a = -domain_span;      // estremi dell'intervallo per arctan(t)
            b =  domain_span;

        
            figure_handle f = TableInit(true, "Derivate", "Calcolo della derivata discreta", 1,2);
            
            // 1) Costruzione della matrice D
            Mat D = derivata_prima(n, a, b);

            // 2) Visualizzazione di D con imagesc
            f->nexttile(curr_tile);
            auto ax = f->current_axes();
            ax->title("Matrice D (differenze finite in avanti)");
            ax->imagesc(D); // questo e' a posto, colorbar() no (ma neanche in es1_campionamento)
            colorbar(); // ne inserisce uno per pigrizia ma devo ancora vedere come si inserisce con metodo.
            //  show();
            curr_tile++;

            // 3) Campionamento di u(t) = arctan(t)
            Vec u = campiona_arctan(n, a, b);

            // 4) Applicazione di D: Du ≈ u'(t)
            Vec Du = derivata_discreta(D, u);

            // 4a) Applicazione di D: Du ≈ u'(t)
            Vec Du_sm = derivata_discreta_senza_matrice(u, a, b);  // ok sono sovrapposte

            // 5) Costruzione vettore dei nodi t_i
            Vec t(n);
            double h = (b - a) / (n - 1);
            for (int i = 0; i < n; ++i) {
                t[i] = a + i * h;
            }

            // 6) Confronto con derivata analitica u'(t) = 1 / (1 + t^2)
            Vec u_deriv_analitica(n);
            for (int i = 0; i < n; ++i) {
                // valutare la derivata analitica u'(t) = 1 / (1 + t^2)
                u_deriv_analitica[i] = 1.0 / (1.0 + pow(t[i],2));
            }

            // 7) Visualizzazione di u(t) e della derivata discreta
            
            f->nexttile(curr_tile);
            ax = f->current_axes();
            ax->title("u(t) = arctan(t) e derivata numerica");
            ax->legend();
            legend_align(matplot::legend(), 1, 0.0, 0.0);

            // u(t)
            hold(on);
            auto p = plot(t, u);
            p->line_width(2);
            p->display_name("u(t) = arctan(t)");

            // Du ha lunghezza n-1, usiamo i primi n-1 nodi
            Vec t_deriv(n - 1);
            for (int i = 0; i < n - 1; ++i) {
                t_deriv[i] = t[i];
            }
            auto q = plot(t_deriv, Du);
            q->line_width(2);
            q->line_style("*");
            q->display_name("u'(t) = Du");

            auto r = plot(t_deriv, Du_sm);
            r->line_width(2);
            r->display_name("u'(t) (senza matrice)");

            // derivata analitica su tutti i nodi
            auto s = plot(t, u_deriv_analitica);
            s->line_width(2);
            s->line_style("--");
            s->display_name("u'(t) analitica");

            matplot::legend();
            xlabel("t [rad]");
            ylabel("valore");
            f->draw();

            curr_tile = 0; // riparte dalla tile di sinistra

            redo = false;
            while (!redo && !leave) {
                cout << "Inserire nuova semiampiezza di dominio di arctg(x) 1 .. 10 rad (<q> per uscire) > ";

                cin_clear();

                if (cin >> domain_span) { 
                    if (domain_span >= 1 && domain_span <= 10) {
                        redo=true;
                        // domain_update(domain_span, a, b);
                    };
                } else {
                    cout << "Key: " << domain_span <<endl; 
                    leave=true;
                } ; 
                cin_clear();
            };
        };
    };

    void f2_cauchy() {
        int n = 50;          // numero di punti della griglia
        double t0 = 0.0;     // tempo iniziale
        double T  = 5.0;     // tempo finale
        double x0 = 1.0;     // condizione iniziale x(t0)

        Vec t, fvals, b;
        Mat L;
        
        bool leave = false;
        bool redo = false;
        while (!leave) {
            int dummy = system("clear");
            redo = false;

            // Costruzione del sistema triangolare L x = b
            costruisci_sistema_cauchy(n, t0, T, x0, t, fvals, L, b, f_x);
            vector_dump(b, 10, 50, " b = t"); 
            // Risoluzione con sostituzione in avanti
            Vec x = forward_substitution(L, b);
            vector_dump(x, 10, 50, " x da Lx = h*t"); 

            auto fig = TableInit(true, "Cauchy", "Soluzione numerica del problema di Cauchy", 1, 1);
            
            // migliorie: aggiornare titolo con parametri 
            
            // Verifica della matrice L
            
            matplot::legend();
            fig->nexttile(0);
            ;
            imagesc(L)->display_name("Verifica matrice");
            colorbar();
            
            // Visualizzazione della soluzione numerica x(t)
            fig->nexttile(1);
            auto p = plot(t, x);
            p->line_width(2);
            p->display_name("x(t)");
            xlabel("t");
            ylabel("x(t)");

            costruisci_sistema_cauchy(n, t0, T, x0, t, fvals, L, b, f_sin);
            vector_dump(b, 10, 50, " b = sin(t) "); 
            // Risoluzione con sostituzione in avanti
            x = forward_substitution(L, b);
            vector_dump(x, 10, 50, " x da Lx = h*sin(t) "); 

            fig->nexttile(2);
            p = plot(t, x);
            p->line_width(2);
            p->display_name("x(t)");
            xlabel("t");
            ylabel("sin(t)");

            costruisci_sistema_cauchy(n, t0, T, x0, t, fvals, L, b, f_cos); 
            vector_dump(b, 10, 50, " b = cos(t) "); 
            // Risoluzione con sostituzione in avanti
            x = forward_substitution(L, b);
            vector_dump(x, 10, 50, " x da Lx = h*cos(t) "); 


            fig->nexttile(3);
            p = plot(t, x);
            p->line_width(2);
            p->display_name("x(t)");
            xlabel("t");
            ylabel("cos(t)");

            fig->draw();
        
            redo = false;
            while (!redo && !leave) {
                std::cout << "Inserire nuovo T finale 5 .. 50 (<q> per uscire) > ";

                cin_clear();

                if (std::cin >> T) { if (T>=5 && T<=50) redo=true;} else {cout << "Key: " << T <<endl; leave=true;} ;
                if (!leave) {
                    std::cout << "Inserire nuovo valore iniziale x0 (<q> per uscire) > " ;
                    if (std::cin >> x0) { if (x0>=t0 && x0<=T) redo=true;} else {cout << "Key: " << x0 <<endl; leave=true;} ;
                };
                cin_clear();
            };
        };
    };

    void f2_bordo() {
        int n = 200;           // numero di punti della griglia
        double t0 = 0.0;        // tempo iniziale
        double T  = 3.14159;     // tempo finale
        double x0 = 0.0;        // condizione iniziale x(t0)
        double x1 = 0.0;        // condizione finale x(T)

        Vec t, fvals, b, x, y;
        Mat A,L,U;

        bool leave = false;
        bool redo = false;
        while (!leave) {

            int dummy = system("clear");
            // Costruzione del sistema triangolare L x = b
            costruisci_sistema_bordo(n, t0, T, x0, x1, t, fvals, A, b);
            vector_dump(fvals, 10, fvals.size(), "fvals");
            vector_dump(b, 10, b.size(), "b");

            // stampa_matrice(A, A.size(), "tridiag");

            auto ffig = TableInit(true, "Bordo", "Costruzione matrice A e fattorizzazione LU", 2, 2);
            
            // Verifica della matrice A tridiag

            ffig->nexttile(0);
            title("Matrix A");
            imagesc(A); // ->display_name("matrice A");
            // matplot::legend();
            colorbar();

            
            // Risoluzione con fattorizzazione LU
            LU(A, L, U);

            // stampa_matrice(L, L.size(), "lower");
            // stampa_matrice(U, U.size(),  "upper");

            ffig->nexttile(2);
            title("Matrix L");
            imagesc(L); //->display_name("matrice L");
            colorbar();

            ffig->nexttile(3);
            title("Matrix U");
            imagesc(U);// ->display_name("matrice U");
            //matplot::legend();
            colorbar();

            // Sostituzione in avanti per L y = b
            y = forward_substitution(L, b);
            vector_dump(y, 10, y.size(), "y");

            // Sostituzione indietro per U x = y
            x = backward_substitution(U, y);
            vector_dump(x, 10, x.size(), "x");

            // costruzione vettore completo con condizioni al bordo
            Vec x_completo(n, 0.0);
            x_completo[0] = x0;
            for(int i = 1; i < n-1; ++i) {
                x_completo[i] = x[i-1];
            };
            x_completo[n-1] = x1;

            cout << "taglia n = " << n << endl;

            vector_dump(x_completo, 10, x_completo.size(), "x_completo");

            // Visualizzazione della soluzione numerica x(t)
            ffig->nexttile(1);

            // auto ffig = figure(true);
            title("Problema differenziale con cond al bordo: soluz. numerica");
            cout << "taglia t = " << t.size() << endl;
            cout << "taglia x = " << x.size() << endl;
            cout << "taglia x_completo = " << x_completo.size() << endl;
            auto p = plot(x_completo);
            p->line_width(2);
            p->display_name("x(t)");
            xlabel("t");
            ylabel("x(t)");

            ffig->draw();

            redo = false;
            while (!redo && !leave) {
                std::cout << "Inserire nuovo T finale 1 .. 10 (<q> per uscire) > ";

                cin_clear();

                if (std::cin >> T) { if (T>=1 && T<=10) redo=true;} else {cout << "Key: " << T <<endl; leave=true;} ;
                if (!leave) {
                    std::cout << "Inserire nuova cond iniziale x0 (<q> per uscire) > " ;
                    if (std::cin >> x0) { if (x0>=t0 && x0<=T) redo=true;} else {cout << "Key: " << x0 <<endl; leave=true;} ;
                    if (redo) {
                        std::cout << "Inserire nuova cond finale x1 (<q> per uscire) > " ;
                        if (std::cin >> x1) { if (x1>=x0 && x1<=T) redo=true;} else {cout << "Key: " << x1 <<endl; leave=true;} ;
                    }
                };
                cin_clear();
           };
        };
    };

    void f2_integral() {

        double a=-7,b=7; // intervallo di integrazione
        int N = 40; // numero di punti di campionamento   
        bool leave = false; 
        Vec fvals;

        while (!leave) {
            bool redo = false;
            int dummy = system("clear");
            double h = h_ticks(a,b,N); // passo di campionamento

            Vec fvals(N, 0.0);                  // qui abbiamo 1/(1+x^2) a integrare, campionata in N punti equispaziati tra a e b
            Vec t(N, 0.0);                     // vettore dei nodi di campionamento
            for (int i = 0; i < N; ++i) {
                double x_i = a + i * h;
                fvals[i] = f_atan_d(x_i);
                t[i] = x_i;
            }
            // vector_dump(fvals, 10, fvals.size(), "fvals = 1/(1+x^2) campionata in N punti");

            Mat T = costruisci_triangolare(N); // costruisce matrice triangolare di dimensione N x N
            T = prodotto_matrice_coeff(T, h); // moltiplica per h
            // stampa_matrice(T, T.size(), "Triangolare");

            Vec v_atan = prodotto_matrice_vettore(T, fvals); // prodotto matrice vettore per ottenere stima integrale
            vector_dump(v_atan, 10, v_atan.size(), "T * fvals = stima integrale per somma discreta");
            v_atan = vector_shift(v_atan, std::atan(a)); // shift di arcotg(a) per centratura
            vector_dump(v_atan, 10, v_atan.size(), "T * fvals = stima integrale riposizionata");
        
            // Mat T_inv_sint = costruisci_inv_triangolare_sint(10);
            // stampa_matrice(T_inv_sint, T_inv_sint.size(), "Inversa sintetica");

            Mat T_inv = calcola_inversa_LU (T);
            //stampa_matrice(T_inv, T_inv.size(), "Inversa con LU");
            double NFT = norma_matrice(-1, T);
            double NFT_inv = norma_matrice(-1, T_inv);
            double N1T = norma_matrice(1, T);
            double N1T_inv = norma_matrice(1, T_inv);   
            double NIT = norma_matrice(0, T);
            double NIT_inv = norma_matrice(0, T_inv); 
            double N2T = norma_matrice(2, T);
            double N2T_inv = norma_matrice(2, T_inv);   
            
            Vec v_atan_d = prodotto_matrice_vettore(T_inv, vector_shift(v_atan, -std::atan(a))); // derivata corrispondente, con backshift di centratura
            Vec v_atan_dcn = prodotto_matrice_vettore(T_inv, v_atan); // derivata analiticamente corretta ma sbagliata per cn 
            vector_dump(v_atan_d, 10, v_atan_d.size(), "T * v_atan = stima derivata simmetrica");
            vector_dump(v_atan_dcn, 10, v_atan_dcn.size(), "T * v_atan = stima derivata analitica");
            
            // Mat I = prodotto_matrici(T, T_inv);
            // stampa_matrice(I, I.size(), "T * T_inv");  

            cout << endl;
            cout << "Con N = " << N << " ||T|| = " << NFT << "\t e ||T_inv|| = " << NFT_inv << "\t con Norma di Frobenius" << endl;
            cout << "Con N = " << N << " ||T|| = " << NIT << "\t e ||T_inv|| = " << NIT_inv << "\t con Norma Infinito" << endl;
            cout << "Con N = " << N << " ||T|| = " << N1T << "\t e ||T_inv|| = " << N1T_inv << "\t con Norma 1" << endl;
            cout << "Con N = " << N << " ||T|| = " << N2T << "\t e ||T_inv|| = " << N2T_inv << "\t con Norma 2" << endl;
            cout << endl;
            cout << "Il numero di condizionamento è: " << NFT*NFT_inv << " (Frobenius), " << NIT*NIT_inv << " (Infinito) e " << N1T*N1T_inv << " (1) e " << N2T*N2T_inv << " (2)" << endl;
            cout << "E vale per le norme piu' adeguate (infinito e 1) N * " << NIT * NIT_inv / N << " e N * " << N1T * N1T_inv / N<< endl;
            cout << "Per Frobenius la crescita e' con N^(3/2), non lineare" << endl;

            Vec Err_Min_1(N, 0.0), Err_0(N, 0.0), Err_Plus_1(N, 0.0);
            for (int i = 1; i < N-1; ++i) {
                Err_Min_1[i] = std::abs(v_atan_d[i] - fvals[i-1]);
                Err_0[i] = std::abs(v_atan_d[i] - fvals[i]);
                Err_Plus_1[i] = std::abs(v_atan_d[i] - fvals[i+1]);
            }

            figure_handle f = TableInit(true, "Integrazione e derivazione", "da 1/(1+x^2) ad atan e viceversa", 2,2);
            auto ax = f->current_axes();

            f->nexttile(0);
            ax = f->current_axes();
            ax->title("Matrice T (Triangolare L di integrazione)");
            ax->imagesc(T); 
            colorbar(); 
            // cout << "Matrice T: triangolare con 1 su diagonale e h su sotto diagonale" << endl;

            f->nexttile(1);
            ax = f->current_axes();
            ax->title("Matrice Inversa di T (Triangolare L di derivazione)");
            ax->imagesc(T_inv); 
            colorbar(); 
            // cout << "Matrice T_inv: triangolare con 1 su diagonale e -h su sotto diagonale" << endl;

            f->nexttile(2);
            ax = f->current_axes();
            ax->title("Integrazione e derivazione discreta");
            // ax->legend();
            // cout << "Integrazione discreta: U = T * fvals" << endl;

            // u(t)
            hold(on);
            auto p = plot(t, fvals);
            // p->line_style("+");
            p->line_width(2);
            p->display_name("u'(t) = 1/(1+t^2)");

            auto q = plot(t, v_atan);
            q->use_y2(true);
            // ax->y2_axis().limits({-1.6, 1.6});  
            q->line_width(2);
            q->display_name("U(t) = Tu'(t)");

            auto r = plot(t, v_atan_d);
            r->line_width(2);
            r->line_style("--");
            r->display_name("u'(t) = T^-1 U(t)");
 

            legend_align(matplot::legend(), 2, 0,0);
            matplot::legend();
            xlabel("t [rad]");
            ylabel("valore");

            f->nexttile(3);
            ax = f->current_axes();
            ax->title("Errore su integrazione e successiva derivazione");
            // ax->legend();
            // cout << "Integrazione discreta: U = T * fvals" << endl;
            hold(on);
            auto p1 = plot(t, Err_Min_1);
            // p->line_style(" -");
            p1->line_width(2);
            p1->display_name("Err i su i-1");

            auto q1 = plot(t, Err_0);
            q1->line_width(2);
            q1->use_y2(true);

            ax->y2_axis().limits({-1.00e-6, 1.00e-6 });
            q1->display_name("Err i su i");

            auto r1 = plot(t, Err_Plus_1);
            r1->line_width(2);
            r1->line_style("- ");
            r1->display_name("Err i su i+1");
 

            legend_align(matplot::legend(), 1, 0,0);
            matplot::legend();
            xlabel("t [rad]");
            ylabel("valore");
            f->draw();

            while (!redo && !leave) {
                std::cout << "Inserire Numero di nodi di campionamento (<q> per uscire) > ";

                cin_clear();

                if (std::cin >> N) { if (N>=1 && N<=1000) redo=true;} else {cout << "Key: " << N <<endl; leave=true;} ;
                if (!leave) {
                    std::cout << "Inserire inizio intervallo di integrazione -10..10 (<q> per uscire) > " ;
                    if (std::cin >> a) { if (a>-10 && a<=10) redo=true;} else {cout << "Key: " << a <<endl; leave=true;} ;
                    if (redo) {
                        std::cout << "Inserire nuova fine intervallo di integrazione "<< a <<"..10 (<q> per uscire) > " ;
                        if (std::cin >> b) { if (b>a && b<=10) redo=true;} else {cout << "Key: " << b <<endl; leave=true;} ;
                    }
                };
                cin_clear();
            };

        };
    };

    void f2_norme() {
        bool leave = false; 
        double a=-7,b=7; // intervallo di integrazione

        int dummy = system("clear");
        while (!leave) {
            bool redo = false;

            std::cout << "\n=== Norma 2 e numero di condizionamento spettrale ===\n";
            
            const int N_values[] = {10, 20, 40, 80, 160, 320, 640};
            const int n_tests = 7;
            
            std::vector<double> x_vals, cond_2_vals, cond_f_vals, cond_1_vals, cond_I_vals;
            
            std::cout << std::setfill(' ')            
                << std::setw(6) << "N" 
                << std::setw(12) << "norm2(A)"
                << std::setw(12) << "norm2(A^-1)"
                << std::setw(12) << "kappa2"
                << std::setw(12) << "kappa2/N^2"
                << std::setw(12) << "kappaF"
                << std::setw(12) << "kappa1"
                << std::setw(12) << "kappaI\n";
            
            for (int n = 0; n < n_tests; ++n) {
                int N = N_values[n];
                double h = h_ticks(a,b,N); // passo di campionamento
            
                Mat A = costruisci_triangolare(N);
                A = prodotto_matrice_coeff(A, h); // moltiplica per h
                Mat A_inv = calcola_inversa_LU(A);
                
                // Norma 2 spettrale
                double norm2_A = norma_matrice(2, A);
                double norm2_Ainv = norma_matrice(2, A_inv);
                double kappa2 = norm2_A * norm2_Ainv;
                
                // Norma Frobenius
                double normF_A = norma_matrice(-1, A);
                double normF_Ainv = norma_matrice(-1, A_inv);
                double kappaF = normF_A * normF_Ainv;
                
                // Norma 1 = max somma colonne
                double norm1_A = norma_matrice(1, A);
                double norm1_Ainv = norma_matrice(1, A_inv);
                double kappa1 = norm1_A * norm1_Ainv;
                
                // Norma inf1 = max somma righe
                double normI_A = norma_matrice(0, A);
                double normI_Ainv = norma_matrice(0, A_inv);
                double kappaI = normI_A * normI_Ainv;
                
                std::cout << std::setw(6) << N
                        << std::setw(12) << std::fixed << std::setprecision(2) << norm2_A
                        << std::setw(12) << norm2_Ainv
                        << std::setw(12) << kappa2
                        << std::setw(12) << kappa2 / (N*N)
                        << std::setw(12) << kappaF
                        << std::setw(12) << kappa1
                        << std::setw(12) << kappaI << "\n";  

                x_vals.push_back(N);
                cond_2_vals.push_back(kappa2);
                cond_f_vals.push_back(kappaF);
                cond_1_vals.push_back(kappa1);
                cond_I_vals.push_back(kappaI);

            }

            std::string title = "Numeri di condizionamento vs N";
            figure_handle f = TableInit(true, "Confronto Norme", title, 1,1);
            f->nexttile(0);
            auto ax = f->current_axes();
            ax->title(title);

            xlabel("N");
            ylabel("kappa");

            // Confronto con curva teorica N^2
            Vec Ns, theory_N2, theory_N;
            for (double N = 10; N < 1000; N *=2) {
                Ns.push_back(N);
                theory_N2.push_back((4.0/M_PI/M_PI) * N * N);
                theory_N.push_back(2.0 * N);
            }
            hold(on);

            legend_align(matplot::legend(), 3, 0,0);

            auto p2 = plot(x_vals, cond_2_vals);
            p2->display_name("kappa_2 (spettrale)");
            p2->color("red");
            p2->line_style("-");
            p2->marker(".");
            p2->line_width(6);

 
            auto p1 = plot(x_vals, cond_1_vals);
            p1->display_name("kappa_1");
            p1->color("green");
            p1->line_style("-");
            p1->marker("+");
            p1->line_width(6);
            
            auto pI = plot(x_vals, cond_I_vals);
            pI->display_name("kappa_I (Infinito)");
            pI->color("magenta");
            pI->line_style("--");
            pI->marker("r");
            pI->line_width(6);

            auto pf = plot(x_vals, cond_f_vals);
            pf->display_name("kappa_F (Frobenius)");
            pf->color("cyan");
            pf->line_style("-");
            pf->line_width(6);
            pf->marker("+");
            
           
            auto ptn2 = semilogy(Ns, theory_N2);
            ptn2->display_name("4N^2/pi^2 (teorico)");
            ptn2->color("blue");
            ptn2->line_style("-");
            ptn2->marker("");
            ptn2->line_width(2);
            
            auto ptn1 = plot(Ns, theory_N);
            ptn1->display_name("2N (teorico)");
            ptn1->color("black");
            ptn1->line_style("_");
            ptn1->marker("");
            ptn1->line_width(2);
           
            f->draw();

            

            while (!redo && !leave) {
                std::cout << "Inserire inizio intervallo di integrazione -10..10 (<q> per uscire) > " ;
                if (std::cin >> a) { if (a>-10 && a<=10) redo=true;} else {cout << "Key: " << a <<endl; leave=true;} ;
                if (redo) {
                    std::cout << "Inserire nuova fine intervallo di integrazione "<< a <<"..10 (<q> per uscire) > " ;
                    if (std::cin >> b) { if (b>a && b<=10) redo=true;} else {cout << "Key: " << b <<endl; leave=true;} ;
                }
                cin_clear();
            };


       };
        


    };

    void f2_deconv() {
        cout << "Deconvoluzione con Kernel di Gauss" << endl;
        double a=0, b=1; // intervallo
        int N = 100; // numero di punti separatori   
        bool leave = false; 
        double sigma = 2.0;
        bool KG_Normal = true;
        char KG_key = '\0';
        Mat kappa2(4, Vec(3, 0.0)); // 0 numero cond, 1 norma2(K), 2 (norma2(K_inv))

        while (!leave) {
            double h  = h_ticks(a,b,N); 
            Mat K = kernel_gaussiano_matrice(N, sigma*h, h, KG_Normal, kappa2[0]);
            Mat K2 = kernel_gaussiano_matrice(N, sigma*2*h, h, KG_Normal, kappa2[1]);
            Mat K4 = kernel_gaussiano_matrice(N, sigma*4*h, h, KG_Normal, kappa2[2]);
            Mat K8 = kernel_gaussiano_matrice(N, sigma*8*h, h, KG_Normal, kappa2[3]);

            // stampa_matrice(K, N, "Matrice Kernel di Gauss");

            Vec ws0 = segnale_finestra(N,(1./4.),(3./4.),1);
            Vec ws1 = prodotto_matrice_vettore(K,ws0);
            Vec ws2 = prodotto_matrice_vettore(K2,ws0);
            Vec ws4 = prodotto_matrice_vettore(K4,ws0);
            Vec ws8 = prodotto_matrice_vettore(K8,ws0);
            cout << endl << endl;
            Vec fs0 = risolvi_sistema_LU(K,ws0);
            Vec fs1 = risolvi_sistema_LU(K,ws1);
            Vec fs2 = risolvi_sistema_LU(K2,ws2);
            Vec fs4 = risolvi_sistema_LU(K4,ws4);
            Vec fs8 = risolvi_sistema_LU(K8,ws8);
            vector_dump(ws0, 10, ws0.size(), "Finestra base");
            vector_dump(fs0, 10, fs0.size(), "Trasf base");
            cout <<endl;
            vector_dump(ws1, 10, ws1.size(), "smooth "+std::to_string(sigma*h));
            vector_dump(fs1, 10, fs1.size(), "Trasf smooth "+std::to_string(sigma*h));
            cout <<endl;
            vector_dump(ws2, 10, ws2.size(), "smooth "+std::to_string(sigma*h*2));
            vector_dump(fs2, 10, fs2.size(), "Trasf smooth "+std::to_string(sigma*h*2));
            cout <<endl;
            vector_dump(ws4, 10, ws4.size(), "smooth "+std::to_string(sigma*h*4));
            vector_dump(fs4, 10, fs4.size(), "Trasf smooth "+std::to_string(sigma*h*4));
            cout <<endl;
            // vector_dump(ws8, 10, ws8.size(), "smooth "+std::to_string(sigma*h*8));
            // vector_dump(fs8, 10, fs8.size(), "Trasf smooth "+std::to_string(sigma*h*8));
            cout << endl << endl;
            string KGN_title = ((KG_Normal) ? "Normalizzato " : "");

{   // namespace anonimo per heatmap 

            figure_handle f = TableInit(true, "Deconvoluzione", "Kernel di Gauss " + KGN_title+ "a diverse StD base "+std::to_string(sigma), 2,2);
            auto ax = f->current_axes();

            f->nexttile(0);
            ax = f->current_axes();
            ax->title("1 * sigma * h = " + std::to_string(sigma*h));
            ax->imagesc(K); 
            colorbar(); 

            f->nexttile(1);
            ax = f->current_axes();
            ax->title("2 * sigma * h = " + std::to_string(sigma*h*2));
            ax->imagesc(K2); 
            colorbar(); 

            f->nexttile(2);
            ax = f->current_axes();
            ax->title("4 * sigma * h = " + std::to_string(sigma*h*4));
            ax->imagesc(K4); 
            colorbar(); 

            f->nexttile(3);
            ax = f->current_axes();
            ax->title("8 * sigma * h = " + std::to_string(sigma*h*8));
            ax->imagesc(K8); 
            colorbar(); 

            f->draw();
}

{   // namespace anonimo per plot deconvoluzioni
    

            Vec x_vals = nodi_equidistanti(a,b,N);
            std::string title = "Trasformazioni ("+KGN_title+") a 1x, 2x, 4x StD base ";

            figure_handle f = TableInit(true, "Deconvoluzione", title + std::to_string(sigma*h), 2, 2);

            legend_align(matplot::legend(), 3, 0,0);

            f->nexttile(0);
            auto ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "Su segnale base con I2="+ format_numstr(kappa2[0][0]) + " |K|=" + format_numstr(kappa2[0][1]) + " |K^-1|="+format_numstr(kappa2[0][2]);
            ax->title(title);
            hold(on);

            auto in0 = plot( ws0);
            in0->display_name("input finestra");
            in0->color("blue");
            in0->line_style("-");
            in0->marker("");
            in0->line_width(2);
            in0->use_y2(false);
            // ax->ylim({-0.1, 1.1});

            auto out0 = plot( fs0);
            out0->display_name("output base");
            out0->color("red");
            out0->line_style("-");
            out0->marker("");
            out0->line_width(2);
            out0->use_y2(true);

            f->nexttile(1);
            ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma)+" con I2="+ format_numstr(kappa2[1][0]) + " |K|=" + format_numstr(kappa2[1][1]) + " |K^-1|="+format_numstr(kappa2[1][2]);
            ax->title(title);
            hold(on);

            auto inb1 = plot( ws1);
            inb1->display_name("input");
            inb1->color("blue");
            inb1->line_style("-");
            inb1->marker("");
            inb1->line_width(2);
            inb1->use_y2(false);

            auto outb1 = plot( fs1);
            outb1->display_name("output");
            outb1->color("red");
            outb1->line_style("-");
            outb1->marker("");
            outb1->line_width(2);
            outb1->use_y2(true);
           
            f->nexttile(2);
            ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma*2)+" con I2="+ format_numstr(kappa2[2][0]) + " |K|=" + format_numstr(kappa2[2][1]) + " |K^-1|="+format_numstr(kappa2[2][2]);
            ax->title(title);
            hold(on);

            auto inb2 = plot( ws2);
            inb2->display_name("input");
            inb2->color("blue");
            inb2->line_style("-");
            inb2->marker("");
            inb2->line_width(2);
            inb2->use_y2(false);
            
            auto outb2 = plot( fs2);
            outb2->display_name("output");
            outb2->color("red");
            outb2->line_style("-");
            outb2->marker("");
            outb2->line_width(2);
            outb2->use_y2(true);

            f->nexttile(3);
            ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma*4)+" con I2="+ format_numstr(kappa2[3][0]) + " |K|=" + format_numstr(kappa2[3][1]) + " |K^-1|="+format_numstr(kappa2[3][2]);
            ax->title(title);
            hold(on);

            auto inb4 = plot(ws4);
            inb4->display_name("input");
            inb4->color("blue");
            inb4->line_style("-");
            inb4->marker("");
            inb4->line_width(2);
            inb4->use_y2(false);
            
            auto outb4 = plot(fs4);
            outb4->display_name("output");
            outb4->color("red");
            outb4->line_style("-");
            outb4->marker("");
            outb4->line_width(2);
            outb4->use_y2(true);

            f->draw();

}

{   // namespace anonimo per test di condizionamento al rumore

        // vector_dump(ws0, 10, ws0.size(), "Ingresso pulito");
        // vector_dump(add_rumore(ws0, 1), 10, ws0.size(), "Perturbato 0.1%");
        // vector_dump(add_rumore(ws0, 10), 10, ws0.size(), "Perturbato 1%");

           
            Vec ws2_r001 = prodotto_matrice_vettore(K, add_rumore(ws0,0.01));
            Vec ws2_r01 = prodotto_matrice_vettore(K, add_rumore(ws0,0.1));
            Vec ws2_r05 = prodotto_matrice_vettore(K, add_rumore(ws0,1));
            Vec ws2_r2 = prodotto_matrice_vettore(K, add_rumore(ws0,10));
            Vec fs2_r001 = risolvi_sistema_LU(K,ws2_r001);
            Vec fs2_r01 = risolvi_sistema_LU(K,ws2_r01);
            Vec fs2_r05 = risolvi_sistema_LU(K,ws2_r05);
            Vec fs2_r2 = risolvi_sistema_LU(K,ws2_r2);
            
            Vec x_vals = nodi_equidistanti(a,b,N);
            std::string title = "Perturbazioni ("+KGN_title+") a 1x StD base ";

            figure_handle f = TableInit(true, "Deconvoluzione perturbata", title + format_numstr(sigma), 1, 1);

            legend_align(matplot::legend(), 3, 0,0);

            f->nexttile(0);
            auto ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma)+" I2="+ format_numstr(kappa2[0][0]) + " |K|=" + format_numstr(kappa2[0][1]) + " |K^-1|="+format_numstr(kappa2[0][2]);
            ax->title(title);
            hold(on);

            auto in0 = plot( ws1);
            in0->display_name("smooth 1x");
            in0->color("blue");
            in0->line_style("-");
            in0->marker("");
            in0->line_width(2);
            in0->use_y2(false);
            // ax->ylim({-0.1, 1.1});

            auto out0 = plot( fs1);
            out0->display_name("deconv 1x");
            out0->color("red");
            out0->line_style("-");
            out0->marker("");
            out0->line_width(2);
            out0->use_y2(false);
            // ax->ylim({-0.1, 1.1});

            auto perturb0 = plot( fs2_r001);
            perturb0->display_name("deconv noise 0.001%");
            perturb0->color("magenta");
            perturb0->line_style("- ");
            perturb0->marker("");
            perturb0->line_width(2);
            perturb0->use_y2(true);

            f->nexttile(1);
            ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma)+" I2="+ format_numstr(kappa2[0][0]) + " |K|=" + format_numstr(kappa2[0][1]) + " |K^-1|="+format_numstr(kappa2[0][2]);
            ax->title(title);
            hold(on);

            auto inb1 = plot( ws1);
            inb1->display_name("smooth 1x");
            inb1->color("blue");
            inb1->line_style("-");
            inb1->marker("");
            inb1->line_width(2);
            inb1->use_y2(false);

            auto outb1 = plot( fs1);
            outb1->display_name("deconv 1x");
            outb1->color("red");
            outb1->line_style("-");
            outb1->marker("");
            outb1->line_width(2);
            outb1->use_y2(false);

            auto perturb1 = plot( fs2_r01);
            perturb1->display_name("deconv noise 0.01%");
            perturb1->color("magenta");
            perturb1->line_style("- ");
            perturb1->marker("");
            perturb1->line_width(2);
            perturb1->use_y2(true);
           
            f->nexttile(2);
            ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma)+" con I2="+ format_numstr(kappa2[0][0]) + " |K|=" + format_numstr(kappa2[0][1]) + " |K^-1|="+format_numstr(kappa2[0][2]);
            ax->title(title);
            hold(on);

            auto inb2 = plot( ws1);
            inb2->display_name("smooth 1x");
            inb2->color("blue");
            inb2->line_style("-");
            inb2->marker("");
            inb2->line_width(2);
            inb2->use_y2(false);

            auto outb2 = plot( fs1);
            outb2->display_name("deconv 1x");
            outb2->color("red");
            outb2->line_style("-");
            outb2->marker("");
            outb2->line_width(2);
            outb2->use_y2(false);
            
            auto perturb2 = plot( fs2_r05);
            perturb2->display_name("deconv noise 0.1%");
            perturb2->color("magenta");
            perturb2->line_style("- ");
            perturb2->marker("");
            perturb2->line_width(2);
            perturb2->use_y2(true);

            f->nexttile(3);
            ax = f->current_axes();
            legend_align(matplot::legend(), 3, 0,0);
            title = "smoothed q=" + std::to_string((int)sigma)+" I2="+ format_numstr(kappa2[0][0]) + " |K|=" + format_numstr(kappa2[0][1]) + " |K^-1|="+format_numstr(kappa2[0][2]);
            ax->title(title);
            hold(on);

            auto inb4 = plot(ws1);
            inb4->display_name("smooth 1x");
            inb4->color("blue");
            inb4->line_style("-");
            inb4->marker("");
            inb4->line_width(2);
            inb4->use_y2(false);

            auto outb4 = plot(fs1);
            outb4->display_name("deconv 1x");
            outb4->color("red");
            outb4->line_style("-");
            outb4->marker("");
            outb4->line_width(2);
            outb4->use_y2(false);
            
            auto perturb4 = plot(fs2_r2);
            perturb4->display_name("deconv noise 1%");
            perturb4->color("magenta");
            perturb4->line_style("- ");
            perturb4->marker("");
            perturb4->line_width(2);
            perturb4->use_y2(true);

            f->draw();


        
}
            bool redo = false;
            while (!redo && !leave) {
                std::cout << "Numero di punti N sul lato della griglia [2..1024](<q> per uscire) > " ;
                if (std::cin >> N) { if (N>=2 && N<=1025) redo=true;} else {cout << "Key: " << N <<endl; leave=true;} ;
                if (redo) {
                    std::cout << "Inserire nuova Deviazione Sigma in q multipli di h=(1/(N-1)) da 1 a "<< N-1 <<" (<q> per uscire) > " ;
                    if (std::cin >> sigma) { if (sigma >=1 && sigma<=N-1) redo=true;} else {cout << "Key: " << sigma <<endl; leave=true;} ;
                }
                if (redo) {
                    std::cout << "Il Kernel K va normalizzato? (<s/n> | <0> per uscire) > " ;
                    if (std::cin >> KG_key) {
                         if (KG_key == 'q') { 
                            redo=false; 
                            leave=true;
                        } else {
                            KG_Normal = (KG_key == 's'); 
                            redo=true;
                        }
                    }
                }
                cin_clear();
            };
        }
    }

    ActionRegistry build_action_registry() 
        {
        return {
            {"f1_sample",      f1_sample},
            {"f1_taylor_exp",  f1_taylor_exp},
            {"f1_taylor_cos",  f1_taylor_cos},
            {"f1_prec",        f1_prec},
            {"f2_deriv",       f2_deriv},
            {"f2_cauchy",      f2_cauchy},
            {"f2_bordo",       f2_bordo},
            {"f2_integral",    f2_integral},
            {"f2_norme",       f2_norme},
            {"f2_deconv",       f2_deconv}
            };
        };
};  
// 
// fine namespace anonimo
//


int main() {
    {
        figure_handle fig = TableInit(true, "Figure vuota per inizializzare matplot++", "Figure vuota per inizializzare matplot++", 2, 2);
        // fig->draw();
    }
    MenuConfig menu;
    ActionRegistry actions;

    try {
        menu = load_menu_config("menu.json");
        actions = build_action_registry();
    } catch (const std::exception& e) {
        std::cerr << "Errore inizializzazione menu: " << e.what() << '\n';
        return 1;
    }

    for (;;) {

        std::cout << "\n" << menu.title << '\n';
        std::cout << "--------------------------------------------------\n";

        for (const auto& item : menu.items) {
            std::cout << item.key << " - " << item.label;
            if (!item.enabled) {
                std::cout << " [disabilitata]";
            }
            std::cout << '\n';
        }

        std::cout << "\nScelta: ";
        int choice;

        if (!(std::cin >> choice)) {
            std::cout << "Input non valido.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }


        const MenuItem* item = find_menu_item(menu, choice);
        if (!item) {
            std::cout << "Voce inesistente.\n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (!item->enabled) {
            std::cout << "Voce presente ma attualmente disabilitata.\n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (item->action == "exit") {
            break;
        }

        auto it = actions.find(item->action);
        if (it == actions.end()) {
            std::cout << "Azione non registrata nel programma: " << item->action << '\n';
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        cin_clear();
        it->second();
        wait_return_to_menu(true);
        
        int dummy = system("clear"); 

    }

    std::cout << "\nUscita dal programma.\n";
    return 0;
}