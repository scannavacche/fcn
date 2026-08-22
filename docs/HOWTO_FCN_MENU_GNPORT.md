# HOWTO — Portare il menu GNPORT dentro FCN

## Scopo

Questo documento riunisce in una sola procedura operativa le decisioni prese per sostituire il menu testuale corrente di FCN con il nuovo motore GNPORT.

L’obiettivo è procedere per passi compilabili e verificabili:

1. usare inizialmente **l’attuale `menu.json` piatto** come equivalente moderno di `MENTAB`;
2. mantenere invariati `ActionRegistry` e tutte le procedure numeriche già collaudate;
3. limitare la modalità terminale non canonica alla sola permanenza nel menu;
4. raggruppare automaticamente le voci su due livelli usando il prefisso delle `action`;
5. soltanto dopo consolidare un formato JSON annidato esplicito;
6. preparare il passaggio successivo: sostituzione graduale dei `cin` finali con il nuovo `entry`.

Il documento distingue sempre:

- **stato attuale** della v0.3;
- **modifiche necessarie** per FCN;
- **checkpoint** da verificare prima di continuare.

---

# 1. Architettura attuale di FCN

FCN separa già correttamente configurazione, dispatch e procedure:

```text
menu.json
    action: "f2_integral"
            ↓
load_menu_config()
            ↓
MenuConfig
            ↓
find_menu_item(menu, choice)
            ↓
ActionRegistry
    "f2_integral" → f2_integral
            ↓
procedura numerica
```

Le strutture applicative attuali sono concettualmente:

```cpp
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

using ActionFunction = void (*)();
using ActionRegistry =
    std::unordered_map<std::string, ActionFunction>;
```

Il registry resta valido e non deve essere spostato nella libreria menu:

```cpp
ActionRegistry build_action_registry()
{
    return {
        {"f1_sample",     f1_sample},
        {"f1_taylor_exp", f1_taylor_exp},
        {"f1_taylor_cos", f1_taylor_cos},
        {"f1_prec",       f1_prec},
        {"f2_deriv",      f2_deriv},
        {"f2_cauchy",     f2_cauchy},
        {"f2_bordo",      f2_bordo},
        {"f2_integral",   f2_integral},
        {"f2_norme",      f2_norme},
        {"f2_deconv",     f2_deconv},
        {"f3_svd_test",   f3_svd_test},
        {"f3_svd",        f3_svd},
        {"f3_svd_pca",    f3_svd_pca},
        {"f3_svd_an",     f3_svd_an},
        {"sym_cauchy",    sym_cauchy},
        {"sym_hhalpha",   sym_hhalpha},
        {"ode_euler",     ode_euler},
        {"ode_osc2d",     ode_osc2d},
        {"ode_lotkav",    ode_lotkav},
        {"al_qr",         al_qr},
        {"sandbox",       sandbox}
    };
}
```

La stringa contenuta nel JSON è quindi un **identificatore logico**, non un nome di funzione risolto automaticamente dal linguaggio.

Questa separazione va conservata:

```text
JSON → action string → registry applicativo → funzione
```

---

# 2. Differenza fondamentale tra la demo GNPORT e FCN

## 2.1 Demo GNPORT v0.3

La demo apre il terminale una volta sola:

```cpp
gn_terminal_open();

while (running) {
    gn_menu_run(&main_menu);
    // entry, menu e attese lavorano tutti in modalità non canonica
}

gn_terminal_close();
```

Per tutta la vita della demo:

- `ICANON` è disabilitato;
- l’echo del terminale è disabilitato;
- lo standard input è non bloccante;
- la tastiera è consumata da `gn_keyboard_poll()`;
- non si usa `std::cin` per input a righe.

## 2.2 FCN

Le procedure numeriche di FCN usano invece normalmente:

```cpp
std::cin >> value;
std::getline(std::cin, text);
```

Quindi FCN deve alternare due regimi:

