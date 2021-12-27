
#pragma once
#include "pch.hpp"

#if _WIN32
#define DEFAULT_PATH "C:\\"
#else
#define DEFAULT_PATH "/tmp"
#endif

static void openNotification() {
    // Notification
    pfd::notify("Important Notification",
                "This is ' a message, pay \" attention \\ to it!",
                pfd::icon::info);
}

static void openMessage() {
    // Message box with nice message
    auto m = pfd::message(
        "Personal Message",
        "You are an amazing person, don’t let anyone make you think otherwise.",
        pfd::choice::yes_no_cancel, pfd::icon::warning);

    // Do something according to the selected button
    switch (m.result()) {
        case pfd::button::yes:
            std::cout << "User agreed.\n";
            break;
        case pfd::button::no:
            std::cout << "User disagreed.\n";
            break;
        case pfd::button::cancel:
            std::cout << "User freaked out.\n";
            break;
        default:
            break;  // Should not happen
    }
}

static std::vector<std::string> getFilesFromUser(
    const char* startPath = DEFAULT_PATH, const char* title = "Choose files",
    std::vector<std::string> filters = {"All Files", "*"},
    bool multiselect = false) {
    pfd::opt choice_option =
        multiselect ? pfd::opt::multiselect : pfd::opt::none;

    auto dialog = pfd::open_file(title, startPath, filters, choice_option);
    return dialog.result();
}

static void directorySelect() {
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
