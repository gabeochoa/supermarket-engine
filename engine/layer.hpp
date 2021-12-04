
#pragma once

#include <vector>

#include "pch.hpp"
#include "window.h"

struct Layer {
    Layer(const std::string& name = "layer") {}
    virtual ~Layer() {}
    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate() {}
    virtual void onEvent(Event& event) {}

    const std::string& getname() const { return name; }
    std::string name;
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