```text
MENU
terminale non canonico e polling passante
        ↓ scelta
ripristino completo del terminale
        ↓
PROCEDURA FCN
std::cin/std::cout canonici
        ↓ fine procedura
nuova apertura temporanea del menu
```

**Regola:** quando `gn_menu_run_session()` ritorna, FCN deve ritrovare lo standard I/O normale.

La libreria deve assumersi questa responsabilità; il `main` non deve conoscere `termios` né ricordarsi manualmente di chiudere il terminale su ogni ramo.

---

# 3. Stato attuale del menu GNPORT v0.3

L’API corrente è statica e basata su puntatori:

```cpp
struct GnMenuItem {
    const char* label;
    char shortcut;
    int action;
    GnMenuItemKind kind;
    const GnMenuDefinition* submenu;
    int enabled;
};

struct GnMenuDefinition {
    const char* title;
    GnMenuLayout layout;
    int rows;
    int columns;
    const GnMenuItem* items;
    std::size_t item_count;
};

struct GnMenuResult {
    GnMenuExitReason reason;
    int action;
};
```

Questa struttura è adatta a menu compilati nel sorgente, ma non è sufficiente per il caricamento da JSON perché:

1. `label` e `title` non possiedono la memoria delle stringhe;
2. `submenu` punta direttamente a un oggetto C++ già costruito;
3. `action` è un intero, mentre FCN usa stringhe;
4. non esiste un contenitore che mantenga vivi tutti i menu caricati;
5. `gn_menu_run()` presuppone che il terminale sia già aperto.

La migrazione richiede quindi due estensioni indipendenti:

```text
A. sessione terminale temporanea
B. repository dinamico dei menu caricati da JSON
```

Non vanno implementate e collaudate nello stesso passo.

---

# 4. Strategia complessiva

La sostituzione viene eseguita in sette checkpoint.

| Fase | Cambiamento | JSON | Procedure FCN |
|---|---|---|---|
| 1 | importazione sorgenti GNPORT | invariato | invariate |
| 2 | wrapper di sessione raw/canonical | invariato | invariate |
| 3 | strutture menu dinamiche e action stringa | invariato | invariate |
| 4 | loader dell’attuale menu piatto | invariato | invariate |
| 5 | sostituzione del loop principale | invariato | invariate |
| 6 | raggruppamento automatico per prefisso | ancora piatto | invariate |
| 7 | JSON annidato esplicito | due livelli | invariate |

Solo dopo il checkpoint 7 si comincia a sostituire l’input finale delle singole funzioni con `entry`.

---

# 5. Fase 1 — Importare GNPORT senza cambiare FCN

## 5.1 File da aggiungere

Per il menu servono almeno:

```text
include/
    gn_terminal.h
    gn_idle.h
    gn_string.h
    gn_menu.h

src/
    gn_terminal_posix.cpp
    gn_idle.cpp
    gn_string.cpp
    gn_menu.cpp
```

Dato che la fase successiva userà anche `entry`, conviene importare subito:

```text
include/gn_entry.h
src/gn_entry.cpp
```

## 5.2 Build

Aggiungere i `.cpp` al Makefile o a CMake, ma lasciare ancora il vecchio menu attivo.

Esempio indicativo:

```make
GNPORT_SRC = \
    src/gn_terminal_posix.cpp \
    src/gn_idle.cpp \
    src/gn_string.cpp \
    src/gn_menu.cpp \
    src/gn_entry.cpp
```

## 5.3 Checkpoint

FCN deve:

- compilare;
- avviarsi con il vecchio menu;
- eseguire almeno una procedura numerica;
- non modificare ancora il terminale.

Commit suggerito:

```text
Vendor GNPORT terminal, menu and entry sources into FCN
```

---

# 6. Fase 2 — Aggiungere una sessione menu temporanea

## 6.1 Nuova API pubblica

Mantenere `gn_menu_run()` come motore basso livello e aggiungere:

