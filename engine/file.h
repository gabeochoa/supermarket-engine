
#pragma once
#include <filesystem>

#include "pch.hpp"

#if _WIN32
#define DEFAULT_PATH "C:\\"
#else
#define DEFAULT_PATH "/tmp"
#endif

// Searches recursively to try to find filename matching
inline std::string find_absolute_path_recursively(
    const std::string& starting_folder, const std::string& filename) {
    if (!std::__fs::filesystem::exists(starting_folder)) return "";
    for (auto const& entry :
         std::__fs::filesystem::recursive_directory_iterator{starting_folder}) {
        if (entry.path().filename() == filename) {
            return entry.path().string();
        }
    }
    return "";
}

inline std::string get_absolute_path_to(const std::string& starting_folder,
                                        const std::string& path) {
    const auto p = std::__fs::filesystem::path(path);
    const std::string abs_path =
        find_absolute_path_recursively(starting_folder, p.filename().string());
    if (abs_path.empty()) {
        log_warn("Failed to find file {}", path);
        return "";
    }

    M_ASSERT(
        p.parent_path().filename().string() ==
            std::__fs::filesystem::path(abs_path)
                .parent_path()
                .filename()
                .string(),
        "Parent of path passed in, should match the parent of the found file");
    return abs_path;
}

__attribute__((unused))  // TODO use this or remove it
static void
openNotification() {
    // Notification
    pfd::notify("Important Notification",
                "This is ' a message, pay \" attention \\ to it!",
                pfd::icon::info);
}

// -1 error, 0 cancel, 1 yes, 2 no
__attribute__((unused))  // TODO use this or remove it
static int openMessage(const char* title, const char* msg,
                       pfd::choice choice_type = pfd::choice::yes_no_cancel,
                       pfd::icon icon_type = pfd::icon::info) {
    // Message box with nice message
    auto m = pfd::message(title, msg, choice_type, icon_type);

    // Do something according to the selected button
    switch (m.result()) {
        case pfd::button::yes:
            return 1;
            break;
        case pfd::button::no:
            return 2;
            break;
        case pfd::button::cancel:
            return 0;
            break;
        default:
            return -1;
            break;  // Should not happen
    }
    return -1;
}

__attribute__((unused))  // TODO use this or remove it
static std::vector<std::string>
getFilesFromUser(const char* startPath = DEFAULT_PATH,
                 const char* title = "Choose files",
                 std::vector<std::string> filters = {"All Files", "*"},
                 bool multiselect = false) {
    pfd::opt choice_option =
        multiselect ? pfd::opt::multiselect : pfd::opt::none;

    auto dialog = pfd::open_file(title, startPath, filters, choice_option);
    return dialog.result();
}

__attribute__((unused))  // TODO use this or remove it
static void
directorySelect() {
    // Check that a backend is available
    if (!pfd::settings::available()) {
        log_error("Portable File Dialogs are not available on this platform.");
        return;
    }

    // Directory selection
    auto dir =
        pfd::select_folder("Select any directory", DEFAULT_PATH).result();
    std::cout << "Selected dir: " << dir << "\n";
}

// NOTE: This is commented out since it has popups
// its great for a test example file
// but not really useful to run everytime i startup the program
//
// static void test_file_functionality() {
// // pfd::settings
// pfd::settings::verbose(true);
// pfd::settings::rescan();
//
// // pfd::notify
// pfd::notify("", "");
// pfd::notify("", "", pfd::icon::info);
// pfd::notify("", "", pfd::icon::warning);
// pfd::notify("", "", pfd::icon::error);
// pfd::notify("", "", pfd::icon::question);
//
// pfd::notify a("", "");
// (void)a.ready();
// (void)a.ready(42);
//
// // pfd::message
// pfd::message("", "");
// pfd::message("", "", pfd::choice::ok);
// pfd::message("", "", pfd::choice::ok_cancel);
// pfd::message("", "", pfd::choice::yes_no);
// pfd::message("", "", pfd::choice::yes_no_cancel);
// pfd::message("", "", pfd::choice::retry_cancel);
// pfd::message("", "", pfd::choice::abort_retry_ignore);
// pfd::message("", "", pfd::choice::ok, pfd::icon::info);
// pfd::message("", "", pfd::choice::ok, pfd::icon::warning);
// pfd::message("", "", pfd::choice::ok, pfd::icon::error);
// pfd::message("", "", pfd::choice::ok, pfd::icon::question);
//
// pfd::message b("", "");
// (void)b.ready();
// (void)b.ready(42);
// (void)b.result();
// }
