
#pragma once

#include <atomic>
#include <vector>

#include "pch.hpp"
#include "time.h"
#include "window.h"

static std::atomic_int s_layer_id;
struct Layer {
    int id;
    std::string name;
    bool isMinimized;

    Layer(const std::string& n = "layer")
        : id(s_layer_id++), name(n), isMinimized(false) {}
    virtual ~Layer() {}
    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate(Time elapsed) = 0;
    virtual void onEvent(Event& event) = 0;

    const std::string& getname() const { return name; }
};

struct LayerStack {
    LayerStack() { insert = layers.begin(); }
    ~LayerStack() {
        for (Layer* layer : layers) {
            delete layer;
        }
    }

    void push(Layer* layer) { insert = layers.emplace(insert, layer); }
    void pop(Layer* layer) {
        auto it = std::find(layers.begin(), layers.end(), layer);
        if (it != layers.end()) {
            layers.erase(it);
            insert--;
        }
    }

    void popOverlay(Layer* layer) { layers.emplace_back(layer); }
    void pushOverlay(Layer* layer) {
        auto it = std::find(layers.begin(), layers.end(), layer);
        if (it != layers.end()) {
            layers.erase(it);
        }
    }

    std::vector<Layer*> layers;
    std::vector<Layer*>::iterator insert;

    std::vector<Layer*>::iterator begin() { return layers.begin(); }
    std::vector<Layer*>::iterator end() { return layers.end(); }
};

