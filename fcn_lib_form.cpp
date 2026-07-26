#include "fcn_lib_form.h"

#include "gn_form_runner.h"
#include "gn_idle.h"
#include "gn_terminal.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

constexpr std::size_t F3_SVD_TEST_FIELD_COUNT = 2;
constexpr std::size_t F3_SVD_FIELD_COUNT = 2;
constexpr std::size_t F3_SVD_PCA_FIELD_COUNT = 4;


namespace fcn_form 
{

    /*
    * Policy lineare condivisa dai primi due form FCN.
    * Non appartiene al runner GN: e' soltanto una rotta disponibile che
    * ciascun form specifico puo' scegliere, sostituire o abbandonare.
    */
    GnFormChangeResult change_linear(
        GnEntryCommand command,
        GnFormContext& context)
    {
        const std::size_t maxlab = context.field_count;

        switch (command) {
            case GN_ENTRY_ENTER:
            case GN_ENTRY_CTRL_ENTER:
            case GN_ENTRY_TAB:
            case GN_ENTRY_DOWN:
            case GN_ENTRY_RIGHT:
            case GN_ENTRY_PAGE_DOWN:
            case GN_ENTRY_CTRL_DOWN:
            case GN_ENTRY_CTRL_RIGHT:
            case GN_ENTRY_CTRL_PAGE_DOWN:
                context.lab = (context.lab + 1) % maxlab;
                return GN_FORM_CHANGE_CONTINUE;

            case GN_ENTRY_SHIFT_TAB:
            case GN_ENTRY_UP:
            case GN_ENTRY_LEFT:
            case GN_ENTRY_PAGE_UP:
            case GN_ENTRY_CTRL_UP:
            case GN_ENTRY_CTRL_LEFT:
            case GN_ENTRY_CTRL_PAGE_UP:
                context.lab = (context.lab + maxlab - 1) % maxlab;
                return GN_FORM_CHANGE_CONTINUE;

            case GN_ENTRY_HOME:
            case GN_ENTRY_CTRL_HOME:
                context.lab = 0;
                return GN_FORM_CHANGE_CONTINUE;

            case GN_ENTRY_END:
            case GN_ENTRY_CTRL_END:
                context.lab = maxlab - 1;
                return GN_FORM_CHANGE_CONTINUE;

            case GN_ENTRY_F1:
                return GN_FORM_CHANGE_HELP;

            case GN_ENTRY_F10:
                return GN_FORM_CHANGE_ACCEPT;

            case GN_ENTRY_ESCAPE:
                return GN_FORM_CHANGE_CANCEL;

            default:
                gn_screen_beep();
                return GN_FORM_CHANGE_CONTINUE;
        }
    }

    void write(
        int row,
        int column,
        const std::string& text,
        int available_columns,
        GnScreenRole role = GN_SCREEN_NORMAL)
    {
        if (row < 1 || column < 1 || available_columns < 1) return;

        const std::size_t maximum =
            static_cast<std::size_t>(available_columns);
        const std::string shown =
            text.size() <= maximum ? text : text.substr(0, maximum);

        gn_screen_move(row, column);
        gn_screen_set_role(role);
        gn_screen_write(shown.c_str());
        gn_screen_set_role(GN_SCREEN_NORMAL);
    }

    void separator(int row, int columns)
    {
        if (row <= 0 || columns < 2) return;
        fcn_form::write(row, 1, std::string(columns - 1, '-'), columns - 1);
    }

    std::string field_value(const std::string& value, std::size_t width)
    {
        if (value.size() >= width) return value.substr(0, width);
        return std::string(width - value.size(), ' ') + value;
    }

    void clear_pending_stdin()
    {
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        const std::streamsize available = std::cin.rdbuf()->in_avail();
        if (available > 0) {
            std::cin.ignore(available, '\n');
        }
    }

        void wait_report()
    {
        std::cout << "\nPremi INVIO per tornare ai parametri...";
        clear_pending_stdin();
        std::cin.get();
        clear_pending_stdin();
    }

} // namespace fcn_form


    namespace f3_svd_test_form
{
        using fcn_form::write;
        using fcn_form::separator;
        using fcn_form::field_value;
        using fcn_form::change_linear;