```cpp
GnMenuResult gn_menu_run_session(
    const GnMenuDefinition* root
);
```

Contratto:

```text
entra con terminale canonico
apre il backend GNPORT
esegue tutto il menu, inclusi i sottomenu
chiude e ripristina il terminale
ritorna l’azione scelta
```

## 6.2 Prima implementazione

```cpp
GnMenuResult gn_menu_run_session(
    const GnMenuDefinition* root)
{
    if (root == nullptr) {
        return GnMenuResult{GN_MENU_ERROR, 0};
    }

    if (gn_terminal_open() != 0) {
        return GnMenuResult{GN_MENU_ERROR, 0};
    }

    const GnMenuResult result = gn_menu_run(root);
    gn_terminal_close();

    return result;
}
```

`gn_terminal_close()` già:

- ripristina gli attributi `termios` originali;
- ripristina i flag di `stdin`;
- mostra il cursore;
- ripristina la forma normale del cursore;
- svuota i buffer interni del decoder.

## 6.3 Protezione dagli errori

La versione definitiva deve chiudere il terminale anche se un’eccezione futura sfugge dal renderer o dagli Idle.

È accettabile usare internamente una piccola guardia RAII, pur mantenendo l’API pubblica procedurale:

```cpp
class TerminalSessionGuard {
public:
    TerminalSessionGuard()
        : active_(gn_terminal_open() == 0)
    {
    }

    ~TerminalSessionGuard()
    {
        if (active_) {
            gn_terminal_close();
        }
    }

    bool active() const
    {
        return active_;
    }

private:
    bool active_;
};
```

## 6.4 Test isolato

Prima del JSON, costruire temporaneamente in FCN un menu statico con due azioni:

```cpp
const GnMenuItem test_items[] = {
    {"Esegui f1_prec", 'P', 1,
     GN_MENU_ACTION, nullptr, 1},

    {"Esci", 'E', 0,
     GN_MENU_ACTION, nullptr, 1}
};
```

Dopo `gn_menu_run_session()`, chiamare una funzione FCN che inizi subito con `std::cin`.

## 6.5 Checkpoint

Verificare:

1. il menu usa le frecce senza Invio;
2. la procedura scelta vede l’echo normale;
3. `std::cin >> ...` attende una riga normalmente;
4. `std::getline()` non riceve un newline residuo dal menu;
5. tornando al menu, il polling non trova vecchi eventi;
6. Ctrl-C torna a funzionare dentro la procedura FCN.

Il `main` non deve chiamare direttamente:

```cpp
gn_terminal_open();
gn_terminal_close();
```

Commit suggerito:

```text
Add scoped GNPORT menu terminal session
```

---

# 7. Fase 3 — Rendere il modello menu caricabile da JSON

## 7.1 Non modificare ancora il file JSON

In questa fase il file rimane quello attuale:

```json
{
  "title": "...",
  "items": [
    {
      "key": 1,
      "label": "...",
      "action": "f1_sample",
      "enabled": true
    }
  ]
}
```

## 7.2 Strutture dinamiche proposte

Creare un livello posseduto, separato dalle strutture statiche della demo:

```cpp
struct GnMenuItemConfig {
    int key = 0;
    std::string label;
    char shortcut = '\0';
    std::string action;
    std::string submenu_id;
    bool enabled = true;
};

struct GnMenuConfig {
    std::string id;
    std::string title;
    GnMenuLayout layout = GN_MENU_PROGRAM_LIST;
    int rows = 8;
    int columns = 1;
    std::vector<GnMenuItemConfig> items;
};

struct GnMenuRepository {
    std::string root_id;
    std::vector<GnMenuConfig> menus;
};
```

Questo repository possiede:

- titoli;
- label;
- action;
- identificatori dei sottomenu;
- vettori delle voci.

Nessun puntatore deve riferirsi a stringhe temporanee prodotte dal parser.

## 7.3 Risultato del menu

