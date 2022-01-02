
#pragma once

#include <any>
#include <variant>

#include "external_include.h"
#include "log.h"
#include "strutil.h"

struct GlobalValueRegister {
    std::map<std::string, void*> globals;

    template <typename T>
    T* get_ptr(const std::string& name) {
        return (T*)(globals[name]);
    }

    template <typename T>
    T get(const std::string& name) {
        return *(get_ptr<T>(name));
    }
    template <typename T>
    void set(const std::string& name, T* value) {
        globals[name] = (void*)value;
    }

    bool contains(const std::string& name) {
        return globals.find(name) != globals.end();
    }
};
static GlobalValueRegister GLOBALS;

template <typename T>
struct Deserializer {
    std::string input;
    Deserializer(std::string i) : input(i) {}
    operator T() = 0;
};

template <>
struct Deserializer<float> {
    std::string input;
    Deserializer(std::string i) : input(i) {}
    operator float() { return ::atof(this->input.c_str()); }
};

template <typename T>
struct Command {
    std::string msg;
    void add(T* value, T nv) { *value = *value + nv; }
    void set(T* value, T nv) { *value = nv; }
};

struct ExitCommand {
    std::string operator()(const std::vector<std::string>&) {
        auto r_ptr = GLOBALS.get_ptr<bool>("__engine__app_running");
        *r_ptr = false;
        return "see ya :)";
    }
};

template <typename T>
struct EditValueCommand : Command<T> {
    std::vector<std::any> convert(const std::vector<std::string>& tokens) {
        // Need to convert to T* and T
        std::vector<std::any> out;
        if (tokens.size() != 2) {
            this->msg = fmt::format("Invalid number of parameters {} wanted {}",
                                    tokens.size(), 2);
            return out;
        }
        if (!GLOBALS.contains(tokens[0])) {
            this->msg = fmt::format(
                "Globals didnt have a matching variable, wanted {}", tokens[0]);
            return out;
        }
        T* value = GLOBALS.get_ptr<T>(tokens[0]);
        out.push_back(value);
        T newvalue = Deserializer<T>(tokens[1]);
        out.push_back(newvalue);
        return out;
    }
};

template <typename T>
struct SetValueCommand : public EditValueCommand<T> {
    std::string operator()(const std::vector<std::string>& params) {
        auto values = this->convert(params);
        if (!values.empty()) {
            T val = std::any_cast<T>(values[1]);
            this->set(std::any_cast<T*>(values[0]), val);
            this->msg = fmt::format("{} is now {}", params[0], val);
        }
        return this->msg;
    }
};

template <typename T>
struct IncrementValueCommand : public EditValueCommand<T> {
    std::string operator()(const std::vector<std::string>& params) {
        auto values = this->convert(params);
        if (!values.empty()) {
            T val = std::any_cast<T>(values[1]);
            this->add(std::any_cast<T*>(values[0]), val);
            this->msg = fmt::format("{} is now {}", params[0], val);
        }
        return this->msg;
    }
};

template <typename T>
struct ToggleBoolCommand : public Command<T> {
    std::vector<std::any> convert(const std::vector<std::string>& tokens) {
        // Need to convert to T* and T
        std::vector<std::any> out;
        if (tokens.size() != 1) {
            this->msg = fmt::format("Invalid number of parameters {} wanted {}",
                                    tokens.size(), 1);
            return out;
        }
        if (!GLOBALS.contains(tokens[0])) {
            this->msg = fmt::format(
                "Globals didnt have a matching variable, wanted {}", tokens[0]);
            return out;
        }
        T* value = GLOBALS.get_ptr<T>(tokens[0]);
        out.push_back(value);
        return out;
    }
    std::string operator()(const std::vector<std::string>& params) {
        auto values = this->convert(params);
        if (!values.empty()) {
            T val = !(*(std::any_cast<T*>(values[0])));
            this->set(std::any_cast<T*>(values[0]), val);
            this->msg = fmt::format("{} is now {}", params[0], val);
        }
        return this->msg;
    }
};

typedef std::function<std::string(std::vector<std::string>)> ActionFuncType;

constexpr int MAX_OUTPUT_HISTORY = 100;
constexpr int MAX_CMD_HISTORY = 100;
struct EditorCommands {
    std::deque<std::string> output_history;
    std::deque<std::string> command_history;
    std::map<std::string, ActionFuncType> commands;
    std::map<std::string, std::string> help;

    EditorCommands() { init_default_commands(); }

    void init_default_commands() {
        // Register a bunch of commands that arent custom
        registerCommand("toggle_bool", ToggleBoolCommand<bool>(),
                        "Flip a bool's value; toggle_bool <varname>");
        registerCommand(
            "edit_float", SetValueCommand<float>(),
            "Set a float to value passed in; edit_float <varname> <value>");
        registerCommand(
            "inc_float", IncrementValueCommand<float>(),
            "Increment float by value; inc_float <varname> <value>");
        registerCommand("exit", ExitCommand(), "force quit the app");
        // TODO write / read from history files to keep commands from last run
    }

    void registerCommand(std::string name, ActionFuncType cmd,
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
    }

    void helpCommand() {
        addToOutputHistory("Help:");
        for (auto kv : help) {
            addToOutputHistory(fmt::format("{}", kv.first));
            addToOutputHistory(fmt::format("\t{}", kv.second));
        }
    }

    void triggerCommand(std::string line) {
        addToCmdHistory(line);

        auto tokens = split(line, " ");

        // Validate Command
        std::string command = tokens[0];
        auto it = commands.find(command);
        if (it == commands.end()) {
            if (command == "help") {
                helpCommand();
                return;
            }
            // TODO output to terminal
            addToOutputHistory(fmt::format(
                "command run was not valid: {} (came from input {})", command,
                line));
            return;
        }
        ActionFuncType aft = it->second;
        tokens.erase(tokens.begin());
        std::string msg = aft(tokens);
        addToOutputHistory(msg);
    }

    void addToCmdHistory(std::string cmd) {
        command_history.push_back(cmd);
        if (command_history.size() > MAX_CMD_HISTORY) {
            command_history.pop_front();
        }
    }

    void addToOutputHistory(std::string msg) {
        if (msg.size() == 0) return;
        log_info("{}", msg);
        for (auto line : split(msg, "\n")) {
            output_history.push_back(line);
        }
        while (output_history.size() > MAX_OUTPUT_HISTORY) {
            output_history.pop_front();
        }
    }

    void reset() {
        commands.clear();
        help.clear();
        output_history.clear();
        command_history.clear();
        init_default_commands();
    }
};
static EditorCommands EDITOR_COMMANDS;

inline void test_global_commands() {
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