    void f3_svd_test_mask(const GnFormContext& context)
    {

        const GnFormLayout& layout = context.layout;
        const int width = std::max(1, layout.terminal_columns - 1);
        const int field_column = std::min(39, std::max(31, width - 10));
        const int first_field_row = layout.viewport.top + 6;

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        fcn_form::write(
            layout.header_top,
            1,
            " SVD - collaudo della bidiagonalizzazione ",
            width,
            GN_SCREEN_TITLE);

        fcn_form::write(
            layout.header_top + 1,
            3,
            "Parametri della matrice casuale usata dal test",
            width - 3);

        fcn_form::separator(layout.top_separator_row, layout.terminal_columns);

        int row = layout.viewport.top;
        fcn_form::write(row++, 3,
            "Il test costruisce una matrice casuale X di dimensione n x d.",
            width - 3);
        fcn_form::write(row++, 3,
            "La riduce a forma bidiagonale e verifica ortogonalita',",
            width - 3);
        fcn_form::write(row++, 3,
            "ricostruzione di X e norme del residuo numerico.",
            width - 3);
        ++row;
        fcn_form::write(row++, 3,
            "Righe e colonne sono indipendenti: la matrice puo' essere",
            width - 3);
        fcn_form::write(row++, 3,
            "alta, quadrata oppure larga.",
            width - 3);

        fcn_form::write(first_field_row, 5, "  Numero di righe n", field_column - 6);
        fcn_form::write(first_field_row + 2, 5, "  Numero di colonne d", field_column - 6);

        fcn_form::separator(layout.bottom_separator_row, layout.terminal_columns);
        fcn_form::separator(layout.status_separator_row, layout.terminal_columns);
    }

    void f3_svd_test_show(const GnFormContext& context)
    {
        static const char* labels[F3_SVD_TEST_FIELD_COUNT] = {
            "Numero di righe n",
            "Numero di colonne d"
        };

        for (std::size_t i = 0; i < context.field_count; ++i) {
            fcn_form::write(
                context.fields[i].row,
                5,
                std::string(i == context.lab ? "> " : "  ") + labels[i],
                context.fields[i].column - 6);

            fcn_form::write(
                context.fields[i].row,
                context.fields[i].column,
                fcn_form::field_value(context.values[i], context.fields[i].length),
                static_cast<int>(context.fields[i].length),
                GN_SCREEN_FIELD);
        }

        gn_screen_clear_line(context.layout.echo_row);
    }

    void f3_svd_test_dispdid(const GnFormContext& context)
    {
        static const char* hints[F3_SVD_TEST_FIELD_COUNT] = {
            "n: numero di righe, naturale [1..9999]",
            "d: numero di colonne, naturale [1..9999]"
        };

        const int width = std::max(1, context.layout.terminal_columns - 1);
        fcn_form::write(
            context.layout.hint_row,
            2,
            hints[context.lab],
            width - 2,
            GN_SCREEN_MESSAGE);
    }

    void f3_svd_test_keyvalid(const GnFormContext& context)
    {
        const int width = std::max(1, context.layout.terminal_columns - 1);

        fcn_form::write(
            context.layout.status_top,
            2,
            "ESC Menu        F1 Help        F10 Start",
            width - 2,
            GN_SCREEN_STATUS);
    }

    void f3_svd_test_message(const GnFormContext& context)
    {
        if (context.message.empty()) return;

        const int width = std::max(1, context.layout.terminal_columns - 1);
        fcn_form::write(
            context.layout.echo_row,
            2,
            context.message,
            width - 2,
            GN_SCREEN_ERROR);
    }

    bool f3_svd_test_prepare(GnFormContext& context, void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F3_SVD_TEST_FIELD_COUNT)
        {
            return false;
        }

        const int field_column =
            std::min(39, std::max(31, context.layout.terminal_columns - 11));
        const int first_field_row = context.layout.viewport.top + 6;

