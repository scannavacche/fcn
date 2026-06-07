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
#include <chrono>

#include <cassert>
#include "dataset.hpp"
#include "svd_qr.cpp" // magari basta solo un header visto che e' compilato nella lib statica

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

const bool bidiag_mode = false; // true upper, false lower 

namespace {     
    //
    // namespace anonimo per funzioni di utilità locali al file, non visibili all'esterno
    //

    using Action = std::function<void()>;
    using ActionRegistry = std::unordered_map<std::string, Action>;

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
    void checksum ( // mostra l'errore di arrotondamento per somma
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

    void preriscaldamento_anti_overhead(Mat& A) {
        //
        // gia' una volta bastava ma proviamo con tre giri 
        //
        for (int i=0; i<3; i++) linear_max_autoval_pwr_any_res(A, 1000, 1e-12);
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
            clear_screen(); 
        // ── Parte A: nodi equidistanti ────────────────────────────────────────────
            NR = 2*PI;
            x_eq = nodi_equidistanti(0, NR, N);
            G_eq  = matrix_build_gram(x_eq, K);
            //vector_dump(x_eq, Cols, N);
            //matrix_dump(G_eq, "Gram nodi eq  da 0 a ");
            
            // ── Parte B: nodi NON equidistanti ────────────────────────────────────────
            //    Creare N nodi a piacere (es. concentrati in [0, pi]) e ricalcolare G.
            //    Cosa cambia?


            NR = PI;
            x_asym1 = nodi_equidistanti(0, NR, N);
            G_asym1 = matrix_build_gram(x_asym1, K);

            NR = PI/2;
            x_asym2 = nodi_equidistanti(0, NR, N);
            G_asym2 = matrix_build_gram(x_asym2, K);
            //vector_dump(x_asym, Cols, N);
            //matrix_dump(G_asym, "Gram nodi eq da 0 a ");

            // veramente casuali

            NR = 100;
            x_rnd = nodi_random(0, NR, N);
            G_rnd  = matrix_build_gram(x_rnd, K);
            // vector_dump(x_rnd, Cols, N);
            // matrix_dump(G_rnd, "Gram nodi random da 0 a ");
            
            // riordinati con nodi crescenti
            // NR = 100;
            // Vec x_srt = nodi_bubblesort(x_rnd, N);
            // Mat G_srt  = matrix_build_gram(x_srt, K);
            // vector_dump(x_srt, Cols, N);
            // matrix_dump(G_srt, "Gram nodi random ord da 0 a "); // inutile 


            // ── Visualizzazione con matplot++ ─────────────────────────────────────────

            // 0) Plot di una delle funzioni base sui nodi equidistanti
        /*
            {
                auto fig = figure(true);
                matplot::title("Una delle funzioni base: phi_2(x) = cos(2*x)");
                Vec phi = vector_campiona_f_k(2, x_eq, f_cos);
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
                    Vec phi = vector_campiona_f_k(k, x_eq,f_cos);
                    plot(x_eq, phi)->display_name("k=" + itostr(k));
                }
                matplot::legend();
                matplot::xlabel("x");
                matplot::ylabel("phi_k(x)");
                matplot::show();
            }
        */
            string atitle = "Gram matrix con " + itostr(N) + " campioni su "+itostr(K) + " funzioni";
            figure_handle fig = matplot_table_init(true, "Test su basi di coseni", atitle, 2, 2);
        
            
            // TODO 5: convertire G_eq in formato adatto a imagesc
            // imagesc accetta un vector<vector<double>>
            // G_eq è già in quel formato — visualizzarla e commentare il risultato
        
            curr_tile = 0 ;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_eq);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi equidistanti in 0..2PI");
            matplot::colorbar();

            // 3) TODO 6: visualizzare anche G_nonEq come immagine e confrontare
            //    con G_eq. Cosa vedi? Riesci a spiegare la differenza?
            
            curr_tile=1;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_rnd);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi random");
            matplot::colorbar();
            
            // 3) TODO 5bis: visualizzare anche G_Eq asimmetrico come immagine 
            //    e confrontare
            //    con G_eq. Cosa vedi? Riesci a spiegare la differenza?