Per FCN il risultato utile è la stringa `action`:

```cpp
struct GnMenuResult {
    GnMenuExitReason reason = GN_MENU_ERROR;
    int key = 0;
    std::string action;
};
```

Il motore non deve conoscere `ActionRegistry`.

Il flusso resta:

```text
GnMenuResult.action
        ↓
actions.find(result.action)
        ↓
function pointer
```

## 7.4 Compatibilità con la demo statica

Ci sono due strade.

### Strada consigliata

Evolvere anche la demo al modello dinamico. Le sue azioni possono diventare stringhe:

```cpp
{"Data entry", 'D', "entry_demo", ...}
```

La demo userà un piccolo registry come FCN. In questo modo collauda la vera API destinata alle applicazioni.

### Strada alternativa

Mantenere temporaneamente entrambe le API:

```cpp
gn_menu_run_static(...);
gn_menu_run_repository(...);
```

È meno invasiva nell’immediato ma duplica il motore e non è consigliata oltre la transizione.

## 7.5 Checkpoint

Costruire in memoria un `GnMenuRepository` senza JSON e verificare:

- menu principale;
- almeno un sottomenu;
- restituzione di una action stringa;
- conservazione della selezione tornando dal figlio;
- `enabled == false` non selezionabile.

Commit suggerito:

```text
Add owning menu repository and string action results
```

---

# 8. Fase 4 — Caricare il menu FCN piatto

## 8.1 Moduli nuovi

```text
include/gn_fs.h
src/gn_fs.cpp

include/gn_menu_io.h
src/gn_menu_io.cpp
```

`gn_fs` gestisce soltanto il filesystem; `gn_menu_io` conosce JSON e menu.

## 8.2 API filesystem minima

```cpp
enum GnIoStatus {
    GN_IO_OK,
    GN_IO_NOT_FOUND,
    GN_IO_ACCESS_DENIED,
    GN_IO_INVALID_FORMAT,
    GN_IO_READ_ERROR,
    GN_IO_WRITE_ERROR
};

struct GnIoError {
    GnIoStatus status = GN_IO_OK;
    std::string path;
    std::string message;
};

bool gn_fs_read_text(
    const std::string& path,
    std::string* content,
    GnIoError* error
);
```

## 8.3 API loader piatto

```cpp
bool gn_menu_load_flat_json(
    const std::string& filename,
    GnMenuRepository* repository,
    GnIoError* error
);
```

Comportamento:

- crea un menu con `id = "root"`;
- usa `title` dal file o `"Menu"`;
- copia `key`, `label`, `action`, `enabled`;
- imposta inizialmente:

```cpp
layout  = GN_MENU_PAGED_COLUMNS;
rows    = 8;
columns = 2;
```

Con oltre 16 voci, il motore usa automaticamente `PgUp` e `PgDn`.

In questa fase è corretto avere ancora più pagine: serve a verificare la compatibilità del file esistente prima di introdurre il nesting.

## 8.4 Diagnostica

Le stampe di debug dell’attuale `load_menu_config()`:

```text
Working directory
nome del file tentato
dimensione del file
```

non devono essere prodotte sempre dalla libreria.

In caso di errore, riempire `GnIoError`; il `main` decide cosa mostrare:

```cpp
GnIoError error{};
GnMenuRepository menus{};

if (!gn_menu_load_flat_json(
        "menu.json", &menus, &error)) {
    std::cerr << error.message << '\n';
    return EXIT_FAILURE;
}
```

## 8.5 Checkpoint

Verificare che tutte le voci dell’attuale `menu.json`:

- siano mostrate;
- conservino `enabled`;
- restituiscano la stessa `action`;
- raggiungano la stessa funzione del registry;
- eseguano correttamente `exit`;
- non richiedano ancora alcuna modifica al JSON.

Commit suggerito:

```text
Load current FCN flat menu.json through GNPORT
```

---

# 9. Fase 5 — Sostituire il loop principale