        context.fields[0] = GnEntrySpec{
            first_field_row,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            4,
            1.0,
            9999.0,
            1,
            context.layout.echo_row
        };

        context.fields[1] = GnEntrySpec{
            first_field_row + 2,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            4,
            1.0,
            9999.0,
            1,
            context.layout.echo_row
        };

        return true;
    }

    void f3_svd_test_render(const GnFormContext& context, void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F3_SVD_TEST_FIELD_COUNT ||
            context.lab >= context.field_count)
        {
            return;
        }

        f3_svd_test_mask(context);
        f3_svd_test_show(context);
        f3_svd_test_dispdid(context);
        f3_svd_test_keyvalid(context);
        f3_svd_test_message(context);

        gn_screen_flush();
    }


    /* CHANGE specifica del form: per ora sceglie la policy lineare FCN. */
    GnFormChangeResult f3_svd_test_change(
        GnEntryCommand command,
        GnFormContext& context,
        void*)
    {
        return fcn_form::change_linear(command, context);
    }

    void f3_svd_test_help(const GnFormContext& context, void*)
    {
        const int width = std::max(1, context.layout.terminal_columns - 1);
        const std::vector<std::string> lines = {
            "F1 HELP - SVD / test di bidiagonalizzazione",
            "",
            "Obiettivo",
            "  Verificare la riduzione X = U0 B V0^T ottenuta con",
            "  trasformazioni di Householder e Golub-Kahan.",
            "",
            "Parametri",
            "  n  numero di righe della matrice casuale X",
            "  d  numero di colonne della matrice casuale X",
            "",
            "Uso del form SVD",
            "  ENTER/TAB e i cursori previsti dalla rotta cambiano campo.",
            "  SPACE modifica il valore; digitazione diretta lo sostituisce.",
            "",
            "Cosa osservare nel rapporto",
            "  - B deve risultare bidiagonale;",
            "  - U0 e V0 devono essere ortogonali;",
            "  - il residuo X - U0 B V0^T deve essere prossimo a zero;",
            "  - le diverse stime della norma spettrale devono concordare.",
            "",
            "Dopo F10 il rapporto usa lo stdio normale: la scrollbar del",
            "terminale consente di rileggere anche l'output oltre lo schermo.",
            "",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        int row = 1;
        for (const std::string& line : lines) {
            if (row > context.layout.terminal_rows) break;
            fcn_form::write(
                row,
                line.empty() ? 1 : 2,
                line,
                line.empty() ? width : width - 2,
                row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
            ++row;
        }
        gn_screen_flush();

        for (;;) {
            GnKeyEvent event{};
            const GnPollResult result = gn_pump_once(&event);

            if (result == GN_POLL_EMPTY) {
                gn_cooperative_pause();
                continue;
            }
            if (result == GN_POLL_ERROR) return;

            if (event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
            {
                return;
            }

            gn_screen_beep();
        }
    }

    bool prompt(int& n, int& d)
    {
        std::string values[F3_SVD_TEST_FIELD_COUNT] = {
            std::to_string(n),
            std::to_string(d)
        };

        GnFormRunSpec spec;
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 68;

        spec.values = values;
        spec.field_count = F3_SVD_TEST_FIELD_COUNT;
        spec.prepare = f3_svd_test_prepare;
        spec.render = f3_svd_test_render;
        spec.change = f3_svd_test_change;
        spec.help = f3_svd_test_help;

        const GnFormRunResult result = gn_form_run_session(spec);

        if (result == GN_FORM_RUN_ACCEPT) {
            // Tipo e bounds sono gia' stati validati da ENTRY nel runner.
            n = std::stoi(values[0]);
            d = std::stoi(values[1]);
            return true;
        }

        if (result == GN_FORM_RUN_ERROR) {
            // L'errore infrastrutturale viene assorbito dal layer prompt:
            // fcn_all.cpp deve decidere solo se eseguire o no l'action.
            std::cerr << "Impossibile aprire il form SVD.\n";
        }

        return false;
    }

} // namespace f3_svd_test_form

namespace f3_svd_form
{
    using fcn_form::write;
    using fcn_form::separator;
    using fcn_form::change_linear;

    void f3_svd_mask(const GnFormContext& context)
    {
        const GnFormLayout& layout = context.layout;
        const int width = std::max(1, layout.terminal_columns - 1);
        const int field_column = std::min(39, std::max(31, width - 10));
        const int first_field_row = layout.viewport.top + 6;

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        fcn_form::write(
            layout.header_top,
            1,
            " SVD - decomposizione completa ",
            width,
            GN_SCREEN_TITLE);

        fcn_form::write(
            layout.header_top + 1,
            3,
            "Parametri della matrice casuale da decomporre",
            width - 3);

        fcn_form::separator(layout.top_separator_row, layout.terminal_columns);

        int row = layout.viewport.top;
        fcn_form::write(row++, 3,
            "Costruisce una matrice casuale X di dimensione n x d,",
            width - 3);
        fcn_form::write(row++, 3,
            "la bidiagonalizza e calcola la SVD completa.",
            width - 3);
        fcn_form::write(row++, 3,
            "Il rapporto confronta ricostruzione, ortogonalita' e",
            width - 3);
        fcn_form::write(row++, 3,
            "valori singolari con gli autovalori di X^T X.",
            width - 3);

        fcn_form::write(first_field_row, 5, "  Numero di righe n", field_column - 6);
        fcn_form::write(first_field_row + 2, 5, "  Numero di colonne d", field_column - 6);

        fcn_form::separator(layout.bottom_separator_row, layout.terminal_columns);
        fcn_form::separator(layout.status_separator_row, layout.terminal_columns);
    }

    void f3_svd_show(const GnFormContext& context)
    {
        static const char* labels[F3_SVD_FIELD_COUNT] = {
            "Numero di righe n",
            "Numero di colonne d"
        };

        for (std::size_t i = 0; i < context.field_count; ++i) {
            fcn_form::write(
                context.fields[i].row,
                5,
                std::string(i == context.lab ? "> " : "  ") + labels[i],
                context.fields[i].column - 6);

            fcn_form::write(
                context.fields[i].row,
                context.fields[i].column,
                fcn_form::field_value(context.values[i], context.fields[i].length),
                static_cast<int>(context.fields[i].length),
                GN_SCREEN_FIELD);
        }

        gn_screen_clear_line(context.layout.echo_row);
    }

    void f3_svd_dispdid(const GnFormContext& context)
    {
        static const char* hints[F3_SVD_FIELD_COUNT] = {
            "n: numero di righe, naturale [3..100]",
            "d: numero di colonne, naturale [3..100]"
        };

        const int width = std::max(1, context.layout.terminal_columns - 1);
        fcn_form::write(
            context.layout.hint_row,
            2,
            hints[context.lab],
            width - 2,
            GN_SCREEN_MESSAGE);
    }

    void f3_svd_keyvalid(const GnFormContext& context)
    {
        const int width = std::max(1, context.layout.terminal_columns - 1);

        fcn_form::write(
            context.layout.status_top,
            2,
            "ESC Menu        F1 Help        F10 Esegui",
            width - 2,
            GN_SCREEN_STATUS);
    }

    void f3_svd_message(const GnFormContext& context)
    {
        if (context.message.empty()) return;

        const int width = std::max(1, context.layout.terminal_columns - 1);
        fcn_form::write(
            context.layout.echo_row,
            2,
            context.message,
            width - 2,
            GN_SCREEN_ERROR);
    }

    bool f3_svd_prepare(GnFormContext& context, void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F3_SVD_FIELD_COUNT)
        {
            return false;
        }

        const int field_column =
            std::min(39, std::max(31, context.layout.terminal_columns - 11));
        const int first_field_row = context.layout.viewport.top + 6;

        context.fields[0] = GnEntrySpec{
            first_field_row,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            3,
            3.0,
            100.0,
            1,
            context.layout.echo_row
        };

        context.fields[1] = GnEntrySpec{
            first_field_row + 2,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            3,
            3.0,
            100.0,
            1,
            context.layout.echo_row
        };

        return true;
    }

    void f3_svd_render(const GnFormContext& context, void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F3_SVD_FIELD_COUNT ||
            context.lab >= context.field_count)
        {
            return;
        }

        f3_svd_mask(context);
        f3_svd_show(context);
        f3_svd_dispdid(context);
        f3_svd_keyvalid(context);
        f3_svd_message(context);

        gn_screen_flush();
    }

    GnFormChangeResult f3_svd_change(
        GnEntryCommand command,
        GnFormContext& context,
        void*)
    {
        return fcn_form::change_linear(command, context);
    }

    void f3_svd_help(const GnFormContext& context, void*)
    {
        const int width = std::max(1, context.layout.terminal_columns - 1);
        const std::vector<std::string> lines = {
            "F1 HELP - SVD completa",
            "",
            "Obiettivo",
            "  Calcolare la decomposizione ai valori singolari di una",
            "  matrice casuale X e verificare numericamente i fattori.",
            "",
            "Parametri",
            "  n  numero di righe della matrice casuale X [3..100]",
            "  d  numero di colonne della matrice casuale X [3..100]",
            "",
            "Uso del form",
            "  ENTER/TAB e i cursori previsti dalla rotta cambiano campo.",
            "  SPACE modifica il valore; digitazione diretta lo sostituisce.",
            "",
            "F10 esegue la decomposizione; ESC torna al menu.",
            "",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        int row = 1;
        for (const std::string& line : lines) {
            if (row > context.layout.terminal_rows) break;
            fcn_form::write(
                row,
                line.empty() ? 1 : 2,
                line,
                line.empty() ? width : width - 2,
                row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
            ++row;
        }
        gn_screen_flush();

        for (;;) {
            GnKeyEvent event{};
            const GnPollResult result = gn_pump_once(&event);

            if (result == GN_POLL_EMPTY) {
                gn_cooperative_pause();
                continue;
            }
            if (result == GN_POLL_ERROR) return;

            if (event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
            {
                return;
            }

            gn_screen_beep();
        }
    }

        bool prompt(int& n, int& d)
    {
        std::string values[F3_SVD_FIELD_COUNT] = {
            std::to_string(n),
            std::to_string(d)
        };

        GnFormRunSpec spec;
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 68;

        spec.values = values;
        spec.field_count = F3_SVD_FIELD_COUNT;
        spec.prepare = f3_svd_prepare;
        spec.render = f3_svd_render;
        spec.change = f3_svd_change;
        spec.help = f3_svd_help;

        const GnFormRunResult result = gn_form_run_session(spec);

        if (result == GN_FORM_RUN_ACCEPT) {
            n = std::stoi(values[0]);
            d = std::stoi(values[1]);
            return true;
        }

        if (result == GN_FORM_RUN_ERROR) {
            std::cerr << "Impossibile aprire il form SVD.\n";
        }

        return false;
    }

} // namespace f3_svd_form

namespace f3_svd_pca_form
{
    using fcn_form::write;
    using fcn_form::separator;
    using fcn_form::change_linear;

    std::string f3_svd_pca_factor_value(float value)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << value;
        std::string text = out.str();

        while (!text.empty() && text.back() == '0') text.pop_back();
        if (!text.empty() && text.back() == '.') text.pop_back();
        if (text.empty()) return "0";
        return text;
    }

    void f3_svd_pca_mask(const GnFormContext& context)
    {
        const GnFormLayout& layout = context.layout;
        const int width = std::max(1, layout.terminal_columns - 1);
        const int field_column = std::min(43, std::max(34, width - 10));
        const int first_field_row = layout.viewport.top + 4;

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        fcn_form::write(
            layout.header_top,
            1,
            " PCA - progressioni su campioni e dimensioni ",
            width,
            GN_SCREEN_TITLE);

        fcn_form::write(
            layout.header_top + 1,
            3,
            "Parametri base e moltiplicatori dell'esperimento PCA",
            width - 3);

        fcn_form::separator(layout.top_separator_row, layout.terminal_columns);

        int row = layout.viewport.top;
        fcn_form::write(row++, 3,
            "Genera cluster n x d e confronta la PCA variando separatamente",
            width - 3);
        fcn_form::write(row++, 3,
            "numero di campioni e numero di caratteristiche.",
            width - 3);
        fcn_form::write(row++, 3,
            "Un moltiplicatore 0 oppure 1 esclude la relativa progressione.",
            width - 3);

        fcn_form::write(first_field_row,     5, "  Campioni base n",        field_column - 6);
        fcn_form::write(first_field_row + 2, 5, "  Dimensione base d",      field_column - 6);
        fcn_form::write(first_field_row + 4, 5, "  Moltiplicatore n",       field_column - 6);
        fcn_form::write(first_field_row + 6, 5, "  Moltiplicatore d",       field_column - 6);

        fcn_form::separator(layout.bottom_separator_row, layout.terminal_columns);
        fcn_form::separator(layout.status_separator_row, layout.terminal_columns);
    }

    void f3_svd_pca_show(const GnFormContext& context)
    {
        static const char* labels[F3_SVD_PCA_FIELD_COUNT] = {
            "Campioni base n",
            "Dimensione base d",
            "Moltiplicatore n",
            "Moltiplicatore d"
        };

        for (std::size_t i = 0; i < context.field_count; ++i) {
            fcn_form::write(
                context.fields[i].row,
                5,
                std::string(i == context.lab ? "> " : "  ") + labels[i],
                context.fields[i].column - 6);

            fcn_form::write(
                context.fields[i].row,
                context.fields[i].column,
                fcn_form::field_value(context.values[i], context.fields[i].length),
                static_cast<int>(context.fields[i].length),
                GN_SCREEN_FIELD);
        }

        gn_screen_clear_line(context.layout.echo_row);
    }

    void f3_svd_pca_dispdid(const GnFormContext& context)
    {
        static const char* hints[F3_SVD_PCA_FIELD_COUNT] = {
            "n: numero base di campioni, naturale [3..500]",
            "d: numero base di caratteristiche, naturale [3..500]",
            "fattore n: decimale non negativo; 0 o 1 salta la progressione n",
            "fattore d: decimale non negativo; 0 o 1 salta la progressione d"
        };

        const int width = std::max(1, context.layout.terminal_columns - 1);
        fcn_form::write(
            context.layout.hint_row,
            2,
            hints[context.lab],
            width - 2,
            GN_SCREEN_MESSAGE);
    }

    void f3_svd_pca_keyvalid(const GnFormContext& context)
    {
        const int width = std::max(1, context.layout.terminal_columns - 1);

        fcn_form::write(
            context.layout.status_top,
            2,
            "ESC Menu        F1 Help        F10 Esegui",
            width - 2,
            GN_SCREEN_STATUS);
    }

    void f3_svd_pca_message(const GnFormContext& context)
    {
        if (context.message.empty()) return;

        const int width = std::max(1, context.layout.terminal_columns - 1);
        fcn_form::write(
            context.layout.echo_row,
            2,
            context.message,
            width - 2,
            GN_SCREEN_ERROR);
    }

    bool f3_svd_pca_prepare(GnFormContext& context, void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F3_SVD_PCA_FIELD_COUNT)
        {
            return false;
        }

        const int field_column =
            std::min(43, std::max(34, context.layout.terminal_columns - 11));
        const int first_field_row = context.layout.viewport.top + 4;

        context.fields[0] = GnEntrySpec{
            first_field_row,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            3,
            3.0,
            500.0,
            1,
            context.layout.echo_row
        };

        context.fields[1] = GnEntrySpec{
            first_field_row + 2,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            3,
            3.0,
            500.0,
            1,
            context.layout.echo_row
        };

        // Il limite superiore e' solo tecnico per il campo: la semantica
        // della progressione (0/1 = skip) resta nella business logic PCA.
        context.fields[2] = GnEntrySpec{
            first_field_row + 4,
            field_column,
            GN_FIELD_BOUNDED_DECIMAL,
            6,
            0.0,
            9999.0,
            1,
            context.layout.echo_row
        };

        context.fields[3] = GnEntrySpec{
            first_field_row + 6,
            field_column,
            GN_FIELD_BOUNDED_DECIMAL,
            6,
            0.0,
            9999.0,
            1,
            context.layout.echo_row
        };

        return true;
    }

    void f3_svd_pca_render(const GnFormContext& context, void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F3_SVD_PCA_FIELD_COUNT ||
            context.lab >= context.field_count)
        {
            return;
        }

        f3_svd_pca_mask(context);
        f3_svd_pca_show(context);
        f3_svd_pca_dispdid(context);
        f3_svd_pca_keyvalid(context);
        f3_svd_pca_message(context);

        gn_screen_flush();
    }

    GnFormChangeResult f3_svd_pca_change(
        GnEntryCommand command,
        GnFormContext& context,
        void*)
    {
        // Quattro campi lineari: per ora adotta esplicitamente la policy FCN.
        return fcn_form::change_linear(command, context);
    }

    void f3_svd_pca_help(const GnFormContext& context, void*)
    {
        const int width = std::max(1, context.layout.terminal_columns - 1);
        const std::vector<std::string> lines = {
            "F1 HELP - PCA / progressioni dimensionali",
            "",
            "Obiettivo",
            "  Confrontare la PCA mantenendo una dimensione fissa e facendo",
            "  progredire l'altra a partire dai valori base n e d.",
            "",
            "Parametri",
            "  n         numero base di campioni [3..500]",
            "  d         numero base di caratteristiche [3..500]",
            "  fattore n moltiplicatore della progressione dei campioni",
            "  fattore d moltiplicatore della progressione delle caratteristiche",
            "",
            "  Un fattore 0 oppure 1 indica che quella progressione va saltata.",
            "  I fattori possono essere frazionari (es. 1.05, 1.25).",
            "",
            "Uso del form",
            "  ENTER/TAB e i cursori previsti dalla rotta cambiano campo.",
            "  SPACE modifica il valore; digitazione diretta lo sostituisce.",
            "",
            "F10 conferma i parametri; ESC torna al menu.",
            "",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        int row = 1;
        for (const std::string& line : lines) {
            if (row > context.layout.terminal_rows) break;
            fcn_form::write(
                row,
                line.empty() ? 1 : 2,
                line,
                line.empty() ? width : width - 2,
                row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
            ++row;
        }
        gn_screen_flush();

        for (;;) {
            GnKeyEvent event{};
            const GnPollResult result = gn_pump_once(&event);

            if (result == GN_POLL_EMPTY) {
                gn_cooperative_pause();
                continue;
            }
            if (result == GN_POLL_ERROR) return;

            if (event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
            {
                return;
            }

            gn_screen_beep();
        }
    }
    bool prompt(F3SvdPcaParams& params)
    {
        std::string values[F3_SVD_PCA_FIELD_COUNT] = {
            std::to_string(params.n),
            std::to_string(params.d),
            f3_svd_pca_factor_value(params.factor_n),
            f3_svd_pca_factor_value(params.factor_d)
        };

        GnFormRunSpec spec;
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 13;
        spec.layout_options.min_viewport_columns = 72;

        spec.values = values;
        spec.field_count = F3_SVD_PCA_FIELD_COUNT;
        spec.prepare = f3_svd_pca_prepare;
        spec.render = f3_svd_pca_render;
        spec.change = f3_svd_pca_change;
        spec.help = f3_svd_pca_help;

        const GnFormRunResult result = gn_form_run_session(spec);

        if (result == GN_FORM_RUN_ACCEPT) {
            params.n = std::stoi(values[0]);
            params.d = std::stoi(values[1]);
            params.factor_n = values[2].empty() ? 0.0f : std::stof(values[2]);
            params.factor_d = values[3].empty() ? 0.0f : std::stof(values[3]);
            return true;
        }

        if (result == GN_FORM_RUN_ERROR) {
            std::cerr << "Impossibile aprire il form PCA.\n";
        }

        return false;
    }

} // namespace f3_svd_pca


