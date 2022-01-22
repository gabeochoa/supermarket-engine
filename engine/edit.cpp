

#include "edit.h"

#include <functional>

#include "commands.h"

struct ExitCommand {
    std::string operator()(const std::vector<std::string>&) {
        auto r_ptr = GLOBALS.get_ptr<bool>("__engine__app_running");
        *r_ptr = false;
        return "see ya :)";
    }
};

void EditorCommands::init_default_commands() {
    // Register a bunch of commands that arent custom
    registerCommand("toggle_bool", ToggleBoolCommand<bool>(),
                    "Flip a bool's value; toggle_bool <varname>");
    registerCommand(
        "edit_float", SetValueCommand<float>(),
        "Set a float to value passed in; edit_float <varname> <value>");
    registerCommand("inc_float", IncrementValueCommand<float>(),
                    "Increment float by value; inc_float <varname> <value>");
    registerCommand("exit", ExitCommand(), "force quit the app");
    // TODO write / read from history files to keep commands from last run

    // intrinsic editor commands
    commandtrie.add("help");
    help["help"] = "<3";
    commandtrie.add("globals");
    help["globals"] = "Prints all registered globals";
    commandtrie.add("clear");
    help["clear"] = "Clears screen";
}

void EditorCommands::registerCommand(std::string name, ActionFuncType cmd,
                                     std::string help_str) {
    if (commands.find(name) != commands.end()) {
        log_warn(
            "Failed to add command, action with name {} "
            "already exists",
            name);
        return;
    }

    // log_info("Adding command \"{}\" to our library", name);
    commands[name] = cmd;
    help[name] = help_str;

    commandtrie.add(name);
}

void test_global_commands() {
    // create a value to use in global
    float value = 12.f;
    // register our float's ptr and name
    GLOBALS.set<float>("test_float", &value);
    // make sure the value is set correctly
    M_ASSERT(GLOBALS.get<float>("test_float") == 12.f,
             "Globals should have the same value we just put it");
    // increment the value outside the system
    value += 1.f;
    // new value to match :)
    M_ASSERT(value == 13.f,
             "Globals should have 13 since it should be keeping track "
             "of the ptr and not the value");

    // register a new command to edit our float
    EDITOR_COMMANDS.registerCommand("test_edit_float", SetValueCommand<float>(),
                                    "TEST");
    M_ASSERT(EDITOR_COMMANDS.output_history.size() == 0,
             "Output should be empty");
    M_ASSERT(EDITOR_COMMANDS.command_history.size() == 0,
             "No commands have been run yet");

    // Trigger a valid command
    EDITOR_COMMANDS.triggerCommand("test_edit_float test_float 15.f");
    // Make sure the value is correct (cmd successful)
    M_ASSERT(value == 15.f, "Value should be 15 at this point");

    M_ASSERT(EDITOR_COMMANDS.output_history.size() == 1,
             "Output should have the result of our last command");
    M_ASSERT(EDITOR_COMMANDS.command_history.size() == 1,
             "We have only run one command");

    // Trigger an invalid command (not enought params)
    EDITOR_COMMANDS.triggerCommand("test_edit_float test_float");
    // invalid, too many params
    EDITOR_COMMANDS.triggerCommand("test_edit_float test_float 15.f 16.f");
    // Make sure the value is correct (cmds failed correctly)
    M_ASSERT(value == 15.f, "Value should be 15 at this point");

    M_ASSERT(EDITOR_COMMANDS.output_history.size() == 3,
             "Output should have the result of our last command");
    M_ASSERT(EDITOR_COMMANDS.command_history.size() == 3,
             "We have only run one command");

    EDITOR_COMMANDS.reset();
}
