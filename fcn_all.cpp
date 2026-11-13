#include <algorithm>
#include <cassert>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>   // per std::numeric_limits
#include <locale>
#include <map>
#include <matplot/matplot.h>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dataset_JL_centroidi_parametrici.hpp"
#include "svd_qr.cpp" // magari basta solo un header visto che e' compilato nella lib statica
#include "fcn_lib_calc.h"
#include "fcn_lib_jl_files.h"


// inclusione nuovo menu e data entry grimm-style migrato da VBDOS

#include "fcn_lib_form.h"
#include "gn_menu_runner.h"

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

    using fcn_form::wait_report;
    using f3_svd_test_form::prompt;
    using f3_svd_form::prompt;
    using f3_svd_pca_form::prompt;

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
        std::string n, 
        double increment, 
        int loops)
    {
        const T steps = static_cast<T>(increment);
        const long double expected =
            static_cast<long double>(increment) * loops;

        std::cout << "\n── Parte B2: somma di "
                << std::setprecision(4) << steps
                << " x " << loops << " "
                << typeid(T).name() << ":"
                << n << " ──\n";

        for (int i = 0; i < loops; ++i)
            s += steps;

        std::cout << std::setprecision(20);
        std::cout << "Somma:  " << s << "\n";
        std::cout << "Atteso: " << expected << "\n";
        std::cout << "Errore: "
                << std::abs(static_cast<long double>(s) - expected)
                << "\n";
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

    void TablePlot_Oscillator(
        const int istage,
        const Mat& euler_mat, 
        const Mat& exact_mat,
        const Mat& t_nodes_mat,
        const int N_values[],
        const MatStr& Table_des)
    {
        figure_handle fig = matplot_table_init(
            true, 
            "Metodi iterativi per problema di Cauchy", 
            Table_des[istage][1],
             2, 2); 

        for (int k = 0; k < 4; k++) {

            fig->nexttile(k);
            axes_handle ax = fig->current_axes();
            ax = fig->current_axes();
            ax->title(Table_des[istage][0]+itostr(N_values[k]) + " Intervalli");
            ax->hold(on);

                // q->use_y2(true);
                // ax->y2_axis().limits({-1.6, 1.6});  

            auto q = plot(t_nodes_mat[k], euler_mat[k]);
            q->line_style(" -");
            q->line_width(6);
            q->color(Table_des[istage][4]);
            q->marker("");
            q->display_name(Table_des[istage][2]);

            auto eq = plot(t_nodes_mat[k], exact_mat[k]);
            eq->color("black");
            eq->line_style("_");
            eq->marker("");
            eq->line_width(2);
            eq->display_name(Table_des[istage][3]);

            matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Right, 0,0);
            matplot::legend();
            xlabel("t (nodi equidistanti)");
            ylabel("y approssimata");
        }
        fig->draw();
    }

    void PhasePlot_Oscillator(
        const int istage, 
        const Mat& q_euler_mat,
        const Mat& p_euler_mat,
        const Mat& q_exact_mat,
        const Mat& p_exact_mat,
        const int N_values[],
        const MatStr& Table_des,
        bool show_exact = true)
    {
        figure_handle fig = matplot_table_init(
            true,
            "Metodi iterativi per problema di Cauchy",
            Table_des[istage][1], 
            2, 2);
        const double tol = 0.05;
        double q_min, q_max, p_min, p_max;
//          PhasingTheLimits(q_euler_mat, q_exact_mat, q_min, q_max, tol);
//          PhasingTheLimits(p_euler_mat, p_exact_mat, p_min, p_max, tol);
            PhasingTheLimits(
                q_euler_mat,
                show_exact ? q_exact_mat : q_euler_mat,
                q_min, q_max, tol
            );

            PhasingTheLimits(
                p_euler_mat,
                show_exact ? p_exact_mat : p_euler_mat,
                p_min, p_max, tol
            );       

        for (int k = 0; k < 4; ++k) {
            fig->nexttile(k);
            auto ax = fig->current_axes();
            ax->title(Table_des[istage][0] + itostr(N_values[k]) + " Intervalli");
            ax->hold(on);

            ax->xlim({q_min, q_max});
            ax->ylim({p_min, p_max});

            // Eulero: curva verde in fase
            auto ph_eul = plot(q_euler_mat[k], p_euler_mat[k]);
            ph_eul->color(Table_des[istage][4]);
            ph_eul->display_name(Table_des[istage][2]);
            ph_eul->line_width(5);
            ph_eul->line_style(" -");
            ph_eul->marker("");

            // Esatta: curva nera in fase
            if (show_exact) {
                auto ph_ex = plot(q_exact_mat[k], p_exact_mat[k]);
                ph_ex->color("black");
                ph_ex->display_name(Table_des[istage][3]);
                ph_ex->line_width(3);
                ph_ex->line_style(" -");
                ph_ex->marker("");
            }

            matplot::legend();
            xlabel("q");
            ylabel("p");

        }
        fig->draw();
    }

    void ErrorPlot_Oscillator(
        int istage,                    // 3
        const Mat& q_euler_mat,
        const Mat& q_exact_mat,
        const Mat& p_euler_mat,
        const Mat& p_exact_mat,
        const Mat& t_nodes_mat,
        const int N_values[],
        const MatStr& Table_des
    ) {
        figure_handle fig = matplot_table_init(
            true,
            "Metodi iterativi per problema di Cauchy",
            Table_des[istage][1],      // es. "Oscillatore 2D: errore nel tempo"
            2, 2
        );

        for (int k = 0; k < 4; ++k) {
            fig->nexttile(k);
            auto ax = fig->current_axes();
            ax->title(Table_des[istage][0] + itostr(N_values[k]) + " Intervalli");
            ax->hold(on);

            // costruisci vettori di errore per questo k
            std::vector<double> e_q, e_p;
            e_q.reserve(q_euler_mat[k].size());
            e_p.reserve(p_euler_mat[k].size());
            for (std::size_t j = 0; j < q_euler_mat[k].size(); ++j) {
                e_q.push_back(q_euler_mat[k][j] - q_exact_mat[k][j]);
                e_p.push_back(p_euler_mat[k][j] - p_exact_mat[k][j]);
            }

            // errore su q(t)
            auto eq = plot(t_nodes_mat[k], e_q);
            eq->color(Table_des[istage][4]);
            eq->display_name(Table_des[istage][2]);
            eq->line_width(5);
            eq->line_style(" -");
            eq->marker("");

            // errore su p(t)
            auto ep = plot(t_nodes_mat[k], e_p);
            ep->color("red");
            ep->display_name(Table_des[istage][3]);
            ep->line_width(4);
            ep->line_style(" -");
            ep->marker("");

            matplot::legend();
            xlabel("t (nodi equidistanti)");
            ylabel("errore");

        }

        fig->draw();
    }

    //
    // Per Johnson-Lindenstrauss
    //
    
    // Prende per ogni cluster le prime n/2 osservazioni dal medesimo master.
    // X_10 e' un sottoinsieme di X_20, ecc. (come insieme di righe).
    static std::pair<Mat, std::vector<int>>
    JL_estrai_bilanciato(
        const Mat& master,
        const std::vector<int>& label_master,
        int n)
    {
        if (n < 2 || n % 2 != 0)
            throw std::invalid_argument("JL_estrai_bilanciato: n deve essere pari e >= 2");
        if (master.size() != label_master.size())
            throw std::invalid_argument("JL_estrai_bilanciato: X e label incoerenti");
        if (static_cast<std::size_t>(n) > master.size())
            throw std::invalid_argument("JL_estrai_bilanciato: n > dimensione master");

        Mat X;
        std::vector<int> label;
        X.reserve(static_cast<std::size_t>(n));
        label.reserve(static_cast<std::size_t>(n));

        for (int classe = 0; classe < 2; ++classe) {
            int presi = 0;
            for (std::size_t i = 0;
                i < master.size() && presi < n/2; ++i) {
                if (label_master[i] == classe) {
                    X.push_back(master[i]);
                    label.push_back(classe);
                    ++presi;
                }
            }
            if (presi != n/2)
                throw std::invalid_argument("JL_estrai_bilanciato: cluster troppo piccolo");
        }
        return {std::move(X), std::move(label)};
    }


    static JLPCA_Spettro JL_PCA_spettro(const Mat& X)
    {
        if (X.size() < 2 || X.front().empty())
            throw std::invalid_argument("JL_PCA_spettro: matrice vuota o n<2");
        const std::size_t d = X.front().size();
        for (const auto& r : X)
            if (r.size() != d)
                throw std::invalid_argument("JL_PCA_spettro: matrice non rettangolare");

        const auto start = std::chrono::steady_clock::now();

        Vec avg_col(d, 0.0), avg_row(X.size(), 0.0);
        matrix_calcola_media(X, avg_col, avg_row);
        Mat Xtilde = matrix_centra_su_media(X, avg_col);

        Mat U0, B, V0;
        trmatrix_bidiagonalizza(Xtilde, U0, B, V0, bidiag_mode, false);

        Mat Ub, Vb_t;
        Vec sigma;
        if (svd_bidiagonale(B, Ub, Vb_t, sigma) != 0)
            throw std::runtime_error("JL_PCA_spettro: SVD non convergente");

        const auto end = std::chrono::steady_clock::now();
        std::sort(sigma.begin(), sigma.end(), std::greater<double>{});
        const std::size_t expected = std::min(X.size(), d);
        if (sigma.size() < expected)
            throw std::runtime_error("JL_PCA_spettro: numero valori singolari insufficiente");
        JLPCA_Spettro result;
        result.sigma = std::move(sigma);
        for (double s : result.sigma) {
            if (!std::isfinite(s) || s < 0.0)
                throw std::runtime_error("JL_PCA_spettro: valore singolare non valido");
            result.energia += s*s;
        }
        result.tempo_s = std::chrono::duration<double>(end-start).count();
        return result;
    }

    // Una sola risoluzione del riferimento storico per tutte le prove PCA.
    static std::filesystem::path JL_PCA_baseline_file(const jl_files::Store& files)
    {
        constexpr auto name = "JL_PCA_originale_20261111_203831_386.csv";
        const auto state_file = files.state_path(name);
        const auto cache_file = files.cache_path(name);
        if (!std::filesystem::exists(state_file) && std::filesystem::exists(cache_file))
            return cache_file;
        return state_file;
    }

    static JLCL_Point JLCL_add(JLCL_Point a, JLCL_Point b)
    { return {a.x+b.x, a.y+b.y}; }
    static JLCL_Point JLCL_sub(JLCL_Point a, JLCL_Point b)
    { return {a.x-b.x, a.y-b.y}; }
    static JLCL_Point JLCL_scale(JLCL_Point a, double q)
    { return {q*a.x, q*a.y}; }
    static double JLCL_dot(JLCL_Point a, JLCL_Point b)
    { return a.x*b.x+a.y*b.y; }
    static double JLCL_norm(JLCL_Point a)
    { return std::sqrt(JLCL_dot(a,a)); }

    static JLCL_PCA2 JLCL_pca2_gram(const Mat& X)
    {
        const std::size_t n=X.size();
        if (n<4 || X.front().empty())
            throw std::invalid_argument("JLCL_pca2_gram: dimensioni insufficienti");
        const std::size_t d=X.front().size();
        for (const auto& r:X)
            if (r.size()!=d)
                throw std::invalid_argument("JLCL_pca2_gram: righe irregolari");

        Vec mean(d,0.0);
        for (const auto& row:X)
            for (std::size_t j=0;j<d;++j) mean[j]+=row[j];
        for (double& a:mean) a/=static_cast<double>(n);
        Mat Xc(n,Vec(d,0.0));
        for (std::size_t i=0;i<n;++i)
            for (std::size_t j=0;j<d;++j)
                Xc[i][j]=X[i][j]-mean[j];

        Mat G(n,Vec(n,0.0));
        JLCL_PCA2 result;
        for (std::size_t i=0;i<n;++i)
            for (std::size_t j=i;j<n;++j) {
                double dot=0.0;
                for (std::size_t c=0;c<d;++c)
                    dot+=Xc[i][c]*Xc[j][c];
                G[i][j]=G[j][i]=dot;
                if (i==j) result.energia+=dot;
            }
        if (!(result.energia>0.0))
            throw std::runtime_error("JLCL_pca2_gram: energia nulla");
        Mat V(n,Vec(n,0.0));
        for (std::size_t i=0;i<n;++i) V[i][i]=1.0;

        // Jacobi ciclico: autovettori colonna di V, matrice G simmetrica PSD.
        bool converged=false;
        for (int sweep=0;sweep<90;++sweep) {
            double max_off=0.0, max_diag=0.0;
            for (std::size_t i=0;i<n;++i) {
                max_diag=std::max(max_diag,std::abs(G[i][i]));
                for (std::size_t j=i+1;j<n;++j)
                    max_off=std::max(max_off,std::abs(G[i][j]));
            }
            const double tol=1e-12*std::max(1.0,max_diag);
            if (max_off<=tol) { converged=true; break; }
            for (std::size_t p=0;p<n;++p)
                for (std::size_t q=p+1;q<n;++q) {
                    const double apq=G[p][q];
                    if (std::abs(apq)<=tol) continue;
                    const double app=G[p][p], aqq=G[q][q];
                    const double tau=(aqq-app)/(2.0*apq);
                    const double t=std::copysign(1.0,tau) /
                        (std::abs(tau)+std::sqrt(1.0+tau*tau));
                    const double c=1.0/std::sqrt(1.0+t*t), s=t*c;
                    for (std::size_t r=0;r<n;++r) {
                        if (r==p || r==q) continue;
                        const double rp=G[r][p],rq=G[r][q];
                        G[r][p]=G[p][r]=c*rp-s*rq;
                        G[r][q]=G[q][r]=s*rp+c*rq;
                    }
                    G[p][p]=app-t*apq;
                    G[q][q]=aqq+t*apq;
                    G[p][q]=G[q][p]=0.0;
                    for (std::size_t r=0;r<n;++r) {
                        const double vp=V[r][p],vq=V[r][q];
                        V[r][p]=c*vp-s*vq;
                        V[r][q]=s*vp+c*vq;
                    }
                }
        }
        if (!converged)
            throw std::runtime_error("JLCL_pca2_gram: Jacobi non convergente");
        std::vector<std::size_t> order(n);
        for (std::size_t i=0;i<n;++i) order[i]=i;
        std::sort(order.begin(),order.end(),[&](std::size_t a,std::size_t b){
            return G[a][a]>G[b][b];
        });
        const double l1=G[order[0]][order[0]];
        const double l2=G[order[1]][order[1]];
        if (!(l1>0.0 && l2>0.0))
            throw std::runtime_error("JLCL_pca2_gram: rango < 2");
        result.sigma1=std::sqrt(l1);
        result.sigma2=std::sqrt(l2);
        result.score.reserve(n);
        for (std::size_t i=0;i<n;++i)
            result.score.push_back({result.sigma1*V[i][order[0]],
                                    result.sigma2*V[i][order[1]]});
        return result;
    }

    static JLCL_ClassMetrics JLCL_evaluate(
        const std::vector<JLCL_Point>& P,const std::vector<int>& labels)
    {
        const std::size_t n=P.size();
        if (n!=labels.size() || n<4)
            throw std::invalid_argument("JLCL_evaluate: P/label non validi");
        JLCL_ClassMetrics r;
        int count[2]={0,0};
        for (std::size_t i=0;i<n;++i) {
            const int h=labels[i];
            if (h!=0 && h!=1)
                throw std::invalid_argument("JLCL_evaluate: label diversa da 0/1");
            ++count[h];
            r.centroid[h]=JLCL_add(r.centroid[h],P[i]);
        }
        if (count[0]<2 || count[1]<2)
            throw std::invalid_argument("JLCL_evaluate: meno di due punti per classe");
        for (int h=0;h<2;++h)
            r.centroid[h]=JLCL_scale(r.centroid[h],1.0/count[h]);
        for (std::size_t i=0;i<n;++i) {
            const int h=labels[i];
            const JLCL_Point dif=JLCL_sub(P[i],r.centroid[h]);
            r.radius[h]+=JLCL_dot(dif,dif);
        }
        for (int h=0;h<2;++h)
            r.radius[h]=std::sqrt(r.radius[h]/count[h]);
        const JLCL_Point delta=JLCL_sub(r.centroid[1],r.centroid[0]);
        r.separation=JLCL_norm(delta);
        if (r.separation<=0)
            throw std::runtime_error("JLCL_evaluate: centroidi coincidenti");
        r.sep_ratio=r.separation/(r.radius[0]+r.radius[1]);
        const JLCL_Point axis=JLCL_scale(delta,1.0/r.separation);
        const JLCL_Point mid=JLCL_scale(
            JLCL_add(r.centroid[0],r.centroid[1]),0.5);
        double max0=-std::numeric_limits<double>::infinity();
        double min1=std::numeric_limits<double>::infinity();
        r.loo_min_margin=std::numeric_limits<double>::infinity();
        for (std::size_t i=0;i<n;++i) {
            const int h=labels[i],other=1-h;
            const double axial=JLCL_dot(JLCL_sub(P[i],mid),axis);
            if (h==0) max0=std::max(max0,axial);
            else      min1=std::min(min1,axial);
            const JLCL_Point do_own=JLCL_sub(P[i],r.centroid[h]);
            const JLCL_Point do_other=JLCL_sub(P[i],r.centroid[other]);
            const double dOwn=JLCL_dot(do_own,do_own);
            const double dOther=JLCL_dot(do_other,do_other);
            if (dOwn>=dOther) ++r.train_errors;
            // Leave-one-out: il centroide della classe vera esclude P[i].
            const JLCL_Point c_loo=JLCL_scale(
                JLCL_sub(JLCL_scale(r.centroid[h],count[h]),P[i]),
                1.0/(count[h]-1));
            const JLCL_Point res=JLCL_sub(P[i],c_loo);
            const double dLoo=JLCL_dot(res,res);
            const int predicted=(dLoo<dOther ? h : other);
            r.loo_prediction.push_back(predicted);
            if (predicted!=h) ++r.loo_errors;
            const double margin=(dOther-dLoo)/(r.separation*r.separation);
            r.loo_margin.push_back(margin);
            r.loo_min_margin=std::min(r.loo_min_margin,margin);
            r.loo_mean_margin+=margin;
        }
        r.signed_gap=min1-max0; // >0: intervalli di proiezione disgiunti
        r.loo_mean_margin/=static_cast<double>(n);
        return r;
    }

    static JLCL_Alignment JLCL_align(
        const std::vector<JLCL_Point>& X,const JLCL_ClassMetrics& cx,
        const std::vector<JLCL_Point>& Y,const JLCL_ClassMetrics& cy)
    {
        if (X.size()!=Y.size())
            throw std::invalid_argument("JLCL_align: dati non accoppiati");
        const JLCL_Point mx=JLCL_scale(JLCL_add(cx.centroid[0],cx.centroid[1]),0.5);
        const JLCL_Point my=JLCL_scale(JLCL_add(cy.centroid[0],cy.centroid[1]),0.5);
        const JLCL_Point vx=JLCL_sub(cx.centroid[1],cx.centroid[0]);
        const JLCL_Point vy=JLCL_sub(cy.centroid[1],cy.centroid[0]);
        const JLCL_Point ux=JLCL_scale(vx,1.0/JLCL_norm(vx));
        const JLCL_Point uy=JLCL_scale(vy,1.0/JLCL_norm(vy));
        const JLCL_Point nx={-ux.y,ux.x};
        const JLCL_Point ny={-uy.y,uy.x};
        // Segno trasversale non fissato dalle PCA indipendenti: confronto
        // entrambe le chiralita' sui punti corrispondenti, senza riscalare.
        auto candidate=[&](int flip) {
            std::vector<JLCL_Point> A;
            A.reserve(Y.size());
            double err=0.0;
            for (std::size_t i=0;i<Y.size();++i) {
                const JLCL_Point q=JLCL_sub(Y[i],my);
                const double along=JLCL_dot(q,uy);
                const double across=JLCL_dot(q,ny);
                const JLCL_Point a=JLCL_add(mx,JLCL_add(
                    JLCL_scale(ux,along),JLCL_scale(nx,flip*across)));
                A.push_back(a);
                const auto residual=JLCL_sub(a,X[i]);
                err+=JLCL_dot(residual,residual);
            }
            return std::make_pair(std::move(A),err);
        };
        auto normal=candidate(+1);
        auto mirror=candidate(-1);
        JLCL_Alignment r;
        r.reflected=(mirror.second<normal.second);
        r.Yaligned=std::move(r.reflected ? mirror.first : normal.first);
        const double pi=std::acos(-1.0);
        r.angle_deg=(std::atan2(vx.y,vx.x)-std::atan2(vy.y,vy.x))*180.0/pi;
        while (r.angle_deg>180.0) r.angle_deg-=360.0;
        while (r.angle_deg<=-180.0) r.angle_deg+=360.0;
        r.rms_displacement=std::sqrt(
            (r.reflected ? mirror.second : normal.second)/X.size());
        return r;
    }

    // Scatter SVG autonomo: niente dipendenza da Matplot++/gnuplot.
    // Originali tenue blu/rosso; JL allineato verde/arancio pieno.
    static void JLCL_write_svg(std::ostream& f,
        const std::vector<JLCL_Point>& X,
        const std::vector<JLCL_Point>& Ya,
        const std::vector<int>& labels,
        const JLCL_ClassMetrics& cx,
        const JLCL_ClassMetrics& cy,
        const JLCL_Alignment& align,
        int n,int d,int k,double eps,bool show_original_points)
    {
        const int W=1080,H=740;
        double xmin=std::numeric_limits<double>::infinity();
        double xmax=-xmin,ymin=xmin,ymax=-xmin;
        auto consider=[&](JLCL_Point p){
            xmin=std::min(xmin,p.x);xmax=std::max(xmax,p.x);
            ymin=std::min(ymin,p.y);ymax=std::max(ymax,p.y);
        };
        for (auto p:X) consider(p);
        for (auto p:Ya) consider(p);
        for (int c=0;c<2;++c) consider(cx.centroid[c]);
        const double xr=std::max(1e-6,xmax-xmin),yr=std::max(1e-6,ymax-ymin);
        const double scale=std::min(880.0/(xr*1.15),525.0/(yr*1.15));
        const double mx=(xmin+xmax)/2.0,my=(ymin+ymax)/2.0;
        auto px=[&](JLCL_Point a){return 510.0+(a.x-mx)*scale;};
        auto py=[&](JLCL_Point a){return 394.0-(a.y-my)*scale;};
        f << std::fixed << std::setprecision(3);
        f << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""<<W
        <<"\" height=\""<<H<<"\" viewBox=\"0 0 "<<W<<' '<<H<<"\">\n";
        f << "<rect width=\"100%\" height=\"100%\" fill=\"#ffffff\"/>\n";
        f << "<text x=\"62\" y=\"40\" font-size=\"24\" font-family=\"sans-serif\" font-weight=\"bold\" fill=\"#192734\">JL + PCA: cluster allineati sui centroidi</text>\n";
        f << "<text x=\"62\" y=\"68\" font-size=\"14\" font-family=\"sans-serif\" fill=\"#435363\">n="<<n<<"  d="<<d<<"  k="<<k<<"  epsilon="<<eps
        <<"  (rotazione e traslazione, scala invariata)</text>\n";
        f << "<rect x=\"60\" y=\"92\" width=\"900\" height=\"595\" fill=\"#fafbfc\" stroke=\"#cbd5e1\"/>\n";
        // Griglia quadrata in unita' di score (stessa scala sugli assi).
        for (int i=-5;i<=5;++i) {
            const double x=510.0+i*50.0,y=394.0+i*50.0;
            f << "<line x1=\""<<x<<"\" y1=\"95\" x2=\""<<x
            <<"\" y2=\"686\" stroke=\"#e6ebf0\"/>\n";
            f << "<line x1=\"62\" y1=\""<<y<<"\" x2=\"958\" y2=\""
            <<y<<"\" stroke=\"#e6ebf0\"/>\n";
        }
        const char* original[2]={"#377ab4","#d14a4a"};
        const char* transformed[2]={"#169c80","#ef8c25"};
        // Originale: alone trasparente (oppure solo centroidi).
        if (show_original_points) for (std::size_t i=0;i<X.size();++i)
            f << "<circle cx=\""<<px(X[i])<<"\" cy=\""<<py(X[i])
            <<"\" r=\"5.1\" fill=\""<<original[labels[i]]
            <<"\" fill-opacity=\"0.26\"/>\n";
        // JL sovrapposto a tinta piena.
        for (std::size_t i=0;i<Ya.size();++i)
            f << "<circle cx=\""<<px(Ya[i])<<"\" cy=\""<<py(Ya[i])
            <<"\" r=\"3.35\" fill=\""<<transformed[labels[i]]
            <<"\" fill-opacity=\"0.90\" stroke=\"#ffffff\" stroke-width=\"0.35\"/>\n";
        // Centroidi reali + trasformati allineati. Midpoint coincidente.
        JLCL_Point mm=JLCL_scale(JLCL_add(cx.centroid[0],cx.centroid[1]),0.5);
        JLCL_Point ca[2];
        const JLCL_Point vx=JLCL_sub(cx.centroid[1],cx.centroid[0]);
        const double len0=JLCL_norm(vx);
        const double len1=cy.separation;
        const JLCL_Point axis=JLCL_scale(vx,1.0/len0);
        ca[0]=JLCL_sub(mm,JLCL_scale(axis,len1/2.0));
        ca[1]=JLCL_add(mm,JLCL_scale(axis,len1/2.0));
        auto line=[&](JLCL_Point a,JLCL_Point b,const char* color,const char* dash){
            f << "<line x1=\""<<px(a)<<"\" y1=\""<<py(a)
            <<"\" x2=\""<<px(b)<<"\" y2=\""<<py(b)
            <<"\" stroke=\""<<color<<"\" stroke-width=\"2.7\"";
            if (*dash) f << " stroke-dasharray=\""<<dash<<"\"";
            f << "/>\n";
        };
        line(cx.centroid[0],cx.centroid[1],"#526070","7,5");
        line(ca[0],ca[1],"#26364a","");
        for (int h=0;h<2;++h) {
            line(cx.centroid[h],ca[h],"#8995a7","3,5");
            f << "<circle cx=\""<<px(cx.centroid[h])<<"\" cy=\""
            <<py(cx.centroid[h])<<"\" r=\"8\" fill=\"#ffffff\" stroke=\""
            <<original[h]<<"\" stroke-width=\"2.2\"/>\n";
            f << "<circle cx=\""<<px(ca[h])<<"\" cy=\""<<py(ca[h])
            <<"\" r=\"6\" fill=\""<<transformed[h]
            <<"\" stroke=\"#ffffff\" stroke-width=\"1.8\"/>\n";
        }
        f << "<text x=\"60\" y=\"713\" font-size=\"13\" font-family=\"sans-serif\" fill=\"#526070\">"
        << (show_original_points ? "Originale: blu / rosso (tenue)" : "Originale: soltanto centroidi")
        << "  |  JL+PCA: verde / arancio (pieno)  |  cerchi grandi = centroidi</text>\n";
        f << "<text x=\"60\" y=\"731\" font-size=\"11\" font-family=\"sans-serif\" fill=\"#697585\">Riflessione trasversale: "<<(align.reflected?"si":"no")
        <<"  |  rotazione coordinate: "<<align.angle_deg
        <<" gradi. L'allineamento e' illustrativo; non e' una prova di isometria.</text>\n";
        f << "</svg>\n";
    }


    void Gershgorin_execute(Mat& A){

        auto rows = linear_gershgorin_disks_rows(A);
        auto cols = linear_gershgorin_disks_columns(A);

        std::cout << "\n=== Gershgorin - dischi per righe ===\n";

        for (const auto& d : rows) {
            std::cout
                << "D" << d.index + 1
                << ": centro = " << d.center
                << ", raggio = " << d.radius
                << "\n";
        }

        std::cout << "\n=== Gershgorin - dischi per colonne ===\n";

        for (const auto& d : cols) {
            std::cout
                << "D" << d.index + 1
                << ": centro = " << d.center
                << ", raggio = " << d.radius
                << "\n";

        }  
        
        std::cout << std::endl;
    }

    void preriscaldamento_anti_overhead(Mat& A) {
        //
        // gia' una volta bastava ma proviamo con tre giri 
        //
        for (int i=0; i<3; i++) linear_max_autoval_pwr_any_res(A, 1000, 1e-12);
    }

    void f1_sample() {

        // ── costanti ─────────────────────────────────────────────────────────────────
        F1SampleParams params{}; // N. di funzioni base e N. di nodi (default 40 e 100) 
        const int   Cols = 10;   // N di colonne per il dump dei vettori 

        // alloco tutto prima per poter usare in un loop

        double NR;
        int curr_tile;
        vector<matrix_ptr> mtpr(4);
        Vec x_eq;
        Vec x_asym1;
        Vec x_asym2;
        Vec x_rnd;
        Mat G_eq;
        Mat G_asym1;
        Mat G_asym2;
        Mat G_rnd;

        int stderr_handle = dup(fileno(stderr)); // salva il vecchio stderr descriptor
        FILE* fres = freopen("/dev/null", "w", stderr);       // evita le menate di gnuplot :)

        while (f1_sample_form::prompt(params)) {
            const int K = params.K;
            const int N = params.N;

            // Silenzia temporaneamente i messaggi Gnuplot
            int stderr_handle = dup(fileno(stderr));
            FILE* fres = freopen("/dev/null", "w", stderr);

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
            mtpr[curr_tile]->display_name(
                itostr(N) + " nodi equidistanti in 0..2PI");
            matplot::colorbar();

            // 3) TODO 6: visualizzare anche G_nonEq come immagine e confrontare
            //    con G_eq. Cosa vedi? Riesci a spiegare la differenza?
            
            curr_tile=1;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_rnd);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name(
                itostr(N) + " nodi random");
            matplot::colorbar();
            
            // 3) TODO 5bis: visualizzare anche G_Eq asimmetrico come immagine 
            //    e confrontare
            //    con G_eq. Cosa vedi? Riesci a spiegare la differenza?

            curr_tile=2;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_asym1);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name(
                itostr(N) + " nodi equidistanti in 0..PI");
            matplot::colorbar();
        

        
            curr_tile=3;
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            mtpr[curr_tile] = matplot::imagesc(G_asym2);
            matplot_legend_align(matplot::legend(), LeAl::Auto, K, 0.4);
            mtpr[curr_tile]->display_name(
                itostr(N) + " nodi equidistanti in 0..PI/2");
            matplot::colorbar();
            

            fig->draw();

            // Ripristina stderr prima di ritornare a GNPORT
            fflush(stderr);                         // tira lo sciacquone
            dup2(stderr_handle, fileno(stderr));    //ripristina lo stderr su console
            close(stderr_handle);   //liberaci dal duplicato tanto non ci induci in errore

            fcn_form::wait_report();
        }
    }

    void f1_taylor_exp() {

        F1TaylorParams params{20, -2};

        Vecn n_val;
        Vecd n_vald;
        Vecf n_valf;
        const bool Cheat_e0 = true;
        double LastD = 0;
        double LastF = 0;

        while (f1_taylor_exp_form::prompt(params)) {
            const int n_terms = params.n_terms;
            const int target_arg = params.target_arg;
            int curr_tile = 0;

            clear_screen();
            auto fig = matplot_table_init(
                false, "Taylor exp",
                "Convergenza di Taylor per exp(x)", 2, 2);

        // Da qui conserviamo il calcolo attuale.            
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
            fcn_form::wait_report();
        }
    }

    void f1_taylor_cos() {
        F1TaylorParams params{20, 1};

        Vecn n_val;
        Vecd n_vald;
        Vecf n_valf;
        const bool Cheat_e0 = true;
        double LastD = 0;
        double LastF = 0;
        double e_esatto;

        while (f1_taylor_cos_form::prompt(params)) {
            const int n_terms = params.n_terms;
            const int target_arg = params.target_arg;
            int curr_tile = 0;

            int stderr_handle = dup(fileno(stderr));
            FILE* fres = freopen("/dev/null", "w", stderr);

            clear_screen();
            auto fig = matplot_table_init(
                true, "Taylor cos",
                "Stima di cos(x) con Taylor centrato in 0", 2, 2);

            // Primo grafico: codice attuale invariato.
            // ── Parte A2: tabella errori per cos(x) con x = 1 ────────────────────────   
            e_esatto = std::cos(deg2rad(target_arg)); // valore dal quale si calcola l'errore
            TablePrint_cos (n_vald, n_valf, n_val, target_arg, e_esatto, n_terms, Cheat_e0, LastD, LastF);
            // ── Parte A3: plot errore in scala log ────────────────────────────────────
            fig->nexttile(curr_tile); // punta al panel corrente, va bene con option base 0
            TablePlot_cos(fig->current_axes(), n_vald, n_valf, n_val, target_arg, curr_tile, Cheat_e0);

            curr_tile++;

            const int reference_arg = 30;
            e_esatto = std::cos(deg2rad(reference_arg));
            TablePrint_cos(
                n_vald, n_valf, n_val, reference_arg,
                e_esatto, n_terms, Cheat_e0, LastD, LastF);
            fig->nexttile(curr_tile);
            TablePlot_cos(
                fig->current_axes(), n_vald, n_valf, n_val,
                reference_arg, curr_tile, Cheat_e0);

            curr_tile++;

            fig->draw();

            fflush(stderr);
            dup2(stderr_handle, fileno(stderr));
            close(stderr_handle);

            fcn_form::wait_report();
        }
    }

    void f1_prec() {
        F1PrecParams params{};

        while (f1_prec_form::prompt(params)) {
            clear_screen();

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

            /*
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
            */

            loop_sum(0.0f, "float",
                    params.increment, params.repetitions);

            loop_sum(0.0, "double",
                    params.increment, params.repetitions);

            loop_sum(0.0L, "long double",
                    params.increment, params.repetitions);
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

            // 1 + 1/N è distinguibile da 1 se N*eps_m < 2.
            // Alla soglia N*eps_m = 2, il tie-to-even restituisce 1.
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

            fcn_form::wait_report();
        }
    }

    void f2_deriv() {

        F2DerivParams params{};

        while (f2_deriv_form::prompt(params)) {
            const int n = params.n;
            const double a = -params.domain_span;
            const double b = params.domain_span;
            int curr_tile = 0;
            clear_screen();

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
            fcn_form::wait_report();
        };
    };

    void f2_cauchy() {

        Vec t, fvals, b;
        Mat L;
        F2CauchyParams params{};

        while (f2_cauchy_form::prompt(params)) {
            
        
            const int n = params.n ;          // numero di punti della griglia
            const double t0 = params.t0 ;     // tempo iniziale
            const double T  = params.T ;      // tempo finale
            const double x0 = params.x0 ;     // condizione iniziale x(t0)
            clear_screen();

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
            fcn_form::wait_report();
        };
    };

    void f2_bordo() {

        Vec t, fvals, b, x, y;
        Mat A,L,U;
        F2BordoParams params{};

        while (f2_bordo_form::prompt(params)) {
            int n = params.n;             // numero di punti della griglia
            double t0 = params.t0;        // tempo iniziale
            double T  = params.T;         // tempo finale
            double x0 = params.x0;        // condizione iniziale x(t0)
            double x1 = params.x1;        // condizione finale x(T)

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

            fcn_form::wait_report();

        };
    };

    void f2_integral() {

        Vec fvals;
        F2IntegralParams params{};
        while (f2_integral_form::prompt(params)) {
            const double a = params.a ;           // inizio intervallo di integrazione
            const double b = params.b ;           // fine intervallo di integrazione
            const int N = params.N ;              // numero di punti di campionamento   

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

            fcn_form::wait_report();
        };
    };

    void f2_norme() {

        F2NormeParams params{640, 1};

        // Mantiene esattamente la scalatura dell'esperimento consolidato.
        const double a = -7.0;
        const double b =  7.0;

        while (f2_norme_form::prompt(params)) {

            clear_screen();

            std::vector<int> N_values;
            for (int N = 10; N <= params.n_max; N *= 2) {
                N_values.push_back(N);
            }

            std::vector<double>
                x_vals,
                cond_2_vals,
                cond_12_vals,
                cond_22_vals,
                cond_f_vals,
                cond_1_vals,
                cond_I_vals;

            std::cout << "\n=== Norma 2 e numero di condizionamento spettrale ===\n";
            std::cout << "Timing: media su " << params.timing_reps
                      << (params.timing_reps == 1
                        ? " esecuzione per metodo\n\n"
                        : " esecuzioni per metodo\n\n");
            std::cout << std::setfill(' ')
                << std::setw(6)  << "N"
                << std::setw(13) << "||A||2 1R"
                << std::setw(12) << "t1R [us]"
                << std::setw(14) << "||A||2 AllR"
                << std::setw(12) << "tAllR [us]"
                << std::setw(14) << "||A||2 AtA"
                << std::setw(12) << "tAtA [us]"
                << std::setw(11) << "AllR/1R"
                << std::setw(11) << "AtA/1R"
                << std::setw(13) << "||A^-1||2"
                << std::setw(13) << "kappa2"
                << std::setw(14) << "kappa2/N"
                << std::setw(13) << "kappaF"
                << std::setw(13) << "kappa1"
                << std::setw(13) << "kappaInf"
                << '\n';    

            for (int N : N_values) {
                double h = h_ticks(a,b,N); // passo di campionamento
            
                Mat A = matrix_build_triang(N);
                A = matrix_prodotto_coeff(A, h); // moltiplica per h
                Mat A_inv = linear_LU_inversa(A);

                preriscaldamento_anti_overhead(A); // warm-up per taratura cronometrica a strutture fatte
                
                // Norma 2 spettrale power con economia
                double norm2_A = 0.0;
                double time2_A = 0.0;
                for (int rep = 0; rep < params.timing_reps; ++rep) {
                    auto t_start = std::chrono::steady_clock::now();
                    norm2_A = matrix_calcola_norma(2, A);
                    auto t_end = std::chrono::steady_clock::now();
                    time2_A +=
                        std::chrono::duration<double, std::micro>(
                            t_end - t_start).count();
                }
                time2_A /= params.timing_reps;
                double norm2_Ainv = matrix_calcola_norma(2, A_inv);
                double kappa2 = norm2_A * norm2_Ainv;
                
                // Norma 2 spettrale power senza economia
                double norm12_A = 0.0;
                double time12_A = 0.0;
                for (int rep = 0; rep < params.timing_reps; ++rep) {
                    auto t_start = std::chrono::steady_clock::now();
                    norm12_A = matrix_calcola_norma(12, A);
                    auto t_end = std::chrono::steady_clock::now();
                    time12_A +=
                        std::chrono::duration<double, std::micro>(
                            t_end - t_start).count();
                }
                time12_A /= params.timing_reps;                
                double norm12_Ainv = matrix_calcola_norma(12, A_inv);
                double kappa12 = norm12_A * norm12_Ainv;

                // Norma 2 spettrale power + Rayleigh
                double norm22_A = 0.0;
                double time22_A = 0.0;
                for (int rep = 0; rep < params.timing_reps; ++rep) {
                    auto t_start = std::chrono::steady_clock::now();
                    norm22_A = matrix_calcola_norma(22, A);
                    auto t_end = std::chrono::steady_clock::now();
                    time22_A +=
                        std::chrono::duration<double, std::micro>(
                            t_end - t_start).count();
                }
                time22_A /= params.timing_reps;
                double norm22_Ainv = matrix_calcola_norma(22, A_inv);
                double kappa22 = norm22_A * norm22_Ainv;

                double ratio12 =
                    static_cast<double>(time12_A) /
                    std::max<long long>(1, time2_A);

                double ratio22 =
                    static_cast<double>(time22_A) /
                    std::max<long long>(1, time2_A);
                            
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
                
                std::cout
                    << std::setw(6) << N

                    << std::fixed << std::setprecision(2)
                    << std::setw(13) << norm2_A

                    << std::setw(12) << time2_A

                    << std::fixed << std::setprecision(2)
                    << std::setw(14) << norm12_A

                    << std::setw(12) << time12_A

                    << std::fixed << std::setprecision(2)
                    << std::setw(14) << norm22_A

                    << std::setw(12) << time22_A

                    << std::fixed << std::setprecision(2)
                    << std::setw(11) << ratio12
                    << std::setw(11) << ratio22

                    << std::setw(13) << norm2_Ainv
                    << std::setw(13) << kappa2

                    << std::scientific << std::setprecision(3)
                    << std::setw(14)
                    << kappa2 / (static_cast<double>(N))

                    << std::fixed << std::setprecision(2)
                    << std::setw(13) << kappaF
                    << std::setw(13) << kappa1
                    << std::setw(13) << kappaI

                    << '\n';
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
            // ax->title(title);

            xlabel("N");
            ylabel("kappa");

            // Confronto con curva teorica N^2
            Vec Ns, theory_kappa2, theory_N;
            for (double N = 10; N <= params.n_max; N *= 2) {
                Ns.push_back(N);
                theory_kappa2.push_back((4.0 / M_PI) * N);
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
            
           
            auto ptn2 = semilogy(Ns, theory_kappa2);
            ptn2->display_name("4N/pi (teorico)");
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

            

            wait_report();

        };
        


    };

    void f2_deconv() {
         
        Mat kappa2(4, Vec(3, 0.0)); // 0 numero cond, 1 norma2(K), 2 (norma2(K_inv))
        F2DeconvParams params{};
        clear_screen();
        while (f2_deconv_form::prompt(params)) {
            const double a = params.a;              // inizio intervallo
            const double b = params.b;              // fine intervallo
            const int N = params.N;                 // numero di punti separatori   
            const double sigma = params.sigma;
            const bool KG_Normal = params.KG_Normal; 

            cout << "Deconvoluzione con Kernel di Gauss" << endl;  // vedere cosa fare di questo cout inutile

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
            fcn_form::wait_report();
        }
    }

    void f3_svd_test(){
        int n = 6;
        int d = 5;

        while (f3_svd_test_form::prompt(n, d)) {
            // Da qui il terminale e' tornato canonico: il rapporto resta stdio puro.
            clear_screen();

            {
                Vec v = {3.0, 1.0, -2.0};
                Vec w = vector_build_householder_bycol(v);

                // Calcola H*v esplicitamente per verifica
                // H*v = v - 2*w*(w^T*v)
                double wtv = 0;
                for (int i = 0; i < 3; ++i) wtv += w[i] * v[i];
                Vec Hv(3);
                for (int i = 0; i < 3; ++i) Hv[i] = v[i] - 2 * w[i] * wtv;

                printf("Verifica Householder colonna:\n");
                printf("  v = [%.3f, %.3f, %.3f]\n", v[0], v[1], v[2]);
                printf("  H*v = [%.6f, %.6f, %.6f]\n", Hv[0], Hv[1], Hv[2]);
                printf("  atteso: [%.6f, 0, 0]\n\n", -std::sqrt(14.0));
                cout << std::string(80, '_') << endl;
            }

            // Verifica della bidiagonalizzazione su matrice n x d.
            {
                std::mt19937 rng(42);
                std::normal_distribution<double> g(0.0, 1.0);
                Mat X = matrix_build_zero(n, d);
                for (auto& r : X) {
                    for (double& v : r) v = g(rng);
                }

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

                Mat X0 = matrix_prodotto_matrix(U0, matrix_prodotto_matrix(B, V0_t));
                matrix_dump(X, "X - Originale");
                matrix_dump(X0, "X - Ricostruita U0 * (B * V0^T)");
                Mat R = matrix_calcola_diff(X, X0);
                matrix_dump(R, "R - Differenza X - U0 * B * V0^T");

                printf(" Error_F_lab ||X - U0*B*V0^T||_F = %.2e\n", matrix_calcola_errore_Fr(X, X0));
                printf(" Error_F_mia ||X - U0*B*V0^T||_F = %.2e\n\n", matrix_calcola_norma(-1, R));
                printf(" Error_F_lab ||In - U0*U0^T ||_F = %.2e\n", matrix_calcola_errore_Fr(In, U0_U0t));
                printf(" Error_F_lab ||Id - V0*V0^T ||_F = %.2e\n\n", matrix_calcola_errore_Fr(Id, V0_V0t));

                printf(" Norma spettrale PM econ ||R||_2 = %.2e\n", matrix_calcola_norma(2, R));
                printf(" Norma spettrale PM lab. ||R||_2 = %.2e\n", matrix_calcola_norma(12, R));
                printf(" Norma spettrale PM rayl ||R||_2 = %.2e\n\n", matrix_calcola_norma(22, R));
            }

            fcn_form::wait_report();
            clear_screen();
        }
    }

    void f3_svd(){
        int n = 6, d = 6;

        while (f3_svd_form::prompt(n, d)) 
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


        

            fcn_form::wait_report();
            clear_screen();
       }
    }

    void f3_svd_pca(){
        F3SvdPcaParams params{100, 10, 2.0f, 2.0f};  // valori di default

        while (f3_svd_pca_form::prompt(params))
        {
            const int n = params.n;
            const int d = params.d;
            const float nmul = params.factor_n;
            const float dmul = params.factor_d;

            // ── TODO 7: PCA e scatter plot ────────────────────────────────────────────
            clear_screen();
            // cout << "Parametri di ritorno da form " << n << "\t" << d << "\t" << nmul << "\t" << dmul << endl ;
            // primo giro prende n come base ed a pari d plotta 4 scatter nx1, nx2, nx4, nx8
            // secondo giro prende d come base ed a pari n plotta 4 scatter dx1, dx2, dx4, dx8
            // params.factor_n e params.factor_d sono ora disponibili alla business logic;
            // i due fattori 2 cablati qui sotto restano volutamente invariati in questo refactoring.
            // bool first_run = true;
            for (int itab = 0; itab < 3; itab++)
            {
                if (itab == 0 && nmul <= 1.0f) continue;
                if (itab == 1 && dmul <= 1.0f) continue;

                std::string pca_title = "PCA — prime 2 componenti principali ";
                if (itab == 0) pca_title += "- progress su n righe (prove)"; 
                if (itab == 1) pca_title += "- progress su d colonne (features)"; 
                if (itab == 2) pca_title += "- progress sia su n che su d"; 
                auto fig = matplot_table_init(true, "PCA", pca_title, 2, 2 );
 
                int npoints = n;
                int ncols = d;


                for (int ik = 0; ik < 4; ik++)
                {
                    // if (first_run) {
                    if (itab == 0 && ik > 0 ) {
                        npoints = (int) (npoints * nmul);
                        ncols = d ;
                    } else if (itab == 1 && ik > 0) {
                        npoints = n;
                        ncols = (int) (ncols * dmul);
                    } else if (itab == 2 && ik > 0){
                        npoints = (int) (npoints * nmul);
                        ncols = (int) (ncols * dmul);
                    }

                    cout << endl << "in generazione cluster da " << npoints << " X " << ncols << " punti\n";

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
                        pc1[i] += U[i][0] * sigma[0];
                        pc2[i] += U[i][1] * sigma[1];

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
                // first_run = !first_run;   // proviamo ad usare itab == 0 per n e itab == 1 per d
            }
            // e) Prova con d variabile: {2, 5, 20, 50}
            // f) Stampa sigma[0], sigma[1] vs il resto: cosa noti?

            fcn_form::wait_report();
            clear_screen();
       }
    }

    void f3_svd_an(){
        
        F3SvdAnParams params{};

        while (f3_svd_an_form::prompt(params)) {
            const int n = params.n;
            const auto& d = params.d;
            const int seed = params.seed;
            clear_screen();
			
            // ── TODO 7: PCA e scatter plot ────────────────────────────────────────────
            
			auto fig = matplot_table_init(true, "PCA", "PCA — prime due componenti principali", 2, 2 );

			for (int ik = 0; ik < 4; ik++) 
			{
				int npoints;
				int ncols;

				npoints = n;
				ncols = d[ik];

				cout << "in generazione cluster da " << npoints << " X " << ncols << " punti\n";

				auto [X, label] = genera_cluster(npoints, ncols, seed);
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
                Mat Vb = matrix_trasposta(Vb_t); // raddrizzo perche' alglib rende Vb_t   
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
					pc1[i] += U[i][0] * sigma[0];
					pc2[i] += U[i][1] * sigma[1];					
				}
				//    (equivalente a U[i][k] * sigma[k]) // ok X_v_i = sigma_i_u_i

				// TODO
				cout << std::setfill(' ') << std::setw(12) << std::fixed << std::setprecision(2) 
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
            fcn_form::wait_report();
        }
    }

	void sym_cauchy() {
		SymCauchyParams params{};

		while (sym_cauchy_form::prompt(params)) {
			const int n = params.n;                 // NODI, non intervalli
			const double t0 = params.t0;           // indipendente da T
			const double T = params.T;
			const double x0 = params.x0;
			const double a_arg = params.a_arg;
			const double f_arg = params.f_arg;

			Vec t, avals, fvals, b;
			Mat L;
			clear_screen();
			auto fig = matplot_table_init(
				true, "Cauchy", "Soluzione numerica del problema di Cauchy", 2, 2);
			matplot::legend();
			Vec x;
			{
				// PROVA 1 (pannelli 0,1).  a(t)=0, f(t)=1/(1+sin^2(t)).
				// backw=true: il valore x0 e' imposto in T, NON in t0.
				matrix_build_cauchy(
					n, t0, T, x0, t, avals, fvals, L, b,
					f_const, 0, f_sin2plus1, 0, true);
				x = linear_subst_BW(L, b);
				fig->nexttile(0);
				imagesc(L)->display_name("Verifica matrice");
				colorbar();
			}
			{
				fig->nexttile(1);
				auto p = plot(t, x);
				p->line_width(2);
				p->display_name("1 / (1 + sin^2(t))");
				xlabel("t");
				ylabel("f(x(t))");
			}
			{
				// PROVA 2 (pannello 2). a(t)=cos(t), stessa f(t).
				// backw=false: condizione z(t0)=x0.
				matrix_build_cauchy(
					n, t0, T, x0, t, avals, fvals, L, b,
					f_cos, 0, f_sin2plus1, 0, false);
				x = linear_subst_FW(L, b);
				fig->nexttile(2);
				auto p = plot(t, x);
				p->line_width(2);
				p->display_name(" x(t) + 1 / (1 + sin^2(t))");
				xlabel("t");
				ylabel("f(x(t))");
			}
			{
				// PROVA 3 (pannello 3). a(t)=a_arg, f(t)=f_arg.
				// f_const e' trattata come sentinella per i due valori costanti.
				// z(t0)=1 e' una scelta dell'esperimento originale: NON usa x0.
				matrix_build_cauchy(
					n, t0, T, 1, t, avals, fvals, L, b,
					f_const, a_arg, f_const, f_arg, false);
				x = linear_subst_FW(L, b);
				fig->nexttile(3);
				auto p = plot(t, x);
				p->line_width(2);
				p->display_name(" x^-1(t) ");
				xlabel("t");
				ylabel("f(x(t))");
			}

			fig->draw();
			// fcn_form::wait_report();
		}
	}
 
    void sym_hhalpha() {
        SymHHAlphaParams params{};
        while (sym_hhalpha_form::prompt(params)) {
            clear_screen();
            const int n = params.n; // numero di pounti della griglie
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
            fcn_form::wait_report();
        };
    };

    void ode_euler()
    {
        OdeEulerParams params{};

        while (ode_euler_form::prompt(params)) {
            const auto& N_values = params.N;

            const double a = 0.0;
            const double b = params.b;
            const double y0 = params.y0;

            ode_set_logistic_params(
                ode_g_logistic.r,
                ode_g_logistic.K,
                y0
            );

            clear_screen(); 
            for (int istage = 0; istage < 2; istage++)
            {

                Vec h_vals;
                Vec err_euler_vals;
                Vec err_heun_vals;
                Vec err_rk4_vals;

                auto fig = matplot_table_init(true, "Eulero vs Heun vs RK4", "Metodi iterativi per problema di Cauchy, approssimazione", 2, 2);            
                matplot::legend();

                if (istage == 0) {
                    std::cout << std::string(23, '_')
                        << "decay (errore in b)" 
                        << std::string(23, '_')
                        << "\n";
                } else {
                    std::cout << std::string(21, '_')
                        << "logistica:(errore in b)" 
                        << std::string(21, '_')
                        << "\n";
                }
                std::cout << std::setw(6)  << "N"
                        << std::setw(12) << "h"
                        << std::setw(16) << "Euler"
                        << std::setw(16) << "Heun"
                        << std::setw(16) << "RK4"
                        << "\n";
                std::cout << std::string(66, '_') << "\n";

                for (std::size_t k = 0; k < N_values.size(); ++k) {
                    const int N = N_values[k];  
                    if (N <= 0) continue;

                    // passo e nodi equidistanti
                    const double h = h_ticks(a, b, N+1); // n_ticks vuole nodi
                    Vec t_nodes = nodi_equidistanti(a, b, N+1); // dovrebbe riempire t_nodes con N_values[k]+1 nodi
                    // vettori di soluzione
                    Vec y_euler_vec(N+1);
                    Vec y_heun_vec(N+1);
                    Vec y_rk4_vec(N+1);
                    Vec y_exact_vec(N+1);

                    // condizioni iniziali
                    double y_euler = y0;
                    double y_heun  = y0;
                    double y_rk4  = y0;

                    y_euler_vec[0] = y_euler;
                    y_heun_vec[0]  = y_heun;
                    y_rk4_vec[0]  = y_rk4;
                    y_exact_vec[0] = y0; // esatta in a
                    double y_exact;

                    for (int i = 0; i < N; ++i) {
                        double t = t_nodes[i];
                        if (istage == 0) {
                            ode_scalar_step_euler(t, h, &y_euler, ode_scalar_rhs_decay);
                            ode_scalar_step_heun(t, h, &y_heun, ode_scalar_rhs_decay);
                            ode_scalar_step_rk4(t, h, &y_rk4, ode_scalar_rhs_decay);
                        } else {
                            ode_scalar_step_euler(t, h, &y_euler, ode_scalar_rhs_logistic);
                            ode_scalar_step_heun(t, h, &y_heun, ode_scalar_rhs_logistic);
                            ode_scalar_step_rk4(t, h, &y_rk4, ode_scalar_rhs_logistic);
                        }
                        y_euler_vec[i+1] = y_euler;
                        y_heun_vec[i+1]  = y_heun;
                        y_rk4_vec[i+1]  = y_rk4;
                        double y_ex;
                        if (istage == 0) {
                            y_ex = y0 * std::exp(-1.0 * (t_nodes[i+1] - a));
                            y_exact = y0 * std::exp(-1.0 * (b - a));
                        } else {
                            y_ex = ode_scalar_exact_logistic(t_nodes[i+1] - a);
                            y_exact = ode_scalar_exact_logistic(b - a);
                        }
                        y_exact_vec[i+1] = y_ex;
                    }
                    double err_euler = std::fabs(y_euler - y_exact);
                    double err_heun = std::fabs(y_heun - y_exact);
                    double err_rk4 = std::fabs(y_rk4 - y_exact);
                    h_vals.push_back(h);
                    err_euler_vals.push_back(err_euler);
                    err_heun_vals.push_back(err_heun);
                    err_rk4_vals.push_back(err_rk4);

                    std::cout << std::setw(6)  << N
                            << std::setw(12) << h
                            << std::setw(16) << err_euler
                            << std::setw(16) << err_heun
                            << std::setw(16) << err_rk4
                            << "\n";

                    // Visualizzazione della soluzione numerica x(t)
                    fig->nexttile(k);
                    auto ax=fig->current_axes();
                    ax->title("Confronto per N intervalli "+itostr(N));
                    hold(on);

                    // q->use_y2(true);
                    // ax->y2_axis().limits({-1.6, 1.6});  

                    auto p_eu = plot(t_nodes, y_euler_vec);
                    p_eu->line_style("-");
                    p_eu->line_width(5);
                    p_eu->color("magenta");
                    p_eu->marker("");
                    p_eu->display_name("Euler");

                    auto p_he = plot(t_nodes, y_heun_vec);
                    p_he->line_width(5);
                    p_he->color("cyan");
                    p_he->line_style("--");
                    p_he->marker("");
                    p_he->display_name("Heun");

                    auto p_rk = plot(t_nodes, y_rk4_vec);
                    p_rk->line_width(5);
                    p_rk->color("yellow");
                    p_rk->line_style("-");
                    p_rk->marker("");
                    p_rk->display_name("Runge-Kutta 4th");

                    auto p_ex = plot(t_nodes, y_exact_vec);
                    p_ex->color("black");
                    p_ex->line_style("-");
                    p_ex->marker("");
                    p_ex->line_width(2);
                    p_ex->display_name("Exact");

                    // p2->marker(".");
                    // p1->marker("+");
                    // pI->marker("r");
    
                    // pI->color("magenta");
                    // pf->color("cyan");
                    // ptn2->color("blue");

                    if (istage==0) {
                        matplot_legend_align(matplot::legend(), LeAl::Top+LeAl::Right, 0,0);
                    }else{
                        matplot_legend_align(matplot::legend(), LeAl::Bottom+LeAl::Right, 0,0);
                    }
                    matplot::legend();
                    xlabel("t (nodi equidistanti)");
                    ylabel("y approssimata");

                }

                std::cout << std::string(66, '_') << "\n\n";
            
                fig->draw();

                auto fig_err = matplot_table_init(true, "Eulero vs Heun vs RK4", "Metodi iterativi per problema di Cauchy, O(h) dell' errore globale in b", 1, 1);
                matplot::hold(on);
                fig_err->nexttile(0); //  ed unico
                auto ax=fig_err->current_axes();

                // Eulero
                auto pe = matplot::loglog(h_vals, err_euler_vals);
                pe->line_style("--");
                pe->marker("");
                pe->line_width(4);
                pe->color("green");
                pe->display_name("Euler error");

                // Heun
                auto ph = matplot::loglog(h_vals, err_heun_vals);
                ph->line_style("- ");
                ph->marker("");
                ph->line_width(4);
                ph->color("cyan");
                ph->display_name("Heun error");

                // Heun
                auto pr = matplot::loglog(h_vals, err_rk4_vals);
                pr->line_style(" -");
                pr->marker("");
                pr->line_width(4);
                pr->color("magenta");
                pr->display_name("RK4 error");

                matplot::xlabel("h");
                if (istage == 0) {
                    matplot::ylabel("exp decay |y(b) - y_num(b)|");
                } else { 
                    matplot::ylabel("logistic |y(b) - y_num(b)|");
                }
                matplot_legend_align(matplot::legend(), LeAl::Bottom+LeAl::Right, 0,0);
                matplot::legend();
                fig_err->draw();
            }
            fcn_form::wait_report();
        };
    };
 
    void ode_osc2d() {
        //
        // confronto Eulero-Heun test runner
        //
        OdeOsc2DParams params{};
        MatStr Table_des(4, VecStr(5));
        // Oscillatore smorzato (Omega = sqrt(omega^2 - gamma^2))


        Table_des[0][0] = "q(t) vs t con ";
        Table_des[0][2] = "RK4";
        Table_des[0][3] = "Esatta";
        Table_des[0][4] = "green";

        Table_des[1][0] = "p(t) vs t con "; 
        Table_des[1][2] = "RK4";
        Table_des[1][3] = "Esatta";
        Table_des[1][4] = "cyan";

        Table_des[2][0] = "q(t) vs p(t) con "; 
        Table_des[2][2] = "RK4";
        Table_des[2][3] = "Esatta";
        Table_des[2][4] = "red";

        Table_des[3][0] = "Error(t)"; 
        Table_des[3][2] = "Errore q(t)";
        Table_des[3][3] = "Errore p(t)";
        Table_des[3][4] = "green";
        const string libero =   "Oscillatore libero:   ";
        const string smorzato = "Oscillatore smorzato: ";
        while (ode_osc2d_form::prompt(params)) {
            const auto& N_values = params.N;
            const double a = 0.0;
            const double b = params.b;
            const double q0 = params.q0;
            const double p0 = params.p0;
            const double omega = params.omega;
            const double gamma = params.gamma;
            clear_screen();    // for (istage=0;istage<2;...): INVARIATO

            for (int istage = 0; istage < 2; istage ++ ) {

                if (istage == 0) {
                    Table_des[0][1] =
                        libero + "q(t) =\\n"
                        "q0*cos(omega*t) + p0/omega*sin(omega*t)";
                    Table_des[1][1] =
                        libero + "p(t) =\\n"
                        "-omega*q0*sin(omega*t) + p0*cos(omega*t)";
                    Table_des[2][1] = 
                        libero + "diagramma di fase";
                    Table_des[3][1] = 
                        libero + "errore nel tempo";
                }
                else {
                    Table_des[0][1] = 
                        smorzato + "q(t) =\\n"
                        "exp(-gamma*t) [q0*cos(Omega*t)"
                        " + (p0+gamma*q0)/Omega*sin(Omega*t)]";

                    Table_des[1][1] =
                        smorzato + "p(t) =\\n"
                        "exp(-gamma*t) [p0*cos(Omega*t)"
                        " - (omega^2*q0+gamma*p0)/Omega*sin(Omega*t)]";
                    Table_des[2][1] = 
                        smorzato + "diagramma di fase";
                    Table_des[3][1] = 
                        smorzato + "errore nel tempo";
                }

                ode_set_oscillator_omega(omega); // loop successivi, se reimpostato utente
                ode_set_oscillator_gamma(gamma); // loop successivi, se reimpostato utente

                // vettori di soluzione(matrici con un vettore per nodelist 0..3)
    
                Mat q_euler_mat(4);
                Mat p_euler_mat(4);
                Mat q_exact_mat(4);
                Mat p_exact_mat(4);
                Mat t_nodes_mat(4);



                for (std::size_t k = 0; k < N_values.size(); ++k) {
                    const int N = N_values[k];
                    if (N <= 0) continue;

                    q_euler_mat[k].resize(N + 1);
                    p_euler_mat[k].resize(N + 1);
                    q_exact_mat[k].resize(N + 1);
                    p_exact_mat[k].resize(N + 1);
                    t_nodes_mat[k].resize(N + 1);

                    // passo e nodi equidistanti
                    const double h = h_ticks(a, b, N+1); // n_ticks vuole nodi
                    t_nodes_mat[k] = nodi_equidistanti(a, b, N+1); // dovrebbe riempire t_nodes con N_values[k]+1 nodi

                    double p0_w = p0 / omega; 
                    double w_q0 = - omega * q0; 
                    double y[2];   // stato: y[0] = q, y[1] = p
                    y[0] = q0;
                    y[1] = p0;

                    // condizioni iniziali
                    q_euler_mat[k][0] = y[0];
                    p_euler_mat[k][0] = y[1];
                    q_exact_mat[k][0] = y[0];
                    p_exact_mat[k][0] = y[1];

                    for (int i = 0; i < N; ++i) {
                        double t = t_nodes_mat[k][i];
                            // In questo punto il problema è vettoriale:
                            //   - y è lo stato del sistema: y[0] = q, y[1] = p
                            //   - 2 è la dimensione logica del sistema (numero di componenti)
                            // Lo stepper lavora in stile C: (double* y, int n, rhs).
                            //
                            // Esempi di chiamata:
                            //
                            // double y[2];  // stato corrente
                            // ode_vecN_step_euler(t, h, y, 2, ode_vec2_rhs_oscillator);
                            //
                            // Vec y(2, 0.0);
                            // ode_vecN_step_euler(t, h, y, y.size(), ode_vec2_rhs_oscillator);
                            // 
                        if (istage == 0) {
                            //
                            // Oscillatore libero 
                            //
                            ode_vecN_step_rk4(t, h, y, 2 , ode_vec2_rhs_oscillator);
                        } else {
                            //
                            // Oscillatore smorzato 
                            //
                            ode_vecN_step_rk4(t, h, y, 2 , ode_vec2_rhs_damped);
                        }
                        q_euler_mat[k][i+1] = y[0];
                        p_euler_mat[k][i+1] = y[1]; 
                        // std::cout << "omega local  = " << omega << "\n";
                        // std::cout << "omega global = " << ode_oscillator_omega << "\n";

                        // originale oscillatore libero 
                        // double wt = omega * t_nodes_mat[k][i+1];  
                        // q_exact_mat[k][i+1] = q0 * f_cos(wt) + p0_w * f_sin(wt);
                        // p_exact_mat[k][i+1] = w_q0 * f_sin(wt) + p0 * f_cos(wt);
                        if (istage == 0) {
                            //
                            // Oscillatore libero 
                            //
                            // rifatta su param globali oscillatore libero 
                            q_exact_mat[k][i+1] = ode_vec2_exact_oscillator_q(t_nodes_mat[k][i+1], q0, p0);
                            p_exact_mat[k][i+1] = ode_vec2_exact_oscillator_p(t_nodes_mat[k][i+1], q0, p0);
                        } else {
                            //
                            // Oscillatore smorzato 
                            //
                            // su param globali per oscillatore smorzato 
                            q_exact_mat[k][i+1] = ode_vec2_exact_damped_q(t_nodes_mat[k][i+1], q0, p0);
                            p_exact_mat[k][i+1] = ode_vec2_exact_damped_p(t_nodes_mat[k][i+1], q0, p0);
                        }
                    }
                }

                TablePlot_Oscillator(
                    0,              // qui gli devo passare quale table plottare
                    q_euler_mat, q_exact_mat, t_nodes_mat,
                    N_values.data(), Table_des 
                );


                TablePlot_Oscillator(
                    1,              // qui gli devo passare quale table plottare
                    p_euler_mat, p_exact_mat, t_nodes_mat,
                    N_values.data(), Table_des 
                );

                PhasePlot_Oscillator(
                    2, 
                    q_euler_mat, p_euler_mat,
                    q_exact_mat, p_exact_mat,
                    N_values.data(), Table_des
                );

                ErrorPlot_Oscillator(
                    3,                  
                    q_euler_mat, q_exact_mat,
                    p_euler_mat, p_exact_mat,
                    t_nodes_mat, N_values.data(), Table_des
                );

                /*
                            manc`a solo questa tabellina sull' errore globale
                            
                max_abs_err_q[k] = max_j |q_euler_mat[k][j] - q_exact_mat[k][j]|,

                max_abs_err_p[k] = max_j |p_euler_mat[k][j] - p_exact_mat[k][j]|           
                */
            }   

            // fcn_form::wait_report();
        };
    };

    void ode_lotkav() {

        OdeLotkaVParams params{};

        MatStr Table_des(4, VecStr(5));
        Table_des[2][0] = "q(t) vs p(t) con ";
        Table_des[2][1] = "Lotka-Volterra: diagramma di fase";
        Table_des[2][2] = "Eulero";
        Table_des[2][3] = "";
        Table_des[2][4] = "red";

        while (ode_lotkav_form::prompt(params)) {
            const auto& N_values = params.N;

            const double a  = 0.0;
            const double b  = params.b;
            const double q0 = params.q0;
            const double p0 = params.p0;
            // Se servissero localmente anche i coefficienti del modello:
            // const double alpha = params.alpha;
            // const double beta  = params.beta;
            // const double gamma = params.gamma;
            // const double delta = params.delta;
            // ma non e' il nostro caso, passano direttamente
            // dal form al setter, quindi allo stato globale letto dalla rhs

            ode_set_lotka_volterra_params( 
                params.alpha,
                params.beta,
                params.gamma,
                params.delta
            );

            clear_screen();

        // Qui riprende il corpo originale:
        // vettori di soluzione...
           // ode_get_lotka_volterra_params(alpha, beta, gamma, delta); // reimpostato da user per next loop

            // vettori di soluzione(matrici con un vettore per nodelist 0..3)
 
            Mat q_euler_mat(4);
            Mat p_euler_mat(4);
            Mat q_exact_mat(4);
            Mat p_exact_mat(4);
            Mat t_nodes_mat(4);



            for (int k = 0; k < N_values.size(); ++k) {
                const int N = N_values[k];  

                if (N <= 0) continue;

                q_euler_mat[k].resize(N + 1);
                p_euler_mat[k].resize(N + 1);
                q_exact_mat[k].resize(N + 1);
                p_exact_mat[k].resize(N + 1);
                t_nodes_mat[k].resize(N + 1);

                // passo e nodi equidistanti
                const double h = h_ticks(a, b, N+1); // n_ticks vuole nodi
                t_nodes_mat[k] = nodi_equidistanti(a, b, N+1); // dovrebbe riempire t_nodes con N_values[k]+1 nodi

                double y[2];   // stato: y[0] = q, y[1] = p
                y[0] = q0;
                y[1] = p0;

                // condizioni iniziali
                q_euler_mat[k][0] = y[0];
                p_euler_mat[k][0] = y[1];
                q_exact_mat[k][0] = y[0];
                p_exact_mat[k][0] = y[1];

                for (int i = 0; i < N; ++i) {
                    double t = t_nodes_mat[k][i];
                        // In questo punto il problema è vettoriale:
                        //   - y è lo stato del sistema: y[0] = q, y[1] = p
                        //   - 2 è la dimensione logica del sistema (numero di componenti)
                        // Lo stepper lavora in stile C: (double* y, int n, rhs).
                        //
                        // Esempi di chiamata:
                        //
                        // double y[2];  // stato corrente
                        // ode_vecN_step_euler(t, h, y, 2, ode_vec2_rhs_oscillator);
                        //
                        // Vec y(2, 0.0);
                        // ode_vecN_step_euler(t, h, y, y.size(), ode_vec2_rhs_oscillator);
                        // 
                    //
                    // Lotka-Volterra
                    //
                    ode_vecN_step_euler(t, h, y, 2 , ode_vec2_rhs_lotkavolterra);

                    q_euler_mat[k][i+1] = y[0];
                    p_euler_mat[k][i+1] = y[1]; 

                    q_exact_mat[k][i+1] = 0; // ci interessa solo il phase diag, niente err
                    p_exact_mat[k][i+1] = 0; // 

                }
            }


            PhasePlot_Oscillator(
                2, 
                q_euler_mat, p_euler_mat,
                q_exact_mat, p_exact_mat,
                N_values.data(), Table_des,
                false
            );


            /*
                        manca solo questa tabellina sull' errore globale
                        
            max_abs_err_q[k] = max_j |q_euler_mat[k][j] - q_exact_mat[k][j]|,

            max_abs_err_p[k] = max_j |p_euler_mat[k][j] - p_exact_mat[k][j]|           
            */
            //
            // quando disponibile allora si reinserisce wait_report per vederla
            //
            
                // fcn_form::wait_report();
        };
    }

    void gg1() {

        Mat A = matrix_random(10);
        matrix_dump(A, "Full random");
        Gershgorin_execute(A);

        A = matrix_random_symmetric(10);
        matrix_dump(A, "Symmetric random");
        Gershgorin_execute(A);

        A = matrix_random_diagonally_dominant(10);
        matrix_dump(A, "Diag dominant");
        Gershgorin_execute(A);

        A = matrix_random_diagonally_dominant_symmetric(10);
        matrix_dump(A, "Symmetric Diag  dominant");
        Gershgorin_execute(A);

      
        fcn_form::wait_report();
        clear_screen();
    }
    void gg2(){

        fcn_form::wait_report();

        clear_screen();


    }

    void al_qr() {
        AlQRParams params{};

        while (al_qr_form::prompt(params)) {
            const int n = params.n; // RIGHE / osservazioni
            const int d = params.d; // COLONNE / feature
            clear_screen();

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

        // ── Verifica TODO 3: bidiagonalizzazione su matrice piccola ──────────────
            // Matrice 4x5 casuale
            std::mt19937 rng(42);
            std::normal_distribution<double> g(0.0, 1.0);
            Mat A = matrix_build_zero(n, d);
            for (auto& r : A) 
                for (double& v : r) 
                    v = g(rng);
            
            Mat Q, R; 
            linear_QR_dec(A, Q, R);
            if (n <= 20) {
                matrix_dump(Q, "Matrice Q unitaria");
                matrix_dump(R, "Matrice R upper");
            }           
            // matrix_test_ortogonale(Q, "matrice Q");
            Mat A_r = matrix_prodotto_matrix(Q,R);
            // matrix_dump(A, "Matrice originale");
            if (n <= 20) {
                matrix_dump(A_r, "Ar = Q * R (Ricostruita)");
            }
            Mat E = matrix_calcola_diff(A,A_r);
            printf(" Error_F_lab         ||A - Q*R||_F = %.2e\n", matrix_calcola_errore_Fr(A, A_r));
            printf(" Error_F_mia         ||A - Q*R||_F = %.2e\n", matrix_calcola_norma(-1, E));
            printf(" Norma spettrale mia ||A - Q*R||_2 = %.2e\n\n", matrix_calcola_norma(2, E));

            Vec b(n);
            for (double& v : b)
                v = g(rng);
            Vec x;
            double residuo = 0.0;

            // 2) Genera x_true (d)
            Vec x_true(d);
            for (double& v : x_true)
                v = g(rng);

            linear_QR_least_squares(A_r, b, x, residuo);
            vector_dump(b, 10, b.size(), "Termine noto b");
            vector_dump(x, 10, x.size(), "Risultato");
            cout << endl << "Residuo: " << residuo << endl << endl;

            Vec Ax = matrix_prodotto_vector(A, x);
            vector_dump(Ax, 10, Ax.size(), "Verifica Ax=b");

            Vec r(n);
            for (int i = 0; i < n; ++i) r[i] = Ax[i] - b[i];

            printf("||Ax - b||_2 (LS) = %.2e\n", vector_calcola_norma(2, r));

            cout << endl << "Ed ora proviamo al contrario b = A x_true + rumore" << endl <<endl;

            // 3) b = A*x_true + rumore controllato
            Vec b_clean = matrix_prodotto_vector(A, x_true);
            b       = vector_add_noise(b_clean, 1.0); // ad esempio 1 millesimo

            linear_QR_least_squares(A, b, x, residuo);
            Ax = matrix_prodotto_vector(A, x); // Nuova soluzione, nuovo Ax
            Vec rn(n);
            for (int i = 0; i < n; ++i) rn[i] = Ax[i] - b[i];
            vector_dump(x_true, 10, x.size(), "x_true Imposto");
            vector_dump(x, 10, x.size(), "x Ricalcolato");
            printf("||Ax - b||_2 (LS) = %.2e\n", vector_calcola_norma(2, rn));

            Vec dx(d);
            for (int i = 0; i < d; ++i)
                dx[i] = x[i] - x_true[i];

            double rel_x = vector_calcola_norma(2, dx) / vector_calcola_norma(2, x_true);
            printf("Errore relativo su x: ||x - x_true||_2 / ||x_true||_2 = %.2e\n", rel_x);

            double nb  = vector_calcola_norma(2, b);
            double rel_r = residuo / nb;
            printf("Errore relativo su b: ||Ax - b||_2 / ||b||_2 = %.2e\n", rel_r);
 
 
            cout << endl << endl << "Verifica condizionamento" <<endl;

            Mat AtA = matrix_prodotto_AtA(A, true);

            Vec lambda;
            Mat V;
            linear_jacobi_autoval_simmetrica(AtA, lambda, V);

            // ordina in senso discendente
            matrix_ordina_diagonale(lambda, V, 1e-14, SortOrder::Desc);

            double lam_max = std::max(0.0, lambda.front());
            double lam_min = std::max(0.0, lambda.back());

            double sigma_max = std::sqrt(lam_max);
            double sigma_min = std::sqrt(lam_min);

            printf("\nAutovalori di A^T A ordinati:\n");
            for (int i = 0; i < (int)lambda.size(); ++i)
                printf(" lambda[%d] = %.8e\n", i, lambda[i]);

            printf("\nValori singolari stimati:\n");
            printf(" sigma_max = %.8e\n", sigma_max);
            printf(" sigma_min = %.8e\n", sigma_min);

            if (sigma_min > 1e-14)
            {
                double kappa2 = sigma_max / sigma_min;
                printf(" cond_2(A) stimata = %.8e\n", kappa2);
            }
            else
            {
                printf(" cond_2(A) stimata = inf (sigma_min ~ 0)\n");
            }
// ...
            fcn_form::wait_report();
       }
    }
    
    void JL_distanze() {
        JLDistanceParams params{};
        while (JL_distanze_form::prompt(params)) {
            clear_screen();
            const Vec x{0.0, 0.0};
            const Vec y{params.dx, params.dy};
            const Vec traslazione{params.t, params.t};
            const Vec xt{x[0] + traslazione[0], x[1] + traslazione[1]};
            const Vec yt{y[0] + traslazione[0], y[1] + traslazione[1]};
            const double prima = vector_distanza2(x, y);
            const double dopo = vector_distanza2(xt, yt);
            const double teorico = params.dx * params.dx + params.dy * params.dy;
            cout << "JL - collaudo distanza quadratica\n\n";
            cout << "D2(x,y)                    = " << prima << '\n';
            cout << "D2(x+t,y+t)                = " << dopo << '\n';
            cout << "dx^2 + dy^2                = " << teorico << '\n';
            cout << "Scarto traslazione         = " << std::abs(dopo-prima) << '\n';
            fcn_form::wait_report();
        }
    }

    void JL_distanze_dataset() {
        JLDistanzeDatasetParams params{};

        while (JL_distanze_dataset_form::prompt(params)) {
            clear_screen();

            cout << "\n=== PARAMETRI ESPERIMENTO JL ===\n";
            cout << "Numero osservazioni n: " << params.n << '\n';
            cout << "Dimensione originale d: " << params.d << '\n';
            cout << "Dimensione ridotta  k: " << params.k << '\n';
            cout << "Seed dataset X:        " << params.seed << '\n';
            cout << "Seed proiezione A:     " << params.seed_A << '\n';
            cout << "Distribuzione A:       Gaussiana\n";

            auto [X, label] = genera_cluster(params.n, params.d, params.seed);
            if (X.size() != label.size())
                throw std::runtime_error("Dataset: X e label incompatibili");

            // Due liste di indici: X rimane l'unico dataset.

            const int k = params.k; // se ti serve per report e controlli dimensionali
            const std::array<double, 6> soglie = {
                0.05, 0.10, 0.20, 0.30, 0.40, 0.50
            };

            std::array<size_t, 6> violazioni{};

            Mat A = matrix_random_jl(
                params.k, 
                params.d, 
                JLMatrixDistribution::Gaussian,
                static_cast<unsigned>(params.seed_A),
                 3
            );

            Mat Y = matrix_prodotto_matrix(X, matrix_trasposta(A)); // vettori in riga

            const double d2x = vector_distanza2(X[0], X[1]);
            const double d2y = vector_distanza2(Y[0], Y[1]);

            cout << "\n=== PRIMA PROIEZIONE JL ===\n";
            cout << "D2 originale  = " << d2x << '\n';
            cout << "D2 proiettata = " << d2y << '\n';
            cout << "Rapporto R01  = " << d2y / d2x << '\n';
            cout << "Distorsione   = " << std::abs(d2y / d2x - 1.0) << '\n';


            double somma_delta = 0.0;
            double max_delta = 0.0;
            size_t i_max = 0, j_max = 0;
            size_t coppie = 0;

            for (size_t i = 0; i < X.size(); ++i) {
                for (size_t j = i + 1; j < X.size(); ++j) {
                    double dx2 = vector_distanza2(X[i], X[j]);
                    if (dx2 == 0.0) continue;

                    double dy2 = vector_distanza2(Y[i], Y[j]);
                    double delta = std::abs(dy2 / dx2 - 1.0);

                    somma_delta += delta;
                    ++coppie;
                    for (size_t s = 0; s < soglie.size(); ++s) {
                        if (delta > soglie[s])
                            ++violazioni[s];
                    }
                    if (delta > max_delta) {
                        max_delta = delta;
                        i_max = i;
                        j_max = j;
                    }
                }
            }

            cout << "\n=== DISTORSIONI JL ===\n";
            cout << "Coppie valide:    " << coppie << '\n';
            cout << "Distorsione media: "
                << (coppie ? somma_delta / coppie : 0.0) << '\n';
            cout << "Distorsione max:  " << max_delta << '\n';
            cout << "Coppia peggiore:  (" << i_max << "," << j_max << ")\n";

            auto old_flags = cout.flags();
            auto old_precision = cout.precision();

            cout << "\n=== VIOLAZIONI SOGLIE JL ===\n";
            cout << "Epsilon     Coppie       Percentuale\n";

            for (size_t s = 0; s < soglie.size(); ++s) {
                double percentuale = coppie
                    ? 100.0 * violazioni[s] / coppie
                    : 0.0;

                cout << fixed
                    << setprecision(2)
                    << setw(7) << soglie[s]
                    << setw(12) << violazioni[s]
                    << setw(16) << percentuale << "%\n";
            }

            cout.flags(old_flags);
            cout.precision(old_precision);

            fcn_form::wait_report();
        }
    }

    void JL_distanze_dataMC() {
        JLDistanzeMCParams params{};

        constexpr std::array<double, 6> soglie{
            0.05, 0.10, 0.20, 0.30, 0.40, 0.50
        };
        constexpr size_t NS = soglie.size();

        struct Coppia {
            size_t i, j;
            double dx2;
        };

        // Ripristina lo stato numerico di cout anche in caso
        // di eccezione, tramite RAII.
        struct CoutGuard {
            std::ios saved{nullptr};
            CoutGuard() { saved.copyfmt(std::cout); }
            ~CoutGuard() { std::cout.copyfmt(saved); }
        };

        while (JL_distanze_dataMC_form::prompt(params)) {
            clear_screen();
            const size_t T = static_cast<size_t>(params.T);
            CoutGuard cout_guard;

            if (params.n < 2 || params.d < 1 ||
                params.k < 1 || params.k >= params.d)
                throw std::runtime_error(
                    "JL MC: dimensioni incompatibili");

            // 1. Dataset generato una sola volta.
            auto [X, label] =
                genera_cluster(params.n, params.d, params.seed);

            if (X.size() != label.size())
                throw std::runtime_error(
                    "JL MC: X e label incompatibili");

            // 2. Distanze originali precalcolate.
            std::vector<Coppia> coppie;
            size_t coincidenti = 0;

            for (size_t i = 0; i < X.size(); ++i) {
                for (size_t j = i + 1; j < X.size(); ++j) {
                    double dx2 = vector_distanza2(X[i], X[j]);

                    if (dx2 == 0.0) {
                        ++coincidenti;
                        continue;
                    }

                    coppie.push_back({i, j, dx2});
                }
            }

            const size_t M = coppie.size();
            if (M == 0)
                throw std::runtime_error(
                    "JL MC: nessuna coppia valida");

            // 3. Accumulatori Monte Carlo.
            std::array<size_t, NS> proiezioni_fallite{};
            std::array<std::uint64_t, NS> violazioni_totali{};

            double somma_medie = 0.0;
            double somma_massimi = 0.0;
            double minimo_massimi =
                std::numeric_limits<double>::infinity();
            double massimo_massimi = 0.0;

            size_t trial_migliore = 0;
            size_t trial_peggiore = 0;

            // Controllo preventivo overflow dei seed.
            using Seed = unsigned;
            const auto seed0 =
                static_cast<std::uint64_t>(params.seed_A);

            if (seed0 >
                static_cast<std::uint64_t>(
                    std::numeric_limits<Seed>::max()) - (T - 1))
                throw std::runtime_error(
                    "JL MC: intervallo seed_A non valido");

            // 4. Ciclo Monte Carlo.
            for (size_t t = 0; t < T; ++t) {
                Seed seed_A =
                    static_cast<Seed>(seed0 + t);

                Mat A = matrix_random_jl(
                    params.k,
                    params.d,
                    JLMatrixDistribution::Gaussian,
                    seed_A,
                    3
                );

                Mat Y = matrix_prodotto_matrix(
                    X, matrix_trasposta(A));

                std::array<size_t, NS> violazioni{};
                double somma_delta = 0.0;
                double max_delta = 0.0;

                for (const auto& c : coppie) {
                    double dy2 =
                        vector_distanza2(Y[c.i], Y[c.j]);

                    double delta =
                        std::abs(dy2 / c.dx2 - 1.0);

                    if (!std::isfinite(delta))
                        throw std::runtime_error(
                            "JL MC: distorsione non finita");

                    somma_delta += delta;
                    max_delta = std::max(max_delta, delta);

                    for (size_t s = 0; s < NS; ++s) {
                        if (delta > soglie[s])
                            ++violazioni[s];
                    }
                }

                const double media_delta =
                    somma_delta / static_cast<double>(M);

                if (t == 0) {
                    std::cout << "\n=== CONTROLLO PRIMO TRIAL ===\n"
                            << "Seed_A:           " << seed_A << '\n'
                            << "Distorsione media: " << media_delta << '\n'
                            << "Distorsione max:   " << max_delta << '\n';
                }
                somma_medie += media_delta;
                somma_massimi += max_delta;

                if (max_delta < minimo_massimi) {
                    minimo_massimi = max_delta;
                    trial_migliore = t;
                }
                if (max_delta > massimo_massimi) {
                    massimo_massimi = max_delta;
                    trial_peggiore = t;
                }

                for (size_t s = 0; s < NS; ++s) {
                    violazioni_totali[s] += violazioni[s];

                    if (violazioni[s] != 0)
                        ++proiezioni_fallite[s];
                }
            }

            // 5. Report.
            std::cout
                << "\n=== PARAMETRI JL MONTE CARLO ===\n"
                << "Numero osservazioni n: " << params.n << '\n'
                << "Dimensione originale d: " << params.d << '\n'
                << "Dimensione ridotta  k: " << params.k << '\n'
                << "Seed dataset X:        " << params.seed << '\n'
                << "Seed_A iniziale:       " << params.seed_A << '\n'
                << "Seed_A finale:         " << seed0 + T - 1 << '\n'
                << "Distribuzione A:       Gaussiana\n"
                << "Proiezioni T:          " << T << '\n'
                << "Coppie valide:         " << M << '\n'
                << "Coppie coincidenti:    " << coincidenti << '\n';

            std::cout << "\n=== STATISTICHE DISTORSIONI ===\n"
                << "Media delle distorsioni medie: "
                << somma_medie / T << '\n'
                << "Media delle distorsioni massime: "
                << somma_massimi / T << '\n'
                << "Minimo dei massimi: "
                << minimo_massimi << '\n'
                << "Massimo dei massimi: "
                << massimo_massimi << '\n'
                << "Seed migliore (max delta): "
                << seed0 + trial_migliore << '\n'
                << "Seed peggiore (max delta): "
                << seed0 + trial_peggiore << '\n';

            std::cout << "\n=== SOGLIE JL MONTE CARLO ===\n"
                << "Epsilon    Coppie %    Fallimenti    Fall. %\n";

            for (size_t s = 0; s < NS; ++s) {
                double pct_coppie =
                    100.0 * static_cast<double>(
                        violazioni_totali[s]) /
                    (static_cast<double>(T) * M);

                double pct_fallimenti =
                    100.0 * proiezioni_fallite[s] / T;

                std::cout << std::fixed
                    << std::setprecision(2)
                    << std::setw(7) << soglie[s]
                    << std::setw(12) << pct_coppie
                    << std::setw(14) << proiezioni_fallite[s]
                    << std::setw(12) << pct_fallimenti
                    << "%\n";
            }

            std::cout << "\n";
            fcn_form::wait_report();
        }
    }

    // Include standard, se non gia' presenti:
    // <algorithm> <cmath> <cstdint> <fstream> <iomanip> <iostream>
    // <iterator> <limits> <map> <set> <stdexcept> <string> <utility> <vector>
    // <chrono> <locale> <sstream>
    //
    // ATTENZIONE: primo prototipo numerico. Il 'k trovato' e' il primo
    // successo OSSERVATO nella griglia campionata, non il minimo matematico.

    void JL_stima_coeff_n() {
        const jl_files::Store files{"data"};
        JLCoeffNParams params{};

        struct Pair {
            std::size_t i;
            std::size_t j;
            double d2;
        };

        struct CoutGuard {
            std::ios saved{nullptr};
            CoutGuard() { saved.copyfmt(std::cout); }
            ~CoutGuard() { std::cout.copyfmt(saved); }
        };

        struct MCResult {
            std::vector<int> successi;  // numero di matrici che rispettano TUTTE le coppie
            double media_delta = 0.0;
            double media_massimi = 0.0;
        };

        // Basta modificare questi due valori per un diverso compromesso
        // tra precisione della ricerca e costo computazionale.
        constexpr int REFINE_STEPS = 3;
        constexpr std::size_t MAX_K_EVAL_PER_N = 28;

        while (JL_stima_coeff_n_form::prompt(params)) {
            clear_screen();
            CoutGuard cout_guard;

            // Progressioni: rispettiamo esattamente min/max e il passo richiesto.
            // Se il passo non raggiunge max, non inventiamo un ultimo punto.
            std::vector<int> n_values;
            for (std::int64_t n = params.n_min; n <= params.n_max;) {
                n_values.push_back(static_cast<int>(n));
                if (params.n_mode == 1) {
                    n += params.n_step;
                } else {
                    n *= params.n_step;
                }
            }

            std::vector<double> eps_values;
            for (int e = params.eps_min_pct;
                e <= params.eps_max_pct;
                e += params.eps_step_pct) {
                eps_values.push_back(static_cast<double>(e) / 100.0);
            }

            if (n_values.empty() || eps_values.empty())
                throw std::runtime_error("JL coefficiente: campagna senza valori");

            const int k_cap = params.d - 1;  // studiamo solo riduzioni effettive k < d
            const std::int64_t last_seed =
                static_cast<std::int64_t>(params.seed_A) + params.T - 1;
            if (last_seed > std::numeric_limits<unsigned>::max())
                throw std::runtime_error("JL coefficiente: seed_A finale fuori gamma");

            // Confronto discreto esatto con il successo richiesto in percentuale.
            const int richiesti =
                (params.T * params.success_pct + 99) / 100;

            const auto output = files.begin_run("coefficiente_n", "JL_K_C_n");
            const auto& run_id = output.id();
            auto scan_csv = output.open_new("scan.csv");
            auto summary_csv = output.open_new("summary.csv");
            // CSV: separatore virgola e punto decimale indipendenti dal locale.
            scan_csv.imbue(std::locale::classic());
            summary_csv.imbue(std::locale::classic());

            // Intestazioni CSV brevi e su UNA riga: adatte a parser e grafici.
            scan_csv << "run_id,n,d,k,eps,k0,T,seed_X,seed_A_start,"
                        "successi,successo_pct,media_delta,media_max,"
                        "obiettivo_pct,n_min,n_max,n_step,n_mode,"
                        "eps_min_pct,eps_max_pct,eps_step_pct,distribuzione\n";
            summary_csv << "run_id,n,d,eps,k0,k_non_successo,k_successo,"
                        "C_hat,successo_pct,T,seed_X,seed_A_start,"
                        "k_valutati,esito,obiettivo_pct,"
                        "n_min,n_max,n_step,n_mode,"
                        "eps_min_pct,eps_max_pct,eps_step_pct,distribuzione,"
                        "k_max_valutato,best_successi,best_successo_pct,best_k,"
                        "k_cap_testato,budget_esaurito,tempo_n_s\n";

            std::cout << "\n=== JL: STIMA COEFFICIENTE DI SCALA ===\n"
                    << "d = " << params.d
                    << "  T = " << params.T
                    << "  successo richiesto = " << params.success_pct << "%\n"
                    << "seed X = " << params.seed_X
                    << "  seed A iniziale = " << params.seed_A << "\n"
                    << "n da " << params.n_min << " a " << params.n_max
                    << " con passo/fattore=" << params.n_step
                    << " e modo=" << params.n_mode << '\n'
                    << "epsilon da " << params.eps_min_pct << "% a "
                    << params.eps_max_pct << "% con passo "
                    << params.eps_step_pct << " punti percentuali\n"
                    << "n: " << n_values.size() << " valori; "
                    << "epsilon: " << eps_values.size() << " soglie\n"
                    << "Campagna: " << output.campaign() << "  run: " << run_id << '\n'
                    << "Directory run: " << output.directory().string() << '\n'
                    << "Scansione k da min(k0), ancorata ai k0 (C=1),"
                        " e raffinamento locale.\n";

            for (const int n : n_values) {
                const auto n_start = std::chrono::steady_clock::now();
                auto [X, label] = genera_cluster(n, params.d, params.seed_X);
                if (X.size() != label.size())
                    throw std::runtime_error("JL coefficiente: X e label incompatibili");

                std::vector<Pair> pairs;
                for (std::size_t i = 0; i < X.size(); ++i) {
                    for (std::size_t j = i + 1; j < X.size(); ++j) {
                        const double d2 = vector_distanza2(X[i], X[j]);
                        if (!std::isfinite(d2) || d2 < 0.0)
                            throw std::runtime_error("JL coefficiente: distanza iniziale non valida");
                        if (d2 > 0.0) pairs.push_back({i, j, d2});
                    }
                }
                if (pairs.empty())
                    throw std::runtime_error("JL coefficiente: nessuna coppia distinta");

                const double log_n = std::log(static_cast<double>(n));
                std::vector<int> k0_values;
                k0_values.reserve(eps_values.size());
                for (double eps : eps_values)
                    k0_values.push_back(static_cast<int>(
                        std::ceil(log_n / (eps * eps))));

                // Calcoli effettuati: la chiave k evita ogni Monte Carlo duplicato.
                std::map<int, MCResult> cache;

                auto evaluate = [&](int k) -> bool {
                    if (k < 1 || k > k_cap) return false;
                    if (cache.find(k) != cache.end()) return true;
                    if (cache.size() >= MAX_K_EVAL_PER_N) return false;

                    std::cout << "\r n=" << std::setw(5) << n
                            << "  k=" << std::setw(5) << k
                            << "  MC=" << (cache.size() + 1)
                            << "/" << MAX_K_EVAL_PER_N << "   " << std::flush;

                    MCResult result;
                    result.successi.assign(eps_values.size(), 0);
                    double sum_medie = 0.0;
                    double sum_max = 0.0;

                    for (int t = 0; t < params.T; ++t) {
                        const unsigned seed_A = static_cast<unsigned>(
                            static_cast<std::uint64_t>(params.seed_A) + t);
                        Mat A = matrix_random_jl(
                            k, params.d, JLMatrixDistribution::Gaussian,
                            seed_A, 3);
                        Mat Y = matrix_prodotto_matrix(X, matrix_trasposta(A));

                        double sum_delta = 0.0;
                        double max_delta = 0.0;
                        for (const Pair& p : pairs) {
                            const double dy2 = vector_distanza2(Y[p.i], Y[p.j]);
                            const double delta = std::abs(dy2 / p.d2 - 1.0);
                            if (!std::isfinite(delta))
                                throw std::runtime_error("JL coefficiente: distorsione non finita");
                            sum_delta += delta;
                            max_delta = std::max(max_delta, delta);
                        }
                        sum_medie += sum_delta / static_cast<double>(pairs.size());
                        sum_max += max_delta;
                        for (std::size_t e = 0; e < eps_values.size(); ++e) {
                            if (max_delta <= eps_values[e])
                                ++result.successi[e];
                        }
                    }

                    result.media_delta = sum_medie / params.T;
                    result.media_massimi = sum_max / params.T;
                    cache.emplace(k, std::move(result));
                    return true;
                };

                const auto ok = [&](int k, const MCResult& r, std::size_t e) {
                    // Politica della campagna: per epsilon si valuta solo k>=k0.
                    return k >= k0_values[e] && r.successi[e] >= richiesti;
                };
                const auto all_ok = [&]() {
                    if (cache.empty()) return false;
                    const MCResult& r = cache.rbegin()->second;
                    for (std::size_t e = 0; e < eps_values.size(); ++e)
                        if (!ok(cache.rbegin()->first, r, e)) return false;
                    return true;
                };

                // Griglia grossolana in k: riferimento C=1 per ogni epsilon,
                // piu' crescita geometrica 1.5x fino al limite k<d.
                // Partenza dal piu' piccolo k0 (epsilon piu' grande):
                // una scelta operativa, NON un limite inferiore del lemma.
                std::set<int> coarse;
                const int start = std::max(1,
                    std::min(k_cap, k0_values.back()));
                for (const int k0 : k0_values)
                    if (k0 >= 1 && k0 <= k_cap) coarse.insert(k0);

                for (int k = start;;) {
                    coarse.insert(k);
                    if (k == k_cap) break;
                    const int next = std::max(k + 1,
                        static_cast<int>(std::ceil(1.5 * k)));
                    k = std::min(k_cap, next);
                }
                for (const int k : coarse) {
                    if (!evaluate(k)) break;
                    if (all_ok()) break;
                }

                // Raffina gli intervalli osservati [fallimento, successo].
                // Per ciascun epsilon usa la cache condivisa; non ripete mai
                // un Monte Carlo sullo stesso k.
                for (std::size_t e = eps_values.size(); e-- > 0;) {
                    for (int pass = 0; pass < REFINE_STEPS; ++pass) {
                        const auto upper = std::find_if(
                            cache.begin(), cache.end(),
                            [&](const auto& v) { return ok(v.first, v.second, e); });
                        if (upper == cache.end()) break;

                        // Il limite k0-1 delimita la ricerca, ma NON e'
                        // un fallimento osservato. Salta tutti i valori sotto k0.
                        int low = k0_values[e] - 1;
                        for (auto it = cache.begin(); it != upper; ++it)
                            if (it->first >= k0_values[e] &&
                                !ok(it->first, it->second, e))
                                low = std::max(low, it->first);

                        const int high = upper->first;
                        if (high - low <= 1) break;
                        const int mid = low + (high - low) / 2;
                        if (!evaluate(mid)) break;
                    }
                }

                const double elapsed_s = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - n_start).count();

                std::cout << "\n\n n=" << n << "  coppie=" << pairs.size()
                        << "  k valutati=" << cache.size()
                        << "  k massimo valutato=" << cache.rbegin()->first
                        << "  k limite=" << k_cap
                        << "  tempo=" << std::fixed << std::setprecision(2)
                        << elapsed_s << " s" << '\n';
                std::cout << "                k non   k con               riusc.\n"
                            "     eps    k0    riusc.  riusc.    C_hat       %   esito\n";

                // Esporta l'intera curva (n,k,epsilon), ordinata per k.
                // Formattazione locale per CAMPO: niente 0.30000000000000004.
                for (const auto& [k, r] : cache) {
                    for (std::size_t e = 0; e < eps_values.size(); ++e) {
                        scan_csv << run_id << ',' << n << ',' << params.d << ','
                                << k << ',' << std::fixed << std::setprecision(2)
                                << eps_values[e] << ',' << k0_values[e] << ','
                                << params.T << ',' << params.seed_X << ','
                                << params.seed_A << ',' << r.successi[e] << ','
                                << std::setprecision(2)
                                << (100.0 * r.successi[e] / params.T) << ','
                                << std::setprecision(6) << r.media_delta << ','
                                << r.media_massimi << ','
                                << params.success_pct << ','
                                << params.n_min << ',' << params.n_max << ','
                                << params.n_step << ',' << params.n_mode << ','
                                << params.eps_min_pct << ',' << params.eps_max_pct << ','
                                << params.eps_step_pct << ",Gaussian" << '\n';
                    }
                }

                // Cerca la prima dimensione CAMPIONATA con successo sufficiente.
                // Il massimo k osservato e il migliore successo distinguono
                // l'obiettivo non raggiunto dalla mancanza totale di successi.
                for (std::size_t e = 0; e < eps_values.size(); ++e) {
                    const auto upper = std::find_if(
                        cache.begin(), cache.end(),
                        [&](const auto& v) { return ok(v.first, v.second, e); });

                    const bool found = (upper != cache.end());
                    const bool at_cap = (cache.count(k_cap) != 0);
                    const bool budget_full = cache.size() >= MAX_K_EVAL_PER_N;
                    const char* status = found ? "SUCCESSO" :
                        (k0_values[e] > k_cap ? "K0_FUORI_RANGE" :
                        (at_cap ? "NON_RAGGIUNTA_CON_K<D" :
                        (budget_full ? "BUDGET_ESAUTITO" : "GRIGLIA_INCOMPLETA")));

                    const int k_hit = found ? upper->first : -1;
                    // Riporta solo il piu' alto k<k_hit realmente fallito.
                    int k_fail = -1;
                    if (found) {
                        for (auto it = cache.begin(); it != upper; ++it)
                            if (it->first >= k0_values[e] &&
                                !ok(it->first, it->second, e))
                                k_fail = it->first;
                    }
                    const double c_hat = found
                        ? static_cast<double>(k_hit) * eps_values[e] * eps_values[e] / log_n
                        : -1.0;
                    const double pct = found
                        ? 100.0 * upper->second.successi[e] / params.T : 0.0;

                    int best_successi = -1;
                    int best_k = -1;
                    for (const auto& [k, r] : cache) {
                        if (k < k0_values[e]) continue;
                        if (r.successi[e] > best_successi || (r.successi[e] == best_successi && k > best_k)) {
                            best_successi = r.successi[e];
                            best_k = k;
                        }
                    }
                    const double best_pct = best_successi >= 0
                        ? 100.0 * best_successi / params.T : -1.0;

                    std::cout << std::right << std::fixed << std::setprecision(2)
                            << std::setw(8) << eps_values[e]
                            << std::setw(6) << k0_values[e];
                    if (found) {
                        if (k_fail >= 0) std::cout << std::setw(9) << k_fail;
                        else std::cout << std::setw(9) << "--";
                        std::cout << std::setw(9) << k_hit
                                << std::setw(9) << c_hat
                                << std::setw(8) << pct << "%  " << status << '\n';
                    } else {
                        std::cout << std::setw(9) << "--"
                                << std::setw(9) << "--"
                                << std::setw(9) << "--";
                        if (best_successi >= 0)
                            std::cout << std::setw(8) << best_pct << '%';
                        else
                            std::cout << std::setw(9) << "--";
                        std::cout << "  " << status << '\n';
                        if (best_successi >= 0)
                            std::cout << "         Migliore osservato con k>=k0: "
                                    << best_successi << '/' << params.T
                                    << " a k=" << best_k << '\n';
                        std::cout << "         k_max_testato="
                                << cache.rbegin()->first << '\n';
                    }

                    summary_csv << run_id << ',' << n << ',' << params.d
                                << ',' << std::fixed << std::setprecision(2)
                                << eps_values[e] << ',' << k0_values[e] << ',';
                    if (found) {
                        if (k_fail >= 0) summary_csv << k_fail;
                        summary_csv << ',' << k_hit << ','
                                    << std::setprecision(4) << c_hat << ','
                                    << std::setprecision(2) << pct;
                    } else {
                        // Quattro campi vuoti: assenza di misura, non zero.
                        summary_csv << ",,,";
                    }
                    summary_csv << ',' << params.T << ',' << params.seed_X
                                << ',' << params.seed_A << ',' << cache.size()
                                << ',' << status
                                << ',' << params.success_pct << ','
                                << params.n_min << ',' << params.n_max << ','
                                << params.n_step << ',' << params.n_mode << ','
                                << params.eps_min_pct << ',' << params.eps_max_pct << ','
                                << params.eps_step_pct << ",Gaussian"
                                << ',' << cache.rbegin()->first
                                << ',';
                    if (best_successi >= 0) summary_csv << best_successi;
                    summary_csv << ',';
                    if (best_pct >= 0.0) summary_csv << std::setprecision(2) << best_pct;
                    summary_csv << ',';
                    if (best_k >= 0) summary_csv << best_k;
                    summary_csv
                                << ',' << (at_cap ? 1 : 0)
                                << ',' << (budget_full ? 1 : 0)
                                << ',' << std::fixed << std::setprecision(3)
                                << elapsed_s << '\n';
                }
                scan_csv.flush();
                summary_csv.flush();
            }

            scan_csv.close();
            summary_csv.close();
            if (!scan_csv || !summary_csv)
                throw std::runtime_error("JL coefficiente: errore di scrittura in " +
                                         output.directory().string());
            std::cout << "\nCSV prodotti:\n  "
                    << output.result_path("scan.csv").string() << "\n  "
                    << output.result_path("summary.csv").string() << '\n'
                    << "\nNota: C_hat e' una stima sulla griglia campionata;"
                        " non una costante universale JL.\n";
            // CoutGuard ripristina flags e precision dopo il report.
            fcn_form::wait_report();
        }
    }

    // ATTENZIONE: campagna di controllo di indipendenza da d.
    // Il dataset cambia dimensionalita' con d; il seed non garantisce
    // annidamento delle righe tra dataset di d differenti.
    // A parita' di n, k0 e' indipendente da d. Il k massimo esplorato e'
    // d_min-1 per rendere confrontabile l'intervallo dei k tra tutti i d.

    // Primo prototipo numerico. Il 'k trovato' e' il primo
    // successo OSSERVATO nella griglia campionata, non il minimo matematico.

    void JL_stima_coeff_d() {
        const jl_files::Store files{"data"};
        JLCoeffDParams params{};

        struct Pair {
            std::size_t i;
            std::size_t j;
            double d2;
        };

        struct CoutGuard {
            std::ios saved{nullptr};
            CoutGuard() { saved.copyfmt(std::cout); }
            ~CoutGuard() { std::cout.copyfmt(saved); }
        };

        struct MCResult {
            std::vector<int> successi;  // numero di matrici che rispettano TUTTE le coppie
            double media_delta = 0.0;
            double media_massimi = 0.0;
        };

        // Basta modificare questi due valori per un diverso compromesso
        // tra precisione della ricerca e costo computazionale.
        constexpr int REFINE_STEPS = 3;
        constexpr std::size_t MAX_K_EVAL_PER_D = 28;

        while (JL_stima_coeff_d_form::prompt(params)) {
            clear_screen();
            CoutGuard cout_guard;

            // Progressioni: rispettiamo esattamente min/max e il passo richiesto.
            // Se il passo non raggiunge max, non inventiamo un ultimo punto.
            std::vector<int> d_values;
            for (std::int64_t d = params.d_min; d <= params.d_max;) {
                d_values.push_back(static_cast<int>(d));
                if (params.d_mode == 1) {
                    d += params.d_step;
                } else {
                    d *= params.d_step;
                }
            }

            std::vector<double> eps_values;
            for (int e = params.eps_min_pct;
                e <= params.eps_max_pct;
                e += params.eps_step_pct) {
                eps_values.push_back(static_cast<double>(e) / 100.0);
            }

            if (d_values.empty() || eps_values.empty())
                throw std::runtime_error("JL coefficiente: campagna senza valori");

            const std::int64_t last_seed =
                static_cast<std::int64_t>(params.seed_A) + params.T - 1;
            if (last_seed > std::numeric_limits<unsigned>::max())
                throw std::runtime_error("JL coefficiente: seed_A finale fuori gamma");

            // Confronto discreto esatto con il successo richiesto in percentuale.
            const int richiesti =
                (params.T * params.success_pct + 99) / 100;

            const auto output = files.begin_run("coefficiente_d", "JL_K_C_d");
            const auto& run_id = output.id();
            auto scan_csv = output.open_new("scan.csv");
            auto summary_csv = output.open_new("summary.csv");
            // CSV: separatore virgola e punto decimale indipendenti dal locale.
            scan_csv.imbue(std::locale::classic());
            summary_csv.imbue(std::locale::classic());

            // Intestazioni CSV brevi e su UNA riga: adatte a parser e grafici.
            scan_csv << "run_id,n,d,k,eps,k0,T,seed_X,seed_A_start,"
                        "successi,successo_pct,media_delta,media_max,"
                        "obiettivo_pct,d_min,d_max,d_step,d_mode,"
                        "eps_min_pct,eps_max_pct,eps_step_pct,distribuzione\n";
            summary_csv << "run_id,n,d,eps,k0,k_non_successo,k_successo,"
                        "C_hat,successo_pct,T,seed_X,seed_A_start,"
                        "k_valutati,esito,obiettivo_pct,"
                        "d_min,d_max,d_step,d_mode,"
                        "eps_min_pct,eps_max_pct,eps_step_pct,distribuzione,"
                        "k_max_valutato,best_successi,best_successo_pct,best_k,"
                        "k_cap_testato,budget_esaurito,tempo_d_s\n";

            std::cout << "\n=== JL: STIMA COEFFICIENTE DI SCALA ===\n"
                    << "n = " << params.n
                    << "  T = " << params.T
                    << "  successo richiesto = " << params.success_pct << "%\n"
                    << "seed X = " << params.seed_X
                    << "  seed A iniziale = " << params.seed_A << "\n"
                    << "d da " << params.d_min << " a " << params.d_max
                    << " con passo/fattore=" << params.d_step
                    << " e modo=" << params.d_mode << '\n'
                    << "epsilon da " << params.eps_min_pct << "% a "
                    << params.eps_max_pct << "% con passo "
                    << params.eps_step_pct << " punti percentuali\n"
                    << "d: " << d_values.size() << " valori; "
                    << "epsilon: " << eps_values.size() << " soglie\n"
                    << "Campagna: " << output.campaign() << "  run: " << run_id << '\n'
                    << "Directory run: " << output.directory().string() << '\n'
                    << "Intervallo k comune: 1.." << (params.d_min - 1) << '\n'
                    << "Scansione k>=k0(eps), condivisa fra le soglie,"
                        " con raffinamento locale.\n";

            const int n = params.n;
            const double log_n = std::log(static_cast<double>(n));

            // Stesso dominio di k per TUTTI i valori di d: confronto controllato.
            // Il vincolo comune e' k < d_min; cambiare d non amplia la griglia.
            const int k_cap_common = params.d_min - 1;

            for (const int d : d_values) {
                const auto d_start = std::chrono::steady_clock::now();
                const int k_cap = k_cap_common;
                auto [X, label] = genera_cluster(n, d, params.seed_X);
                if (X.size() != label.size())
                    throw std::runtime_error("JL coefficiente: X e label incompatibili");

                std::vector<Pair> pairs;
                for (std::size_t i = 0; i < X.size(); ++i) {
                    for (std::size_t j = i + 1; j < X.size(); ++j) {
                        const double d2 = vector_distanza2(X[i], X[j]);
                        if (!std::isfinite(d2) || d2 < 0.0)
                            throw std::runtime_error("JL coefficiente: distanza iniziale non valida");
                        if (d2 > 0.0) pairs.push_back({i, j, d2});
                    }
                }
                if (pairs.empty())
                    throw std::runtime_error("JL coefficiente: nessuna coppia distinta");

                std::vector<int> k0_values;
                k0_values.reserve(eps_values.size());
                for (double eps : eps_values)
                    k0_values.push_back(static_cast<int>(
                        std::ceil(log_n / (eps * eps))));

                // Calcoli effettuati: la chiave k evita ogni Monte Carlo duplicato.
                std::map<int, MCResult> cache;

                auto evaluate = [&](int k) -> bool {
                    if (k < 1 || k > k_cap) return false;
                    if (cache.find(k) != cache.end()) return true;
                    if (cache.size() >= MAX_K_EVAL_PER_D) return false;

                    std::cout << "\r d=" << std::setw(5) << d
                            << "  k=" << std::setw(5) << k
                            << "  MC=" << (cache.size() + 1)
                            << "/" << MAX_K_EVAL_PER_D << "   " << std::flush;

                    MCResult result;
                    result.successi.assign(eps_values.size(), 0);
                    double sum_medie = 0.0;
                    double sum_max = 0.0;

                    for (int t = 0; t < params.T; ++t) {
                        const unsigned seed_A = static_cast<unsigned>(
                            static_cast<std::uint64_t>(params.seed_A) + t);
                        Mat A = matrix_random_jl(
                            k, d, JLMatrixDistribution::Gaussian,
                            seed_A, 3);
                        Mat Y = matrix_prodotto_matrix(X, matrix_trasposta(A));

                        double sum_delta = 0.0;
                        double max_delta = 0.0;
                        for (const Pair& p : pairs) {
                            const double dy2 = vector_distanza2(Y[p.i], Y[p.j]);
                            const double delta = std::abs(dy2 / p.d2 - 1.0);
                            if (!std::isfinite(delta))
                                throw std::runtime_error("JL coefficiente: distorsione non finita");
                            sum_delta += delta;
                            max_delta = std::max(max_delta, delta);
                        }
                        sum_medie += sum_delta / static_cast<double>(pairs.size());
                        sum_max += max_delta;
                        for (std::size_t e = 0; e < eps_values.size(); ++e) {
                            if (max_delta <= eps_values[e])
                                ++result.successi[e];
                        }
                    }

                    result.media_delta = sum_medie / params.T;
                    result.media_massimi = sum_max / params.T;
                    cache.emplace(k, std::move(result));
                    return true;
                };

                const auto ok = [&](int k, const MCResult& r, std::size_t e) {
                    // Politica della campagna: nessun k inferiore al proprio k0.
                    return k >= k0_values[e] && r.successi[e] >= richiesti;
                };
                const auto all_ok = [&]() {
                    if (cache.empty()) return false;
                    const MCResult& r = cache.rbegin()->second;
                    for (std::size_t e = 0; e < eps_values.size(); ++e)
                        if (!ok(cache.rbegin()->first, r, e)) return false;
                    return true;
                };

                // Griglia grossolana in k: riferimento C=1 per ogni epsilon,
                // piu' crescita geometrica 1.5x fino al limite k<d.
                // Partenza dal piu' piccolo k0 (epsilon piu' grande):
                // una scelta operativa, NON un limite inferiore del lemma.
                std::set<int> coarse;
                const int start = std::max(1,
                    std::min(k_cap, k0_values.back()));
                for (const int k0 : k0_values)
                    if (k0 >= 1 && k0 <= k_cap) coarse.insert(k0);

                for (int k = start;;) {
                    coarse.insert(k);
                    if (k == k_cap) break;
                    const int next = std::max(k + 1,
                        static_cast<int>(std::ceil(1.5 * k)));
                    k = std::min(k_cap, next);
                }
                for (const int k : coarse) {
                    if (!evaluate(k)) break;
                    if (all_ok()) break;
                }

                // Raffina gli intervalli osservati [fallimento, successo].
                // Per ciascun epsilon usa la cache condivisa; non ripete mai
                // un Monte Carlo sullo stesso k.
                for (std::size_t e = eps_values.size(); e-- > 0;) {
                    for (int pass = 0; pass < REFINE_STEPS; ++pass) {
                        const auto upper = std::find_if(
                            cache.begin(), cache.end(),
                            [&](const auto& v) { return ok(v.first, v.second, e); });
                        if (upper == cache.end()) break;

                        // Il limite iniziale NON e' un fallimento osservato:
                        // impedisce soltanto la ricerca sotto il k0 di epsilon.
                        int low = k0_values[e] - 1;
                        for (auto it = cache.begin(); it != upper; ++it)
                            if (it->first >= k0_values[e] &&
                                !ok(it->first, it->second, e))
                                low = std::max(low, it->first);

                        const int high = upper->first;
                        if (high - low <= 1) break;
                        const int mid = low + (high - low) / 2;
                        if (!evaluate(mid)) break;
                    }
                }

                const double elapsed_s = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - d_start).count();

                std::cout << "\n\n d=" << d << "  coppie=" << pairs.size()
                        << "  k valutati=" << cache.size()
                        << "  k massimo valutato=" << cache.rbegin()->first
                        << "  k limite comune=" << k_cap
                        << "  tempo=" << std::fixed << std::setprecision(2)
                        << elapsed_s << " s" << '\n';
                std::cout << "                k non   k con               riusc.\n"
                            "     eps    k0    riusc.  riusc.    C_hat       %   esito\n";

                // Esporta l'intera curva (n,k,epsilon), ordinata per k.
                // Formattazione locale per CAMPO: niente 0.30000000000000004.
                for (const auto& [k, r] : cache) {
                    for (std::size_t e = 0; e < eps_values.size(); ++e) {
                        scan_csv << run_id << ',' << n << ',' << d << ','
                                << k << ',' << std::fixed << std::setprecision(2)
                                << eps_values[e] << ',' << k0_values[e] << ','
                                << params.T << ',' << params.seed_X << ','
                                << params.seed_A << ',' << r.successi[e] << ','
                                << std::setprecision(2)
                                << (100.0 * r.successi[e] / params.T) << ','
                                << std::setprecision(6) << r.media_delta << ','
                                << r.media_massimi << ','
                                << params.success_pct << ','
                                << params.d_min << ',' << params.d_max << ','
                                << params.d_step << ',' << params.d_mode << ','
                                << params.eps_min_pct << ',' << params.eps_max_pct << ','
                                << params.eps_step_pct << ",Gaussian" << '\n';
                    }
                }

                // Cerca la prima dimensione CAMPIONATA con successo sufficiente.
                // Il massimo k osservato e il migliore successo distinguono
                // l'obiettivo non raggiunto dalla mancanza totale di successi.
                for (std::size_t e = 0; e < eps_values.size(); ++e) {
                    const auto upper = std::find_if(
                        cache.begin(), cache.end(),
                        [&](const auto& v) { return ok(v.first, v.second, e); });

                    const bool found = (upper != cache.end());
                    const bool at_cap = (cache.count(k_cap) != 0);
                    const bool budget_full = cache.size() >= MAX_K_EVAL_PER_D;
                    const char* status = found ? "SUCCESSO" :
                        (k0_values[e] > k_cap ? "K0_FUORI_RANGE" :
                        (at_cap ? "NON_RAGGIUNTA_ENTRO_K_COMUNE" :
                        (budget_full ? "BUDGET_ESAUTITO" : "GRIGLIA_INCOMPLETA")));

                    const int k_hit = found ? upper->first : -1;
                    // k_non_successo deve essere un FALLIMENTO OSSERVATO,
                    // non semplicemente il k precedente nella mappa.
                    int k_fail = -1;
                    if (found) {
                        for (auto it = cache.begin(); it != upper; ++it)
                            if (it->first >= k0_values[e] &&
                                !ok(it->first, it->second, e))
                                k_fail = it->first;
                    }
                    const double c_hat = found
                        ? static_cast<double>(k_hit) * eps_values[e] * eps_values[e] / log_n
                        : -1.0;
                    const double pct = found
                        ? 100.0 * upper->second.successi[e] / params.T : 0.0;

                    int best_successi = -1;
                    int best_k = -1;
                    for (const auto& [k, r] : cache) {
                        if (k < k0_values[e]) continue;
                        if (r.successi[e] > best_successi) {
                            best_successi = r.successi[e];
                            best_k = k;
                        }
                    }
                    const double best_pct = best_successi >= 0
                        ? 100.0 * best_successi / params.T : -1.0;

                    std::cout << std::right << std::fixed << std::setprecision(2)
                            << std::setw(8) << eps_values[e]
                            << std::setw(6) << k0_values[e];
                    if (found) {
                        if (k_fail >= 0) std::cout << std::setw(9) << k_fail;
                        else std::cout << std::setw(9) << "--";
                        std::cout << std::setw(9) << k_hit
                                << std::setw(9) << c_hat
                                << std::setw(8) << pct << "%  " << status << '\n';
                    } else {
                        std::cout << std::setw(9) << "--"
                                << std::setw(9) << "--"
                                << std::setw(9) << "--";
                        if (best_successi >= 0)
                            std::cout << std::setw(8) << best_pct << '%';
                        else
                            std::cout << std::setw(9) << "--";
                        std::cout << "  " << status << '\n';
                        if (best_successi >= 0)
                            std::cout << "         Migliore osservato con k>=k0: "
                                    << best_successi << '/' << params.T
                                    << " a k=" << best_k << '\n';
                        std::cout << "         k_max_testato="
                                << cache.rbegin()->first << '\n';
                    }

                    summary_csv << run_id << ',' << n << ',' << d
                                << ',' << std::fixed << std::setprecision(2)
                                << eps_values[e] << ',' << k0_values[e] << ',';
                    if (found) {
                        if (k_fail >= 0) summary_csv << k_fail;
                        summary_csv << ',' << k_hit << ','
                                    << std::setprecision(4) << c_hat << ','
                                    << std::setprecision(2) << pct;
                    } else {
                        // Quattro campi vuoti: assenza di misura, non zero.
                        summary_csv << ",,,";
                    }
                    summary_csv << ',' << params.T << ',' << params.seed_X
                                << ',' << params.seed_A << ',' << cache.size()
                                << ',' << status
                                << ',' << params.success_pct << ','
                                << params.d_min << ',' << params.d_max << ','
                                << params.d_step << ',' << params.d_mode << ','
                                << params.eps_min_pct << ',' << params.eps_max_pct << ','
                                << params.eps_step_pct << ",Gaussian"
                                << ',' << cache.rbegin()->first
                                << ',';
                    if (best_successi >= 0) summary_csv << best_successi;
                    summary_csv
                                << ',';
                    if (best_pct >= 0.0) summary_csv << std::setprecision(2) << best_pct;
                    summary_csv
                                << ',';
                    if (best_k >= 0) summary_csv << best_k;
                    summary_csv
                                << ',' << (at_cap ? 1 : 0)
                                << ',' << (budget_full ? 1 : 0)
                                << ',' << std::fixed << std::setprecision(3)
                                << elapsed_s << '\n';
                }
                scan_csv.flush();
                summary_csv.flush();
            }

            scan_csv.close();
            summary_csv.close();
            if (!scan_csv || !summary_csv)
                throw std::runtime_error("JL coefficiente: errore di scrittura in " +
                                         output.directory().string());
            std::cout << "\nCSV prodotti:\n  "
                    << output.result_path("scan.csv").string() << "\n  "
                    << output.result_path("summary.csv").string() << '\n'
                    << "\nNota: C_hat e' una stima sulla griglia campionata;"
                        " non una costante universale JL.\n";
            // CoutGuard ripristina flags e precision dopo il report.
            fcn_form::wait_report();
        }
    }


    void JL_effetto_PCA()
    {
        constexpr int d = 1000;
        constexpr int n_max = 160;
        constexpr int seed_X = 42;
        constexpr int first_values = 6;

        // Primo collaudo: n=10, epsilon=0.40.
        // Mettere false per eseguire tutte le 25 combinazioni.
        constexpr bool prova_rapida = false;

        const std::vector<int> n_values = {10, 20, 40, 80, 160};
        const std::vector<int> eps_percent = {30, 35, 40, 45, 50};

        // k riusciti nella campagna 20261110_204027_870.
        // Righe: n. Colonne: epsilon.
        const std::vector<std::vector<int>> k_values = {
            {258, 165, 135, 107,  95},
            {288, 201, 175, 140, 116},
            {388, 264, 207, 191, 150},
            {413, 308, 249, 201, 157},
            {502, 387, 304, 237, 202}
        };

        const jl_files::Store files{"data"};
        const auto baseline_file = JL_PCA_baseline_file(files);

        // --------------------------------------------------------
        // 1. Recupero spettri PCA originali, gia' calcolati.
        // --------------------------------------------------------

        const auto split_csv = [](const std::string& line) {
            std::vector<std::string> fields;
            std::istringstream input(line);
            std::string field;

            while (std::getline(input, field, ','))
                fields.push_back(field);

            return fields;
        };

        std::ifstream baseline(baseline_file);
        if (!baseline)
            throw std::runtime_error(
                "PCA_JL: CSV originale non apribile: " + baseline_file.string());

        std::string line;
        if (!std::getline(baseline, line))
            throw std::runtime_error("PCA_JL: CSV originale vuoto");

        std::map<std::string, std::size_t> columns;
        const auto header = split_csv(line);

        for (std::size_t i = 0; i < header.size(); ++i)
            columns[header[i]] = i;

        const auto get_field = [&](const std::vector<std::string>& row,
                                const std::string& name) -> std::string {
            const auto it = columns.find(name);
            if (it == columns.end() || it->second >= row.size())
                throw std::runtime_error(
                    "PCA_JL: colonna CSV mancante: " + name);
            return row[it->second];
        };

        std::map<int, JLPCA_Spettro> originali;

        while (std::getline(baseline, line)) {
            if (line.empty()) continue;

            const auto row = split_csv(line);

            const int n = std::stoi(get_field(row, "n"));
            const int cd = std::stoi(get_field(row, "d"));
            const int cs = std::stoi(get_field(row, "seed_X"));
            const int c0 = std::stoi(get_field(row, "cluster0"));
            const int c1 = std::stoi(get_field(row, "cluster1"));

            if (cd != d || cs != seed_X ||
                c0 != n / 2 || c1 != n / 2)
                throw std::runtime_error(
                    "PCA_JL: baseline incompatibile con il dataset");

            JLPCA_Spettro s;
            s.tempo_s = std::stod(get_field(row, "tempo_pca_s"));

            for (int j = 1; j <= first_values; ++j)
                s.sigma.push_back(
                    std::stod(get_field(row, "sigma" + std::to_string(j))));

            const double ve1 = std::stod(
                get_field(row, "var_spiegata1"));

            if (ve1 <= 0.0 || s.sigma[0] <= 0.0)
                throw std::runtime_error(
                    "PCA_JL: energia originale non valida");

            // Ricostruzione dell'energia totale dalla quota PC1.
            s.energia = s.sigma[0] * s.sigma[0] / ve1;

            if (!originali.emplace(n, std::move(s)).second)
                throw std::runtime_error(
                    "PCA_JL: n duplicato nel CSV originale");
        }

        for (int n : n_values)
            if (originali.count(n) != 1)
                throw std::runtime_error(
                    "PCA_JL: baseline assente per n=" + std::to_string(n));

        // --------------------------------------------------------
        // 2. Dataset master e CSV della nuova campagna.
        // --------------------------------------------------------

        const auto [X_master, label_master] =
            genera_cluster(n_max, d, seed_X);

        const auto output = files.begin_run("confronti_spettrali", "PCA_JL");
        const auto& id = output.id();
        auto summary = output.open_new("confronto.csv");
        auto spectra = output.open_new("spettri.csv");

        summary
            << "campagna,baseline,n,d,eps,k,seed_X,seed_A,"
            << "coppie,violazioni,delta_media,delta_max,JL_ok,"
            << "tempo_A_s,tempo_proiezione_s,tempo_PCA_JL_s,"
            << "tempo_totale_s,tempo_PCA_X_s,speedup_PCA,"
            << "var1_X,var1_JL\n";

        spectra
            << "campagna,n,d,eps,k,seed_A,indice,"
            << "sigma_X,sigma_JL,rapporto_X,rapporto_JL,Q,"
            << "var_X,var_JL,var_diff\n";

        summary << std::fixed << std::setprecision(9);
        spectra << std::fixed << std::setprecision(9);

        std::cout
            << "\n=== JL + PCA: CONFRONTO SPETTRALE ===\n"
            << "Campagna: " << output.campaign() << "  run: " << id << '\n'
            << "Directory run: " << output.directory().string() << '\n'
            << "Baseline: " << baseline_file.string() << '\n'
            << "Dataset master: n=" << n_max << " d=" << d << '\n'
            << "seed_X=" << seed_X << '\n'
            << "Distribuzione JL: Gaussiana\n"
            << "Modalita': "
            << (prova_rapida ? "COLLAUDO" : "COMPLETA")
            << "\n\n";

        struct Pair {
            std::size_t i;
            std::size_t j;
            double d2;
        };

        const auto seconds = [](auto start, auto end) {
            return std::chrono::duration<double>(end - start).count();
        };

        // --------------------------------------------------------
        // 3. Ciclo sui dataset annidati.
        // --------------------------------------------------------

        for (std::size_t ni = 0; ni < n_values.size(); ++ni) {
            const int n = n_values[ni];

            if (prova_rapida && ni != 0)
                continue;

            const auto [X, labels] =
                JL_estrai_bilanciato(X_master, label_master, n);

            const JLPCA_Spettro& sx = originali.at(n);

            // Distanze originali: calcolate una volta per n.
            std::vector<Pair> pairs;
            pairs.reserve(static_cast<std::size_t>(n) * (n - 1) / 2);

            for (std::size_t i = 0; i < X.size(); ++i) {
                for (std::size_t j = i + 1; j < X.size(); ++j) {
                    const double dx2 = vector_distanza2(X[i], X[j]);
                    if (!(dx2 > 0.0) || !std::isfinite(dx2))
                        throw std::runtime_error(
                            "PCA_JL: distanza originale non valida");

                    pairs.push_back({i, j, dx2});
                }
            }

            std::cout << "n=" << n
                    << "  d=" << d
                    << "  coppie=" << pairs.size()
                    << "  PCA originale="
                    << std::fixed << std::setprecision(3)
                    << sx.tempo_s << " s\n";

            // ----------------------------------------------------
            // 4. Ciclo epsilon: JL + PCA.
            // ----------------------------------------------------

            for (std::size_t ei = 0; ei < eps_percent.size(); ++ei) {
                if (prova_rapida && ei != 2)
                    continue;

                const double eps = eps_percent[ei] / 100.0;
                const int k = k_values[ni][ei];

                if (k <= 0 || k >= d)
                    throw std::runtime_error(
                        "PCA_JL: dimensione k non valida");

                // Seed nuovi, distinti da quelli della ricerca MC.
                const unsigned seed_A =
                    50000u + static_cast<unsigned>(ni * 10 + ei);

                const auto t0 = std::chrono::steady_clock::now();

                Mat A = matrix_random_jl(
                    k, d, JLMatrixDistribution::Gaussian, seed_A, 3);

                const auto t1 = std::chrono::steady_clock::now();

                Mat Y = matrix_prodotto_matrix(
                    X, matrix_trasposta(A));

                const auto t2 = std::chrono::steady_clock::now();

                const double t_A = seconds(t0, t1);
                const double t_JL = seconds(t1, t2);

                if (Y.size() != X.size() ||
                    Y.empty() ||
                    Y.front().size() != static_cast<std::size_t>(k))
                    throw std::runtime_error(
                        "PCA_JL: dimensioni Y inattese");

                // PCA con lo stesso algoritmo della baseline.
                const JLPCA_Spettro sy = JL_PCA_spettro(Y);

                if (sy.sigma.size() < first_values)
                    throw std::runtime_error(
                        "PCA_JL: spettro proiettato insufficiente");

                // ------------------------------------------------
                // 5. Controllo geometrico della proiezione.
                // ------------------------------------------------

                double delta_sum = 0.0;
                double delta_max = 0.0;
                std::size_t violazioni = 0;

                for (const Pair& p : pairs) {
                    const double dy2 = vector_distanza2(
                        Y[p.i], Y[p.j]);

                    const double delta = std::abs(dy2 / p.d2 - 1.0);

                    if (!std::isfinite(delta))
                        throw std::runtime_error(
                            "PCA_JL: distorsione non finita");

                    delta_sum += delta;
                    delta_max = std::max(delta_max, delta);

                    if (delta > eps)
                        ++violazioni;
                }

                const double delta_media =
                    delta_sum / static_cast<double>(pairs.size());

                const bool jl_ok = (violazioni == 0);

                // ------------------------------------------------
                // 6. Confronto dei sei valori singolari.
                // ------------------------------------------------

                const double tempo_totale =
                    t_A + t_JL + sy.tempo_s;

                const double speedup =
                    tempo_totale > 0.0
                    ? sx.tempo_s / tempo_totale : 0.0;

                const double var1_X =
                    sx.sigma[0] * sx.sigma[0] / sx.energia;

                const double var1_JL =
                    sy.sigma[0] * sy.sigma[0] / sy.energia;

                std::cout
                    << "  eps=" << std::setprecision(2) << eps
                    << " k=" << k
                    << " seed_A=" << seed_A
                    << " JL=" << (jl_ok ? "OK" : "VIOLATO")
                    << " delta_max=" << std::setprecision(4)
                    << delta_max << '\n';

                std::cout
                    << "    tempo A=" << std::setprecision(3) << t_A
                    << "  proiezione=" << t_JL
                    << "  PCA/JL=" << sy.tempo_s
                    << "  totale=" << tempo_totale << " s\n";

                std::cout
                    << "    varianza PC1: "
                    << std::setprecision(5)
                    << var1_X << " -> " << var1_JL
                    << "\n    Q(2..6):";

                for (int j = 0; j < first_values; ++j) {
                    if (sx.sigma[j] <= 0.0 ||
                        sy.sigma[j] <= 0.0)
                        throw std::runtime_error(
                            "PCA_JL: denominatore spettrale nullo");

                    const double ratio_X =
                        sx.sigma[0] / sx.sigma[j];

                    const double ratio_JL =
                        sy.sigma[0] / sy.sigma[j];

                    const double Q = ratio_JL / ratio_X;

                    const double var_X =
                        sx.sigma[j] * sx.sigma[j] / sx.energia;

                    const double var_JL =
                        sy.sigma[j] * sy.sigma[j] / sy.energia;

                    spectra
                        << id << ',' << n << ',' << d << ','
                        << eps << ',' << k << ',' << seed_A << ','
                        << j + 1 << ','
                        << sx.sigma[j] << ',' << sy.sigma[j] << ','
                        << ratio_X << ',' << ratio_JL << ',' << Q << ','
                        << var_X << ',' << var_JL << ','
                        << (var_JL - var_X) << '\n';

                    if (j > 0)
                        std::cout << ' ' << Q;
                }

                std::cout << "\n\n";

                summary
                    << id << ",20261111_203831_386,"
                    << n << ',' << d << ',' << eps << ','
                    << k << ',' << seed_X << ',' << seed_A << ','
                    << pairs.size() << ',' << violazioni << ','
                    << delta_media << ',' << delta_max << ','
                    << (jl_ok ? 1 : 0) << ','
                    << t_A << ',' << t_JL << ',' << sy.tempo_s << ','
                    << tempo_totale << ',' << sx.tempo_s << ','
                    << speedup << ',' << var1_X << ',' << var1_JL
                    << '\n';

                summary.flush();
                spectra.flush();
            }
        }

        summary.close();
        spectra.close();
        if (!summary || !spectra)
            throw std::runtime_error("PCA_JL: errore di scrittura in " +
                                     output.directory().string());

        std::cout
            << "CSV prodotti:\n"
            << "  " << output.result_path("confronto.csv").string() << '\n'
            << "  " << output.result_path("spettri.csv").string() << '\n'
            << "\nNota: una singola proiezione per ogni (n,epsilon)."
            << "\nI k sono quelli della precedente campagna Monte Carlo."
            << "\nIl successo JL va verificato sul dataset attuale.\n";

        fcn_form::wait_report();
    }

    // ============================================================================
    // PCA_JL_MC -- replica Monte Carlo del confronto spettrale JL + PCA
    // Riutilizza JL_estrai_bilanciato(), JL_PCA_spettro() e jl_files::Store.
    //
    // Header standard: <algorithm>, <chrono>, <cmath>, <fstream>, <iomanip>,
    // <iostream>, <numeric>, <sstream>, <stdexcept>, <string>, <vector>.
    // ============================================================================

    void JL_effetto_PCA_MC()
    {
        // Prima prova: caso della campagna completa con n=160, eps=0.50.
        constexpr int n = 160;
        constexpr int d = 1000;
        constexpr int k = 202;
        constexpr double eps = 0.50;
        constexpr int T = 20;                  // Numero di nuove proiezioni
        constexpr int seed_X = 42;
        constexpr unsigned seed_A_iniziale = 60000u;
        constexpr int primi = 6;
        constexpr const char* baseline_id = "20261111_203831_386";
        const jl_files::Store files{"data"};
        const auto baseline_file = JL_PCA_baseline_file(files);

        if (k <= 0 || k >= d || T <= 0 || n < primi + 1 || !(eps > 0.0 && eps < 1.0))
            throw std::invalid_argument("PCA_JL_MC: parametri non validi");

        // 1. Legge il riferimento PCA, senza ripetere la SVD originale.
        std::ifstream base(baseline_file);
        if (!base)
            throw std::runtime_error("PCA_JL_MC: CSV originale non apribile: " + baseline_file.string());

        auto split_csv = [](const std::string& line) {
            std::vector<std::string> v;
            std::istringstream is(line);
            std::string cell;
            while (std::getline(is, cell, ',')) v.push_back(cell);
            return v;
        };
        std::string line;
        if (!std::getline(base, line))
            throw std::runtime_error("PCA_JL_MC: baseline vuota");
        const auto intestazione = split_csv(line);

        auto field = [&intestazione](const std::vector<std::string>& row,
                                    const std::string& name) -> std::string {
            auto it = std::find(intestazione.begin(), intestazione.end(), name);
            if (it == intestazione.end())
                throw std::runtime_error("PCA_JL_MC: colonna mancante: " + name);
            std::size_t i = static_cast<std::size_t>(it - intestazione.begin());
            if (i >= row.size() || row[i].empty())
                throw std::runtime_error("PCA_JL_MC: campo vuoto: " + name);
            return row[i];
        };

        JLPCA_Spettro sx;
        bool trovato = false;
        while (std::getline(base, line)) {
            if (line.empty()) continue;
            auto row = split_csv(line);
            int n_csv = std::stoi(field(row, "n"));
            if (n_csv != n) continue;
            if (trovato)
                throw std::runtime_error("PCA_JL_MC: n duplicato nella baseline");
            if (std::stoi(field(row, "d")) != d ||
                std::stoi(field(row, "seed_X")) != seed_X ||
                std::stoi(field(row, "cluster0")) != n/2 ||
                std::stoi(field(row, "cluster1")) != n/2 ||
                field(row, "campagna") != baseline_id)
                throw std::runtime_error("PCA_JL_MC: riferimento incompatibile");

            sx.tempo_s = std::stod(field(row, "tempo_pca_s"));
            for (int j = 1; j <= primi; ++j)
                sx.sigma.push_back(std::stod(field(row, "sigma" + std::to_string(j))));
            const double ve1 = std::stod(field(row, "var_spiegata1"));
            if (!(ve1 > 0.0 && ve1 <= 1.0) || sx.sigma[0] <= 0.0)
                throw std::runtime_error("PCA_JL_MC: spettro baseline non valido");
            sx.energia = sx.sigma[0] * sx.sigma[0] / ve1;
            trovato = true;
        }
        if (!trovato)
            throw std::runtime_error("PCA_JL_MC: n richiesto assente dalla baseline");
        for (double s : sx.sigma)
            if (!std::isfinite(s) || s <= 0.0)
                throw std::runtime_error("PCA_JL_MC: valori singolari originali non validi");

        // 2. Ricostruisce ESATTAMENTE il master della campagna originale.
        const auto [X_master, label_master] = genera_cluster(n, d, seed_X);
        const auto [X, labels] = JL_estrai_bilanciato(X_master, label_master, n);

        // Controllo ulteriore del riferimento: energia Frobenius della matrice
        // centrata, senza eseguire nuovamente la costosa SVD originale.
        Vec avg_col(d, 0.0), avg_row(n, 0.0);
        matrix_calcola_media(X, avg_col, avg_row);
        const Mat Xc = matrix_centra_su_media(X, avg_col);
        double energia_dati = 0.0;
        for (const auto& row : Xc)
            for (double x : row) energia_dati += x*x;
        if (!(sx.energia > 0.0) ||
            std::abs(energia_dati / sx.energia - 1.0) > 1e-5)
            throw std::runtime_error(
                "PCA_JL_MC: energia della baseline diversa da quella del master");

        // Distanze originarie calcolate UNA sola volta per tutti i trial.
        struct Coppia { std::size_t i, j; double d2; };
        std::vector<Coppia> coppie;
        coppie.reserve(static_cast<std::size_t>(n)*(n-1)/2);
        for (std::size_t i = 0; i < X.size(); ++i)
            for (std::size_t j = i+1; j < X.size(); ++j) {
                double d2 = vector_distanza2(X[i], X[j]);
                if (!(d2 > 0.0) || !std::isfinite(d2))
                    throw std::runtime_error("PCA_JL_MC: distanza originale non valida");
                coppie.push_back({i,j,d2});
            }

        // 3. Output separati: dettaglio di ogni trial + statistiche finali.
        const auto output = files.begin_run("monte_carlo_spettrale", "PCA_JL_MC");
        const auto& id = output.id();
        auto trial_csv = output.open_new("trial.csv");
        auto stat_csv = output.open_new("stat.csv");
        trial_csv << std::fixed << std::setprecision(9);
        stat_csv << std::fixed << std::setprecision(9);

        trial_csv << "campagna,baseline,trial,n,d,k,eps,seed_X,seed_A,"
                << "coppie,violazioni,delta_media,delta_max,JL_ok,"
                << "tempo_A_s,tempo_proiezione_s,tempo_PCA_s,tempo_totale_s,"
                << "tempo_PCA_X_s,speedup_indicativo,"
                << "sigma1_su_X,sigma2_su_X,var1_X,var1_Y,energia_Y_su_X";
        for (int j = 1; j <= primi; ++j) trial_csv << ",sigmaY" << j;
        for (int j = 2; j <= primi; ++j) trial_csv << ",Q" << j;
        trial_csv << '\n';

        // Le serie seguenti alimentano le statistiche descrittive.
        std::vector<double> s1ratio, s2ratio, q2, q2_solo_ok, var1, energia_ratio;
        std::vector<double> delta_max_values, tempi, speedups, indicatori_ok;
        std::vector<double> q[primi-1];
        int successi = 0;
        int sigma2_cresciuto = 0;
        int q2_sotto_uno = 0;

        const auto tempo = [](auto a, auto b) {
            return std::chrono::duration<double>(b-a).count();
        };

        std::cout << "\n=== JL + PCA: MONTE CARLO SPETTRALE ===\n"
                << "Campagna: " << output.campaign() << "  run: " << id << '\n'
                << "Directory run: " << output.directory().string() << '\n'
                << "Baseline: " << baseline_file.string() << '\n'
                << "n=" << n << " d=" << d << " k=" << k
                << " epsilon=" << eps << " T=" << T << '\n'
                << "seed_X=" << seed_X << " seed_A=" << seed_A_iniziale
                << ".." << seed_A_iniziale+T-1 << '\n'
                << "Coppie: " << coppie.size() << '\n';

        // 4. Monte Carlo: stessa X, matrice A differente in ogni trial.
        for (int t = 0; t < T; ++t) {
            unsigned seed_A = seed_A_iniziale + static_cast<unsigned>(t);
            auto start = std::chrono::steady_clock::now();
            Mat A = matrix_random_jl(k, d, JLMatrixDistribution::Gaussian, seed_A, 3);
            auto after_A = std::chrono::steady_clock::now();
            Mat Y = matrix_prodotto_matrix(X, matrix_trasposta(A));
            auto after_projection = std::chrono::steady_clock::now();
            if (Y.size() != X.size() || Y.empty() ||
                Y.front().size() != static_cast<std::size_t>(k))
                throw std::runtime_error("PCA_JL_MC: dimensioni di Y non valide");

            JLPCA_Spettro sy = JL_PCA_spettro(Y);
            if (sy.sigma.size() < primi || !(sy.energia > 0.0))
                throw std::runtime_error("PCA_JL_MC: spettro proiettato non valido");

            std::size_t violazioni = 0;
            double delta_somma = 0.0;
            double delta_max = 0.0;
            for (const auto& p : coppie) {
                double dy2 = vector_distanza2(Y[p.i], Y[p.j]);
                double delta = std::abs(dy2 / p.d2 - 1.0);
                if (!std::isfinite(delta))
                    throw std::runtime_error("PCA_JL_MC: distorsione non finita");
                delta_somma += delta;
                delta_max = std::max(delta_max, delta);
                if (delta > eps) ++violazioni;
            }
            const bool ok = (violazioni == 0);
            if (ok) ++successi;
            double delta_media = delta_somma / coppie.size();

            const double r1 = sy.sigma[0] / sx.sigma[0];
            const double r2 = sy.sigma[1] / sx.sigma[1];
            const double ve1_orig = sx.sigma[0]*sx.sigma[0] / sx.energia;
            const double ve1_Y = sy.sigma[0]*sy.sigma[0] / sy.energia;
            const double r_energia = sy.energia / sx.energia;
            double Q[primi-1];
            for (int j = 1; j < primi; ++j) {
                if (!(sy.sigma[j] > 0.0))
                    throw std::runtime_error("PCA_JL_MC: valore singolare nullo");
                Q[j-1] = (sy.sigma[0] / sy.sigma[j]) /
                        (sx.sigma[0] / sx.sigma[j]);
                q[j-1].push_back(Q[j-1]);
            }
            if (r2 > 1.0) ++sigma2_cresciuto;
            if (Q[0] < 1.0) ++q2_sotto_uno;
            if (ok) q2_solo_ok.push_back(Q[0]);

            const double t_A = tempo(start, after_A);
            const double t_proj = tempo(after_A, after_projection);
            const double t_pca = sy.tempo_s;
            const double t_tot = t_A+t_proj+t_pca;
            const double speedup = t_tot > 0 ? sx.tempo_s/t_tot : 0.0;

            s1ratio.push_back(r1);
            s2ratio.push_back(r2);
            q2.push_back(Q[0]);
            var1.push_back(ve1_Y);
            energia_ratio.push_back(r_energia);
            delta_max_values.push_back(delta_max);
            tempi.push_back(t_tot);
            speedups.push_back(speedup);
            indicatori_ok.push_back(ok ? 1.0 : 0.0);

            trial_csv << id << ',' << baseline_id << ',' << t+1 << ','
                    << n << ',' << d << ',' << k << ',' << eps << ','
                    << seed_X << ',' << seed_A << ',' << coppie.size() << ','
                    << violazioni << ',' << delta_media << ',' << delta_max << ','
                    << (ok?1:0) << ',' << t_A << ',' << t_proj << ','
                    << t_pca << ',' << t_tot << ',' << sx.tempo_s << ','
                    << speedup << ',' << r1 << ',' << r2 << ','
                    << ve1_orig << ',' << ve1_Y << ',' << r_energia;
            for (int j=0; j<primi; ++j) trial_csv << ',' << sy.sigma[j];
            for (int j=0; j<primi-1; ++j) trial_csv << ',' << Q[j];
            trial_csv << '\n';
            trial_csv.flush(); // conserva anche i trial completi in caso di stop

            std::cout << "trial=" << std::setw(2) << t+1
                    << " seed=" << seed_A
                    << " JL=" << (ok ? "OK" : "VIOLATO")
                    << " delta_max=" << std::fixed << std::setprecision(4)
                    << delta_max << "  s1/X=" << r1
                    << "  s2/X=" << r2 << "  Q2=" << Q[0]
                    << "  var1=" << ve1_Y
                    << "  tempo=" << std::setprecision(3) << t_tot << " s\n";
        }

        // 5. Tabelle statistiche; dev. standard campionaria (T-1).
        stat_csv << "campagna,baseline,n,d,k,eps,T,metrica,campioni,media,"
                    "dev_std_campionaria,min,max\n";
        struct Stat { double media=0, ds=0, minimo=0, massimo=0; };
        auto statistica = [](const std::vector<double>& v) -> Stat {
            if (v.empty()) return {};
            Stat a;
            a.minimo = *std::min_element(v.begin(),v.end());
            a.massimo = *std::max_element(v.begin(),v.end());
            for (double x : v) a.media += x;
            a.media /= v.size();
            for (double x : v) a.ds += (x-a.media)*(x-a.media);
            a.ds = v.size()>1 ? std::sqrt(a.ds/(v.size()-1)) : 0.0;
            return a;
        };
        auto write_stat = [&](const char* name, const std::vector<double>& v) {
            if (v.empty()) return;
            Stat a = statistica(v);
            stat_csv << id << ',' << baseline_id << ',' << n << ',' << d
                    << ',' << k << ',' << eps << ',' << T << ',' << name
                    << ',' << v.size() << ',' << a.media << ',' << a.ds
                    << ',' << a.minimo << ',' << a.massimo << '\n';
        };
        write_stat("JL_ok_01",indicatori_ok);
        write_stat("delta_max",delta_max_values);
        write_stat("sigma1_Y_su_X",s1ratio);
        write_stat("sigma2_Y_su_X",s2ratio);
        for (int j=0; j<primi-1; ++j) {
            const std::string name = "Q" + std::to_string(j+2);
            write_stat(name.c_str(),q[j]);
        }
        write_stat("Q2_solo_JL_ok",q2_solo_ok);
        write_stat("var1_Y",var1);
        write_stat("energia_Y_su_X",energia_ratio);
        write_stat("tempo_totale_s",tempi);
        write_stat("speedup_indicativo",speedups);

        trial_csv.close();
        stat_csv.close();
        if (!trial_csv || !stat_csv)
            throw std::runtime_error("PCA_JL_MC: errore di scrittura in " +
                                     output.directory().string());

        Stat a1 = statistica(s1ratio), a2 = statistica(s2ratio);
        Stat aq = statistica(q2), av = statistica(var1);
        std::cout << "\n--- RISULTATI MONTE CARLO ---\n"
                << "JL riuscite: " << successi << '/' << T << " ("
                << std::setprecision(2) << 100.0*successi/T << "%)\n"
                << "sigma1(Y)/sigma1(X): " << std::setprecision(5)
                << a1.media << " +/- " << a1.ds << '\n'
                << "sigma2(Y)/sigma2(X): " << a2.media << " +/- " << a2.ds
                << "  [>1 in " << sigma2_cresciuto << '/' << T << "]\n"
                << "Q2: " << aq.media << " +/- " << aq.ds
                << "  [<1 in " << q2_sotto_uno << '/' << T << "]\n"
                << "Varianza PC1 originale: "
                << sx.sigma[0]*sx.sigma[0]/sx.energia
                << "  dopo JL (media): " << av.media << '\n'
                << "CSV trial: " << output.result_path("trial.csv").string() << '\n'
                << "CSV statistiche: " << output.result_path("stat.csv").string() << '\n'
                << "Nota: 20 trial forniscono frequenze e dispersioni descrittive,"
                    " non una stima precisa della probabilita' di successo.\n";

        fcn_form::wait_report();
    }

    // ============================================================================
    // PCA_JL_K_MC: effetto di k sullo spettro PCA (Gaussian JL, trial accoppiati).
    // Riusa: JL_estrai_bilanciato(), JL_PCA_spettro(), jl_files::Store,
    // genera_cluster(), matrix_random_jl(), matrix_prodotto_matrix(),
    // matrix_trasposta(), matrix_calcola_media(), matrix_centra_su_media(),
    // vector_distanza2(), fcn_form::wait_report().
    // Standard: algorithm, chrono, cmath, fstream, iomanip, iostream,
    // sstream, stdexcept, string, utility, vector.
    // ============================================================================

    void JL_effetto_PCA_K_MC()
    {
        const jl_files::Store files{"data"};
        JLPCAKMCParams p{};
        while (JL_PCA_K_MC_form::prompt(p)) {
            clear_screen();

            const int n = p.n;
            const int d = p.d;
            const int seed_X = p.seed_X;
            const int n_max = std::max(160,n);
            const double eps = static_cast<double>(p.eps_pct) / 100.0;
            constexpr int primi = 6;
            const unsigned seed_A_start = static_cast<unsigned>(p.seed_A);
            const int T = p.T;
            const int all_k[4] = {p.k1, p.k2, p.k3, p.k4};
            const std::vector<int> k_values(all_k, all_k + p.k_count);
            constexpr const char* baseline_id = "20261111_203831_386";

            if (n < primi+1 || n > n_max || n%2 || T <= 0 ||
                !(eps > 0.0 && eps < 1.0) || k_values.empty())
                throw std::invalid_argument("PCA_JL_K_MC: parametri non validi");
            for (std::size_t j=0; j<k_values.size(); ++j)
                if (k_values[j] <= 0 || k_values[j] > d ||
                    (j && k_values[j] <= k_values[j-1]))
                    throw std::invalid_argument("PCA_JL_K_MC: k non ordinati/validi");
            const int K = k_values.back();

            // 1) Dataset secondo il modello originale di Federico.
            // Quando n<=160 si usa lo stesso master di 160 osservazioni della
            // campagna baseline, cosi' anche i sottoinsiemi annidati coincidono.
            const auto [Xmaster, lmaster] = genera_cluster(n_max,d,seed_X);
            const auto [X, labels] = JL_estrai_bilanciato(Xmaster,lmaster,n);

            // Il CSV storico e' valido SOLO con gli stessi parametri e con n
            // effettivamente presente. Negli altri casi la PCA originale viene
            // ricalcolata sul nuovo dataset: mai confrontare spettri incompatibili.
            const bool reference_family = (n<=160 && d==1000 && seed_X==42);
            JLPCA_Spettro sx;
            bool found = false;
            std::string baseline_used = "DINAMICA";
            std::filesystem::path baseline_file;
            if (reference_family) {
                baseline_file = JL_PCA_baseline_file(files);
                std::ifstream base(baseline_file);
                if (base) {
                    auto split = [](const std::string& row_text) {
                        std::vector<std::string> row;
                        std::istringstream in(row_text);
                        std::string value;
                        while(std::getline(in,value,',')) row.push_back(value);
                        return row;
                    };
                    std::string line;
                    if (std::getline(base,line)) {
                        const auto columns = split(line);
                        auto field = [&columns](const std::vector<std::string>& row,
                                                const std::string& name) -> std::string {
                            const auto it = std::find(columns.begin(),columns.end(),name);
                            if (it == columns.end())
                                throw std::runtime_error("PCA_JL_K_MC: colonna mancante " + name);
                            const std::size_t index = static_cast<std::size_t>(it-columns.begin());
                            if (index >= row.size() || row[index].empty())
                                throw std::runtime_error("PCA_JL_K_MC: campo mancante " + name);
                            return row[index];
                        };
                        while (std::getline(base,line)) {
                            if (line.empty()) continue;
                            const auto row = split(line);
                            if (std::stoi(field(row,"n")) != n) continue;
                            if (found || std::stoi(field(row,"d")) != d ||
                                std::stoi(field(row,"seed_X")) != seed_X ||
                                std::stoi(field(row,"cluster0")) != n/2 ||
                                std::stoi(field(row,"cluster1")) != n/2 ||
                                field(row,"campagna") != baseline_id)
                                throw std::runtime_error("PCA_JL_K_MC: baseline incoerente");
                            sx.tempo_s = std::stod(field(row,"tempo_pca_s"));
                            for (int j=1;j<=primi;++j)
                                sx.sigma.push_back(std::stod(field(row,"sigma"+std::to_string(j))));
                            const double ve1 = std::stod(field(row,"var_spiegata1"));
                            if (!(ve1>0.0 && ve1<=1.0))
                                throw std::runtime_error("PCA_JL_K_MC: quota PC1 non valida");
                            sx.energia=sx.sigma.front()*sx.sigma.front()/ve1;
                            found = true;
                            baseline_used = baseline_id;
                        }
                    }
                }
            }
            if (!found) {
                std::cout << "PCA originale non presente nel CSV per questi parametri; "
                            "ricalcolo SVD di riferimento.\n";
                sx = JL_PCA_spettro(X);
            } else {
                std::cout << "PCA originale recuperata da "
                        << baseline_file.string() << '\n';
            }
            if (sx.sigma.size()<primi || !(sx.energia>0.0))
                throw std::runtime_error("PCA_JL_K_MC: spettro X incompleto");
            for (double sig:sx.sigma)
                if (!(sig>0.0) || !std::isfinite(sig))
                    throw std::runtime_error("PCA_JL_K_MC: sigma X non validi");

            Vec mean_col(d,0.0), mean_row(n,0.0);
            matrix_calcola_media(X,mean_col,mean_row);
            const Mat Xc = matrix_centra_su_media(X,mean_col);
            double energia_verificata = 0.0;
            for (const auto& row : Xc)
                for (double x : row) energia_verificata += x*x;
            if (std::abs(energia_verificata/sx.energia-1.0) > 1e-5)
                throw std::runtime_error("PCA_JL_K_MC: dataset diverso dalla baseline");

            double energia_top6_X = 0.0;
            for (double s : sx.sigma) energia_top6_X += s*s;
            const double energia_coda_X = sx.energia-energia_top6_X;
            if (!(energia_coda_X > 0.0))
                throw std::runtime_error("PCA_JL_K_MC: energia coda originale nulla");
            const double var1_X = sx.sigma[0]*sx.sigma[0]/sx.energia;
            const double var2_X = sx.sigma[1]*sx.sigma[1]/sx.energia;
            const double coda_X = energia_coda_X/sx.energia;

            struct Pair { std::size_t i,j; double d2; };
            std::vector<Pair> pairs;
            pairs.reserve(static_cast<std::size_t>(n)*(n-1)/2);
            for (std::size_t i=0;i<X.size();++i)
                for (std::size_t j=i+1;j<X.size();++j) {
                    const double d2 = vector_distanza2(X[i],X[j]);
                    if (!(d2 > 0.0) || !std::isfinite(d2))
                        throw std::runtime_error("PCA_JL_K_MC: distanza X non valida");
                    pairs.push_back({i,j,d2});
                }

            const auto output = files.begin_run("stabilita_spettrale_k", "PCA_JL_K_MC");
            const auto& id = output.id();
            const auto trial_file = output.result_path("trial.csv");
            const auto stats_file = output.result_path("stat.csv");
            auto detail = output.open_new("trial.csv");
            auto stats = output.open_new("stat.csv");
            detail << std::fixed << std::setprecision(9);
            stats << std::fixed << std::setprecision(9);
            detail << "campagna,baseline,trial,seed_A,n,d,k,K_master,eps,coppie,"
                    "violazioni,delta_media,delta_max,JL_ok,"
                    "sigma1_Y_su_X,sigma2_Y_su_X,Q2,"
                    "var1_X,var1_Y,var2_X,var2_Y,"
                    "energia_Y_su_X,coda6_Y_su_X,coda6_quota_X,coda6_quota_Y,"
                    "tempo_A_master_s,tempo_proiezione_master_s,"
                    "tempo_estrazione_k_s,tempo_PCA_k_s";
            for (int j=1;j<=primi;++j) detail << ",sigmaY" << j;
            for (int j=3;j<=primi;++j) detail << ",Q" << j;
            detail << ",seed_X,sigma1_X,sigma2_X,energia_X\n";
            stats << "campagna,baseline,n,d,k,K_master,eps,T,metrica,"
                    "campioni,media,dev_std_campionaria,min,max,seed_X,energia_X\n";

            struct Acc {
                int good=0, sigma2_up=0, q2_down=0;
                std::vector<double> s1,s2,q2,var1,var2,energia,coda_ratio,coda_quota;
                std::vector<double> delta,tempo_pca;
                std::vector<double> Q[5];
            };
            std::vector<Acc> data(k_values.size());
            struct Stat { double avg=0.0, sd=0.0, lo=0.0, hi=0.0; };
            auto stat = [](const std::vector<double>& v)->Stat {
                Stat a;
                if (v.empty()) return a;
                a.lo = *std::min_element(v.begin(),v.end());
                a.hi = *std::max_element(v.begin(),v.end());
                for (double x:v) a.avg += x;
                a.avg /= static_cast<double>(v.size());
                for (double x:v) a.sd += (x-a.avg)*(x-a.avg);
                a.sd = v.size()>1 ? std::sqrt(a.sd/(v.size()-1)) : 0.0;
                return a;
            };
            auto elapsed = [](auto a,auto b) {
                return std::chrono::duration<double>(b-a).count();
            };

            std::cout << "\n=== JL + PCA: STABILITA' SPETTRALE AL VARIARE DI k ===\n"
                    << "Campagna: " << output.campaign() << "  run: " << id
                    << "  baseline: " << baseline_used << '\n'
                    << "Directory run: " << output.directory().string() << '\n'
                    << "n=" << n << " d=" << d << " seed_X=" << seed_X
                    << " eps=" << eps << " T=" << T
                    << "  k=";
            for (int k:k_values) std::cout << k << ' ';
            std::cout << "\nseed_A=" << seed_A_start << ".."
                    << seed_A_start+T-1 << "  coppie=" << pairs.size() << '\n'
                    << "Accoppiamento: stesse prime righe gaussiane, scala 1/sqrt(k).\n"
                    << "PC1 X=" << var1_X << "  PC2 X=" << var2_X
                    << "  coda (j>=7) X=" << coda_X << "\n";

            for (int t=0;t<T;++t) {
                const unsigned seed_A = seed_A_start + static_cast<unsigned>(t);
                const auto a0 = std::chrono::steady_clock::now();
                const Mat Amax = matrix_random_jl(
                    K,d,JLMatrixDistribution::Gaussian,seed_A,3);
                const auto a1 = std::chrono::steady_clock::now();
                if (Amax.size() != static_cast<std::size_t>(K) ||
                    Amax.front().size() != static_cast<std::size_t>(d))
                    throw std::runtime_error("PCA_JL_K_MC: Amax malformata");
                // Ymax_ij e' la proiezione sulla riga gaussiana j, scalata 1/sqrt(K).
                const Mat Ymax = matrix_prodotto_matrix(X,matrix_trasposta(Amax));
                const auto a2 = std::chrono::steady_clock::now();
                if (Ymax.size() != static_cast<std::size_t>(n) ||
                    Ymax.front().size() != static_cast<std::size_t>(K))
                    throw std::runtime_error("PCA_JL_K_MC: Ymax malformata");
                const double tempo_A = elapsed(a0,a1);
                const double tempo_proiezione = elapsed(a1,a2);

                for (std::size_t ik=0;ik<k_values.size();++ik) {
                    const int k = k_values[ik];
                    const auto b0 = std::chrono::steady_clock::now();
                    const double scala = std::sqrt(static_cast<double>(K)/k);
                    Mat Y(n,Vec(k));
                    for (int i=0;i<n;++i)
                        for (int j=0;j<k;++j)
                            Y[i][j] = Ymax[i][j]*scala;
                    const auto b1 = std::chrono::steady_clock::now();
                    const JLPCA_Spettro sy = JL_PCA_spettro(Y);
                    if (sy.sigma.size()<primi || !(sy.energia>0.0))
                        throw std::runtime_error("PCA_JL_K_MC: SVD invalida");

                    double delta_sum=0.0,delta_max=0.0;
                    std::size_t violations=0;
                    for (const auto& p:pairs) {
                        const double d2 = vector_distanza2(Y[p.i],Y[p.j]);
                        const double delta = std::abs(d2/p.d2-1.0);
                        if (!std::isfinite(delta))
                            throw std::runtime_error("PCA_JL_K_MC: delta non finito");
                        delta_sum+=delta;
                        delta_max=std::max(delta_max,delta);
                        if (delta>eps) ++violations;
                    }
                    const bool ok = (violations==0);
                    double ey6=0.0;
                    for (int j=0;j<primi;++j) ey6+=sy.sigma[j]*sy.sigma[j];
                    // Protezione da arrotondamenti numerici minuscoli sul residuo.
                    const double tail_Y = sy.energia-ey6;
                    if (tail_Y < -1e-8*sy.energia)
                        throw std::runtime_error("PCA_JL_K_MC: energia coda negativa");
                    const double r1=sy.sigma[0]/sx.sigma[0];
                    const double r2=sy.sigma[1]/sx.sigma[1];
                    const double Q2=r1/r2;
                    const double var1_Y=sy.sigma[0]*sy.sigma[0]/sy.energia;
                    const double var2_Y=sy.sigma[1]*sy.sigma[1]/sy.energia;
                    const double rEnergia=sy.energia/sx.energia;
                    const double rTail=std::max(0.0,tail_Y)/energia_coda_X;
                    const double vTail=std::max(0.0,tail_Y)/sy.energia;
                    const double time_pca=sy.tempo_s;

                    Acc& acc=data[ik];
                    if (ok) ++acc.good;
                    if (r2>1.0) ++acc.sigma2_up;
                    if (Q2<1.0) ++acc.q2_down;
                    acc.s1.push_back(r1);acc.s2.push_back(r2);acc.q2.push_back(Q2);
                    acc.var1.push_back(var1_Y);acc.var2.push_back(var2_Y);
                    acc.energia.push_back(rEnergia);acc.coda_ratio.push_back(rTail);
                    acc.coda_quota.push_back(vTail);acc.delta.push_back(delta_max);
                    acc.tempo_pca.push_back(time_pca);
                    double Q[5];
                    for (int j=1;j<primi;++j) {
                        if (!(sy.sigma[j]>0.0))
                            throw std::runtime_error("PCA_JL_K_MC: sigma Y nullo");
                        Q[j-1]=(sy.sigma[0]/sy.sigma[j]) /
                            (sx.sigma[0]/sx.sigma[j]);
                        acc.Q[j-1].push_back(Q[j-1]);
                    }
                    detail << id << ',' << baseline_used << ',' << t+1 << ','
                        << seed_A << ',' << n << ',' << d << ',' << k << ',' << K << ','
                        << eps << ',' << pairs.size() << ',' << violations << ','
                        << delta_sum/pairs.size() << ',' << delta_max << ',' << (ok?1:0)
                        << ',' << r1 << ',' << r2 << ',' << Q2
                        << ',' << var1_X << ',' << var1_Y
                        << ',' << var2_X << ',' << var2_Y
                        << ',' << rEnergia << ',' << rTail
                        << ',' << coda_X << ',' << vTail
                        << ',' << tempo_A << ',' << tempo_proiezione
                        << ',' << elapsed(b0,b1) << ',' << time_pca;
                    for (int j=0;j<primi;++j) detail << ',' << sy.sigma[j];
                    for (int j=1;j<5;++j) detail << ',' << Q[j];
                    detail << ',' << seed_X << ',' << sx.sigma[0]
                        << ',' << sx.sigma[1] << ',' << sx.energia << '\n';
                    detail.flush();
                    std::cout << "trial=" << std::setw(2) << t+1 << " k=" << std::setw(3)
                            << k << " seed=" << seed_A
                            << " JL=" << (ok?"OK":"VIOLATO")
                            << " dmax=" << std::fixed << std::setprecision(4) << delta_max
                            << " s1/X=" << r1 << " s2/X=" << r2 << " Q2=" << Q2
                            << " tail6/X=" << rTail << " PCA="
                            << std::setprecision(3) << time_pca << " s\n";
                }
            }

            auto emit_stat=[&](int k,const char* metric,const std::vector<double>& v) {
                if (v.empty()) return;
                const auto s=stat(v);
                stats << id << ',' << baseline_used << ',' << n << ',' << d << ',' << k
                    << ',' << K << ',' << eps << ',' << T << ',' << metric << ','
                    << v.size() << ',' << s.avg << ',' << s.sd << ','
                    << s.lo << ',' << s.hi << ',' << seed_X
                    << ',' << sx.energia << '\n';
            };
            std::cout << "\n--- RIEPILOGO (medie +/- dev.std campionaria) ---\n";
            for (std::size_t ik=0;ik<k_values.size();++ik) {
                const int k=k_values[ik];
                const Acc& a=data[ik];
                if (a.q2.empty()) continue;
                emit_stat(k,"sigma1_Y_su_X",a.s1);
                emit_stat(k,"sigma2_Y_su_X",a.s2);
                for (int j=0;j<5;++j) {
                    const auto name="Q"+std::to_string(j+2);
                    emit_stat(k,name.c_str(),a.Q[j]);
                }
                emit_stat(k,"var1_Y",a.var1);
                emit_stat(k,"var2_Y",a.var2);
                emit_stat(k,"energia_Y_su_X",a.energia);
                emit_stat(k,"energia_coda6_Y_su_X",a.coda_ratio);
                emit_stat(k,"quota_coda6_Y",a.coda_quota);
                emit_stat(k,"delta_max",a.delta);
                emit_stat(k,"tempo_PCA_s",a.tempo_pca);
                const auto r2=stat(a.s2), q=stat(a.q2), tail=stat(a.coda_ratio);
                std::cout << "k=" << std::setw(3) << k << " JL=" << a.good
                        << '/' << a.q2.size() << " s2/X=" << std::setprecision(5)
                        << r2.avg << " +/- " << r2.sd
                        << " Q2=" << q.avg << " +/- " << q.sd
                        << " coda6/X=" << tail.avg
                        << " s2>1:" << a.sigma2_up
                        << " Q2<1:" << a.q2_down << '\n';
            }
            detail.close();
            stats.close();
            if (!detail || !stats)
                throw std::runtime_error("PCA_JL_K_MC: errore di scrittura in " + output.directory().string());
            std::cout << "\nCSV trial: " << trial_file.string()
                    << "\nCSV statistiche: " << stats_file.string()
                    << "\nNOTA: le matrici per diversi k condividono gli stessi coefficienti"
                        " gaussiani (righe iniziali).\n"
                    << "Il confronto temporale misura la PCA per ciascun k; la"
                        " creazione/proiezione master e' un costo condiviso.\n"
                    << "Non aspettarsi monotonia rigorosa trial-per-trial.\n";

            fcn_form::wait_report();
        }
    }

    // Richiede gli header STL: algorithm, array, chrono, cmath, fstream, iomanip,
    // iostream, limits, sstream, stdexcept, string, vector.
    // Le primitive gia' presenti: genera_cluster, JL_estrai_bilanciato,
    // matrix_random_jl, matrix_prodotto_matrix, matrix_trasposta,
    // vector_distanza2, fcn_form::wait_report.
    //
    // Nota: JL_PCA_spettro restituisce SOLO i sigma, non gli score 2D.
    // Qui gli score 2D sono ottenuti diagonalizzando G=Xc Xc^T (n x n)
    // con Jacobi simmetrico; gli autovalori sono sigma^2. Il controllo sui
    // sigma originali verifica la coerenza con il riferimento SVD preesistente.
    // Il plot SVG non necessita di API Matplot++ non verificate nel repo.


    void JL_classificazione_PCA()
    {
        const jl_files::Store files{"data"};
        JLClassPCAParams p{};
        while (JL_CLASS_PCA_form::prompt(p)) {
            clear_screen();

        const int n=p.n, d=p.d, seed_X=p.seed_X;
        const int master_n=std::max(160,n);
        const int k=p.k;
        const unsigned seed_A=static_cast<unsigned>(p.seed_A);
        const double eps=static_cast<double>(p.eps_pct)/100.0;
        std::cout << "\n=== JL + PCA: CLASSIFICAZIONE E OVERLAY CLUSTER ===\n"
                << "Dataset: n="<<n<<" d="<<d<<" seed_X="<<seed_X<<'\n'
                << "Archivio JL: " << files.root().string() << '\n'
                << "k="<<k<<" seed_A="<<seed_A<<" epsilon="<<eps<<"\n";
        // n<=160: mantiene il campionamento bilanciato dai 160 punti master
        // (la geometria resta quella storica soltanto in modo Federico).
        // Usa anche i default di JLConfigCentroidi definiti in dataset.hpp.
        // La chiamata a TRE argomenti usa SEMPRE Federico, ignorando cfg.
        const JLConfigCentroidi cfg{};
        std::cout << "Modello centroidi: "
                << (cfg.modo==JLScalaCentroidi::Federico ? "Federico" : "LeggeDiPotenza")
                << " D_ref=" << cfg.distanza_ref
                << " d_ref=" << cfg.d_ref << " alfa=" << cfg.alfa << '\n';
        const auto [master,labelMaster]=genera_cluster(master_n,d,seed_X,cfg);
        const auto [X,labels]=JL_estrai_bilanciato(master,labelMaster,n);
        const auto t0=std::chrono::steady_clock::now();
        JLCL_PCA2 Xp=JLCL_pca2_gram(X);
        const auto t1=std::chrono::steady_clock::now();

        // Controllo indipendente contro la SVD originale del commit c089613.
        // Se presente, il CSV permette di rilevare qualunque errore di convenzione.
        {
            // La SVD storica appartiene esclusivamente ai dati originali
            // di Federico: i parametri di dimensione e seed non bastano.
            const bool reference_family =
                (cfg.modo==JLScalaCentroidi::Federico &&
                n<=160 && d==1000 && seed_X==42);
            std::ifstream base;
            if (reference_family) base.open(JL_PCA_baseline_file(files));
            if (base.is_open()) {
                std::string header,body;
                std::getline(base,header);
                const auto split=[](const std::string& text) {
                    std::vector<std::string> a;
                    std::istringstream in(text);std::string s;
                    while(std::getline(in,s,','))a.push_back(s);
                    return a;
                };
                const auto keys=split(header);
                auto index=[&](const std::string& name)->std::size_t{
                    const auto it=std::find(keys.begin(),keys.end(),name);
                    if(it==keys.end())throw std::runtime_error("baseline: campo mancante "+name);
                    return static_cast<std::size_t>(it-keys.begin());
                };
                const auto in=index("n"),is1=index("sigma1"),is2=index("sigma2");
                const auto id=index("d"),iseed=index("seed_X");
                bool found=false;
                while(std::getline(base,body)) {
                    const auto a=split(body);
                    if(a.size()!=keys.size())continue;
                    if(std::stoi(a[in])!=n)continue;
                    if(std::stoi(a[id])!=d || std::stoi(a[iseed])!=seed_X)
                        throw std::runtime_error("baseline: dimensioni/seed incompatibili");
                    const double b1=std::stod(a[is1]),b2=std::stod(a[is2]);
                    if(std::abs(Xp.sigma1/b1-1)>1e-5 ||
                    std::abs(Xp.sigma2/b2-1)>1e-5)
                        throw std::runtime_error("PCA Gram: sigma discordi dalla SVD baseline");
                    found=true;break;
                }
                if (found)
                    std::cout << "PCA originale: sigma1, sigma2 coincidono con baseline.\n";
                else
                    std::cout << "PCA originale: n assente dal CSV, riferimento "
                                "calcolato nel nuovo test.\n";
            } else {
                std::cout << "PCA originale: riferimento ricalcolato sul dataset "
                            "corrente (baseline storica non applicabile oppure "
                            "CSV assente).\n";
            }
        }

        const auto a0=std::chrono::steady_clock::now();
        const Mat A=matrix_random_jl(
            k,d,JLMatrixDistribution::Gaussian,seed_A,3);
        const Mat Y=matrix_prodotto_matrix(X,matrix_trasposta(A));
        const auto a1=std::chrono::steady_clock::now();
        JLCL_PCA2 Yp=JLCL_pca2_gram(Y);
        const auto a2=std::chrono::steady_clock::now();
        const auto cx=JLCL_evaluate(Xp.score,labels);
        const auto cy=JLCL_evaluate(Yp.score,labels);
        const auto alignment=JLCL_align(Xp.score,cx,Yp.score,cy);

        // Conservazione delle distanze JL sull'insieme finito di coppie.
        double delta_sum=0.0,delta_max=0.0;
        int violations=0,pairs=0;
        for(int i=0;i<n;++i)for(int j=i+1;j<n;++j){
            const double dx=vector_distanza2(X[i],X[j]);
            const double dy=vector_distanza2(Y[i],Y[j]);
            if(!(dx>0))throw std::runtime_error("JL_CLASS: distanza X nulla");
            const double dd=std::abs(dy/dx-1.0);
            ++pairs;delta_sum+=dd;delta_max=std::max(delta_max,dd);
            if(dd>eps)++violations;
        }

        const auto output = files.begin_run("classificazione", "PCA_JL_CLASS");
        const auto& run = output.id();
        auto out = output.open_new("summary.csv");
        auto perpoint = output.open_new("points.csv");
        out << std::fixed << std::setprecision(9);
        perpoint << std::fixed << std::setprecision(9);
        out << "campagna,n,d,k,seed_X,seed_A,eps,coppie,violazioni,delta_media,delta_max,"
            "sigma1_X,sigma2_X,sigma1_Y,sigma2_Y,"
            "var1_X,var1_Y,var2_X,var2_Y,"
            "dist_centroidi_X,dist_centroidi_Y,"
            "raggio0_X,raggio1_X,raggio0_Y,raggio1_Y,"
            "sep_ratio_X,sep_ratio_Y,gap_X,gap_Y,"
            "errori_train_X,errori_train_Y,errori_LOO_X,errori_LOO_Y,"
            "accuratezza_LOO_X,accuratezza_LOO_Y,"
            "margine_min_LOO_X,margine_min_LOO_Y,"
            "angolo_allineamento_deg,riflessione_trasversale,rms_spostamento_plot,"
            "tempo_PCA_X_s,tempo_proiezione_s,tempo_PCA_Y_s,"
            "modo_centroidi,D_ref,d_ref,alfa\n";
        out << run<<','<<n<<','<<d<<','<<k<<','<<seed_X<<','<<seed_A<<','
            <<eps<<','<<pairs<<','<<violations<<','<<delta_sum/pairs<<','<<delta_max<<','
            <<Xp.sigma1<<','<<Xp.sigma2<<','<<Yp.sigma1<<','<<Yp.sigma2<<','
            <<Xp.sigma1*Xp.sigma1/Xp.energia<<','<<Yp.sigma1*Yp.sigma1/Yp.energia<<','
            <<Xp.sigma2*Xp.sigma2/Xp.energia<<','<<Yp.sigma2*Yp.sigma2/Yp.energia<<','
            <<cx.separation<<','<<cy.separation<<','
            <<cx.radius[0]<<','<<cx.radius[1]<<','<<cy.radius[0]<<','<<cy.radius[1]<<','
            <<cx.sep_ratio<<','<<cy.sep_ratio<<','<<cx.signed_gap<<','<<cy.signed_gap<<','
            <<cx.train_errors<<','<<cy.train_errors<<','<<cx.loo_errors<<','<<cy.loo_errors<<','
            <<1.0-cx.loo_errors/static_cast<double>(n)<<','
            <<1.0-cy.loo_errors/static_cast<double>(n)<<','
            <<cx.loo_min_margin<<','<<cy.loo_min_margin<<','
            <<alignment.angle_deg<<','<<(alignment.reflected?1:0)<<','
            <<alignment.rms_displacement<<','
            <<std::chrono::duration<double>(t1-t0).count()<<','
            <<std::chrono::duration<double>(a1-a0).count()<<','
            <<std::chrono::duration<double>(a2-a1).count()<<','
            <<(cfg.modo==JLScalaCentroidi::Federico?"Federico":"LeggeDiPotenza")<<','
            <<cfg.distanza_ref<<','<<cfg.d_ref<<','<<cfg.alfa<<'\n';
        perpoint << "campagna,id,label,PC1_X,PC2_X,PC1_JL_raw,PC2_JL_raw,"
                    "PC1_JL_aligned,PC2_JL_aligned,pred_LOO_X,pred_LOO_JL,"
                    "margin_LOO_X,margin_LOO_JL\n";
        for(int i=0;i<n;++i)
            perpoint << run<<','<<i<<','<<labels[i]<<','
                <<Xp.score[i].x<<','<<Xp.score[i].y<<','
                <<Yp.score[i].x<<','<<Yp.score[i].y<<','
                <<alignment.Yaligned[i].x<<','<<alignment.Yaligned[i].y<<','
                <<cx.loo_prediction[i]<<','<<cy.loo_prediction[i]<<','
                <<cx.loo_margin[i]<<','<<cy.loo_margin[i]<<'\n';
        auto graphic = output.open_new("overlay.svg");
        JLCL_write_svg(graphic,Xp.score,alignment.Yaligned,labels,cx,cy,
                    alignment,n,d,k,eps,true);
        auto graphic_centroids = output.open_new("centroidi.svg");
        JLCL_write_svg(graphic_centroids,Xp.score,alignment.Yaligned,labels,cx,cy,
                    alignment,n,d,k,eps,false);
        out.close();
        perpoint.close();
        graphic.close();
        graphic_centroids.close();
        if (!out || !perpoint || !graphic || !graphic_centroids)
            throw std::runtime_error("JL_CLASS: errore di scrittura in " + output.directory().string());
        std::cout << std::fixed << std::setprecision(5)
            << "PCA X: sigma1="<<Xp.sigma1<<" sigma2="<<Xp.sigma2
            <<" var1="<<Xp.sigma1*Xp.sigma1/Xp.energia<<'\n'
            << "PCA Y: sigma1="<<Yp.sigma1<<" sigma2="<<Yp.sigma2
            <<" var1="<<Yp.sigma1*Yp.sigma1/Yp.energia<<'\n'
            << "JL: delta_max="<<delta_max<<" violazioni="<<violations<<'/'<<pairs<<'\n'
            << "Classificazione 2PC (leave-one-out): X="<<n-cx.loo_errors<<'/'<<n
            <<"  Y="<<n-cy.loo_errors<<'/'<<n<<'\n'
            << "Distanza centroidi: X="<<cx.separation<<" Y="<<cy.separation<<'\n'
            << "Separazione/raggi: X="<<cx.sep_ratio<<" Y="<<cy.sep_ratio<<'\n'
            << "Gap assiale: X="<<cx.signed_gap<<" Y="<<cy.signed_gap<<'\n'
            << "Allineamento grafico: rotazione="<<alignment.angle_deg
            <<" gradi, riflessione="<<(alignment.reflected?"si":"no")<<'\n'
            << "Campagna: " << output.campaign() << "  run: " << run << '\n'
            << "Directory run: " << output.directory().string() << '\n'
            << "CSV: summary.csv  points.csv\n"
            << "Grafici SVG: overlay.svg  centroidi.svg\n";

            fcn_form::wait_report();
        }
    }

    void sandbox() {
            SandboxParams params{};
            while (sandbox_form::prompt(params)) {
                clear_screen();
                cout << "SANDBOX GNPORT\n";
                cout << "Inizio     : " << params.i_ini << '\n';
                cout << "Fine       : " << params.i_max << '\n';
                cout << "Parametro  : " << params.i_val << '\n';
                fcn_form::wait_report();
            }
        }


    /*
    * ============================================================
    * INTEGRAZIONE MENU GNPORT — AZIONI DEL PROGRAMMA
    * ============================================================
    *
    * Ogni action_id presente in menu.json deve corrispondere
    * a una funzione registrata qui.
    */

    using Action = std::function<void()>;
    using ActionRegistry = std::unordered_map<std::string, Action>;

    ActionRegistry build_action_registry() 
        {
        return {
            {"f1_sample",      f1_sample},              // form ok
            {"f1_taylor_exp",  f1_taylor_exp},          // form ok
            {"f1_taylor_cos",  f1_taylor_cos},          // form ok
            {"f1_prec",        f1_prec},                // form ok
            {"f2_deriv",       f2_deriv},               // form ok
            {"f2_cauchy",      f2_cauchy},              // form ok
            {"f2_bordo",       f2_bordo},               // form ok
            {"f2_integral",    f2_integral},            // form ok
            {"f2_norme",       f2_norme},               // form ok
            {"f2_deconv",       f2_deconv},             // form ok
            {"f3_svd",          f3_svd},                // form ok
            {"f3_svd_test",     f3_svd_test},           // form ok
            {"f3_svd_pca",      f3_svd_pca},            // form ok 
            {"f3_svd_an",       f3_svd_an},             // form ok
            {"sym_cauchy",      sym_cauchy},            // form ok
            {"sym_hhalpha",     sym_hhalpha},           // form ok
            {"ode_euler",       ode_euler},             // form ok
            {"ode_osc2d",       ode_osc2d},             // form ok
            {"ode_lotkav",      ode_lotkav},            // form ok
            {"gg1",             gg1},
            {"gg2",             gg2},
            {"al_qr",           al_qr},                 // form ok
            {"JL_dist",         JL_distanze},           // form ok
            {"JL_X_rand",       JL_distanze_dataset},   // form ok
            {"JL_A_rand",       JL_distanze_dataMC},    // form ok
            {"JL_K_C_n",        JL_stima_coeff_n},      // form ok
            {"JL_K_C_d",        JL_stima_coeff_d},      // form ok
            {"PCA_JL",          JL_effetto_PCA},        //straight
            {"PCA_JL_MC",       JL_effetto_PCA_MC},     //straight
            {"PCA_JL_K_MC",     JL_effetto_PCA_K_MC},   //straight
            {"PCA_JL_CLASS",    JL_classificazione_PCA},//straight
            {"sandbox",         sandbox}
            };
        }

        /*
        * Avvia il menu FCN usando il runner generico GNPORT.
        */

        GnMenuRunnerResult run_fcn_menu()
        {
            const ActionRegistry actions =
                build_action_registry();

            return gn_menu_run_registry(
                "menu.json",
                actions
            );
        }
    /*
    * ============================================================
    * INTEGRAZIONE MENU GNPORT — FINE DICHIARAZIONI
    * ============================================================
    */

};  

// 
// fine namespace anonimo
//


int main()
{
    {
        figure_handle fig = matplot_table_init(
            true,
            "Figure vuota per inizializzare matplot++",
            "Figure vuota per inizializzare matplot++",
            2,
            2
        );

        // fig->draw();
    }

    /*
    * ============================================================
    * INTEGRAZIONE MENU GNPORT — ESECUZIONE
    * ============================================================
    */

/*
* 
* versione liscia
*
const GnMenuRunnerResult result =
    run_fcn_menu();

if (result.status != GN_MENU_RUNNER_OK) {
    std::cerr << result.message << '\n';
    return 1;
}
*
* e versione con Exception handling
*
*/

    try {
        const GnMenuRunnerResult menu_result =
            run_fcn_menu();

        if (menu_result.status !=
            GN_MENU_RUNNER_OK) {

            std::cerr
                << "Errore menu: "
                << menu_result.message
                << '\n';

            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr
            << "Errore inizializzazione o "
            "esecuzione delle azioni: "
            << e.what()
            << '\n';

        return 1;
    }

    /*
    * ============================================================
    * INTEGRAZIONE MENU GNPORT — FINE ESECUZIONE
    * ============================================================
    */

    std::cout << "\nUscita dal programma.\n";
    return 0;
}