            curr_tile=2;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_asym1);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi equidistanti in 0..PI");
            matplot::colorbar();
        

        
            curr_tile=3;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_asym2);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name("100 nodi equidistanti in 0..PI/2");
            matplot::colorbar();
            
            fig->draw();

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
            clear_screen();
            auto fig = matplot_table_init (false, "Taylor exp", "Convergenza di Taylor per exp(x)", 2, 2);
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
            clear_screen(); 
            auto fig = matplot_table_init (true, "Taylor cos", "Stima di cos(x) con Taylor centrato in 0", 2,2 );
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
        clear_screen(); 

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
            clear_screen();
            redo = false;
            a = -domain_span;      // estremi dell'intervallo per arctan(t)
            b =  domain_span;

        
            figure_handle f = matplot_table_init(true, "Derivate", "Calcolo della derivata discreta", 1,2);
            
            // 1) Costruzione della matrice D
            Mat D = matrix_build_derivata1(n, a, b);

            // 2) Visualizzazione di D con imagesc
            f->nexttile(curr_tile);
            auto ax = f->current_axes();
            ax->title("Matrice D (differenze finite in avanti)");
            ax->imagesc(D); // questo e' a posto, colorbar() no (ma neanche in es1_campionamento)
            colorbar(); // ne inserisce uno per pigrizia ma devo ancora vedere come si inserisce con metodo.
            //  show();
            curr_tile++;

            // 3) Campionamento di u(t) = arctan(t)
            Vec x_eq = nodi_equidistanti(a, b, n);
            Vec u = vector_campiona_f(x_eq, f_atan); 

            // 4) Applicazione di D: Du ≈ u'(t)
            Vec Du = matrix_calcola_deriv_bymatr(D, u);

            // 4a) Applicazione di D: Du ≈ u'(t)
            Vec Du_sm = matrix_calcola_deriv_byiter(u, a, b);  // ok sono sovrapposte

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
            matplot_legend_align(matplot::legend(), LeAl::Bottom+LeAl::Right, 0.0, 0.0);

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
            clear_screen();
            redo = false;

            // Costruzione del sistema triangolare L x = b
            matrix_build_cauchy_sep(n, t0, T, x0, t, fvals, L, b, f_x, false);
            vector_dump(b, 10, 50, " b = t"); 
            // Risoluzione con sostituzione in avanti
            Vec x = linear_subst_FW(L, b);
            vector_dump(x, 10, 50, " x da Lx = h*t"); 

            auto fig = matplot_table_init(true, "Cauchy", "Soluzione numerica del problema di Cauchy", 1, 1);
            
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

            matrix_build_cauchy_sep(n, t0, T, x0, t, fvals, L, b, f_sin, false);
            vector_dump(b, 10, 50, " b = sin(t) "); 
            // Risoluzione con sostituzione in avanti
            x = linear_subst_FW(L, b);
            vector_dump(x, 10, 50, " x da Lx = h*sin(t) "); 

            fig->nexttile(2);
            p = plot(t, x);
            p->line_width(2);
            p->display_name("x(t)");
            xlabel("t");
            ylabel("sin(t)");

            matrix_build_cauchy_sep(n, t0, T, x0, t, fvals, L, b, f_cos, false); 
            vector_dump(b, 10, 50, " b = cos(t) "); 
            // Risoluzione con sostituzione in avanti
            x = linear_subst_FW(L, b);
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

            clear_screen();
            // Costruzione del sistema triangolare L x = b
            costruisci_sistema_bordo(n, t0, T, x0, x1, t, fvals, A, b);
            vector_dump(fvals, 10, fvals.size(), "fvals");
            vector_dump(b, 10, b.size(), "b");

            // matrix_dump(A, "tridiag");

            auto ffig = matplot_table_init(true, "Bordo", "Costruzione matrice A e fattorizzazione LU", 2, 2);
            
            // Verifica della matrice A tridiag

            ffig->nexttile(0);
            title("Matrix A");
            imagesc(A); // ->display_name("matrice A");
            // matplot::legend();
            colorbar();

            
            // Risoluzione con fattorizzazione LU
            linear_LU_dec(A, L, U);

            // matrix_dump(L, "lower");
            // matrix_dump(U,  "upper");

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
            y = linear_subst_FW(L, b);
            vector_dump(y, 10, y.size(), "y");

            // Sostituzione indietro per U x = y
            x = linear_subst_BW(U, y);
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
            clear_screen();
            double h = h_ticks(a,b,N); // passo di campionamento

            Vec fvals(N, 0.0);                  // qui abbiamo 1/(1+x^2) a integrare, campionata in N punti equispaziati tra a e b
            Vec t(N, 0.0);                     // vettore dei nodi di campionamento
            for (int i = 0; i < N; ++i) {
                double x_i = a + i * h;
                fvals[i] = f_atan_d(x_i);
                t[i] = x_i;
            }
            // vector_dump(fvals, 10, fvals.size(), "fvals = 1/(1+x^2) campionata in N punti");

            Mat T = matrix_build_triang(N); // costruisce matrice triangolare di dimensione N x N
            T = matrix_prodotto_coeff(T, h); // moltiplica per h
            // matrix_dump(T, "Triangolare");

            Vec v_atan = matrix_prodotto_vector(T, fvals); // prodotto matrice vettore per ottenere stima integrale
            vector_dump(v_atan, 10, v_atan.size(), "T * fvals = stima integrale per somma discreta");
            v_atan = vector_shift(v_atan, std::atan(a)); // shift di arcotg(a) per centratura
            vector_dump(v_atan, 10, v_atan.size(), "T * fvals = stima integrale riposizionata");
        
            // Mat T_inv_sint = matrix_build_triang_inv(10);
            // matrix_dump(T_inv_sint, "Inversa sintetica");

            Mat T_inv = linear_LU_inversa (T);
            //matrix_dump(T_inv, "Inversa con LU");
            double NFT = matrix_calcola_norma(-1, T);
            double NFT_inv = matrix_calcola_norma(-1, T_inv);
            double N1T = matrix_calcola_norma(1, T);
            double N1T_inv = matrix_calcola_norma(1, T_inv);   
            double NIT = matrix_calcola_norma(0, T);
            double NIT_inv = matrix_calcola_norma(0, T_inv); 
            double N2T = matrix_calcola_norma(2, T);
            double N2T_inv = matrix_calcola_norma(2, T_inv);   
            
            Vec v_atan_d = matrix_prodotto_vector(T_inv, vector_shift(v_atan, -std::atan(a))); // derivata corrispondente, con backshift di centratura
            Vec v_atan_dcn = matrix_prodotto_vector(T_inv, v_atan); // derivata analiticamente corretta ma sbagliata per cn 
            vector_dump(v_atan_d, 10, v_atan_d.size(), "T * v_atan = stima derivata simmetrica");
            vector_dump(v_atan_dcn, 10, v_atan_dcn.size(), "T * v_atan = stima derivata analitica");
            
            // Mat I = matrix_prodotto_matrix(T, T_inv);
            // matrix_dump(I, "T * T_inv");  

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

            figure_handle f = matplot_table_init(true, "Integrazione e derivazione", "da 1/(1+x^2) ad atan e viceversa", 2,2);
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
 

            matplot_legend_align(matplot::legend(), LeAl::VCenter+LeAl::Left, 0,0);
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
 

            matplot_legend_align(matplot::legend(), LeAl::Bottom+LeAl::Right, 0,0);
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

        clear_screen();
        while (!leave) {
            bool redo = false;

            std::cout << "\n=== Norma 2 e numero di condizionamento spettrale ===\n";
            
            const int N_values[] = {10, 20, 40, 80, 160, 320, 640};
            const int n_tests = 7;
            
            std::vector<double> x_vals, cond_2_vals, cond_12_vals, cond_22_vals, cond_f_vals, cond_1_vals, cond_I_vals;
            
            std::cout << std::setfill(' ')            
                << std::setw(6) << "N" 
                << std::setw(12) << "norm2(A)"
                << std::setw(12) << "t2(A) 1R"
                << std::setw(12) << "norm12(A)"
                << std::setw(12) << "t12(A) AllR"
                << std::setw(12) << "norm22(AtA)"
                << std::setw(12) << "t22(AtA)"
                << std::setw(12) << "norm2(A^-1)"
                << std::setw(12) << "kappa2"
                << std::setw(12) << "kappa2/N^2"
                << std::setw(12) << "kappaF"
                << std::setw(12) << "kappa1"
                << std::setw(12) << "kappaI\n";
            
            for (int n = 0; n < n_tests; ++n) {
                int N = N_values[n];
                double h = h_ticks(a,b,N); // passo di campionamento
            
                Mat A = matrix_build_triang(N);
                A = matrix_prodotto_coeff(A, h); // moltiplica per h
                Mat A_inv = linear_LU_inversa(A);

                preriscaldamento_anti_overhead(A); // warm-up per taratura cronometrica a strutture fatte
                
                // Norma 2 spettrale power con economia
                auto t_start = std::chrono::steady_clock::now();
                double norm2_A = matrix_calcola_norma(2, A);
                auto t_end = std::chrono::steady_clock::now();
                auto time2_A = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
                double norm2_Ainv = matrix_calcola_norma(2, A_inv);
                double kappa2 = norm2_A * norm2_Ainv;
                
                // Norma 2 spettrale power senza economia
                t_start = std::chrono::steady_clock::now();
                double norm12_A = matrix_calcola_norma(12, A);
                t_end = std::chrono::steady_clock::now();
                auto time12_A = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
                double norm12_Ainv = matrix_calcola_norma(12, A_inv);
                double kappa12 = norm12_A * norm12_Ainv;

                // Norma 2 spettrale power + Rayleigh
                t_start = std::chrono::steady_clock::now();
                double norm22_A = matrix_calcola_norma(22, A);
                t_end = std::chrono::steady_clock::now();
                auto time22_A = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
                double norm22_Ainv = matrix_calcola_norma(22, A_inv);
                double kappa22 = norm22_A * norm22_Ainv;

              
                // Norma Frobenius
                double normF_A = matrix_calcola_norma(-1, A);
                double normF_Ainv = matrix_calcola_norma(-1, A_inv);
                double kappaF = normF_A * normF_Ainv;
                
                // Norma 1 = max somma colonne
                double norm1_A = matrix_calcola_norma(1, A);
                double norm1_Ainv = matrix_calcola_norma(1, A_inv);
                double kappa1 = norm1_A * norm1_Ainv;
                
                // Norma inf1 = max somma righe
                double normI_A = matrix_calcola_norma(0, A);
                double normI_Ainv = matrix_calcola_norma(0, A_inv);
                double kappaI = normI_A * normI_Ainv;
                
                std::cout << std::setw(6) << N
                        << std::setw(12) << std::fixed << std::setprecision(2) << norm2_A
                        << std::setw(12) << time2_A
                        << std::setw(12) << norm12_A
                        << std::setw(12) << time12_A
                        << std::setw(12) << norm22_A
                        << std::setw(12) << time22_A
                        << std::setw(12) << norm2_Ainv
                        << std::setw(12) << kappa2
                        << std::setw(12) << kappa2 / (N*N)
                        << std::setw(12) << kappaF
                        << std::setw(12) << kappa1
                        << std::setw(12) << kappaI << "\n";  

                x_vals.push_back(N);
                cond_2_vals.push_back(kappa2);
                cond_12_vals.push_back(kappa12);
                cond_22_vals.push_back(kappa22);
                cond_f_vals.push_back(kappaF);
                cond_1_vals.push_back(kappa1);
                cond_I_vals.push_back(kappaI);

            }

            std::string title = "Numeri di condizionamento vs N";
            figure_handle f = matplot_table_init(true, "Confronto Norme", title, 1,1);
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

            matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left,0,0);

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
            Mat K = matrix_build_gausskernel(N, sigma*h, h, KG_Normal, kappa2[0]);
            Mat K2 = matrix_build_gausskernel(N, sigma*2*h, h, KG_Normal, kappa2[1]);
            Mat K4 = matrix_build_gausskernel(N, sigma*4*h, h, KG_Normal, kappa2[2]);
            Mat K8 = matrix_build_gausskernel(N, sigma*8*h, h, KG_Normal, kappa2[3]);

            // matrix_dump(K, "Matrice Kernel di Gauss");

            Vec ws0 = vector_build_segnale_finestra(N,(1./4.),(3./4.),1);
            Vec ws1 = matrix_prodotto_vector(K,ws0);
            Vec ws2 = matrix_prodotto_vector(K2,ws0);
            Vec ws4 = matrix_prodotto_vector(K4,ws0);
            Vec ws8 = matrix_prodotto_vector(K8,ws0);
            cout << endl << endl;
            Vec fs0 = linear_LU_risolve_sistema(K,ws0);
            Vec fs1 = linear_LU_risolve_sistema(K,ws1);
            Vec fs2 = linear_LU_risolve_sistema(K2,ws2);
            Vec fs4 = linear_LU_risolve_sistema(K4,ws4);
            Vec fs8 = linear_LU_risolve_sistema(K8,ws8);
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

            {   
                // namespace anonimo per heatmap 

                figure_handle f = matplot_table_init(true, "Deconvoluzione", "Kernel di Gauss " + KGN_title+ "a diverse StD base "+std::to_string(sigma), 2,2);
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

            {   
                // namespace anonimo per plot deconvoluzioni
    

                Vec x_vals = nodi_equidistanti(a,b,N);
                std::string title = "Trasformazioni ("+KGN_title+") a 1x, 2x, 4x StD base ";

                figure_handle f = matplot_table_init(true, "Deconvoluzione", title + std::to_string(sigma*h), 2, 2);

                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);

                f->nexttile(0);
                auto ax = f->current_axes();
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
            
            {
                // namespace anonimo per test di condizionamento al rumore

                /* 
                // vector_dump(ws0, 10, ws0.size(), "Ingresso pulito");
                // vector_dump(vector_add_noise(ws0, 1), 10, ws0.size(), "Perturbato 0.1%");
                // vector_dump(vector_add_noise(ws0, 10), 10, ws0.size(), "Perturbato 1%");
                */
            
                Vec ws2_r001 = matrix_prodotto_vector(K, vector_add_noise(ws0,0.01));
                Vec ws2_r01 = matrix_prodotto_vector(K, vector_add_noise(ws0,0.1));
                Vec ws2_r05 = matrix_prodotto_vector(K, vector_add_noise(ws0,1));
                Vec ws2_r2 = matrix_prodotto_vector(K, vector_add_noise(ws0,10));
                Vec fs2_r001 = linear_LU_risolve_sistema(K,ws2_r001);
                Vec fs2_r01 = linear_LU_risolve_sistema(K,ws2_r01);
                Vec fs2_r05 = linear_LU_risolve_sistema(K,ws2_r05);
                Vec fs2_r2 = linear_LU_risolve_sistema(K,ws2_r2);
                
                Vec x_vals = nodi_equidistanti(a,b,N);
                std::string title = "Perturbazioni ("+KGN_title+") a 1x StD base ";


                figure_handle f = matplot_table_init(true, "Deconvoluzione perturbata", title + format_numstr(sigma), 1, 1);

                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);

                f->nexttile(0);
                auto ax = f->current_axes();
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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
                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Left, 0,0);
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

    void f3_svd_test(){
        bool leave = false;
        clear_screen();
        int n = 6, d = 5;

        while (!leave) 
        {
            clear_screen();
            {
                Vec v = {3.0, 1.0, -2.0};
                Vec w = vector_build_householder_bycol(v);

                // Calcola H*v esplicitamente per verifica
                // H*v = v - 2*w*(w^T*v)
                double wtv = 0; for (int i = 0; i < 3; ++i) wtv += w[i]*v[i];
                Vec Hv(3);
                for (int i = 0; i < 3; ++i) Hv[i] = v[i] - 2*w[i]*wtv;

                printf("Verifica Householder colonna:\n");
                printf("  v = [%.3f, %.3f, %.3f]\n", v[0], v[1], v[2]);
                printf("  H*v = [%.6f, %.6f, %.6f]\n", Hv[0], Hv[1], Hv[2]);
                printf("  atteso: [%.6f, 0, 0]\n\n", -std::sqrt(14.0));
                cout << std::string(80, '_') << endl;
            }

            // ── Verifica TODO 3: bidiagonalizzazione su matrice piccola ──────────────
            {
                // Matrice 4x5 casuale
                std::mt19937 rng(42);
                std::normal_distribution<double> g(0.0, 1.0);
                Mat X = matrix_build_zero(n, d);
                for (auto& r : X) for (double& v : r) v = g(rng);

                Mat U0, B, V0;
                trmatrix_bidiagonalizza(X, U0, B, V0, bidiag_mode, false);
                Mat V0_t = matrix_trasposta(V0);
                Mat In = matrix_build_Id(n);
                Mat Id = matrix_build_Id(d);
                Mat U0_U0t = matrix_prodotto_matrix(U0, matrix_trasposta(U0));
                Mat V0_V0t = matrix_prodotto_matrix(V0, V0_t);
                matrix_dump(U0, "U0 - Ortogonale?");
                matrix_test_ortogonale(U0_U0t, "U0 * U0_t - test ortogonalita'");
                matrix_dump(B, "B - Bidiagonale");
                matrix_dump(V0, "V0 - Ortogonale?");
                matrix_dump(V0_t, "V0_t - Ortogonale trasposta");
                matrix_test_ortogonale(V0_V0t, "V0 * V0_t - test ortogonalita'");
                //
                // Verifiche
                //

                // TODO 4: verifica ||X - U0*B*V0^T||_F
                Mat X0 = matrix_prodotto_matrix(U0, matrix_prodotto_matrix(B, V0_t));
                matrix_dump(X, "X - Originale");
                matrix_dump(X0, "X - Ricostruita U0 * (B * V0^T)");
                Mat R = matrix_calcola_diff(X,X0);
                matrix_dump(R, "R - Differenza X - U0 * B * V0^T");

                printf(" Error_F_lab ||X - U0*B*V0^T||_F = %.2e\n", matrix_calcola_errore_Fr(X, X0));
                printf(" Error_F_mia ||X - U0*B*V0^T||_F = %.2e\n\n", matrix_calcola_norma(-1, R));
                printf(" Error_F_lab ||In - U0*U0^T ||_F = %.2e\n", matrix_calcola_errore_Fr(In, U0_U0t));
                printf(" Error_F_lab ||Id - V0*V0^T ||_F = %.2e\n\n", matrix_calcola_errore_Fr(Id, V0_V0t));
           
            // ── TODO 5: norma spettrale dell'errore di bidiagonalizzazione ───────────
            
                // Dopo aver implementato norma_spettrale e matmat:
                printf(" Norma spettrale PM econ ||R||_2 = %.2e\n",  matrix_calcola_norma(2,R));
                printf(" Norma spettrale PM lab. ||R||_2 = %.2e\n",  matrix_calcola_norma(12,R));
                printf(" Norma spettrale PM rayl ||R||_2 = %.2e\n\n",  matrix_calcola_norma(22,R));
                //  test condizione di errore: Mat E = matrix_calcola_diff(U0, V0);
           }
 
            bool redo = false;
            while (!redo && !leave) {

                std::cout << "Numero di righe della matrice di test = n [3..20] (<q> per uscire) > " ;
                if (std::cin >> n) { if (n>=3 && n<=20) redo=true;} else {cout << "Key: " << n <<endl; leave=true;} ;
                if (redo) {
                    std::cout << " numero di colonne = d da n a "<< 30 <<" (<q> per uscire) > " ;
                    if (std::cin >> d) { if (d >=n && d<=30) redo=true;} else {cout << "Key: " << d <<endl; leave=true;} ;
                }
                cin_clear();
            };
       }
    }
    
    void f3_svd(){
        bool leave = false;
        int n = 6, d = 6;

        while (!leave) 
        {
            clear_screen();
            // ── TODO 6: SVD completa ─────────────────────────────────────────────────
            {
                std::mt19937 rng(7);
                std::normal_distribution<double> g(0.0, 1.0);
                Mat X = matrix_build_zero(n, d);
                for (auto& r : X) for (double& v : r) v = g(rng);

                Vec testv; 
                Mat VJ;
                linear_jacobi_autoval_simmetrica(matrix_prodotto_AtA(X, true), testv, VJ);
                vector_dump(testv, 10, testv.size(), "Autovalori di Xt_X");
                matrix_dump(VJ, "Matrice V di autovettori (dx)");

                cout << "\n" << std::string(80, '-')<< endl;
                cout << "*** Fine collaudo jacobi: teniamo gli autovalori di XtX per verificare i SV di X  ***\n\n";
                cout << std::string(80, '-')<< endl << endl;


                Mat U0, B, V0;
                trmatrix_bidiagonalizza(X, U0, B, V0, bidiag_mode, false); // ora l'arg optional e' il sesto (debug)

                // TODO 4: verifica ||X - U0*B*V0^T||_F

                Mat V0_t = matrix_trasposta(V0);
                Mat X0 = matrix_prodotto_matrix(matrix_prodotto_matrix(U0, B), V0_t);
                // matrix_dump(X, "X Originale");
                // matrix_dump(X0, "X = U0 * B * V0_t Ricostruita");
                Mat R = matrix_calcola_diff(X,X0);


                printf(" Error_F_lab         ||X - U0*B*V0^T||_F = %.2e\n", matrix_calcola_errore_Fr(X, X0));
                printf(" Error_F_mia         ||X - U0*B*V0^T||_F = %.2e\n", matrix_calcola_norma(-1, R));
                printf(" Norma spettrale err ||X - U0*B*V0^T||_2 = %.2e\n\n", matrix_calcola_norma(2, R));

                cout << "\n" << std::string(80, '-')<< endl;
                cout << "*** Fine collaudo bidiagonalizza: teniamo U0 e V0 per dopo e diamo B a SVD per avere Ub e Vb ***\n\n";
                cout << std::string(80, '-')<< endl << endl;

                Mat Ub, Vb_t;
                Vec sigma; // s minuscolo vettore, poi S maiuscolo matrice diagonale

                matrix_dump(B, "B All'entrata di SVD");
/*

                //
                // collaudo trmatrix_SVDQR_ridotta
                //

                Mat Vbred;
                trmatrix_SVDQR_ridotta(B, Ub, sigma, Vbred);
                matrix_dump(B, "B All'entrata di SVD");

                matrix_dump(Ub, "Ub");
                matrix_ortogonalizza_GSmod(Ub);
                matrix_dump(Ub, "Ub_ortonormalizzata GS");
                matrix_dump(Vbred, "Vbred");
                Mat Vbred_t = matrix_trasposta(Vbred);
                Mat Ubt_Ub = matrix_prodotto_matrix(matrix_trasposta(Ub), Ub);
                matrix_dump(Ubt_Ub, "Ubt_Ub");
                Mat Sigma_red = vector_to_matrix_diag(sigma, sigma.size(),sigma.size());
                matrix_dump(Sigma_red, "Sigma_red");
                Mat Vbredt_Vbred = matrix_prodotto_matrix(Vbred_t, Vbred);
                matrix_dump(Vbredt_Vbred, "Vbredt_Vbred");
                Mat B_rebuilt = matrix_prodotto_matrix(Ub, matrix_prodotto_matrix(Sigma_red, Vbred_t));
                matrix_dump(B_rebuilt, ,"B_rebuilt");
                Mat Rred = matrix_calcola_diff(B, B_rebuilt);
                matrix_dump(Rred, "Rred differenze su B");
                printf(" Error_F_lab         ||B - Ub*B*Vbred^T||_F = %.2e\n", matrix_calcola_errore_Fr(B, B_rebuilt));
                printf(" Error_F_mia         ||X - Ub*B*Vbred^T||_F = %.2e\n", matrix_calcola_norma(-1, Rred));
                printf(" Norma spettrale err ||X - Ub*B*Vbred^T||_2 = %.2e\n\n", matrix_calcola_norma(2, Rred));

                Mat X_rebuilt = matrix_prodotto_matrix(U0, matrix_prodotto_matrix(B_rebuilt, V0_t));
                matrix_dump(X_rebuilt, "X_rebuilt");
                Rred = matrix_calcola_diff(X, X_rebuilt);
                matrix_dump(Rred, "Rred differenze su X");
                printf(" Error_F_lab         ||X - U0*B_rebuilt*V0^T||_F = %.2e\n", matrix_calcola_errore_Fr(B, B_rebuilt));
                printf(" Error_F_mia         ||X - U0*B_rebuilt*V0^T||_F = %.2e\n", matrix_calcola_norma(-1, Rred));
                printf(" Norma spettrale err ||X - U0*B_rebuilt*V0^T||_2 = %.2e\n\n", matrix_calcola_norma(2, Rred));

                cout << "\n" << std::string(80, '-')<< endl;
                cout << "*** Fine collaudo svd_bidiag ridotta nostrana  ***\n\n";
                cout << std::string(80, '-')<< endl << endl;

                //
                //  fine collaudo ridotta
                //
*/
                
                svd_bidiagonale(B, Ub, Vb_t, sigma);  // questa e' la versione alglib e rende Vb_t
                Mat Vb = matrix_trasposta(Vb_t);

                // trmatrix_SVDQR(B, Ub, Vb, sigma);   // questa era la versione provvisoria e rende Vb

                matrix_dump(Ub, "Ub");
                Mat Sigma = vector_to_matrix_diag(sigma, Ub[0].size(), Vb[0].size()); 
                matrix_dump(Sigma, "Sigma");
                matrix_dump(Vb, "Vb");
                matrix_dump(Vb_t, "Vb_t");

                cout << "\n" << std::string(80, '-')<< endl;
                cout << ">>                 Prodotti della SVD\n";
                cout << std::string(80, '-')<< endl << endl;

                Mat Ub_S = matrix_prodotto_matrix(Ub, Sigma);
                Mat Ub_S_Vbt = matrix_prodotto_matrix(Ub_S, Vb_t);
                matrix_dump(Ub_S_Vbt, "B = Ub_S_Vbt");
                matrix_dump(B, "B Originale");

                Mat RB = matrix_calcola_diff(B , Ub_S_Vbt);
                printf(" Error_F_mia || B - Ub * (Sigma * Vb_t)||_F = %.2e\n\n", matrix_calcola_norma(-1, RB));
                matrix_dump(RB, "Matrice differenza B = B - Ub * (Sigma * Vb_t)");

                // TODO: U = U0 * Ub,  V = V0 * Vb
                Mat U = matrix_prodotto_matrix(U0, Ub);
                matrix_dump(U, "U = U0 * Ub");

                //Mat Vb_red = matrix_estende_ridotta(Vb, sigma.size(), true); // true limita le colonne
                // matrix_dump(Vb_red, "Vb Ridotta alle col di Sigma");
                Mat V = matrix_prodotto_matrix(V0, Vb);
                Mat V_t = matrix_prodotto_matrix(Vb_t, V0_t);
                matrix_dump(V_t, "V_T = Vb_t * V0_t");

                Mat IU = matrix_prodotto_matrix(U, matrix_trasposta(U));
                Mat IV = matrix_prodotto_matrix(V, matrix_trasposta(V));
                matrix_dump(IU, "U * U_t = I"+std::to_string(n));
                matrix_dump(IV, "V * V_t = I"+std::to_string(d));
                cout << "\n" << std::string(80, '-')<< endl;
                cout << ">>                 Test di ortogonalita'\n";
                cout << std::string(80, '-')<< endl << endl;


                Mat U_S = matrix_prodotto_matrix(U, Sigma);
                Mat U_S_Vt = matrix_prodotto_matrix(U_S, V_t);
                matrix_dump(X, "X");
                matrix_dump(U_S_Vt, "U * Sigma * Vt");


                // TODO: verifica ||X - U*Sigma*V^T||_F, ||U^T*U - I||_F, ||V^T*V - I||_F

                Mat RX = matrix_calcola_diff(X, U_S_Vt);
                matrix_dump(RX, "Differenze X - U Sigma V^t");

                Mat RU = matrix_calcola_diff(matrix_build_Id(U.size()), matrix_prodotto_matrix(U, matrix_trasposta(U)));
                Mat RV = matrix_calcola_diff(matrix_build_Id(V.size()), matrix_prodotto_matrix(V, V_t));
                printf(" Error_F_mia ||X - U*Sigma*V^T||_F = %.2e\n", matrix_calcola_norma(-1, RX));
                printf(" Error_F_mia ||U^T*U - I||_F = %.2e\n", matrix_calcola_norma(-1, RU));
                printf(" Error_F_mia ||V^T*V - I||_F = %.2e\n\n", matrix_calcola_norma(-1, RV));

                // matrix_dump(RX, "Matrice differenza RX = X - U * Sigma * V_t");

                // TODO: stampa valori singolari

                std::cout << "Valori singolari sigma (attesi in ordine decrescente):\n";
                for (double s : sigma) printf("  %.6f\n", s);

                trmatrix_test_sv_autoval(testv, sigma);
            }


        

            bool redo = false;
            while (!redo && !leave) {

                std::cout << "Numero di righe della matrice per SVD = n [3..100] (<q> per uscire) > " ;
                if (std::cin >> n) { if (n>=3 && n<=100) redo=true;} else {cout << "Key: " << n <<endl; leave=true;} ;
                if (redo) {
                    std::cout << " numero di colonne = d [3..100] (<q> per uscire) > " ;
                    if (std::cin >> d) { if (d >=3 && d<=100) redo=true;} else {cout << "Key: " << d <<endl; leave=true;} ;
                }
                cin_clear();
            };
       }
    }

    void f3_svd_pca(){
        bool leave = false;
        int n = 100, d = 10; 

        while (!leave) 
        {
            // ── TODO 7: PCA e scatter plot ────────────────────────────────────────────
            clear_screen();
            // primo giro prende n come base ed a pari d plotta 4 scatter nx1, nx2, nx4, nx8 
            // secondo giro prende d come base ed a pari n plotta 4 scatter dx1, dx2, dx4, dx8 
            bool first_run = true;
            for (int itab = 0; itab < 2; itab++) 
            {
                auto fig = matplot_table_init(true, "PCA", "PCA — prime due componenti principali", 2, 2 );

                for (int ik = 0; ik < 4; ik++) 
                {
                    int npoints;
                    int ncols;

                    if (first_run) {
                        npoints = n * pow(2, ik);
                        ncols = d ;
                    } else{
                        npoints = n;
                        ncols = d  * pow(2, ik);
                    }
                    cout << "in generazione cluster da " << npoints << " X " << ncols << " punti\n";

                    auto [X, label] = genera_cluster(npoints, ncols, /*seed=*/42);
                    // a) Centrare X: sottrai la media di ogni colonna

                    // TODO: calcola mean_j = sum_i X[i][j] / n
                    //       Xtilde[i][j] = X[i][j] - mean_j
                    Vec avg_col(X[0].size(), 0.0);
                    Vec avg_row(X.size(), 0.0);
                    matrix_calcola_media(X, avg_col, avg_row);
                    // vector_dump(avg_col, 10, avg_col.size(), "Medie in colonna");
                    // vector_dump(avg_row, 10, avg_row.size(), "Medie in riga");
                    Mat Xtilde = matrix_centra_su_media(X, avg_col);
                    // matrix_dump(X, "Matrice originale");
                    // matrix_dump(Xtilde, "Matrice centrasta");

                    // b) SVD di Xtilde

                    Mat U0, B, V0;
                    trmatrix_bidiagonalizza(Xtilde, U0, B, V0, bidiag_mode, false);
                    Mat Ub, Vb_t; Vec sigma;
                    svd_bidiagonale(B, Ub, Vb_t, sigma);
                    Mat Vb = matrix_trasposta(Vb_t);

                    Mat U = matrix_prodotto_matrix(U0, Ub);
                    Mat V = matrix_prodotto_matrix(V0, Vb);
                    // matrix_test_ortogonale(V, "V");
                    // vector_dump(sigma, 10, sigma.size(), "sigmas al giro "+ std::to_string(ik+1));

                    // c) Coordinate PCA: pc_k(i) = sum_j Xtilde[i][j] * V[j][k]
                    // Mat pc1(4, Vec(npoints, 0.0));
                    // Mat pc2(4, Vec(npoints, 0.0));
                    Vec pc1(npoints, 0.0);
                    Vec pc2(npoints, 0.0);
                    // for (int i = 0; i < n; i++)
                        // for (int j = 0; j < d; j++) {
                        //    pc1[ik][i] += Xtilde[i][j] * V[j][1];
                        //    pc2[ik][i] += Xtilde[i][j] * V[j][2];
                        // }
                    for (int i =0; i < npoints; i++) {
                        pc1[i] += U[i][1] * sigma[1];
                        pc2[i] += U[i][2] * sigma[2];
    
                    }
                    //    (equivalente a U[i][k] * sigma[k]) // ok X_v_i = sigma_i_u_i

                    // TODO

                    // d) Scatter plot colorato per etichetta
                    Vec x0, y0, x1, y1;
                    for (int i = 0; i < npoints; ++i) {
                        if (label[i] == 0) { x0.push_back(pc1[i]); y0.push_back(pc2[i]); }
                        else               { x1.push_back(pc1[i]); y1.push_back(pc2[i]); }
                    }
                    fig->nexttile(ik);
                    auto ax = fig->current_axes();
                    ax->hold(on);
                    ax->legend();
                    ax->scatter(x0, y0)->marker_color({0,0,1}).display_name("Cluster 0");
                    ax->scatter(x1, y1)->marker_color({1,0,0}).display_name("Cluster 1");
                    ax->xlabel("PC 1 a punti " + std::to_string(npoints) + " X " + std::to_string(ncols));
                    ax->ylabel("PC 2");


                }
                fig->draw();
                first_run = !first_run;
            }   
            // e) Prova con d variabile: {2, 5, 20, 50}
            // f) Stampa sigma[0], sigma[1] vs il resto: cosa noti?


        

            bool redo = false;
            while (!redo && !leave) {

                std::cout << "Numero di righe della matrice per PCA = n [3..500] (<q> per uscire) > " ;
                if (std::cin >> n) { if (n>=3 && n<=500) redo=true;} else {cout << "Key: " << n <<endl; leave=true;} ;
                if (redo) {
                    std::cout << " numero di colonne = d da n a "<< 500 <<" (<q> per uscire) > " ;
                    if (std::cin >> d) { if (d >=n && d<=500) redo=true;} else {cout << "Key: " << d <<endl; leave=true;} ;
                }
                cin_clear();
            };
       }
    }

    void f3_svd_an(){
        bool leave = false;
        
        while (!leave) 
        {
			int n = 200;
			vector<int> d = {4, 8, 40, 80};
			
            // ── TODO 7: PCA e scatter plot ────────────────────────────────────────────
            
            clear_screen();
            bool first_run = true;
			auto fig = matplot_table_init(true, "PCA", "PCA — prime due componenti principali", 2, 2 );

			for (int ik = 0; ik < 4; ik++) 
			{
				int npoints;
				int ncols;

				npoints = n;
				ncols = d[ik];

				cout << "in generazione cluster da " << npoints << " X " << ncols << " punti\n";

				auto [X, label] = genera_cluster(npoints, ncols, /*seed=*/42);
				// a) Centrare X: sottrai la media di ogni colonna

				// TODO: calcola mean_j = sum_i X[i][j] / n
				//       Xtilde[i][j] = X[i][j] - mean_j
				Vec avg_col(X[0].size(), 0.0);
				Vec avg_row(X.size(), 0.0);
				matrix_calcola_media(X, avg_col, avg_row);
				// vector_dump(avg_col, 10, avg_col.size(), "Medie in colonna");
				// vector_dump(avg_row, 10, avg_row.size(), "Medie in riga");
				Mat Xtilde = matrix_centra_su_media(X, avg_col);
				// matrix_dump(X, "Matrice originale");
				// matrix_dump(Xtilde, "Matrice centrasta");

				// b) SVD di Xtilde

				Mat U0, B, V0;
				trmatrix_bidiagonalizza(Xtilde, U0, B, V0, bidiag_mode, false);
				Mat Ub, Vb_t; Vec sigma;
				svd_bidiagonale(B, Ub, Vb_t, sigma);
                Mat Vb = matrix_trasposta(Vb); // raddrizzo perche' alglib rende Vb_t   
				Mat U = matrix_prodotto_matrix(U0, Ub);
				Mat V = matrix_prodotto_matrix(V0, Vb);
				// matrix_test_ortogonale(V, "V");
				vector_dump(sigma, 10, sigma.size(), "sigmas al giro "+ std::to_string(ik+1));

				// c) Coordinate PCA: pc_k(i) = sum_j Xtilde[i][j] * V[j][k]
				// Mat pc1(4, Vec(npoints, 0.0));
				// Mat pc2(4, Vec(npoints, 0.0));
				Vec pc1(npoints, 0.0);
				Vec pc2(npoints, 0.0);
				// for (int i = 0; i < n; i++)
					// for (int j = 0; j < d; j++) {
					//    pc1[ik][i] += Xtilde[i][j] * V[j][1];
					//    pc2[ik][i] += Xtilde[i][j] * V[j][2];
					// }
				for (int i =0; i < npoints; i++) {
					pc1[i] += U[i][1] * sigma[1];
					pc2[i] += U[i][2] * sigma[2];
					
				}
				//    (equivalente a U[i][k] * sigma[k]) // ok X_v_i = sigma_i_u_i

				// TODO
				cout << std::setw(12) << std::fixed << std::setprecision(2) 
					<< "matrice " << npoints << "_X_" << ncols << " sigma_1 " 
					<< sigma[0] << " sigma_2 " << sigma[1] 
					<< " S_1/_2 ratio " << sigma[0]/sigma[1] << endl;


				// d) Scatter plot colorato per etichetta
				Vec x0, y0, x1, y1;
				for (int i = 0; i < npoints; ++i) {
					if (label[i] == 0) { x0.push_back(pc1[i]); y0.push_back(pc2[i]); }
					else               { x1.push_back(pc1[i]); y1.push_back(pc2[i]); }
				}
				fig->nexttile(ik);
				auto ax = fig->current_axes();
				ax->hold(on);
				ax->legend();
				ax->scatter(x0, y0)->marker_color({0,0,1}).display_name("Cluster 0");
				ax->scatter(x1, y1)->marker_color({1,0,0}).display_name("Cluster 1");
				ax->xlabel("PC 1 a punti " + std::to_string(npoints) + " X " + std::to_string(ncols));
				ax->ylabel("PC 2");


                fig->draw();
            }   
            // e) Prova con d variabile: {2, 5, 20, 50}
            // f) Stampa sigma[0], sigma[1] vs il resto: cosa noti?


        

            bool redo = false;
            /* 
            while (!redo && !leave) {

                std::cout << "Numero di righe della matrice per PCA = n [3..500] (<q> per uscire) > " ;
                if (std::cin >> n) { if (n>=3 && n<=500) redo=true;} else {cout << "Key: " << n <<endl; leave=true;} ;
                if (redo) {
                    std::cout << " numero di colonne = d da n a "<< 500 <<" (<q> per uscire) > " ;
                    if (std::cin >> d) { if (d >=n && d<=500) redo=true;} else {cout << "Key: " << d <<endl; leave=true;} ;
                }
                cin_clear();
            };
            */ 
            wait_return_to_menu(false);
            wait_return_to_menu(false);
            leave = true;
       }
    }

    void sym_cauchy() {
        int n = 100;          // numero di punti della griglia
        double t0 = -PI;     // tempo iniziale
        double T  = PI;     // tempo finale
        double x0 = -1.0;     // condizione iniziale x(t0)
        double a_arg = 1;
        double f_arg = 0;

        Vec t, avals, fvals, b;
        Mat L;
        
        bool leave = false;
        bool redo = false;
        while (!leave) {
            clear_screen();
            redo = false;
            auto fig = matplot_table_init(true, "Cauchy", "Soluzione numerica del problema di Cauchy", 2, 2);            
            matplot::legend();
            Vec x;
            {
                // Costruzione del sistema triangolare L x = b
                // matrix_build_cauchy_int(n, t0, T, x0, t, fvals, L, b, f_sin2plus1, true);
                matrix_build_cauchy( n, t0, T, x0, t, avals, fvals, L, b, f_const, 0, f_sin2plus1, 0, true) ;
                // vector_dump(b, 10, b.size(), " b = 1/(1+sin^2) "); 
                x = linear_subst_BW(L, b);
                // vector_dump(x, 10, x.size(), " x da Lx = b"); 

                fig->nexttile(0);
                imagesc(L)->display_name("Verifica matrice");
                colorbar();
            }

            {        
                // Visualizzazione della soluzione numerica x(t)
                fig->nexttile(1);
                auto p = plot(t, x);
                p->line_width(2);
                p->display_name("1 / (1 + sin^2(t))");
                xlabel("t");
                ylabel("f(x(t))");
                auto ax=fig->current_axes();
                // ax->y_axis().limits({-1.00e-1, 1.00e-1 });
            }
            {
                matrix_build_cauchy( n, t0, T, x0, t, avals, fvals, L, b, f_cos, 0, f_sin2plus1, 0, false) ;
                x = linear_subst_FW(L, b);
                fig->nexttile(2);
                auto p = plot(t, x);
                p->line_width(2);
                p->display_name(" x(t) + 1 / (1 + sin^2(t))");
                xlabel("t");
                ylabel("f(x(t))");
                auto ax=fig->current_axes();
            }
            {
                matrix_build_cauchy( n, t0, T, 1, t, avals, fvals, L, b, f_const, a_arg, f_const, f_arg, false) ;
                x = linear_subst_FW(L, b);
                fig->nexttile(3);
                auto p = plot(t, x);
                p->line_width(2);
                p->display_name(" x^-1(t) ");
                xlabel("t");
                ylabel("f(x(t))");
                auto ax=fig->current_axes();
            }


            fig->draw();
        
            redo = false;
            while (!redo && !leave) {
                std::cout << "Inserire nuovo T finale -10 .. 10 (<q> per uscire) > ";

                cin_clear();

                if (std::cin >> T) { if (T>=-10 && T<=-10) redo=true;} else {cout << "Key: " << T <<endl; leave=true;} ;
                if (!leave) {
                    std::cout << "Inserire nuovo valore iniziale x0 (<q> per uscire) > " ;
                    if (std::cin >> x0) { if (x0>=t0 && x0<=T) redo=true;} else {cout << "Key: " << x0 <<endl; leave=true;} ;
                    if (!leave) {

                        std::cout << "Inserire numero di nodi n (<q> per uscire) > " ;
                        if (std::cin >> n) { if (n>=1 && n<=2000) redo=true;} else {cout << "Key: " << n <<endl; leave=true;} ;
                        if (!leave) {
                            std::cout << "Inserire coeff di z(t) [-2k, 2k] (<q> per uscire) > " ;
                            if (std::cin >> a_arg) { if (a_arg>=-2000 && a_arg<=2000) redo=true;} else {cout << "Key: " << a_arg <<endl; leave=true;} ;
                            if (!leave) {
                                std::cout << "Inserire coeff di f(t) [-10k, 10k] (<q> per uscire) > " ;
                                if (std::cin >> f_arg) { if (f_arg>=-1e5 && f_arg<=1e5) redo=true;} else {cout << "Key: " << f_arg <<endl; leave=true;} ;
                            }
                        }
                    }
                };
                cin_clear();
            };
        };
    };
 
    void sym_hhalpha() {

        int n = 100;          // numero di punti della griglia        
        bool leave = false;
        bool redo = false;
        while (!leave) {
            clear_screen();
            redo = false;
            auto fig = matplot_table_init(true, "Householder", "Monitoraggio del riflesso", 1, 1);            
            matplot::legend();
            Vec v, w;
            Vec x(n , 0.0), t( n , 1.0);

            for (int i = 2; i < n; i++) 
            {
                v = vector_build_one_minus_one(i);
                // if (i <= 5) vector_dump(v, 10, v.size(), "input vec -1/1");  
                if (i == 6) 
                {
                    Mat P = matrix_householder_reflector(v);
                    matrix_dump(P, "Reflector per n = 6");
                    matrix_test_ortogonale(P, "Riflettore di Householder");
                }
                w = vector_householder_reflected(v);
                x[i]=(w[0]);
                t[i]=i;
            }
            vector_dump(x, 10, x.size(), "w[0] serie"); 

            // Visualizzazione della soluzione numerica x(t)

            fig->nexttile(0);
            auto ax=fig->current_axes();
            // ax->x_axis().limits({2,0});
            // ax->y_axis().limits({-1.00e-1, 1.00e-1 });

            auto p = plot(t, x);
            p->line_width(2);
            p->display_name("test 1");
            xlabel("n");
            ylabel("alpha");

            fig->draw();
        
            redo = false;
            while (!redo && !leave) {
                std::cout << "Inserire nuovo n 1..100 (<q> per uscire) > ";
                cin_clear();
                if (std::cin >> n) { if (n>=1 && n<=100) redo=true;} else {cout << "Key: " << n <<endl; leave=true;} ;
                cin_clear();
            };
        };
    };

    void ode_euler() {
        //
        // confronto Eulero-Heun test runner
        //
        int N_values[] = {10, 20, 40, 80}; // numero di intervalli (i nodi sono +1)  su cui iterare 
        int nN = 4;  // dimensione di N_values

        const double a = 0.0;
        double b = 5.0;
        double y0 = 1.0;     // condizione al bordo x(t0)

        std::cout << "# N h err_euler err_heun\n";

        bool leave = false;
        bool redo = false;
        while (!leave) {
            clear_screen();
            redo = false;

            Vec h_vals;
            Vec err_euler_vals;
            Vec err_heun_vals;

            auto fig = matplot_table_init(true, "Eulero vs Heun", "Metodi iterativi per problema di Cauchy, approssimazione", 2, 2);            
            matplot::legend();

            for (int k = 0; k < nN; ++k) {
                const int N = N_values[k];  
                if (N <= 0) continue;

                // passo e nodi equidistanti
                const double h = h_ticks(a, b, N+1); // n_ticks vuole nodi
                Vec t_nodes = nodi_equidistanti(a, b, N+1); // dovrebbe riempire t_nodes con N_values[k]+1 nodi
                // vettori di soluzione
                Vec y_euler_vec(N+1);
                Vec y_heun_vec(N+1);
                Vec y_exact_vec(N+1);

                // condizioni iniziali
                double y_euler = y0;
                double y_heun  = y0;

                y_euler_vec[0] = y_euler;
                y_heun_vec[0]  = y_heun;
                y_exact_vec[0] = y0; // esatta in a

                for (int i = 0; i < N; ++i) {
                    double t = t_nodes[i];
                    ode_scalar_step_euler(t, h, &y_euler, ode_scalar_rhs_decay);
                    ode_scalar_step_heun(t, h, &y_heun, ode_scalar_rhs_decay);
                    // ode_vecN_step_euler(t,h,y,2,ode_vec2_rhs_oscillator);
                    y_euler_vec[i+1] = y_euler;
                    y_heun_vec[i+1]  = y_heun;
                    double y_ex = y0 * std::exp(-1.0 * (t_nodes[i+1] - a));
                    y_exact_vec[i+1] = y_ex;
                }
                double y_exact = y0 * std::exp(-1.0 * (b - a));
                double err_euler = std::fabs(y_euler - y_exact);
                double err_heun = std::fabs(y_heun - y_exact);
                h_vals.push_back(h);
                err_euler_vals.push_back(err_euler);
                err_heun_vals.push_back(err_heun);

                std::cout << N << " " << h << " "
                        << err_euler << " " << err_heun << "\n";

                // Visualizzazione della soluzione numerica x(t)
                fig->nexttile(k);
                auto ax=fig->current_axes();
                ax->title("Confronto per N intervalli "+itostr(N));
                hold(on);

                // q->use_y2(true);
                // ax->y2_axis().limits({-1.6, 1.6});  

                auto p = plot(t_nodes, y_euler_vec);
                p->line_style(" -");
                p->line_width(6);
                p->color("green");
                p->marker("");
                p->display_name("Euler");

                auto q = plot(t_nodes, y_heun_vec);
                q->line_width(6);
                q->color("red");
                q->line_style("- ");
                q->marker("");
                q->display_name("Heun");

                auto r = plot(t_nodes, y_exact_vec);
                r->color("black");
                r->line_style("_");
                r->marker("");
                r->line_width(2);
                r->display_name("Exact");

                // p2->marker(".");
                // p1->marker("+");
                // pI->marker("r");
  
                // pI->color("magenta");
                // pf->color("cyan");
                // ptn2->color("blue");

                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Right, 0,0);
                matplot::legend();
                xlabel("t (nodi equidistanti)");
                ylabel("y approssimata");

            }

        
            fig->draw();

            auto fig_err = matplot_table_init(true, "Eulero vs Heun", "Metodi iterativi per problema di Cauchy, O(h) dell' errore globale in b", 1, 1);
            matplot::hold(on);
            fig_err->nexttile(0); //  ed unico
            auto ax=fig_err->current_axes();

            // Eulero
            auto pe = matplot::loglog(h_vals, err_euler_vals);
            pe->line_style("--");
            pe->marker("");
            pe->line_width(3);
            pe->color("green");
            pe->display_name("Euler error");

            // Heun
            auto ph = matplot::loglog(h_vals, err_heun_vals);
            ph->line_style("-.");
            ph->marker("");
            ph->line_width(3);
            ph->color("red");
            ph->display_name("Heun error");

            matplot::xlabel("h");
            matplot::ylabel("|y(b) - y_num(b)|");
            matplot_legend_align(matplot::legend(), LeAl::Bottom+LeAl::Right, 0,0);
            matplot::legend();
            fig_err->draw();

            redo = false;
            while (!redo && !leave) {
                std::cout << "Inserire nuovo max b 0 .. 100 (<q> per uscire) > ";

                cin_clear();

                if (std::cin >> b) { if (b>=0 && b<=100) redo=true;} else {cout << "Key: " << b <<endl; leave=true;} ;
                if (!leave) {
                    std::cout << "Inserire nuovo valore iniziale y0 (<q> per uscire) > " ;
                    if (std::cin >> y0) { if (y0>=a && y0<=b) redo=true;} else {cout << "Key: " << y0 <<endl; leave=true;} ;
                };
                cin_clear();
            };
        };
    };
 
    void ode_osc2d() {
        //
        // confronto Eulero-Heun test runner
        //
        int N_values[] = {10, 20, 40, 80}; // numero di intervalli (i nodi sono +1)  su cui iterare 
        int nN = 4;  // dimensione di N_values

        const double a = 0.0;
        double b = 5.0;

        double q0 = 1.0;     // condizione al bordo q(t0)
        double p0 = 1.0;     // condizione al bordo p(t0)
        double omega = ode_oscillator_omega;
        std::cout << "# N h err_euler err_heun\n";

        bool leave = false;
        bool redo = false;
        while (!leave) {
            clear_screen();
            redo = false;
            ode_set_oscillator_omega(omega); // loop successivi, se reimpostato utente
            Vec h_vals;
            Vec err_euler_vals;     // <===
            Vec err_heun_vals;      // <===

            // figure_handle fig[4];
            // axes_handle ax[4];

            auto fig2 = matplot_table_init(true, "Metodi iterativi per problema di Cauchy", "Oscillatore 2D: diagramma di fase (q,p)", 2, 2);            
            auto fig3 = matplot_table_init(true, "Metodi iterativi per problema di Cauchy", "Oscillatore 2D: Energia = 1/2 * p(t)^2 + w^2 * q(t)^2", 2, 2);            
            auto fig0 = matplot_table_init(true, "Metodi iterativi per problema di Cauchy", "Oscillatore 2D: q(t) = q0 * cos(wt) + p0/w * sin(wt)", 2, 2);            
            auto fig1 = matplot_table_init(true, "Metodi iterativi per problema di Cauchy", "Oscillatore 2D: p(t) = -w * q0 sin(wt) + p0 * cos(wt)", 2, 2);            

            for (int k = 0; k < nN; ++k) {
                const int N = N_values[k];  
                if (N <= 0) continue;

                // passo e nodi equidistanti
                const double h = h_ticks(a, b, N+1); // n_ticks vuole nodi
                Vec t_nodes = nodi_equidistanti(a, b, N+1); // dovrebbe riempire t_nodes con N_values[k]+1 nodi

                // vettori di soluzione
                Vec q_euler_vec(N+1);
                Vec p_euler_vec(N+1);
                Vec p_exact_vec(N+1);
                Vec q_exact_vec(N+1);

                double y[2];   // stato: y[0] = q, y[1] = p
                y[0] = q0;
                y[1] = p0;

                // condizioni iniziali
                q_euler_vec[0] = y[0];
                p_euler_vec[0] = y[1];
                q_exact_vec[0] = y[0];
                p_exact_vec[0] = y[1];

                double p0_w = p0 / omega; 
                double w_q0 = - omega * q0; 

                for (int i = 0; i < N; ++i) {
                    double t = t_nodes[i];
                    ode_vecN_step_euler(t, h, y, 2 , ode_vec2_rhs_oscillator);
                    q_euler_vec[i+1] = y[0];
                    p_euler_vec[i+1] = y[1]; 
                    double wt = omega * t_nodes[i+1];  
                    q_exact_vec[i+1] = q0 * f_cos(wt) + p0_w * f_sin(wt);
                    p_exact_vec[i+1] = w_q0 * f_sin(wt) + p0 * f_cos(wt);      
                }
                h_vals.push_back(h);
                // Visualizzazione  confronto q(t) 
                fig0->nexttile(k);
                auto ax0 = fig0->current_axes();
                ax0->title("q(t) vs t con "+itostr(N) + " Intervalli");
                hold(on);

                // q->use_y2(true);
                // ax->y2_axis().limits({-1.6, 1.6});  

                auto q = plot(t_nodes, q_euler_vec);
                q->line_style(" -");
                q->line_width(6);
                q->color("green");
                q->marker("");
                q->display_name("Euler");

                auto eq = plot(t_nodes, q_exact_vec);
                eq->color("black");
                eq->line_style("_");
                eq->marker("");
                eq->line_width(2);
                eq->display_name("Exact");

                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Right, 0,0);
                matplot::legend();
                xlabel("t (nodi equidistanti)");
                ylabel("q approssimata");
/*
                // Visualizzazione  confronto p(t) 
                fig1->nexttile(k);
                auto ax1 = fig1->current_axes();
                ax1->title("p(t) vs t con "+itostr(N) + " Intervalli");
                ax1->hold(on);

                // q->use_y2(true);
                // ax->y2_axis().limits({-1.6, 1.6});  

                auto p = plot(t_nodes, p_euler_vec);
                p->line_style(" -");
                p->line_width(6);
                p->color("cyan");
                p->marker("");
                p->display_name("Euler");

                auto ep = plot(t_nodes, p_exact_vec);
                ep->color("black");
                ep->line_style("_");
                ep->marker("");
                ep->line_width(2);
                ep->display_name("Exact");

                matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Right, 0,0);
                matplot::legend();
                xlabel("t (nodi equidistanti)");
                ylabel("p approssimata");
*/
            }
        
            fig0->draw();
 //           fig1->draw();


            redo = false;
            while (!redo && !leave) {
                std::cout << "Inserire nuovo max b 0 .. 100 (<q> per uscire) > ";

                cin_clear();

                if (std::cin >> b) { if (b>=0 && b<=100) redo=true;} else {cout << "Key: " << b <<endl; leave=true;} ;
                if (!leave) {
                    std::cout << "Inserire nuovo valore iniziale p0=q0 (<q> per uscire) > " ;
                    if (std::cin >> p0) { if (p0>=a && p0<=b) redo=true;} else {cout << "Key: " << p0 <<endl; leave=true;} ;
                    if (!leave) {
                        std::cout << "Inserire nuova frequenza w > 0 (<q> per uscire) > " ;
                        if (std::cin >> omega) { if (omega>0 && omega<=10) redo=true;} else {cout << "Key: " << omega <<endl; leave=true;} ;
                    };
                };
                cin_clear();
            };
        };
    };
 

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
            {"f2_deconv",       f2_deconv},
            {"f3_svd_test",     f3_svd_test},
            {"f3_svd",          f3_svd},
            {"f3_svd_pca",      f3_svd_pca},
            {"f3_svd_an",       f3_svd_an}, 
            {"sym_cauchy",      sym_cauchy},
            {"sym_hhalpha",     sym_hhalpha},
            {"ode_euler",       ode_euler},
            {"ode_osc2d",       ode_osc2d}
            };
        };
};  
// 
// fine namespace anonimo
//


int main() {
    {
        figure_handle fig = matplot_table_init(true, "Figure vuota per inizializzare matplot++", "Figure vuota per inizializzare matplot++", 2, 2);
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
        
        clear_screen(); 

    }

    std::cout << "\nUscita dal programma.\n";
    return 0;
}