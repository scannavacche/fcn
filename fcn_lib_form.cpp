#include "fcn_lib_form.h"

#include "gn_form_runner.h"
#include "gn_idle.h"
#include "gn_terminal.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

constexpr std::size_t F1_SAMPLE_FIELD_COUNT = 2;  // 3. aggiungi il conteggio
constexpr std::size_t F2_DERIV_FIELD_COUNT = 2;
constexpr std::size_t F2_NORME_FIELD_COUNT = 2;
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

namespace f1_sample_form {

    // 4. prepara i campi
    bool prepare(GnFormContext& context, void*)
    {
        if (!context.fields || !context.values ||
            context.field_count != F1_SAMPLE_FIELD_COUNT)
            return false;

        const int col = std::min(
            45, std::max(35, context.layout.terminal_columns - 11));

        const int row = context.layout.viewport.top + 4;

        // Prima definiamo i campi
        context.fields[0] = GnEntrySpec{
            row, col,
            GN_FIELD_BOUNDED_NATURAL,
            3, 1.0, 255.0, 1,
            context.layout.echo_row
        };

        context.fields[1] = GnEntrySpec{
            row + 2, col,
            GN_FIELD_BOUNDED_NATURAL,
            3, 2.0, 500.0, 1,
            context.layout.echo_row
        };

        // Poi li allineiamo sul bordo destro comune
        const int max_length = std::max(
            static_cast<int>(context.fields[0].length),
            static_cast<int>(context.fields[1].length));

        const int right_col = col + max_length - 1;

        for (std::size_t i = 0; i < context.field_count; ++i)
            context.fields[i].column =
                right_col - static_cast<int>(context.fields[i].length) + 1;

        return true;
    }

    // 5a. Scrivi maschera
    
    
    void mask(const GnFormContext& context, void*)
    {
        const auto& layout = context.layout;
        const int width = std::max(1, layout.terminal_columns - 1);

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        // Banda superiore
        fcn_form::write(
            layout.header_top, 1,
            " Campionamento e matrici di Gram ",
            width, GN_SCREEN_TITLE);

        fcn_form::write(
            layout.header_top + 1, 3,
            "Confronto delle matrici di Gram con diversi campionamenti",
            width - 3);

        fcn_form::separator(
            layout.top_separator_row, layout.terminal_columns);

        // Descrizione all'interno della viewport
        fcn_form::write(
            layout.viewport.top, 3,
            "Impostare il numero di funzioni base K e di nodi N.",
            width - 3);

        fcn_form::separator(
            layout.bottom_separator_row, layout.terminal_columns);

        fcn_form::separator(
            layout.status_separator_row, layout.terminal_columns);
    }

    // 5b. e scrivi Help

