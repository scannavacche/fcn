#include "fcn_lib_jl_files.h"

#include <atomic>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <utility>

#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#endif

namespace jl_files {
namespace {
std::atomic<unsigned long long> next_id{0};

bool valid_basename(std::string_view name) {
    if (name.empty() || name.size() > 128 || name == "." || name == "..") return false;
    if (name.back() == '.') return false;
    for (unsigned char c : name) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'))
            return false;
    }
    // Nomi riservati su Windows, anche con estensione.
    std::string prefix;
    for (char c : name) {
        if (c == '.') break;
        prefix.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (prefix == "CON" || prefix == "PRN" || prefix == "AUX" || prefix == "NUL") return false;
    if (prefix.size() == 4 && (prefix.substr(0, 3) == "COM" || prefix.substr(0, 3) == "LPT") &&
        prefix[3] >= '1' && prefix[3] <= '9') return false;
    return true;
}

std::string checked(std::string_view name) {
    if (!valid_basename(name))
        throw std::invalid_argument("JL files: nome file non valido (solo basename ASCII): " + std::string{name});
    return std::string{name};
}

std::string safe_segment(std::string_view input, std::string_view fallback) {
    std::string out;
    for (unsigned char c : input) {
        const bool good = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                          (c >= '0' && c <= '9') || c == '-' || c == '_';
        const char chosen = good ? static_cast<char>(c) : '_';
        if (chosen != '_' || out.empty() || out.back() != '_') out.push_back(chosen);
        if (out.size() == 60) break;
    }
    while (!out.empty() && out.back() == '_') out.pop_back();
    if (out.empty()) out = std::string(fallback);
    if (!valid_basename(out)) out += '_';
    return out;
}

std::tm local_time(std::time_t t) {
    std::tm result{};
#ifdef _WIN32
    if (localtime_s(&result, &t) != 0) throw std::runtime_error("JL files: localtime_s fallita");
#else
    if (!localtime_r(&t, &result)) throw std::runtime_error("JL files: localtime_r fallita");
#endif
    return result;
}

void create_dirs(const fs::path& dir) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) throw fs::filesystem_error("JL files: creazione directory", dir, ec);
    if (!fs::is_directory(dir)) throw std::runtime_error("JL files: il percorso non e' una directory: " + dir.string());
}

std::string fixed_int(int x, int width) {
    std::ostringstream s;
    s << std::setw(width) << std::setfill('0') << x;
    return s.str();
}

void replace_file(const fs::path& src, const fs::path& dest) {
#ifdef _WIN32
    if (!MoveFileExW(src.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("JL files: MoveFileExW fallita, codice " + std::to_string(GetLastError()));
#else
    fs::rename(src, dest);
#endif
}
}

Run::Run(fs::path dir, std::string id, std::string campaign, std::string action)
    : directory_(std::move(dir)), id_(std::move(id)),
      campaign_(std::move(campaign)), action_(std::move(action)) {}

fs::path Run::result_path(std::string_view filename) const {
    return directory_ / checked(filename);
}

std::ofstream Run::open_new(std::string_view filename, bool binary) const {
    const fs::path p = result_path(filename);
    // Controllo utile contro riutilizzo accidentale; ogni Run ha gia'
    // una directory nuova esclusiva. L'apertura C++17 non e' O_EXCL.
    if (fs::exists(p)) throw std::runtime_error("JL files: risultato gia' esistente: " + p.string());
    std::ofstream out(p, std::ios::out | (binary ? std::ios::binary : std::ios::openmode{}));
    if (!out) throw std::runtime_error("JL files: impossibile creare: " + p.string());
    return out;
}

Store::Store(fs::path root) {
    if (root.empty()) throw std::invalid_argument("JL files: root vuota");
    // Una CWD modificata successivamente non sposta i nostri archivi.
    root_ = fs::absolute(root).lexically_normal();
}

fs::path Store::state_dir() const { return root_ / "state"; }
fs::path Store::cache_dir() const { return state_dir() / "cache"; }
fs::path Store::results_dir() const { return root_ / "results"; }

void Store::prepare_state() const {
    create_dirs(cache_dir()); // crea anche state/
}

fs::path Store::state_path(std::string_view filename) const {
    return state_dir() / checked(filename);
}
fs::path Store::cache_path(std::string_view filename) const {
    return cache_dir() / checked(filename);
}

std::optional<std::string> Store::read_state_text(std::string_view filename) const {
    const fs::path p = state_path(filename);
    std::ifstream input(p, std::ios::binary);
    if (!input) {
        if (!fs::exists(p)) return std::nullopt;
        throw std::runtime_error("JL files: impossibile leggere stato: " + p.string());
    }
    std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (input.bad()) throw std::runtime_error("JL files: errore lettura stato: " + p.string());
    return data;
}

void Store::write_state_text(std::string_view filename, std::string_view content) const {
    const fs::path target = state_path(filename);
    create_dirs(state_dir());
    // Un file temporaneo nuovo, nella STESSA directory del target.
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const std::string temp_name = "." + checked(filename) + ".tmp_" +
        std::to_string(stamp) + "_" + std::to_string(next_id.fetch_add(1));
    const fs::path temp = state_dir() / temp_name;
    try {
        {
            std::ofstream out(temp, std::ios::binary | std::ios::out);
            if (!out) throw std::runtime_error("JL files: apertura temporaneo fallita: " + temp.string());
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
            out.flush();
            if (!out) throw std::runtime_error("JL files: scrittura stato incompleta: " + temp.string());
            out.close();
            if (!out) throw std::runtime_error("JL files: chiusura stato fallita: " + temp.string());
        }
        replace_file(temp, target);
    } catch (...) {
        std::error_code ignored;
        fs::remove(temp, ignored);
        throw;
    }
}

Run Store::begin_run(std::string_view campaign, std::string_view action) const {
    const std::string camp = safe_segment(campaign, "campagna");
    const std::string act = safe_segment(action, "JL");
    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    const std::tm tm = local_time(t);
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    const fs::path parent = results_dir() /
        fixed_int(tm.tm_year + 1900, 4) /
        fixed_int(tm.tm_mon + 1, 2) /
        fixed_int(tm.tm_mday, 2) / camp;
    create_dirs(parent);
    const std::string base = fixed_int(tm.tm_hour, 2) + fixed_int(tm.tm_min, 2) +
        fixed_int(tm.tm_sec, 2) + "_" + fixed_int(static_cast<int>(us % 1000000), 6) + "_" + act;
    // create_directory e' operazione di riserva della cartella finale.
    // Nessun controllo su numero/elenco dei prodotti gia' presenti.
    for (int retry = 0; retry < 100; ++retry) {
        const std::string id = base + "_" + std::to_string(next_id.fetch_add(1));
        const fs::path dir = parent / id;
        std::error_code ec;
        if (fs::create_directory(dir, ec)) return Run(dir, id, camp, act);
        if (ec) throw fs::filesystem_error("JL files: creazione run", dir, ec);
    }
    throw std::runtime_error("JL files: impossibile riservare directory run");
}

} // namespace jl_files
