
#include "renderer.h"

#include "pch.hpp"

Renderer::Statistics Renderer::stats;
Renderer::SceneData* Renderer::sceneData = new Renderer::SceneData;

INCBIN(char, flat_shader, "./resources/shaders/flat.glsl");
INCBIN(char, line_shader, "./resources/shaders/line.glsl");
INCBIN(char, poly_shader, "./resources/shaders/poly.glsl");
INCBIN(char, texture_shader, "./resources/shaders/texture.glsl");

void Renderer::init_default_shaders() {
    Renderer::sceneData->shaderLibrary.load_binary("flat", g_flat_shader_data,
                                                   g_flat_shader_size);
    Renderer::sceneData->shaderLibrary.load_binary(
        "texture", g_texture_shader_data, g_texture_shader_size);
    Renderer::sceneData->shaderLibrary.load_binary("line", g_line_shader_data,
                                                   g_line_shader_size);
    Renderer::sceneData->shaderLibrary.load_binary("poly", g_poly_shader_data,
                                                   g_poly_shader_size);
}

