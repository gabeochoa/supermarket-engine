
#pragma once

#include <any>
#include <variant>

#include "external_include.h"

template <typename T>
struct SetValueCommand {
    void operator()(std::any a, std::any b) {
        set(std::any_cast<T*>(a), std::any_cast<T>(b));
    }

    void set(T* value, T nv) { *value = nv; }
};

struct EditorCommands {
    std::map<std::string, std::function<void(std::any, std::any)> > commands;

    void registerCommand(std::string name,
                         std::function<void(std::any, std::any)> cmd) {
        if (commands.find(name) != commands.end()) {
            log_warn(
                "Failed to add command, action with name {} "
                "already exists",
                name);
            return;
        }

        log_info("Adding command \"{}\" to our library", name);
        commands[name] = cmd;
    }

    template <class... Args>
    void triggerCommand(std::string name, Args... args) {
        commands[name](args...);
    }
};
static EditorCommands EDITOR_COMMANDS;

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
};
static GlobalValueRegister GLOBALS;