    void help(const GnFormContext& context, void*)
    {
        const std::vector<std::string> lines = {
            "F1 HELP - Campionamento e matrici di Gram",
            "",
            "Costruzione e confronto di matrici di Gram",
            "ottenute con differenti campionamenti.",
            "",
            "Parametri:",
            "  K: numero di funzioni base [1..255]",
            "  N: numero di nodi [2..500]",
            "",
            "La procedura costruisce quattro matrici",
            "di Gram e ne visualizza i risultati.",
            "",
            "F10 esegue il calcolo.",
            "ESC ritorna al menu.",
            "",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        int row = 1;

        for (const auto& line : lines) {
            if (row >= context.layout.terminal_rows)
                break;

            fcn_form::write(
                row, line.empty() ? 1 : 2,
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

            if (result == GN_POLL_ERROR ||
                event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
                return;

            gn_screen_beep();
        }
    }


    // 6a. Disegna 
    void render(const GnFormContext& context, void* user_data)
    {
        if (!context.fields || !context.values ||
            context.field_count != F1_SAMPLE_FIELD_COUNT ||
            context.lab >= context.field_count)
            return;

        // Disegna la struttura statica
        mask(context, user_data);

        // Etichette e suggerimenti dei campi
        static const char* labels[] = {
            "Numero funzioni base K",
            "Numero nodi N"
        };

        static const char* hints[] = {
            "K: numero di funzioni base [1..255]",
            "N: numero di nodi di campionamento [2..500]"
        };

        // Disegna i campi nelle posizioni fissate da prepare()
        for (std::size_t i = 0; i < context.field_count; ++i) {
            const auto& field = context.fields[i];

            fcn_form::write(
                field.row, 5,
                std::string(i == context.lab ? "> " : "  ") + labels[i],
                field.column - 6);

            fcn_form::write(
                field.row, field.column,
                fcn_form::field_value(
                    context.values[i], field.length),
                static_cast<int>(field.length),
                GN_SCREEN_FIELD);
        }

        // Riga dei messaggi
        gn_screen_clear_line(context.layout.echo_row);

        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        // Suggerimento relativo al campo attivo
        fcn_form::write(
            context.layout.hint_row, 2,
            hints[context.lab],
            width - 2, GN_SCREEN_MESSAGE);

        // Barra dei comandi
        fcn_form::write(
            context.layout.status_top, 2,
            "ESC Menu        F1 Help        F10 Esegui",
            width - 2, GN_SCREEN_STATUS);

        // Eventuali messaggi di errore
        if (!context.message.empty()) {
            fcn_form::write(
                context.layout.echo_row, 2,
                context.message,
                width - 2, GN_SCREEN_ERROR);
        }

        gn_screen_flush();
    }

    // 6b. e naviga
    GnFormChangeResult change(
        GnEntryCommand command,
        GnFormContext& context,
        void*)
    {
        return fcn_form::change_linear(command, context);
    }

    // 7. Prompt 

    bool prompt(F1SampleParams& params)
    {
        // 1. Valori iniziali degli ENTRY
        std::string values[F1_SAMPLE_FIELD_COUNT] = {
            std::to_string(params.K),
            std::to_string(params.N)
        };

        // 2. Configurazione della sessione
        GnFormRunSpec spec;

        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 72;

        // 3. Dati e callback
        spec.values = values;
        spec.field_count = F1_SAMPLE_FIELD_COUNT;

        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;

        // 4. Esecuzione del form
        const GnFormRunResult result = gn_form_run_session(spec);

        // 5. Restituzione dei parametri confermati
        if (result == GN_FORM_RUN_ACCEPT) {
            params.K = std::stoi(values[0]);
            params.N = std::stoi(values[1]);
            return true;
        }

        if (result == GN_FORM_RUN_ERROR)
            std::cerr << "Impossibile aprire il form Gram.\n";

        return false;
    }
}

namespace f1_pair_form
{
    constexpr std::size_t FIELD_COUNT = 2;

    struct Field {
        const char* label;
        const char* hint;
        GnFieldType type;
        std::size_t length;
        double minimum;
        double maximum;
    };

    struct Config {
        const char* title;
        const char* subtitle;
        const char* description;
        const char* explanation;
        Field fields[FIELD_COUNT];
    };

    void mask(const GnFormContext& ctx, void* data)
    {
        const auto& cfg = *static_cast<Config*>(data);
        const auto& l = ctx.layout;
        const int width = std::max(1, l.terminal_columns - 1);

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        fcn_form::write(l.header_top, 1, cfg.title,
                        width, GN_SCREEN_TITLE);
        fcn_form::write(l.header_top + 1, 3, cfg.subtitle,
                        width - 3);
        fcn_form::separator(l.top_separator_row, l.terminal_columns);
        fcn_form::write(l.viewport.top, 3, cfg.description,
                        width - 3);
        fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
        fcn_form::separator(l.status_separator_row, l.terminal_columns);
    }

    bool prepare(GnFormContext& ctx, void* data)
    {
        if (!ctx.fields || !ctx.values ||
            ctx.field_count != FIELD_COUNT || !data)
            return false;

        const auto& cfg = *static_cast<Config*>(data);
        const int col = std::min(
            48, std::max(35, ctx.layout.terminal_columns - 12));
        const int row = ctx.layout.viewport.top + 4;

        for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
            const auto& f = cfg.fields[i];
            ctx.fields[i] = GnEntrySpec{
                row + 2 * static_cast<int>(i), col,
                f.type, f.length,
                f.minimum, f.maximum,
                1, ctx.layout.echo_row
            };
        }

        // Convenzione GRIMM: bordo destro comune.
        const auto max_length = std::max(
            ctx.fields[0].length, ctx.fields[1].length);
        const int right_col = col + static_cast<int>(max_length) - 1;

        for (auto i = std::size_t{0}; i < FIELD_COUNT; ++i)
            ctx.fields[i].column =
                right_col - static_cast<int>(ctx.fields[i].length) + 1;

        return true;
    }

    void render(const GnFormContext& ctx, void* data)
    {
        if (!data || !ctx.fields || !ctx.values ||
            ctx.field_count != FIELD_COUNT || ctx.lab >= FIELD_COUNT)
            return;

        const auto& cfg = *static_cast<Config*>(data);
        mask(ctx, data);

        for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
            const auto& f = ctx.fields[i];

            fcn_form::write(
                f.row, 5,
                std::string(i == ctx.lab ? "> " : "  ") +
                    cfg.fields[i].label,
                f.column - 6);

            fcn_form::write(
                f.row, f.column,
                fcn_form::field_value(ctx.values[i], f.length),
                static_cast<int>(f.length),
                GN_SCREEN_FIELD);
        }

        gn_screen_clear_line(ctx.layout.echo_row);
        const int width = std::max(1, ctx.layout.terminal_columns - 1);

        fcn_form::write(ctx.layout.hint_row, 2,
                        cfg.fields[ctx.lab].hint,
                        width - 2, GN_SCREEN_MESSAGE);
        fcn_form::write(ctx.layout.status_top, 2,
                        "ESC Menu     F1 Help     F10 Esegui",
                        width - 2, GN_SCREEN_STATUS);

        if (!ctx.message.empty())
            fcn_form::write(ctx.layout.echo_row, 2,
                            ctx.message, width - 2, GN_SCREEN_ERROR);

        gn_screen_flush();
    }

    GnFormChangeResult change(
        GnEntryCommand command, GnFormContext& ctx, void*)
    {
        return fcn_form::change_linear(command, ctx);
    }

    void help(const GnFormContext& ctx, void* data)
    {
        const auto& cfg = *static_cast<Config*>(data);

        const std::vector<std::string> lines = {
            std::string("F1 HELP - ") + cfg.title,
            "",
            cfg.explanation,
            "",
            std::string("1. ") + cfg.fields[0].hint,
            std::string("2. ") + cfg.fields[1].hint,
            "",
            "F10 esegue il calcolo.",
            "ESC ritorna al menu.",
            "",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };

        gn_screen_clear();
        gn_screen_cursor_visible(0);
        const int width = std::max(1, ctx.layout.terminal_columns - 1);

        int row = 1;
        for (const auto& line : lines) {
            if (row > ctx.layout.terminal_rows) break;
            fcn_form::write(
                row, 2, line, width - 2,
                row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
            ++row;
        }
        gn_screen_flush();

        for (;;) {
            GnKeyEvent event{};
            const auto result = gn_pump_once(&event);

            if (result == GN_POLL_EMPTY) {
                gn_cooperative_pause();
                continue;
            }
            if (result == GN_POLL_ERROR ||
                event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
                return;

            gn_screen_beep();
        }
    }

    bool validate(GnFormContext& ctx, void* data)
    {
        const auto& cfg = *static_cast<Config*>(data);

        for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
            const auto& f = cfg.fields[i];
            const auto& value = ctx.values[i];

            if (value.empty()) {
                ctx.lab = i;
                ctx.message = "Il campo non puo' essere vuoto";
                return false;
            }

            // GN_FIELD_INTEGER consente il segno, ma non ha bounds.
            if (f.type == GN_FIELD_INTEGER) {
                try {
                    std::size_t consumed = 0;
                    const int n = std::stoi(value, &consumed);
                    if (consumed != value.size() ||
                        n < f.minimum || n > f.maximum) {
                        ctx.lab = i;
                        ctx.message = "Argomento fuori intervallo";
                        return false;
                    }
                } catch (const std::exception&) {
                    ctx.lab = i;
                    ctx.message = "Intero non valido";
                    return false;
                }
            }
        }
        return true;
    }

    bool run(Config& cfg, std::string (&values)[FIELD_COUNT])
    {
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 72;

        spec.values = values;
        spec.field_count = FIELD_COUNT;
        spec.user_data = &cfg;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;

        const auto result = gn_form_run_session(spec);

        if (result == GN_FORM_RUN_ERROR)
            std::cerr << "Errore nella sessione FORM.\n";

        return result == GN_FORM_RUN_ACCEPT;
    }
}

namespace f1_taylor_exp_form
{
    bool prompt(F1TaylorParams& params)
    {
        static f1_pair_form::Config cfg{
            " Taylor - funzione esponenziale ",
            "Convergenza, accuratezza e confronto float/double",
            "Sviluppo di Taylor per exp(x)",
            "Confronto tra diversi centri dello sviluppo.",
            {
                {"Numero termini", "Termini: da 1 a 40",
                 GN_FIELD_BOUNDED_NATURAL, 2, 1, 40},
                {"Argomento x", "x: intero da -99 a 99",
                 GN_FIELD_INTEGER, 4, -99, 99}
            }
        };

        std::string values[2] = {
            std::to_string(params.n_terms),
            std::to_string(params.target_arg)
        };

        if (!f1_pair_form::run(cfg, values)) return false;
        params.n_terms = std::stoi(values[0]);
        params.target_arg = std::stoi(values[1]);
        return true;
    }
}

namespace f1_taylor_cos_form
{
    bool prompt(F1TaylorParams& params)
    {
        static f1_pair_form::Config cfg{
            " Taylor - funzione coseno ",
            "Errore dello sviluppo di Maclaurin",
            "Argomento espresso in gradi",
            "Confronto con cos(x) e riferimento a 30 gradi.",
            {
                {"Numero termini", "Termini: da 1 a 40",
                 GN_FIELD_BOUNDED_NATURAL, 2, 1, 40},
                {"Angolo (gradi)", "Angolo: da -360 a 360 gradi",
                 GN_FIELD_INTEGER, 4, -360, 360}
            }
        };

        std::string values[2] = {
            std::to_string(params.n_terms),
            std::to_string(params.target_arg)
        };

        if (!f1_pair_form::run(cfg, values)) return false;
        params.n_terms = std::stoi(values[0]);
        params.target_arg = std::stoi(values[1]);
        return true;
    }
}

namespace f1_prec_form
{
    bool prompt(F1PrecParams& params)
    {
        static f1_pair_form::Config cfg{
            " Precisione di macchina - IEEE 754 ",
            "Epsilon, arrotondamenti e somme ripetute",
            "Parametri per l'esperimento della somma",
            "Le prove sull'epsilon rimangono inalterate.",
            {
                {"Incremento", "Incremento: 0.001 .. 1",
                 GN_FIELD_BOUNDED_DECIMAL, 8, 0.001, 1.0},
                {"Ripetizioni", "Numero di somme: 1 .. 999",
                 GN_FIELD_BOUNDED_NATURAL, 3, 1, 999}
            }
        };

        std::ostringstream out;
        out << params.increment;

        std::string values[2] = {
            out.str(),
            std::to_string(params.repetitions)
        };

        if (!f1_pair_form::run(cfg, values)) return false;
        params.increment = std::stod(values[0]);
        params.repetitions = std::stoi(values[1]);
        return true;
    }
}

namespace f2_deriv_form
{
    void mask(const GnFormContext& context, void*)
    {
        const auto& layout = context.layout;
        const int width = std::max(1, layout.terminal_columns - 1);

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        fcn_form::write(
            layout.header_top, 1,
            " Derivata discreta - differenze finite ",
            width, GN_SCREEN_TITLE);

        fcn_form::write(
            layout.header_top + 1, 3,
            "Confronto fra derivazione matriciale, iterativa e analitica",
            width - 3);

        fcn_form::separator(
            layout.top_separator_row, layout.terminal_columns);

        fcn_form::write(
            layout.viewport.top, 3,
            "Funzione: arctan(t). Intervallo simmetrico [-s, s].",
            width - 3);

        fcn_form::separator(
            layout.bottom_separator_row, layout.terminal_columns);

        fcn_form::separator(
            layout.status_separator_row, layout.terminal_columns);
    }

    bool prepare(GnFormContext& context, void*)
    {
        if (!context.fields || !context.values ||
            context.field_count != F2_DERIV_FIELD_COUNT)
            return false;

        const int col = std::min(
            45, std::max(35, context.layout.terminal_columns - 11));
        
        const int right_col = col + 6 - 1;
        
        const int row = context.layout.viewport.top + 4;

        context.fields[0] = GnEntrySpec{
            row, right_col - 3 + 1, //campo n lunghezza 3
            GN_FIELD_BOUNDED_NATURAL,
            3, 2.0, 500.0, 1,
            context.layout.echo_row
        };

        context.fields[1] = GnEntrySpec{
            row + 2, right_col - 6 +1, //campo span lunghezza 6
            GN_FIELD_BOUNDED_DECIMAL,
            6, 1.0, 10.0, 1,
            context.layout.echo_row
        };

        return true;
    }

    void render(const GnFormContext& context, void* user_data)
    {
        if (!context.fields || !context.values ||
            context.field_count != F2_DERIV_FIELD_COUNT ||
            context.lab >= context.field_count)
            return;

        mask(context, user_data);

        static const char* labels[] = {
            "Numero di nodi n",
            "Semiampiezza s"
        };

        static const char* hints[] = {
            "n: numero di punti di campionamento [2..500]",
            "s: intervallo di campionamento [-s,s], s in [1..10]"
        };

        for (std::size_t i = 0; i < context.field_count; ++i) {
            const auto& field = context.fields[i];

            fcn_form::write(
                field.row, 5,
                std::string(i == context.lab ? "> " : "  ") + labels[i],
                field.column - 6);

            fcn_form::write(
                field.row, field.column,
                fcn_form::field_value(
                    context.values[i], field.length),
                static_cast<int>(field.length),
                GN_SCREEN_FIELD);
        }

        gn_screen_clear_line(context.layout.echo_row);

        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        fcn_form::write(
            context.layout.hint_row, 2,
            hints[context.lab],
            width - 2, GN_SCREEN_MESSAGE);

        fcn_form::write(
            context.layout.status_top, 2,
            "ESC Menu        F1 Help        F10 Esegui",
            width - 2, GN_SCREEN_STATUS);

        if (!context.message.empty()) {
            fcn_form::write(
                context.layout.echo_row, 2,
                context.message,
                width - 2, GN_SCREEN_ERROR);
        }

        gn_screen_flush();
    }

    GnFormChangeResult change(
        GnEntryCommand command,
        GnFormContext& context,
        void*)
    {
        return fcn_form::change_linear(command, context);
    }

    void help(const GnFormContext& context, void*)
    {
        const std::vector<std::string> lines = {
            "F1 HELP - Derivazione numerica",
            "",
            "Si campiona u(t) = arctan(t) su n nodi equidistanti.",
            "",
            "Parametri:",
            "  n: numero di nodi [2..500]",
            "  s: semiampiezza del dominio [1..10]",
            "",
            "Intervallo: [-s, s]",
            "Passo: h = 2s / (n-1)",
            "",
            "Vengono confrontate:",
            "  - derivazione mediante matrice D",
            "  - derivazione mediante iterazione",
            "  - derivata analitica 1/(1+t^2)",
            "",
            "F10 esegue il calcolo.",
            "ESC ritorna al menu.",
            "",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        int row = 1;
        for (const auto& line : lines) {
            if (row > context.layout.terminal_rows)
                break;

            fcn_form::write(
                row, line.empty() ? 1 : 2,
                line, line.empty() ? width : width - 2,
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

            if (result == GN_POLL_ERROR ||
                event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
                return;

            gn_screen_beep();
        }
    }

    bool prompt(F2DerivParams& params)
    {
        std::ostringstream span;
        span << params.domain_span;

        std::string values[F2_DERIV_FIELD_COUNT] = {
            std::to_string(params.n),
            span.str()
        };

        GnFormRunSpec spec;

        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 72;

        spec.values = values;
        spec.field_count = F2_DERIV_FIELD_COUNT;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;

        const GnFormRunResult result = gn_form_run_session(spec);

        if (result == GN_FORM_RUN_ACCEPT) {
            params.n = std::stoi(values[0]);
            params.domain_span = std::stod(values[1]);
            return true;
        }

        if (result == GN_FORM_RUN_ERROR)
            std::cerr << "Impossibile aprire il form Derivate.\n";

        return false;
    }

} // f2_deriv_form

namespace f2_multi_form {

    constexpr std::size_t MAX_FIELDS = 5;

    struct Field {
        const char* label;
        const char* hint;
        GnFieldType type;
        std::size_t length;
        double lower;
        double upper;
    };

    struct Config {
        const char* title;
        const char* subtitle;
        const char* description;
        const char* explanation;
        std::size_t count;
        Field fields[MAX_FIELDS];
        std::size_t interval_start;  // indice campo t0/a
        std::size_t interval_end;    // indice campo T/b
        bool sigma_bounded_by_n = false; // solo deconvoluzione
    };

    std::string decimal(double value) {
        std::ostringstream out;
        out << std::setprecision(10) << value;
        return out.str();
    }

    void mask(const GnFormContext& ctx, void* data) {
        const auto& cfg = *static_cast<const Config*>(data);
        const auto& l = ctx.layout;
        const int width = std::max(1, l.terminal_columns - 1);
        gn_screen_clear();
        gn_screen_cursor_visible(0);
        fcn_form::write(l.header_top, 1, cfg.title,
                        width, GN_SCREEN_TITLE);
        fcn_form::write(l.header_top + 1, 3, cfg.subtitle,
                        width - 3);
        fcn_form::separator(l.top_separator_row, l.terminal_columns);
        fcn_form::write(l.viewport.top, 3, cfg.description,
                        width - 3);
        fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
        fcn_form::separator(l.status_separator_row, l.terminal_columns);
    }

    bool prepare(GnFormContext& ctx, void* data) {
        if (!data || !ctx.fields || !ctx.values) return false;
        const auto& cfg = *static_cast<const Config*>(data);
        if (ctx.field_count != cfg.count || cfg.count > MAX_FIELDS ||
            cfg.count < 1) return false;

        const int first_col = std::min(
            59, std::max(45, ctx.layout.terminal_columns - 23));
        const int first_row = ctx.layout.viewport.top + 3;
        std::size_t max_length = 0;
        for (std::size_t i = 0; i < cfg.count; ++i) {
            const auto& f = cfg.fields[i];
            ctx.fields[i] = GnEntrySpec{
                first_row + static_cast<int>(2 * i),
                first_col, f.type, f.length, f.lower, f.upper,
                1, ctx.layout.echo_row
            };
            max_length = std::max(max_length, f.length);
        }
        const int right_col = first_col + static_cast<int>(max_length) - 1;
        for (std::size_t i = 0; i < cfg.count; ++i) {
            ctx.fields[i].column = right_col -
                static_cast<int>(ctx.fields[i].length) + 1;
        }
        return true;
    }

    void render(const GnFormContext& ctx, void* data) {
        if (!data || !ctx.fields || !ctx.values ||
            ctx.lab >= ctx.field_count) return;
        const auto& cfg = *static_cast<const Config*>(data);
        if (ctx.field_count != cfg.count) return;
        mask(ctx, data);
        for (std::size_t i = 0; i < cfg.count; ++i) {
            const auto& f = ctx.fields[i];
            fcn_form::write(f.row, 5,
                std::string(i == ctx.lab ? "> " : "  ") +
                    cfg.fields[i].label,
                f.column - 6);
            fcn_form::write(f.row, f.column,
                fcn_form::field_value(ctx.values[i], f.length),
                static_cast<int>(f.length), GN_SCREEN_FIELD);
        }
        gn_screen_clear_line(ctx.layout.echo_row);
        const int width = std::max(1, ctx.layout.terminal_columns - 1);
        fcn_form::write(ctx.layout.hint_row, 2,
            cfg.fields[ctx.lab].hint, width - 2, GN_SCREEN_MESSAGE);
        fcn_form::write(ctx.layout.status_top, 2,
            "ESC Menu     F1 Help     F10 Esegui",
            width - 2, GN_SCREEN_STATUS);
        if (!ctx.message.empty()) {
            fcn_form::write(ctx.layout.echo_row, 2,
                ctx.message, width - 2, GN_SCREEN_ERROR);
        }
        gn_screen_flush();
    }

    GnFormChangeResult change(GnEntryCommand command,
                              GnFormContext& ctx, void*) {
        return fcn_form::change_linear(command, ctx);
    }

    void help(const GnFormContext& ctx, void* data) {
        const auto& cfg = *static_cast<const Config*>(data);
        const int width = std::max(1, ctx.layout.terminal_columns - 1);
        std::vector<std::string> lines = {
            std::string("F1 HELP - ") + cfg.title,
            "", cfg.explanation, "", "Parametri:"
        };
        for (std::size_t i = 0; i < cfg.count; ++i) {
            lines.push_back(
                std::string("  ") + cfg.fields[i].hint);
        }
        lines.push_back("");
        lines.push_back("F10 esegue. ESC torna al menu.");
        lines.push_back("F1 / ENTER / ESC: ritorno al form");
        gn_screen_clear();
        gn_screen_cursor_visible(0);
        int row = 1;
        for (const auto& line : lines) {
            if (row > ctx.layout.terminal_rows) break;
            fcn_form::write(row, 2, line, width - 2,
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
            if (result == GN_POLL_ERROR ||
                event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE) return;
            gn_screen_beep();
        }
    }

    bool validate(GnFormContext& ctx, void* data) {
        const auto& cfg = *static_cast<const Config*>(data);
        for (std::size_t i = 0; i < cfg.count; ++i) {
            if (ctx.values[i].empty()) {
                ctx.lab = i;
                ctx.message = "Il campo non puo' essere vuoto";
                return false;
            }
        }
        // Dopo la validazione individuale GNPORT, controlla la relazione.
        const double left = std::stod(ctx.values[cfg.interval_start]);
        const double right = std::stod(ctx.values[cfg.interval_end]);
        if (!(right > left)) {
            ctx.lab = cfg.interval_end;
            ctx.message = "Estremo finale deve essere > iniziale";
            return false;
        }
        if (cfg.sigma_bounded_by_n) {
            const int n = std::stoi(ctx.values[2]);
            const double sigma = std::stod(ctx.values[3]);
            if (sigma > n - 1) {
                ctx.lab = 3;
                ctx.message = "sigma deve essere <= N-1";
                return false;
            }
        }
        return true;
    }

    bool run(Config& cfg, std::string* values) {
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 14;
        spec.layout_options.min_viewport_columns = 80;
        spec.values = values;
        spec.field_count = cfg.count;
        spec.user_data = &cfg;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;
        const auto result = gn_form_run_session(spec);
        if (result == GN_FORM_RUN_ERROR)
            std::cerr << "Errore nella sessione form Foglio 2.\n";
        return result == GN_FORM_RUN_ACCEPT;
    }
}

namespace f2_cauchy_form {
    bool prompt(F2CauchyParams& p) {
        static f2_multi_form::Config cfg{
            " Cauchy - problema ai valori iniziali ",
            "Risoluzione discreta per tre termini forzanti",
            "Griglia temporale [t0,T], condizione x(t0) = x0",
            "Prove per f(t)=t, sin(t), cos(t); schema in avanti.",
            4,
            {
                {"Numero nodi n", "n: 2 .. 400", GN_FIELD_BOUNDED_NATURAL, 3, 2, 400},
                {"Tempo iniziale t0", "t0: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Tempo finale T", "T: -100 .. 100, T > t0", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Condizione iniziale x0", "x0: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100}
            },
            1, 2
        };
        std::string v[] = {std::to_string(p.n),
            f2_multi_form::decimal(p.t0), f2_multi_form::decimal(p.T),
            f2_multi_form::decimal(p.x0)};
        if (!f2_multi_form::run(cfg, v)) return false;
        p.n = std::stoi(v[0]); p.t0 = std::stod(v[1]);
        p.T = std::stod(v[2]); p.x0 = std::stod(v[3]);
        return true;
    }
}

namespace f2_bordo_form {
    bool prompt(F2BordoParams& p) {
        static f2_multi_form::Config cfg{
            " Problema differenziale ai limiti ",
            "Risoluzione per fattorizzazione LU",
            "Griglia [t0,T] con valori imposti x(t0), x(T)",
            "BVP con 2 condizioni ai limiti; forzante cos(t).",
            5,
            {
                {"Numero nodi n", "n: 3 .. 300", GN_FIELD_BOUNDED_NATURAL, 3, 3, 300},
                {"Tempo iniziale t0", "t0: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Tempo finale T", "T: -100 .. 100, T > t0", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Valore al bordo x0", "x0: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Valore al bordo x1", "x1: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100}
            },
            1, 2
        };
        std::string v[] = {std::to_string(p.n), f2_multi_form::decimal(p.t0),
            f2_multi_form::decimal(p.T), f2_multi_form::decimal(p.x0),
            f2_multi_form::decimal(p.x1)};
        if (!f2_multi_form::run(cfg, v)) return false;
        p.n = std::stoi(v[0]); p.t0 = std::stod(v[1]);
        p.T = std::stod(v[2]); p.x0 = std::stod(v[3]);
        p.x1 = std::stod(v[4]);
        return true;
    }
}

namespace f2_integral_form {
    bool prompt(F2IntegralParams& p) {
        static f2_multi_form::Config cfg{
            " Integrazione numerica discreta ",
            "Integrazione, derivazione, norme e condizionamento",
            "Intervallo di integrazione [a,b] e N nodi",
            "Integranda 1/(1+x^2), confronto con arctan(x).",
            3,
            {
                {"Estremo a", "a: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Estremo b", "b: -100 .. 100, b > a", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Numero nodi N", "N: 3 .. 200", GN_FIELD_BOUNDED_NATURAL, 3, 3, 200}
            },
            0, 1
        };
        std::string v[] = {f2_multi_form::decimal(p.a),
            f2_multi_form::decimal(p.b), std::to_string(p.N)};
        if (!f2_multi_form::run(cfg, v)) return false;
        p.a = std::stod(v[0]); p.b = std::stod(v[1]);
        p.N = std::stoi(v[2]);
        return true;
    }
}

namespace f2_deconv_form {
    bool prompt(F2DeconvParams& p) {
        static f2_multi_form::Config cfg{
            " Deconvoluzione - kernel di Gauss ",
            "Confronto di convoluzioni e condizionamento",
            "Intervallo, griglia, ampiezza sigma*h e normalizzazione",
            "Sigma moltiplica il passo h; 1 <= sigma <= N-1; normalizzazione 0/1.",
            5,
            {
                {"Estremo a", "a: -100 .. 100", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Estremo b", "b: -100 .. 100, b > a", GN_FIELD_BOUNDED_DECIMAL, 12, -100, 100},
                {"Numero nodi N", "N: 10 .. 120 (LU e matrici dense)", GN_FIELD_BOUNDED_NATURAL, 3, 10, 120},
                {"Sigma (in passi h)", "sigma: 1 .. N-1", GN_FIELD_BOUNDED_DECIMAL, 8, 1, 119},
                {"Kernel normalizzato", "KG_Normal: 1=si, 0=no", GN_FIELD_BOUNDED_NATURAL, 1, 0, 1}
            },
            0, 1, true
        };
        std::string v[] = {f2_multi_form::decimal(p.a),
            f2_multi_form::decimal(p.b), std::to_string(p.N),
            f2_multi_form::decimal(p.sigma), p.KG_Normal ? "1" : "0"};
        if (!f2_multi_form::run(cfg, v)) return false;
        p.a = std::stod(v[0]); p.b = std::stod(v[1]);
        p.N = std::stoi(v[2]); p.sigma = std::stod(v[3]);
        p.KG_Normal = std::stoi(v[4]) != 0;
        return true;
    }
}

namespace f2_norme_form
{
    using fcn_form::write;
    using fcn_form::separator;
    using fcn_form::field_value;
    using fcn_form::change_linear;


    void f2_norme_mask(const GnFormContext& context)
    {
        const GnFormLayout& layout = context.layout;
        const int width = std::max(1, layout.terminal_columns - 1);
        const int field_column =
            std::min(45, std::max(35, width - 10));
        const int first_field_row = layout.viewport.top + 5;

        gn_screen_clear();
        gn_screen_cursor_visible(0);

        write(
            layout.header_top,
            1,
            " Norme matriciali - confronto numerico ",
            width,
            GN_SCREEN_TITLE);

        write(
            layout.header_top + 1,
            3,
            "Confronto fra norme e numeri di condizionamento",
            width - 3);

        separator(
            layout.top_separator_row,
            layout.terminal_columns);

        int row = layout.viewport.top;

        write(
            row++,
            3,
            "Confronta diverse norme della matrice triangolare di test",
            width - 3);

        write(
            row++,
            3,
            "al crescere della dimensione N.",
            width - 3);

        write(
            row++,
            3,
            "Le dimensioni usate sono 10, 20, 40, ... fino a N max.",
            width - 3);

        write(
            first_field_row,
            5,
            "  Dimensione massima N",
            field_column - 6);

        write(
            first_field_row + 2,
            5,
            "  Ripetizioni timing",
            field_column - 6);

        separator(
            layout.bottom_separator_row,
            layout.terminal_columns);

        separator(
            layout.status_separator_row,
            layout.terminal_columns);
    }


    void f2_norme_show(const GnFormContext& context)
    {
        static const char* labels[F2_NORME_FIELD_COUNT] = {
            "Dimensione massima N",
            "Ripetizioni timing"
        };

        for (std::size_t i = 0; i < context.field_count; ++i) {

            write(
                context.fields[i].row,
                5,
                std::string(i == context.lab ? "> " : "  ") + labels[i],
                context.fields[i].column - 6);

            write(
                context.fields[i].row,
                context.fields[i].column,
                field_value(
                    context.values[i],
                    context.fields[i].length),
                static_cast<int>(context.fields[i].length),
                GN_SCREEN_FIELD);
        }

        gn_screen_clear_line(context.layout.echo_row);
    }


    void f2_norme_dispdid(const GnFormContext& context)
    {
        static const char* hints[F2_NORME_FIELD_COUNT] = {
            "N max: ultima dimensione della progressione 10,20,40,... [10..640]",
            "timing: numero di misure da mediare per ciascun metodo [1..20]"
        };

        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        write(
            context.layout.hint_row,
            2,
            hints[context.lab],
            width - 2,
            GN_SCREEN_MESSAGE);
    }


    void f2_norme_keyvalid(const GnFormContext& context)
    {
        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        write(
            context.layout.status_top,
            2,
            "ESC Menu        F1 Help        F10 Esegui",
            width - 2,
            GN_SCREEN_STATUS);
    }


    void f2_norme_message(const GnFormContext& context)
    {
        if (context.message.empty()) return;

        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        write(
            context.layout.echo_row,
            2,
            context.message,
            width - 2,
            GN_SCREEN_ERROR);
    }


    bool f2_norme_prepare(
        GnFormContext& context,
        void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F2_NORME_FIELD_COUNT)
        {
            return false;
        }

        const int field_column =
            std::min(
                45,
                std::max(
                    35,
                    context.layout.terminal_columns - 11));

        const int first_field_row =
            context.layout.viewport.top + 5;


        context.fields[0] = GnEntrySpec{
            first_field_row,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            3,
            10.0,
            640.0,
            1,
            context.layout.echo_row
        };


        context.fields[1] = GnEntrySpec{
            first_field_row + 2,
            field_column,
            GN_FIELD_BOUNDED_NATURAL,
            2,
            1.0,
            20.0,
            1,
            context.layout.echo_row
        };


        return true;
    }


    void f2_norme_render(
        const GnFormContext& context,
        void*)
    {
        if (context.fields == nullptr ||
            context.values == nullptr ||
            context.field_count != F2_NORME_FIELD_COUNT ||
            context.lab >= context.field_count)
        {
            return;
        }

        f2_norme_mask(context);
        f2_norme_show(context);
        f2_norme_dispdid(context);
        f2_norme_keyvalid(context);
        f2_norme_message(context);

        gn_screen_flush();
    }


    GnFormChangeResult f2_norme_change(
        GnEntryCommand command,
        GnFormContext& context,
        void*)
    {
        // Due campi lineari: adotta la policy condivisa FCN.
        return change_linear(command, context);
    }


    void f2_norme_help(
        const GnFormContext& context,
        void*)
    {
        const int width =
            std::max(1, context.layout.terminal_columns - 1);

        const std::vector<std::string> lines = {

            "F1 HELP - Norme matriciali e condizionamento",
            "",
            "Norme disponibili in matrix_calcola_norma:",
            "  1   norma 1 indotta: massima somma assoluta per colonna",
            "  0   norma infinito indotta: massima somma assoluta per riga",
            " -1   norma di Frobenius: radice della somma dei quadrati",
            "",
            "Norma 2 spettrale, quattro implementazioni sperimentate:",
            "  2 (1R)   A^T A implicita; Rayleigh calcolato solo alla fine",
            " 12 (AllR) A^T A implicita; Rayleigh ad ogni iterazione",
            " 22 (AtA)  costruisce esplicitamente A^T A, poi usa le potenze",
            "  3        tentativo storico mediante fattorizzazione LU di A^T A",
            "",
            "Costo computazionale:",
            "  1R evita il ricalcolo di A^T(Aq) per il quoziente di Rayleigh;",
            "  AllR paga quindi due prodotti matrice-vettore aggiuntivi per iterazione.",
            "  AtA evita questi prodotti iterativi ma costruisce A^T A: costo O(n^3).",
            "",
            "I codici 2, 12, 22 e 3 cercano tutti la stessa norma spettrale.",
            "La variante LU e' una bozza sperimentale, non un metodo affidabile.",
            "",
            "Parametri:",
            "  N max   progressione 10, 20, 40, ... fino al limite indicato",
            "  timing  ripetizioni usate per stabilizzare le misure dei tempi",
            "",
            "F10 esegue il confronto; ESC torna al menu.",
            "F1, ENTER oppure ESC: ritorno ai parametri"
        };


        gn_screen_clear();
        gn_screen_cursor_visible(0);

        int row = 1;

        for (const std::string& line : lines) {

            if (row > context.layout.terminal_rows)
                break;

            write(
                row,
                line.empty() ? 1 : 2,
                line,
                line.empty() ? width : width - 2,
                row == 1
                    ? GN_SCREEN_TITLE
                    : GN_SCREEN_NORMAL);

            ++row;
        }

        gn_screen_flush();


        for (;;) {

            GnKeyEvent event{};

            const GnPollResult result =
                gn_pump_once(&event);

            if (result == GN_POLL_EMPTY) {
                gn_cooperative_pause();
                continue;
            }

            if (result == GN_POLL_ERROR)
                return;

            if (event.code == GN_KEY_F1 ||
                event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE)
            {
                return;
            }

            gn_screen_beep();
        }
    }


    bool prompt(F2NormeParams& params)
    {
        std::string values[F2_NORME_FIELD_COUNT] = {
            std::to_string(params.n_max),
            std::to_string(params.timing_reps)
        };


        GnFormRunSpec spec;

        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;

        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 72;

        spec.values = values;
        spec.field_count = F2_NORME_FIELD_COUNT;

        spec.prepare = f2_norme_prepare;
        spec.render = f2_norme_render;
        spec.change = f2_norme_change;
        spec.help = f2_norme_help;


        const GnFormRunResult result =
            gn_form_run_session(spec);


        if (result == GN_FORM_RUN_ACCEPT) {

            // Tipo e bounds sono gia' validati da ENTRY.
            params.n_max =
                std::stoi(values[0]);

            params.timing_reps =
                std::stoi(values[1]);

            return true;
        }


        if (result == GN_FORM_RUN_ERROR) {
            std::cerr
                << "Impossibile aprire il form Norme.\n";
        }


        return false;
    }

} // namespace f2_norme_form

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

namespace f3_svd_an_form {
    namespace {
        constexpr std::size_t FIELD_COUNT = 6;
        constexpr const char* labels[FIELD_COUNT] = {
            "Numero campioni n", "Dimensione d[0]", "Dimensione d[1]",
            "Dimensione d[2]", "Dimensione d[3]", "Seed generatore"
        };
        constexpr const char* hints[FIELD_COUNT] = {
            "n: campioni per ciascuno dei quattro esperimenti [3..500]",
            "d[0]: caratteristiche per il primo esperimento [2..n]",
            "d[1]: caratteristiche per il secondo esperimento [2..n]",
            "d[2]: caratteristiche per il terzo esperimento [2..n]",
            "d[3]: caratteristiche per il quarto esperimento [2..n]",
            "Seed: inizializzazione riproducibile dei cluster [0..999999]"
        };

        void mask(const GnFormContext& ctx, void*) {
            const auto& l = ctx.layout;
            const int width = std::max(1, l.terminal_columns - 1);
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            fcn_form::write(l.header_top, 1,
                " PCA - analisi su quattro dimensioni ", width, GN_SCREEN_TITLE);
            fcn_form::write(l.header_top + 1, 3,
                "Confronto di quattro dataset con n fisso e d variabile",
                width - 3);
            fcn_form::separator(l.top_separator_row, l.terminal_columns);
            fcn_form::write(l.viewport.top, 3,
                "Ogni pannello usa n campioni e una diversa dimensione d.",
                width - 3);
            fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
            fcn_form::separator(l.status_separator_row, l.terminal_columns);
        }

        bool prepare(GnFormContext& ctx, void*) {
            if (!ctx.fields || !ctx.values || ctx.field_count != FIELD_COUNT)
                return false;
            const int col = std::min(54,
                std::max(43, ctx.layout.terminal_columns - 18));
            const int row = ctx.layout.viewport.top + 3;
            for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                const bool seed = (i == FIELD_COUNT - 1);
                ctx.fields[i] = GnEntrySpec{
                    row + 2 * static_cast<int>(i), col,
                    GN_FIELD_BOUNDED_NATURAL,
                    seed ? std::size_t{6} : std::size_t{3},
                    seed ? 0.0 : (i == 0 ? 3.0 : 2.0),
                    seed ? 999999.0 : 500.0,
                    1, ctx.layout.echo_row
                };
            }
            std::size_t max_len = 0;
            for (std::size_t i = 0; i < FIELD_COUNT; ++i)
                max_len = std::max(max_len, ctx.fields[i].length);
            const int right_col = col + static_cast<int>(max_len) - 1;
            for (std::size_t i = 0; i < FIELD_COUNT; ++i)
                ctx.fields[i].column = right_col -
                    static_cast<int>(ctx.fields[i].length) + 1;
            return true;
        }

        void render(const GnFormContext& ctx, void* data) {
            if (!ctx.fields || !ctx.values ||
                ctx.field_count != FIELD_COUNT || ctx.lab >= FIELD_COUNT)
                return;
            mask(ctx, data);
            for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                const auto& f = ctx.fields[i];
                fcn_form::write(f.row, 5,
                    std::string(i == ctx.lab ? "> " : "  ") + labels[i],
                    f.column - 6);
                fcn_form::write(f.row, f.column,
                    fcn_form::field_value(ctx.values[i], f.length),
                    static_cast<int>(f.length), GN_SCREEN_FIELD);
            }
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            gn_screen_clear_line(ctx.layout.echo_row);
            fcn_form::write(ctx.layout.hint_row, 2,
                hints[ctx.lab], width - 2, GN_SCREEN_MESSAGE);
            fcn_form::write(ctx.layout.status_top, 2,
                "ESC Menu     F1 Help     F10 Esegui",
                width - 2, GN_SCREEN_STATUS);
            if (!ctx.message.empty())
                fcn_form::write(ctx.layout.echo_row, 2,
                    ctx.message, width - 2, GN_SCREEN_ERROR);
            gn_screen_flush();
        }

        GnFormChangeResult change(GnEntryCommand command,
                                GnFormContext& ctx, void*) {
            return fcn_form::change_linear(command, ctx);
        }

        void help(const GnFormContext& ctx, void*) {
            std::vector<std::string> lines = {
                "F1 HELP - PCA: analisi per dimensioni",
                "",
                "Quattro esperimenti PCA/SVD con numero campioni n fisso.",
                "Ogni pannello usa la dimensione d[i] corrispondente.",
                "Le dimensioni devono essere almeno 2 e non superiori a n.",
                "Il seed controlla la riproducibilita' dei dati generati.",
                "",
                "F10 esegue; ESC torna al menu.",
                "F1 / ENTER / ESC: ritorno ai parametri."
            };
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            int row = 1;
            for (const auto& line : lines) {
                if (row > ctx.layout.terminal_rows) break;
                fcn_form::write(row, 2, line, width - 2,
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
                if (result == GN_POLL_ERROR || event.code == GN_KEY_F1 ||
                    event.code == GN_KEY_ENTER || event.code == GN_KEY_ESCAPE)
                    return;
                gn_screen_beep();
            }
        }

        bool validate(GnFormContext& ctx, void*) {
            if (!ctx.values || ctx.field_count != FIELD_COUNT) return false;
            try {
                for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                    if (ctx.values[i].empty()) {
                        ctx.lab = i;
                        ctx.message = "Campo obbligatorio";
                        return false;
                    }
                }
                const int n = std::stoi(ctx.values[0]);
                for (std::size_t i = 1; i <= 4; ++i) {
                    const int d = std::stoi(ctx.values[i]);
                    if (d > n) {
                        ctx.lab = i;
                        ctx.message = "La dimensione d deve essere <= n";
                        return false;
                    }
                }
            } catch (const std::exception&) {
                ctx.message = "Valore numerico non valido";
                return false;
            }
            return true;
        }
    }

    bool prompt(F3SvdAnParams& params) {
        std::string values[FIELD_COUNT] = {
            std::to_string(params.n), std::to_string(params.d[0]),
            std::to_string(params.d[1]), std::to_string(params.d[2]),
            std::to_string(params.d[3]), std::to_string(params.seed)
        };
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 16;
        spec.layout_options.min_viewport_columns = 80;
        spec.values = values;
        spec.field_count = FIELD_COUNT;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;

        const auto result = gn_form_run_session(spec);
        if (result != GN_FORM_RUN_ACCEPT) {
            if (result == GN_FORM_RUN_ERROR)
                std::cerr << "Impossibile aprire il form PCA analisi.\n";
            return false;
        }
        params.n = std::stoi(values[0]);
        for (std::size_t i = 0; i < 4; ++i)
            params.d[i] = std::stoi(values[i + 1]);
        params.seed = std::stoi(values[5]);
        return true;
    }
} // namespace f3_svd_an_form

namespace sym_cauchy_form {
    // Simulazione d'esame: form a sei campi per sym_cauchy().
    // Si appoggia ai servizi fcn_form gia' definiti in questo file.

    namespace {
        constexpr std::size_t FIELD_COUNT = 6;
        struct FieldInfo {
            const char* label;
            const char* hint;
            GnFieldType type;
            std::size_t length;
            double minimum;
            double maximum;
        };
        constexpr FieldInfo fields[FIELD_COUNT] = {
            {"Numero nodi n", "n: nodi [2..2000], intervalli = n-1",
                GN_FIELD_BOUNDED_NATURAL, 4, 2, 2000},
            {"Tempo iniziale t0", "t0: [-100..100], indipendente da T",
                GN_FIELD_BOUNDED_DECIMAL, 18, -100, 100},
            {"Tempo finale T", "T: [-100..100], deve essere maggiore di t0",
                GN_FIELD_BOUNDED_DECIMAL, 18, -100, 100},
            {"Valore al bordo x0", "x0: [-100000..100000], usato nelle prove 1 e 2",
                GN_FIELD_BOUNDED_DECIMAL, 18, -100000, 100000},
            {"Coefficiente a_arg", "a_arg: [-2000..2000], costante solo nella prova 3",
                GN_FIELD_BOUNDED_DECIMAL, 18, -2000, 2000},
            {"Forzante f_arg", "f_arg: [-100000..100000], costante solo nella prova 3",
                GN_FIELD_BOUNDED_DECIMAL, 18, -100000, 100000}
        };

        std::string decimal(double value) {
            // Evita la notazione scientifica, non adatta al campo DECIMAL.
            std::ostringstream os;
            os << std::fixed << std::setprecision(14) << value;
            std::string s = os.str();
            const auto dot = s.find('.');
            if (dot != std::string::npos) {
                while (!s.empty() && s.back() == '0') s.pop_back();
                if (!s.empty() && s.back() == '.') s.pop_back();
            }
            return s == "-0" ? "0" : s;
        }

        void mask(const GnFormContext& ctx, void*) {
            const auto& l = ctx.layout;
            const int width = std::max(1, l.terminal_columns - 1);
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            fcn_form::write(l.header_top, 1,
                " Simulazione E1b - Cauchy generalizzato ",
                width, GN_SCREEN_TITLE);
            fcn_form::write(l.header_top + 1, 3,
                "z'(t) = a(t)*z(t) + f(t)  |  tre prove a funzioni fissate",
                width - 3);
            fcn_form::separator(l.top_separator_row, l.terminal_columns);
            fcn_form::write(l.viewport.top, 3,
                "Intervallo [t0,T], n nodi; F1 spiega funzioni e valori al bordo.",
                width - 3);
            fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
            fcn_form::separator(l.status_separator_row, l.terminal_columns);
        }

        bool prepare(GnFormContext& ctx, void*) {
            if (!ctx.fields || !ctx.values || ctx.field_count != FIELD_COUNT)
                return false;
            const int col = std::min(54,
                std::max(45, ctx.layout.terminal_columns - 26));
            const int row = ctx.layout.viewport.top + 2;
            std::size_t longest = 0;
            for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                const auto& info = fields[i];
                ctx.fields[i] = GnEntrySpec{
                    row + 2 * static_cast<int>(i), col,
                    info.type, info.length, info.minimum, info.maximum,
                    1, ctx.layout.echo_row
                };
                longest = std::max(longest, info.length);
            }
            // GRIMM: stessa colonna di fine campo per tutti gli entry.
            const int right_col = col + static_cast<int>(longest) - 1;
            for (std::size_t i = 0; i < FIELD_COUNT; ++i)
                ctx.fields[i].column = right_col -
                    static_cast<int>(ctx.fields[i].length) + 1;
            return true;
        }

        void render(const GnFormContext& ctx, void* data) {
            if (!ctx.fields || !ctx.values ||
                ctx.field_count != FIELD_COUNT || ctx.lab >= FIELD_COUNT)
                return;
            mask(ctx, data);
            for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                const auto& f = ctx.fields[i];
                fcn_form::write(f.row, 5,
                    std::string(i == ctx.lab ? "> " : "  ") + fields[i].label,
                    f.column - 6);
                fcn_form::write(f.row, f.column,
                    fcn_form::field_value(ctx.values[i], f.length),
                    static_cast<int>(f.length), GN_SCREEN_FIELD);
            }
            gn_screen_clear_line(ctx.layout.echo_row);
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            fcn_form::write(ctx.layout.hint_row, 2, fields[ctx.lab].hint,
                            width - 2, GN_SCREEN_MESSAGE);
            fcn_form::write(ctx.layout.status_top, 2,
                "ESC Menu      F1 Help (2 pagine)      F10 Esegui",
                width - 2, GN_SCREEN_STATUS);
            if (!ctx.message.empty())
                fcn_form::write(ctx.layout.echo_row, 2, ctx.message,
                                width - 2, GN_SCREEN_ERROR);
            gn_screen_flush();
        }

        GnFormChangeResult change(
            GnEntryCommand command, GnFormContext& ctx, void*) {
            return fcn_form::change_linear(command, ctx);
        }

        void help(const GnFormContext& ctx, void*) {
            // Due schermate per documentare TUTTI gli esperimenti senza
            // troncare il testo nelle consuete 24 righe del terminale.
            const std::vector<std::string> page1 = {
                "E1b - Cauchy generalizzato  [1/2]  ESPERIMENTI", "",
                "Problema: z'(t) = a(t)*z(t) + f(t)",
                "n NODI uniformi in [t0,T]; h=(T-t0)/(n-1).", "",
                "Prova 1 - matrice + curva (pannelli 0 e 1):",
                "  a(t)=0; f(t)=1/(1+sin(t)^2).",
                "  Schema BACKWARD: il valore x0 e' imposto in T!", "",
                "Prova 2 - curva (pannello 2):",
                "  a(t)=cos(t); f(t)=1/(1+sin(t)^2).",
                "  Schema FORWARD: z(t0)=x0.", "",
                "Prova 3 - curva (pannello 3):",
                "  a(t)=a_arg, f(t)=f_arg (costanti).",
                "  Schema FORWARD: z(t0)=1, NON x0.", "",
                "Le tre prove sono fissate nel chiamante sym_cauchy().",
                "Non esiste ancora la scelta delle funzioni da form."
            };
            const std::vector<std::string> page2 = {
                "E1b - Cauchy generalizzato  [2/2]  PARAMETRI/API", "",
                "n: nodi [2..2000]. t0,T: estremi indipendenti, T>t0.",
                "x0: valore di bordo per le prove 1 e 2 (non prova 3).",
                "a_arg e f_arg: costanti usate solo nella prova 3.", "",
                "matrix_build_cauchy riceve callback:",
                "  double (*at)(double) per a(t)",
                "  double (*ft)(double) per f(t)",
                "Passando f_const, il costruttore usa a_arg/f_arg",
                "come valore costante, anziche' valutare il callback.",
                "nullptr e' interpretato come funzione nulla.",
                "backw=false: condizione in t0, matrice inferiore.",
                "backw=true:  condizione in T, matrice superiore.", "",
                "I limiti delle caselle sono vincoli della UI, non",
                "ipotesi matematiche sui coefficienti dell'ODE.", "",
                "F10: esegui. ESC nel form: menu."
            };
            std::size_t page = 0;
            for (;;) {
                gn_screen_clear();
                gn_screen_cursor_visible(0);
                const auto& lines = page == 0 ? page1 : page2;
                const int width = std::max(1, ctx.layout.terminal_columns - 1);
                int row = 1;
                for (const auto& line : lines) {
                    if (row >= ctx.layout.terminal_rows - 2) break;
                    fcn_form::write(row, 2, line, width - 2,
                        row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
                    ++row;
                }
                fcn_form::write(ctx.layout.terminal_rows - 1, 2,
                    "INVIO/F1: cambia pagina       ESC: ritorna al form",
                    width - 2, GN_SCREEN_STATUS);
                gn_screen_flush();
                for (;;) {
                    GnKeyEvent ev{};
                    const auto result = gn_pump_once(&ev);
                    if (result == GN_POLL_EMPTY) {
                        gn_cooperative_pause();
                        continue;
                    }
                    if (result == GN_POLL_ERROR || ev.code == GN_KEY_ESCAPE)
                        return;
                    if (ev.code == GN_KEY_ENTER || ev.code == GN_KEY_F1) {
                        page = 1 - page;
                        break;
                    }
                    gn_screen_beep();
                }
            }
        }

        bool validate(GnFormContext& ctx, void*) {
            if (!ctx.values) return false;
            for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                try {
                    if (ctx.values[i].empty()) throw std::invalid_argument("empty");
                    std::size_t consumed = 0;
                    const double v = std::stod(ctx.values[i], &consumed);
                    if (consumed != ctx.values[i].size() ||
                        !std::isfinite(v) || v < fields[i].minimum ||
                        v > fields[i].maximum)
                        throw std::out_of_range("bounds");
                } catch (const std::exception&) {
                    ctx.lab = i;
                    ctx.message = "Campo vuoto, non numerico o fuori intervallo";
                    return false;
                }
            }
            if (!(std::stod(ctx.values[2]) > std::stod(ctx.values[1]))) {
                ctx.lab = 2;
                ctx.message = "L'estremo finale T deve essere > t0";
                return false;
            }
            return true;
        }
    } // namespace anonimo

    bool prompt(SymCauchyParams& p) {
        std::string values[FIELD_COUNT] = {
            std::to_string(p.n), decimal(p.t0), decimal(p.T),
            decimal(p.x0), decimal(p.a_arg), decimal(p.f_arg)
        };
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 16;
        spec.layout_options.min_viewport_columns = 80;
        spec.values = values;
        spec.field_count = FIELD_COUNT;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;
        const GnFormRunResult result = gn_form_run_session(spec);
        if (result != GN_FORM_RUN_ACCEPT) {
            if (result == GN_FORM_RUN_ERROR)
                std::cerr << "Impossibile aprire il form Cauchy E1b.\n";
            return false;
        }
        // Commit atomico: ESC non cambia la struct del chiamante.
        p.n = std::stoi(values[0]);
        p.t0 = std::stod(values[1]);
        p.T = std::stod(values[2]);
        p.x0 = std::stod(values[3]);
        p.a_arg = std::stod(values[4]);
        p.f_arg = std::stod(values[5]);
        return true;
    }
} // namespace sym_cauchy_form

// Simulazione E1c - un campo, implementazione privata al prompt.
// Richiede i servizi fcn_form e GNPORT gia' usati negli altri form.
namespace sym_hhalpha_form {
    namespace {
        constexpr std::size_t FIELD_COUNT = 1;
        constexpr int N_MIN = 3;
        constexpr int N_MAX = 100; // limite del vecchio input; n=1,2 non calcola nulla

        void mask(const GnFormContext& ctx, void*) {
            const auto& l = ctx.layout;
            const int width = std::max(1, l.terminal_columns - 1);
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            fcn_form::write(l.header_top, 1,
                " Simulazione E1c - riflessione di Householder ",
                width, GN_SCREEN_TITLE);
            fcn_form::write(l.header_top + 1, 3,
                "Monitoraggio del primo elemento del vettore riflesso",
                width - 3);
            fcn_form::separator(l.top_separator_row, l.terminal_columns);
            fcn_form::write(l.viewport.top, 3,
                "Vettori alternati +/-1, dimensioni i = 2,...,n-1. F1: dettagli.",
                width - 3);
            fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
            fcn_form::separator(l.status_separator_row, l.terminal_columns);
        }

        bool prepare(GnFormContext& ctx, void*) {
            if (!ctx.fields || !ctx.values || ctx.field_count != FIELD_COUNT)
                return false;
            const int right_col = std::min(67,
                std::max(43, ctx.layout.terminal_columns - 12));
            ctx.fields[0] = GnEntrySpec{
                ctx.layout.viewport.top + 4, right_col - 2,
                GN_FIELD_BOUNDED_NATURAL, 3, N_MIN, N_MAX,
                1, ctx.layout.echo_row
            };
            return true;
        }

        void render(const GnFormContext& ctx, void* data) {
            if (!ctx.fields || !ctx.values ||
                ctx.field_count != FIELD_COUNT || ctx.lab >= FIELD_COUNT)
                return;
            mask(ctx, data);
            const auto& f = ctx.fields[0];
            fcn_form::write(f.row, 5, "> Dimensione massima n", f.column - 6);
            fcn_form::write(f.row, f.column,
                fcn_form::field_value(ctx.values[0], f.length),
                static_cast<int>(f.length), GN_SCREEN_FIELD);
            gn_screen_clear_line(ctx.layout.echo_row);
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            fcn_form::write(ctx.layout.hint_row, 2,
                "n: 3..100; prova su dimensioni da 2 a n-1",
                width - 2, GN_SCREEN_MESSAGE);
            fcn_form::write(ctx.layout.status_top, 2,
                "ESC Menu      F1 Help      F10 Esegui",
                width - 2, GN_SCREEN_STATUS);
            if (!ctx.message.empty())
                fcn_form::write(ctx.layout.echo_row, 2, ctx.message,
                                width - 2, GN_SCREEN_ERROR);
            gn_screen_flush();
        }

        GnFormChangeResult change(
            GnEntryCommand command, GnFormContext& ctx, void*) {
            return fcn_form::change_linear(command, ctx);
        }

        void help(const GnFormContext& ctx, void*) {
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            const std::vector<std::string> lines = {
                "E1c - Monitoraggio di Householder", "",
                "Il parametro n stabilisce la dimensione massima dei vettori usati",
                "per verificare il comportamento del riflettore di Householder.",
                "Per ogni i da 2 a n-1 si costruisce un vettore di",
                "dimensione i con componenti 1,-1,1,-1,... .",
                "Si forma il riflettore H e si calcola w = H*v.",
                "Il grafico mostra w[0] in funzione della dimensione i.", "",
                "Il riflettore e' ortogonale: ||H*v||2 = ||v||2.",
                "Per il vettore alternato ||v||2 = sqrt(i),",
                "e il primo componente riflesso e' w[0] = -sqrt(i).", "",
                "Se n >= 7, per i=6 si stampano la matrice H e",
                "l'esito del controllo di ortogonalita'.", "",
                "n = 100 di default; campo ammesso 3..100.",
                "Le posizioni 0 e 1 negli array del grafico sono",
                "segnaposto e NON rappresentano prove numeriche.", "",
                "Il report numerico e' stampato nella console:",
                "dopo il grafico, INVIO ritorna al form.", "",
                "F10: esegui; ESC: torna al menu."
            };
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            int row = 1;
            for (const auto& line : lines) {
                if (row >= ctx.layout.terminal_rows - 2) break;
                fcn_form::write(row, 2, line, width - 2,
                    row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
                ++row;
            }
            fcn_form::write(ctx.layout.terminal_rows - 1, 2,
                "ESC: ritorna al form", width - 2, GN_SCREEN_STATUS);
            gn_screen_flush();
            for (;;) {
                GnKeyEvent ev{};
                const auto result = gn_pump_once(&ev);
                if (result == GN_POLL_EMPTY) {
                    gn_cooperative_pause();
                    continue;
                }
                if (result == GN_POLL_ERROR || ev.code == GN_KEY_ESCAPE ||
                    ev.code == GN_KEY_ENTER || ev.code == GN_KEY_F1)
                    return;
                gn_screen_beep();
            }
        }

        bool validate(GnFormContext& ctx, void*) {
            if (!ctx.values) return false;
            try {
                std::size_t consumed = 0;
                const int n = std::stoi(ctx.values[0], &consumed);
                if (consumed == ctx.values[0].size() &&
                    n >= N_MIN && n <= N_MAX)
                    return true;
            } catch (const std::exception&) {
                // Il messaggio viene assegnato sotto.
            }
            ctx.lab = 0;
            ctx.message = "n deve essere un intero compreso tra 3 e 100";
            return false;
        }
    } // namespace anonimo

    bool prompt(SymHHAlphaParams& p) {
        std::string values[FIELD_COUNT] = {std::to_string(p.n)};
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 80;
        spec.values = values;
        spec.field_count = FIELD_COUNT;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;

        const GnFormRunResult result = gn_form_run_session(spec);
        if (result != GN_FORM_RUN_ACCEPT) {
            if (result == GN_FORM_RUN_ERROR)
                std::cerr << "Impossibile aprire il form Householder E1c.\n";
            return false;
        }
        p.n = std::stoi(values[0]);
        return true;
    }
} // namespace sym_hhalpha_form

namespace ode_multi_form {
    constexpr std::size_t MAX_FIELDS = 11;
    enum class Kind { Euler, Osc2D, LotkaV };

    struct Field {
        const char* label;
        const char* hint;
        GnFieldType type;
        std::size_t length;
        double min_value;
        double max_value;
    };
    struct Config {
        const char* title;
        const char* subtitle;
        const char* description;
        Kind kind;
        std::size_t count;
        Field fields[MAX_FIELDS];
    };

    std::string decimal(double x) {
        std::ostringstream os;
        os << std::setprecision(10) << x;
        return os.str();
    }
    void mask(const GnFormContext& ctx, void* user_data) {
        const auto& cfg = *static_cast<const Config*>(user_data);
        const auto& l = ctx.layout;
        const int w = std::max(1, l.terminal_columns - 1);
        gn_screen_clear();
        gn_screen_cursor_visible(0);
        fcn_form::write(l.header_top, 1, cfg.title, w, GN_SCREEN_TITLE);
        fcn_form::write(l.header_top + 1, 3, cfg.subtitle, w - 3);
        fcn_form::separator(l.top_separator_row, l.terminal_columns);
        fcn_form::write(l.viewport.top, 3, cfg.description, w - 3);
        fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
        fcn_form::separator(l.status_separator_row, l.terminal_columns);
    }
    bool prepare(GnFormContext& ctx, void* user_data) {
        if (!user_data || !ctx.fields || !ctx.values) return false;
        const auto& cfg = *static_cast<const Config*>(user_data);
        if (cfg.count == 0 || cfg.count > MAX_FIELDS ||
            ctx.field_count != cfg.count) return false;
        const int first_col = std::min(59, std::max(48, ctx.layout.terminal_columns - 23));
        const int first_row = ctx.layout.viewport.top + 1;
        std::size_t max_length = 0;
        for (std::size_t i = 0; i < cfg.count; ++i) {
            const auto& f = cfg.fields[i];
            ctx.fields[i] = GnEntrySpec{
                first_row + static_cast<int>(i), first_col,
                f.type, f.length, f.min_value, f.max_value,
                1, ctx.layout.echo_row
            };
            max_length = std::max(max_length, f.length);
        }
        // GRIMM: stesso bordo destro, una sola lunghezza per campo.
        const int right_col = first_col + static_cast<int>(max_length) - 1;
        for (std::size_t i = 0; i < cfg.count; ++i)
            ctx.fields[i].column = right_col -
                static_cast<int>(ctx.fields[i].length) + 1;
        return true;
    }
    void render(const GnFormContext& ctx, void* user_data) {
        if (!user_data || !ctx.fields || !ctx.values ||
            ctx.lab >= ctx.field_count) return;
        const auto& cfg = *static_cast<const Config*>(user_data);
        if (ctx.field_count != cfg.count) return;
        mask(ctx, user_data);
        for (std::size_t i = 0; i < cfg.count; ++i) {
            const auto& field = ctx.fields[i];
            fcn_form::write(field.row, 5,
                std::string(i == ctx.lab ? "> " : "  ") + cfg.fields[i].label,
                field.column - 6);
            fcn_form::write(field.row, field.column,
                fcn_form::field_value(ctx.values[i], field.length),
                static_cast<int>(field.length), GN_SCREEN_FIELD);
        }
        gn_screen_clear_line(ctx.layout.echo_row);
        const int w = std::max(1, ctx.layout.terminal_columns - 1);
        fcn_form::write(ctx.layout.hint_row, 2,
            cfg.fields[ctx.lab].hint, w - 2, GN_SCREEN_MESSAGE);
        fcn_form::write(ctx.layout.status_top, 2,
            "ESC Menu      F1 Help      F10 Esegui", w - 2, GN_SCREEN_STATUS);
        if (!ctx.message.empty())
            fcn_form::write(ctx.layout.echo_row, 2,
                ctx.message, w - 2, GN_SCREEN_ERROR);
        gn_screen_flush();
    }
    GnFormChangeResult change(GnEntryCommand cmd, GnFormContext& ctx, void*) {
        return fcn_form::change_linear(cmd, ctx);
    }
    void help(const GnFormContext& ctx, void* user_data) {
        const auto& cfg = *static_cast<const Config*>(user_data);
        std::vector<std::string> lines{
            std::string("F1 HELP - ") + cfg.title, "",
            cfg.description, "",
            "N1..N4 sono numeri di INTERVALLI (nodi = N+1).",
            "Le quattro risoluzioni vanno in ordine crescente.", ""
        };
        for (std::size_t i = 0; i < cfg.count; ++i)
            lines.push_back(std::string("  ") + cfg.fields[i].hint);
        if (cfg.kind == Kind::Osc2D) {
            lines.push_back("Oscillatore: sia libero sia smorzato.");
            lines.push_back("Per la soluzione esatta: 0 <= gamma < omega.");
        }
        if (cfg.kind == Kind::LotkaV)
            lines.push_back("Lotka-Volterra: coefficienti positivi.");
        lines.push_back("F10 esegue. ESC ritorna al menu.");
        lines.push_back("F1 / INVIO / ESC: ritorno ai parametri.");
        gn_screen_clear();
        gn_screen_cursor_visible(0);
        const int w = std::max(1, ctx.layout.terminal_columns - 1);
        int row = 1;
        for (const auto& s : lines) {
            if (row > ctx.layout.terminal_rows) break;
            const GnScreenRole role = row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL;
            fcn_form::write(row, 2, s, w - 2, role);
            ++row;
        }
        gn_screen_flush();
        for (;;) {
            GnKeyEvent event{};
            const auto result = gn_pump_once(&event);
            if (result == GN_POLL_EMPTY) {
                gn_cooperative_pause();
                continue;
            }
            if (result == GN_POLL_ERROR ||
                event.code == GN_KEY_F1 || event.code == GN_KEY_ENTER ||
                event.code == GN_KEY_ESCAPE) return;
            gn_screen_beep();
        }
    }
    bool validate(GnFormContext& ctx, void* user_data) {
        if (!ctx.values || !user_data) return false;
        const auto& cfg = *static_cast<const Config*>(user_data);
        double v[MAX_FIELDS]{};
        for (std::size_t i = 0; i < cfg.count; ++i) {
            try {
                if (ctx.values[i].empty()) throw std::invalid_argument("empty");
                std::size_t count = 0;
                v[i] = std::stod(ctx.values[i], &count);
                if (count != ctx.values[i].size() ||
                    !(v[i] >= cfg.fields[i].min_value &&
                      v[i] <= cfg.fields[i].max_value))
                    throw std::out_of_range("value");
            } catch (const std::exception&) {
                ctx.lab = i;
                ctx.message = "Valore non valido o fuori intervallo";
                return false;
            }
        }
        for (std::size_t i = 1; i < 4; ++i) {
            if (!(v[i] > v[i-1])) {
                ctx.lab = i;
                ctx.message = "Impostare N1 < N2 < N3 < N4";
                return false;
            }
        }
        if (cfg.kind == Kind::Osc2D && !(v[8] < v[7])) {
            ctx.lab = 8;
            ctx.message = "Per la soluzione esatta: gamma < omega";
            return false;
        }
        return true;
    }
    bool run(Config& cfg, std::string* values) {
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 14;
        spec.layout_options.min_viewport_columns = 80;
        spec.values = values;
        spec.field_count = cfg.count;
        spec.user_data = &cfg;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;
        const auto result = gn_form_run_session(spec);
        if (result == GN_FORM_RUN_ERROR)
            std::cerr << "Errore sessione form ODE.\n";
        return result == GN_FORM_RUN_ACCEPT;
    }
}

namespace ode_euler_form {
    bool prompt(OdeEulerParams& p) {
        static ode_multi_form::Config cfg{
            " ODE scalari - Euler, Heun, RK4 ",
            "Confronto per decadimento esponenziale e logistica",
            "Quattro griglie e condizioni iniziali. t0 = 0.",
            ode_multi_form::Kind::Euler, 6, {
                {"Intervalli N1", "N1: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N2", "N2: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N3", "N3: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N4", "N4: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Tempo finale b", "b: 0.1..30", GN_FIELD_BOUNDED_DECIMAL, 10, 0.1, 30},
                {"Condizione y0", "y0: 0.01..9.99 (logistica K=10)",
                    GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 9.99}
            }
        };
        std::string v[] = {
            std::to_string(p.N[0]), std::to_string(p.N[1]),
            std::to_string(p.N[2]), std::to_string(p.N[3]),
            ode_multi_form::decimal(p.b), ode_multi_form::decimal(p.y0)
        };
        if (!ode_multi_form::run(cfg, v)) return false;
        for (std::size_t i = 0; i < 4; ++i) p.N[i] = std::stoi(v[i]);
        p.b = std::stod(v[4]); p.y0 = std::stod(v[5]);
        return true;
    }
}
namespace ode_osc2d_form {
    bool prompt(OdeOsc2DParams& p) {
        static ode_multi_form::Config cfg{
            " ODE vettoriali - oscillatore 2D ",
            "RK4 libero e smorzato; soluzioni esatte e fase",
            "Quattro griglie, condizioni iniziali e parametri. t0 = 0.",
            ode_multi_form::Kind::Osc2D, 9, {
                {"Intervalli N1", "N1: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N2", "N2: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N3", "N3: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N4", "N4: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Tempo finale b", "b: 0.1..30", GN_FIELD_BOUNDED_DECIMAL, 10, 0.1, 30},
                {"Posizione iniziale q0", "q0: -10..10", GN_FIELD_BOUNDED_DECIMAL, 10, -10, 10},
                {"Velocita iniziale p0", "p0: -10..10", GN_FIELD_BOUNDED_DECIMAL, 10, -10, 10},
                {"Frequenza omega", "omega: 0.1..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.1, 10},
                {"Smorzamento gamma", "gamma: 0..9.99, gamma < omega",
                    GN_FIELD_BOUNDED_DECIMAL, 10, 0, 9.99}
            }
        };
        std::string v[] = {
            std::to_string(p.N[0]), std::to_string(p.N[1]),
            std::to_string(p.N[2]), std::to_string(p.N[3]),
            ode_multi_form::decimal(p.b), ode_multi_form::decimal(p.q0),
            ode_multi_form::decimal(p.p0), ode_multi_form::decimal(p.omega),
            ode_multi_form::decimal(p.gamma)
        };
        if (!ode_multi_form::run(cfg, v)) return false;
        for (std::size_t i = 0; i < 4; ++i) p.N[i] = std::stoi(v[i]);
        p.b = std::stod(v[4]); p.q0 = std::stod(v[5]);
        p.p0 = std::stod(v[6]); p.omega = std::stod(v[7]);
        p.gamma = std::stod(v[8]);
        return true;
    }
}
namespace ode_lotkav_form {
    bool prompt(OdeLotkaVParams& p) {
        static ode_multi_form::Config cfg{
            " ODE - Lotka Volterra ",
            "Predatori e prede: piano delle fasi con Euler",
            "Quattro griglie; popolazioni iniziali; coefficienti del modello.",
            ode_multi_form::Kind::LotkaV, 11, {
                {"Intervalli N1", "N1: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N2", "N2: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N3", "N3: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Intervalli N4", "N4: 2..800", GN_FIELD_BOUNDED_NATURAL, 3, 2, 800},
                {"Tempo finale b", "b: 0.1..30", GN_FIELD_BOUNDED_DECIMAL, 10, 0.1, 30},
                {"Prede iniziali q0", "q0: 0.01..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 10},
                {"Predatori iniziali p0", "p0: 0.01..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 10},
                {"Alpha crescita prede", "alpha: 0.01..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 10},
                {"Beta predazione", "beta: 0.01..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 10},
                {"Gamma mortalita", "gamma: 0.01..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 10},
                {"Delta riproduzione", "delta: 0.01..10", GN_FIELD_BOUNDED_DECIMAL, 10, 0.01, 10}
            }
        };
        std::string v[] = {
            std::to_string(p.N[0]), std::to_string(p.N[1]),
            std::to_string(p.N[2]), std::to_string(p.N[3]),
            ode_multi_form::decimal(p.b), ode_multi_form::decimal(p.q0),
            ode_multi_form::decimal(p.p0), ode_multi_form::decimal(p.alpha),
            ode_multi_form::decimal(p.beta), ode_multi_form::decimal(p.gamma),
            ode_multi_form::decimal(p.delta)
        };
        if (!ode_multi_form::run(cfg, v)) return false;
        for (std::size_t i = 0; i < 4; ++i) p.N[i] = std::stoi(v[i]);
        p.b = std::stod(v[4]); p.q0 = std::stod(v[5]);
        p.p0 = std::stod(v[6]); p.alpha = std::stod(v[7]);
        p.beta = std::stod(v[8]); p.gamma = std::stod(v[9]);
        p.delta = std::stod(v[10]);
        return true;
    }
}

// Form GNPORT per al_qr(): 2 campi, unico punto d'ingresso pubblico.
namespace al_qr_form {
    namespace {
        constexpr std::size_t FIELD_COUNT = 2;
        constexpr int N_MIN = 3;
        constexpr int N_MAX = 20;
        constexpr int D_MIN = 3;

        void mask(const GnFormContext& ctx, void*) {
            const auto& l = ctx.layout;
            const int width = std::max(1, l.terminal_columns - 1);
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            fcn_form::write(l.header_top, 1,
                " QR di Householder - minimi quadrati ", width, GN_SCREEN_TITLE);
            fcn_form::write(l.header_top + 1, 3,
                "A in R^(n x d): n righe (misure), d colonne (feature)", width - 3);
            fcn_form::separator(l.top_separator_row, l.terminal_columns);
            fcn_form::write(l.viewport.top, 3,
                "Matrice tall: n >= d. F1: QR completa e residuo LS.", width - 3);
            fcn_form::separator(l.bottom_separator_row, l.terminal_columns);
            fcn_form::separator(l.status_separator_row, l.terminal_columns);
        }

        bool prepare(GnFormContext& ctx, void*) {
            if (!ctx.fields || !ctx.values || ctx.field_count != FIELD_COUNT)
                return false;
            const int right_col = std::min(67,
                std::max(43, ctx.layout.terminal_columns - 12));
            const int top = ctx.layout.viewport.top;
            // Allineamento destro comune, come negli altri form GRIMM/GNPORT.
            ctx.fields[0] = GnEntrySpec{top + 3, right_col - 1,
                GN_FIELD_BOUNDED_NATURAL, 2, N_MIN, N_MAX,
                1, ctx.layout.echo_row};
            ctx.fields[1] = GnEntrySpec{top + 5, right_col - 1,
                GN_FIELD_BOUNDED_NATURAL, 2, D_MIN, N_MAX,
                1, ctx.layout.echo_row};
            return true;
        }

        void render(const GnFormContext& ctx, void* data) {
            if (!ctx.fields || !ctx.values || ctx.field_count != FIELD_COUNT ||
                ctx.lab >= FIELD_COUNT)
                return;
            mask(ctx, data);
            const char* labels[FIELD_COUNT] = {
                "> Numero righe n (osservazioni)",
                "> Numero colonne d (feature)"
            };
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            for (std::size_t i = 0; i < FIELD_COUNT; ++i) {
                const auto& f = ctx.fields[i];
                fcn_form::write(f.row, 5, labels[i], f.column - 6);
                fcn_form::write(f.row, f.column,
                    fcn_form::field_value(ctx.values[i], f.length),
                    static_cast<int>(f.length), GN_SCREEN_FIELD);
            }
            gn_screen_clear_line(ctx.layout.echo_row);
            fcn_form::write(ctx.layout.hint_row, 2,
                ctx.lab == 0 ? "n: 3..20; numero di equazioni / misure"
                            : "d: 3..n; colonne / incognite, non superiori alle righe",
                width - 2, GN_SCREEN_MESSAGE);
            fcn_form::write(ctx.layout.status_top, 2,
                "ESC Menu      F1 Help      F10 Esegui", width - 2, GN_SCREEN_STATUS);
            if (!ctx.message.empty())
                fcn_form::write(ctx.layout.echo_row, 2, ctx.message,
                    width - 2, GN_SCREEN_ERROR);
            gn_screen_flush();
        }

        GnFormChangeResult change(
            GnEntryCommand command, GnFormContext& ctx, void*) {
            return fcn_form::change_linear(command, ctx);
        }

        void help(const GnFormContext& ctx, void*) {
            gn_screen_clear();
            gn_screen_cursor_visible(0);
            const std::vector<std::string> lines = {
                "QR di Householder e minimi quadrati", "",
                "CONVENZIONE DI al_qr: A ha n righe e d colonne (n x d).",
                "n: numero di osservazioni / equazioni, da 3 a 20.",
                "d: numero di feature / incognite, da 3 a n.",
                "Per i minimi quadrati implementati: n >= d (matrice tall).", "",
                "QR completa: A = Q R, Q di ordine n e R di taglia n x d.",
                "Se A ha rango colonna pieno, R = [R1; 0], R1 d x d.",
                "Si risolve R1*x = c, con c prime d componenti di Q^T*b.",
                "Le restanti n-d componenti danno la norma del residuo LS.", "",
                "Prova 1: b casuale, verifica ||A-QR|| e ||Ax-b||.",
                "Prova 2: b = A*x_true + rumore, verifica recupero di x.",
                "Stima del condizionamento da autovalori di A^T A.",
                "Gli algoritmi numerici non sono modificati dal form.", "",
                "I risultati sono stampati in console; INVIO torna al form.",
                "F10 esegue, ESC torna al menu."
            };
            const int width = std::max(1, ctx.layout.terminal_columns - 1);
            int row = 1;
            for (const auto& line : lines) {
                if (row >= ctx.layout.terminal_rows - 2) break;
                fcn_form::write(row, 2, line, width - 2,
                    row == 1 ? GN_SCREEN_TITLE : GN_SCREEN_NORMAL);
                ++row;
            }
            fcn_form::write(ctx.layout.terminal_rows - 1, 2,
                "ESC: ritorna al form", width - 2, GN_SCREEN_STATUS);
            gn_screen_flush();
            for (;;) {
                GnKeyEvent ev{};
                const auto result = gn_pump_once(&ev);
                if (result == GN_POLL_EMPTY) {
                    gn_cooperative_pause();
                    continue;
                }
                if (result == GN_POLL_ERROR || ev.code == GN_KEY_ESCAPE ||
                    ev.code == GN_KEY_ENTER || ev.code == GN_KEY_F1)
                    return;
                gn_screen_beep();
            }
        }

        bool validate(GnFormContext& ctx, void*) {
            if (!ctx.values) return false;
            int n = 0;
            int d = 0;
            try {
                std::size_t used_n = 0, used_d = 0;
                n = std::stoi(ctx.values[0], &used_n);
                d = std::stoi(ctx.values[1], &used_d);
                if (used_n != ctx.values[0].size() ||
                    n < N_MIN || n > N_MAX) {
                    ctx.lab = 0;
                    ctx.message = "n: numero di righe compreso tra 3 e 20";
                    return false;
                }
                if (used_d != ctx.values[1].size() ||
                    d < D_MIN || d > n) {
                    ctx.lab = 1;
                    ctx.message = "d: numero di colonne tra 3 e n (n >= d)";
                    return false;
                }
                return true;
            } catch (const std::exception&) {
                ctx.lab = 0;
                ctx.message = "n e d devono essere interi validi (3 <= d <= n <= 20)";
                return false;
            }
        }
    } // namespace anonimo: dettagli strumentali al solo prompt

    bool prompt(AlQRParams& p) {
        std::string values[FIELD_COUNT] = {
            std::to_string(p.n), std::to_string(p.d)
        };
        GnFormRunSpec spec{};
        spec.layout_options.header_rows = 2;
        spec.layout_options.top_separator = true;
        spec.layout_options.bottom_separator = true;
        spec.layout_options.hint_line = true;
        spec.layout_options.echo_line = true;
        spec.layout_options.status_rows = 1;
        spec.layout_options.min_viewport_rows = 10;
        spec.layout_options.min_viewport_columns = 80;
        spec.values = values;
        spec.field_count = FIELD_COUNT;
        spec.prepare = prepare;
        spec.render = render;
        spec.change = change;
        spec.help = help;
        spec.validate = validate;
        const auto result = gn_form_run_session(spec);
        if (result != GN_FORM_RUN_ACCEPT) {
            if (result == GN_FORM_RUN_ERROR)
                std::cerr << "Impossibile aprire il form QR/LS.\n";
            return false;
        }
        // Commit atomico: ESC lascia invariata la struct.
        p.n = std::stoi(values[0]);
        p.d = std::stoi(values[1]);
        return true;
    }
} // namespace al_qr_form
