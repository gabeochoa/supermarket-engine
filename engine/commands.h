
#pragma once

#include <any>

#include "camera.h"
#include "deserializer.h"
#include "external_include.h"
#include "globals.h"

template <typename T>
struct Command {
    std::string msg;
    void add(T* value, T nv) { *value = *value + nv; }
    void set(T* value, T nv) { *value = nv; }
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

// TODO idk if this is the best way to handle Structs
// basically Command<struct> and EditValue<struct>
// we lose access to add / set since those will be add<struct> instead of
// add<struct->value>
//
// we also have to manually tokens[0] == struct->var every time
// since c++ has no reflection
//
// Nice: we dont have to label every struct / create macros
// Bad: have to do it here in the command which is probably worse
//
//          Example use
// GLOBALS.set<OrthoCameraController>("gameUICameraController",
// gameUICameraController.get()); EDITOR_COMMANDS.registerCommand(
// "gameui_cam_set", SetValueCommand<OrthoCameraController>(), "Edit game UI
// camera controller settings");
//
template <>
struct EditValueCommand<OrthoCameraController>
    : Command<OrthoCameraController> {
    std::vector<std::any> convert(const std::vector<std::string>& tokens) {
        // Need to convert to T* and T
        std::vector<std::any> out;
        if (tokens.size() != 2) {
            this->msg = fmt::format("Invalid number of parameters {} wanted {}",
                                    tokens.size(), 2);
            return out;
        }
        // Normally we would put this in globals but too bad
        //
        OrthoCameraController* guicc =
            GLOBALS.get_ptr<OrthoCameraController>("gameUICameraController");

        if (tokens[0] == "movementEnabled") {
            bool* mvt = &(guicc->movementEnabled);
            out.push_back(mvt);
            bool val = Deserializer<bool>(tokens[1]);
            out.push_back(val);
            return out;
        }
        this->msg = "no matching supported camera controller variable";
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

template <>
struct SetValueCommand<OrthoCameraController>
    : public EditValueCommand<OrthoCameraController> {
    std::string operator()(const std::vector<std::string>& params) {
        auto values = this->convert(params);
        if (!values.empty()) {
            if (params[0] == "movementEnabled") {
                bool* mvt = std::any_cast<bool*>(values[0]);
                bool val = std::any_cast<bool>(values[1]);
                *mvt = val;
                this->msg = fmt::format("{} is now {}", params[0], val);
            }
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