## 9.1 Parti eliminate

Dal `main` scompaiono:

```cpp
std::cout << menu.title;
for (const auto& item : menu.items) { ... }
std::cin >> choice;
find_menu_item(menu, choice);
```

`find_menu_item()` può restare come utility di compatibilità, ma non è più necessaria nel normale ciclo TUI.

## 9.2 Nuovo loop

```cpp
int main()
{
    GnIoError error{};
    GnMenuRepository menus{};
    ActionRegistry actions;

    try {
        if (!gn_menu_load_flat_json(
                "menu.json", &menus, &error)) {
            throw std::runtime_error(error.message);
        }

        actions = build_action_registry();
    }
    catch (const std::exception& e) {
        std::cerr
            << "Errore inizializzazione menu: "
            << e.what()
            << '\n';

        return EXIT_FAILURE;
    }

    for (;;) {
        const GnMenuResult result =
            gn_menu_run_session(&menus);

        if (result.reason == GN_MENU_CANCEL) {
            break;
        }

        if (result.reason != GN_MENU_SELECTED) {
            std::cerr << "Errore nel menu.\n";
            return EXIT_FAILURE;
        }

        if (result.action == "exit") {
            break;
        }

        const auto it = actions.find(result.action);

        if (it == actions.end()) {
            std::cout
                << "Azione non registrata nel programma: "
                << result.action
                << '\n';

            continue;
        }

        // Il terminale è già nuovamente canonico.
        it->second();

        // Conservare inizialmente il comportamento corrente.
        wait_return_to_menu(true);
        clear_screen();
    }

    return EXIT_SUCCESS;
}
```

La firma concreta di `gn_menu_run_session()` può ricevere il repository e risolvere internamente `root_id`, oppure ricevere il menu root già trovato:

```cpp
const GnMenuConfig* root =
    gn_menu_find(&menus, menus.root_id);

GnMenuResult result =
    gn_menu_run_session(&menus, root);
```

La scelta va fissata nell’API, non lasciata ambigua nei vari `main`.

## 9.3 `cin_clear()`

Non chiamare automaticamente:

```cpp
cin_clear();
```

prima della procedura.

Il vecchio menu usava:

```cpp
std::cin >> choice;
```

e lasciava il newline nel buffer. Il menu GNPORT consuma gli eventi tramite `read()` e il decoder, poi chiudendo il terminale svuota i propri buffer interni.

La prima procedura che utilizza `std::getline()` è il test decisivo.

## 9.4 `wait_return_to_menu(true)`

Nell’implementazione corrente `true` significa bypass dell’attesa. Mantenerlo così nel primo porting evita di cambiare contemporaneamente la UX delle procedure.

In un secondo momento è opportuno sostituire il doppio negativo con un enum o una funzione dal nome esplicito:

```cpp
enum class ReturnPolicy {
    immediate,
    wait_for_enter
};
```

## 9.5 Checkpoint

Eseguire almeno:

- una procedura senza input;
- una con `std::cin >> numero`;
- una con `std::getline()`;
- una che esegue più loop;
- `ode_lotkav`;
- `exit`;
- una action intenzionalmente assente dal registry.

Commit suggerito:

```text
Replace FCN numeric menu loop with GNPORT dispatch
```

---

# 10. Fase 6 — Raggruppare automaticamente per prefisso

Questa fase usa ancora il file piatto corrente. Il nesting viene costruito in memoria.

## 10.1 Regola generale

Per una action contenente `_`:

```cpp
prefix = action.substr(0, action.find('_'));
```

Esempi:

| Action | Prefisso | Sottomenu |
|---|---|---|
| `f1_sample` | `f1` | Laboratorio F1 |
| `f2_integral` | `f2` | Laboratorio F2 |
| `f3_svd_pca` | `f3` | SVD e PCA |
| `sym_cauchy` | `sym` | Calcolo simbolico |
| `ode_lotkav` | `ode` | ODE |
| `al_qr` | `al` | Test e sviluppo |

