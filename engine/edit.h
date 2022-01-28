
#pragma once

#include <sstream>

//
#include "globals.h"
//

#include "external_include.h"
#include "log.h"
#include "strutil.h"
#include "trie.h"

typedef std::function<std::string(std::vector<std::string>)> ActionFuncType;

constexpr int MAX_OUTPUT_HISTORY = 100;
constexpr int MAX_CMD_HISTORY = 100;
struct EditorCommands {
    std::deque<std::string> output_history;
    std::deque<std::string> command_history;
    std::map<std::string, ActionFuncType> commands;
    std::map<std::string, std::string> help;
    Trie commandtrie;

    EditorCommands() { init_default_commands(); }

    void init_default_commands();

    void registerCommand(std::string name, ActionFuncType cmd,
                         std::string help_str);

    void helpCommand() {
        addToOutputHistory("Help:");
        for (auto kv : help) {
            addToOutputHistory(fmt::format("{}", kv.first));
            addToOutputHistory(
                fmt::format("{0}{0}{0}{1}", "\t\t\t", kv.second));
        }
    }

    // TODO support prefix matching?
    void globalsCommand() {
        addToOutputHistory("Globals:");
        for (auto kv : GLOBALS.globals) {
            addToOutputHistory(fmt::format("{0}{0}{0}{1}", "\t\t\t", kv.first));
        }
    }

    std::vector<std::string> tabComplete(const std::string& prefix) {
        return commandtrie.dump(prefix);
    }

    bool processIntrinsicCommands(std::string command,
                                  std::vector<std::string>) {
        if (command == "help") {
            helpCommand();
            return true;
        }
        if (command == "globals") {
            globalsCommand();
            return true;
        }
        if (command == "clear") {
            // TODO this relies on knowing how tall the terminal is
            // and probably the resolution...
            // can we just do like \033[2J or something somehow

            for (int i = 0; i < 12; i++) addToOutputHistory(" ");
            return true;
        }

        return false;
    }

    void triggerCommand(std::string line) {
        addToCmdHistory(line);

        auto tokens = split(line, " ");

        // Validate Command
        std::string command = tokens[0];
        tokens.erase(tokens.begin());

        auto it = commands.find(command);
        if (it == commands.end()) {
            if (processIntrinsicCommands(command, tokens)) {
                return;
            }
            addToOutputHistory(fmt::format(
                "command run was not valid: \"{}\" (came from input \"{}\")",
                command, line));
            return;
        }
        // TODO if theres an error in parsing,
        // we should probably let the person know what we saw them run
        // similar to above ^^
        // Right now theres no way to know if the command was successful or not
        // because all we return is output string
        ActionFuncType aft = it->second;
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
void test_global_commands();
