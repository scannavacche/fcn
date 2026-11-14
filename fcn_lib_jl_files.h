#ifndef FCN_LIB_JL_FILES_H
#define FCN_LIB_JL_FILES_H

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

// FCN Lab — archivio degli esperimenti JL, C++17, senza dipendenze GNPORT.
// Tutti i percorsi relativi sono risolti rispetto alla directory di lavoro
// del processo (o al root esplicito passato al costruttore).
namespace jl_files {
namespace fs = std::filesystem;

// Una cartella esclusiva per UNA esecuzione F10. Non si riaprono run passate.
class Run {
public:
    const fs::path& directory() const noexcept { return directory_; }
    const std::string& id() const noexcept { return id_; }
    const std::string& campaign() const noexcept { return campaign_; }
    const std::string& action() const noexcept { return action_; }

    // Percorso di un file dentro la directory di questa esecuzione.
    // Non crea il file. filename deve essere un basename sicuro.
    fs::path result_path(std::string_view filename) const;

    // Apre un file nuovo; rifiuta un nome gia' presente.
    // Per evitare collisioni fra esecuzioni, usare sempre un nuovo Run.
    std::ofstream open_new(std::string_view filename, bool binary = false) const;

private:
    friend class Store;
    Run(fs::path dir, std::string id, std::string campaign, std::string action);
    fs::path directory_;
    std::string id_, campaign_, action_;
};

class Store {
public:
    // Esempio: Store{"data"} => data/state e data/results.
    explicit Store(fs::path root = fs::path{"data"});
    const fs::path& root() const noexcept { return root_; }

    fs::path state_dir() const;
    fs::path cache_dir() const;
    fs::path results_dir() const;

    // Prepara soltanto stato e cache, senza creare alcun risultato.
    void prepare_state() const;
    fs::path state_path(std::string_view filename) const;
    fs::path cache_path(std::string_view filename) const;

    // Lettura facoltativa. nullopt = file assente (non errore I/O).
    std::optional<std::string> read_state_text(std::string_view filename) const;

    // Aggiornamento dello stato via file temporaneo + rename.
    // Su Linux rename sostituisce atomicamente la vecchia versione; su Windows
    // usa MoveFileExW con REPLACE_EXISTING (filesystem locale).
    void write_state_text(std::string_view filename, std::string_view content) const;

    // results/AAAA/MM/GG/<campagna>/<HHMMSS_microsec>_<azione>_<progressivo>/
    // La directory finale e' riservata con create_directory: nessuna scansione
    // dei file preesistenti, nessuna sovrascrittura, thread/process safe per i run.
    Run begin_run(std::string_view campaign, std::string_view action) const;

private:
    fs::path root_;
};

} // namespace jl_files
#endif