Azioni speciali:

| Action | Trattamento |
|---|---|
| `exit` | resta nel menu principale |
| `sandbox` | gruppo `test` |
| prefisso sconosciuto | gruppo `test` oppure errore, a scelta configurabile |

Per FCN la mappa predefinita è:

```cpp
struct GnMenuPrefixRule {
    std::string prefix;
    std::string menu_id;
    std::string label;
    std::string title;
};
```

```cpp
const std::vector<GnMenuPrefixRule> rules = {
    {"f1",  "f1",   "F1 - Campionamento, Taylor e precisione",
                    "FCN - Laboratorio F1"},

    {"f2",  "f2",   "F2 - Derivate, problemi e integrazione",
                    "FCN - Laboratorio F2"},

    {"f3",  "f3",   "F3 - SVD e PCA",
                    "FCN - Laboratorio F3 / SVD e PCA"},

    {"sym", "sym",  "SYM - Calcolo simbolico",
                    "FCN - Calcolo simbolico"},

    {"ode", "ode",  "ODE - Equazioni differenziali",
                    "FCN - Equazioni differenziali"},

    {"al",  "test", "Test, algebra lineare e sviluppo",
                    "FCN - Test e sviluppo"}
};
```

## 10.2 API proposta

```cpp
bool gn_menu_group_by_action_prefix(
    const GnMenuConfig& flat_menu,
    const std::vector<GnMenuPrefixRule>& rules,
    GnMenuRepository* repository,
    GnIoError* error
);
```

Oppure un caricatore specializzato:

```cpp
bool gn_menu_load_fcn_json(
    const std::string& filename,
    GnMenuRepository* repository,
    GnIoError* error
);
```

Il primo approccio è più generale e testabile; il secondo può essere un wrapper dell’applicazione FCN, non della libreria generale.

## 10.3 Algoritmo

```text
crea menu root
per ogni regola crea il menu figlio vuoto

per ogni item del menu piatto:
    se action == exit:
        aggiungilo al root
    altrimenti:
        calcola il prefisso
        trova la regola
        aggiungi l’item al menu figlio corrispondente

per ogni menu figlio non vuoto:
    aggiungi al root una voce submenu
```

Le chiavi originali delle foglie vengono conservate. Le chiavi del primo livello possono essere assegnate in sequenza da 1 a 6.

## 10.4 `enabled`

Una voce padre è abilitata quando contiene almeno una voce figlia abilitata:

```cpp
parent.enabled = std::any_of(
    child.items.begin(),
    child.items.end(),
    [](const GnMenuItemConfig& item) {
        return item.enabled;
    }
);
```

## 10.5 Layout risultante

Menu principale:

```text
layout  = GN_MENU_PAGED_COLUMNS
rows    = 4
columns = 2
```

Capacità: 8 voci, sufficienti per sei gruppi più `Esci`.

Sottomenu:

```text
layout  = GN_MENU_PROGRAM_LIST
rows    = 8
columns = 1
```

Il gruppo F2 contiene sei voci; nessun sottomenu corrente richiede paginazione.

## 10.6 Checkpoint

Senza modificare `menu.json`, verificare:

- sette voci al primo livello;
- Escape dal figlio torna al padre;
- la voce selezionata del padre resta memorizzata;
- ogni action raggiunge la stessa funzione;
- le chiavi originali delle foglie restano disponibili;
- `exit` resta al primo livello;
- `sandbox` compare in Test e sviluppo.

Commit suggerito:

```text
Build two-level FCN menus from action prefixes
```

---

# 11. Fase 7 — Rendere esplicito il JSON annidato

Dopo aver verificato il raggruppamento automatico, il file può essere promosso a formato repository:

```json
{
  "version": 1,
  "root": "main",
  "menus": [
    {
      "id": "main",
      "title": "FCN - Funzioni Calcolo Numerico",
      "layout": "paged_columns",
      "rows": 4,
      "columns": 2,
      "items": [
        {
          "key": 1,
          "label": "F1 - Campionamento, Taylor e precisione",
          "submenu": "f1",
          "enabled": true
        },
        {
          "key": 0,
          "label": "Esci",
          "action": "exit",
          "enabled": true
        }
      ]
    },
    {
      "id": "f1",
      "title": "FCN - Laboratorio F1",
      "layout": "program_list",
      "rows": 8,
      "columns": 1,
      "items": [
        {
          "key": 1,
          "label": "F1-E1 Campionamento di funzione Cos",
          "action": "f1_sample",
          "enabled": true
        }
      ]
    }
  ]
}
```

## 11.1 Validazione

Ogni item selezionabile deve contenere **esattamente uno** tra:

```text
action
submenu
```

Errore quando:

- entrambi sono presenti;
- entrambi mancano;
- il `submenu` non esiste;
- due menu hanno lo stesso `id`;
- manca il menu indicato da `root`;
- due voci dello stesso menu hanno la stessa `key`, se la selezione numerica viene conservata;
- `rows <= 0` o `columns <= 0`.

## 11.2 Retrocompatibilità

Il loader generale può accettare entrambi i formati:

```text
forma piatta:
    title + items
    → repository implicito con root

forma repository:
    version + root + menus
    → nesting esplicito
```

Il raggruppamento per prefisso resta utile come:

- migrazione automatica;
- fallback;
- test di coerenza;
- generatore iniziale del nuovo file.

Commit suggerito:

```text
Adopt explicit nested FCN menu repository JSON
```

---

# 12. Tabella delle responsabilità dopo il porting

| Componente | Responsabilità |
|---|---|
| `menu.json` | struttura, label, action, enabled, nesting |
| `gn_fs` | lettura del file |
| `gn_menu_io` | parsing e validazione JSON |
| `GnMenuRepository` | proprietà dei menu e delle stringhe |
| `gn_menu_run` | rendering e navigazione |
| `gn_menu_run_session` | possesso temporaneo del terminale |
| `ActionRegistry` | associazione action → funzione |
| `main` | ciclo applicativo, `exit`, errori di dispatch |
| procedure FCN | calcolo numerico e input canonico |

Né il JSON né la libreria GNPORT devono includere puntatori alle procedure numeriche.

---

# 13. Errori e diagnostica

Distinguere almeno:

```text
file non trovato
JSON non valido
schema menu non valido
sottomenu inesistente
action non registrata
errore del terminale
```

Il loader segnala errori strutturali prima di entrare nel loop.

L’action non registrata resta un errore applicativo non fatale:

```cpp
const auto it = actions.find(result.action);

if (it == actions.end()) {
    std::cout
        << "Azione non registrata nel programma: "
        << result.action
        << '\n';

    continue;
}
```

È utile aggiungere una verifica preventiva:

```cpp
bool validate_action_registry(
    const GnMenuRepository& menus,
    const ActionRegistry& actions,
    std::vector<std::string>* missing
);
```

`exit` viene esclusa dalla verifica perché è gestita direttamente dal `main`.

Questa funzione avrebbe individuato immediatamente la temporanea assenza di `ode_lotkav` dal registry.

---

# 14. Matrice minima di test

## 14.1 Loader

- file esistente;
- file assente;
- JSON sintatticamente errato;
- `items` assente;
- `action` assente;
- voce disabilitata;
- action sconosciuta.

## 14.2 Menu

- frecce verticali;
- frecce orizzontali;
- Home/End;
- PgUp/PgDn;
- shortcut;
- Enter;
- Escape al figlio;
- Escape alla radice;
- ritorno alla precedente selezione del padre;
- voce disabilitata saltata;
- più di 16 voci nel menu piatto.

## 14.3 Confine terminale/FCN

- `std::cin >> int` dopo il menu;
- `std::getline()` come primo input;
- echo visibile;
- Backspace normale;
- Ctrl-C normale;
- ritorno ripetuto menu → procedura → menu;
- uscita durante una procedura e ripristino terminale.

## 14.4 Dispatch

- tutte le action presenti nel registry;
- `exit` gestita dal main;
- action assente segnalata senza crash;
- `enabled == false` mai invocata.

---

# 15. Ponte verso la sostituzione dei `cin` finali con `entry`

Il porting del menu deve essere completato prima di iniziare questa seconda fase.

Dopo il menu, ogni procedura FCN continuerà inizialmente a lavorare in modalità canonica.

Per sostituire gradualmente un prompt finale con `entry`, usare lo stesso principio della sessione menu:

```text
procedura numerica in stdio canonico
        ↓
serve chiedere nuovi parametri
        ↓
gn_entry_run_session(...)
    apre terminale non canonico
    mostra form
    usa gn_entry_field
    chiude terminale
        ↓
procedura riprende in stdio canonico
```

API futura indicativa:

```cpp
GnEntryFormResult gn_entry_run_session(
    const GnEntryFormSpec* form,
    GnEntryFormValues* values
);
```

Questo permette di convertire una funzione per volta senza:

- riscrivere subito tutti i suoi input;
- mantenere il terminale raw durante il calcolo;
- mescolare `std::cin` e `gn_keyboard_poll()` nello stesso intervallo.

## Ordine consigliato

1. menu GNPORT completo;
2. una procedura FCN semplice;
3. sostituzione del solo prompt “eseguire un nuovo loop?”;
4. sostituzione dei parametri del loop;
5. generalizzazione in un piccolo form;
6. applicazione alle altre procedure.

---

# 16. Cose deliberatamente escluse da questa migrazione

Non implementare ora:

- database e FABS;
- record store a slot;
- palette configurabile;
- form caricati da file;
- routing generale/formato/stazione;
- drive multiuser;
- split musica/pubblicità;
- line browser;
- menubar stile Borland;
- esportazione.

Il solo formato esterno necessario in questa fase è `menu.json`.

---

# 17. Checklist operativa finale

```text
[ ] Importare GNPORT e compilare FCN invariato
[ ] Aggiungere gn_menu_run_session
[ ] Verificare raw → canonical con una funzione std::cin
[ ] Introdurre GnMenuRepository proprietario
[ ] Cambiare GnMenuResult da action int ad action stringa
[ ] Caricare l’attuale menu.json piatto
[ ] Sostituire il loop numerico del main
[ ] Eliminare il cin_clear precedente alla action
[ ] Verificare tutte le procedure FCN
[ ] Validare menu contro ActionRegistry
[ ] Raggruppare in memoria per prefisso action
[ ] Verificare nesting ed Escape
[ ] Salvare il nuovo JSON annidato esplicito
[ ] Solo dopo iniziare la migrazione dei prompt a entry
```

---

# 18. Esito atteso

Al termine della migrazione il `main` FCN deve essere concettualmente ridotto a:

```cpp
int main()
{
    GnMenuRepository menus{};
    GnIoError error{};

    if (!gn_menu_load_json(
            "menu.json", &menus, &error)) {
        std::cerr << error.message << '\n';
        return EXIT_FAILURE;
    }

    const ActionRegistry actions =
        build_action_registry();

    for (;;) {
        const GnMenuResult result =
            gn_menu_run_session(&menus);

        if (result.reason == GN_MENU_CANCEL ||
            result.action == "exit") {
            break;
        }

        const auto it = actions.find(result.action);

        if (it == actions.end()) {
            std::cout
                << "Azione non registrata: "
                << result.action
                << '\n';
            continue;
        }

        it->second();
        wait_return_to_menu(true);
        clear_screen();
    }

    return EXIT_SUCCESS;
}
```

Il menu possiede temporaneamente il terminale; FCN possiede le procedure e il registry; il JSON possiede la struttura navigabile.
