
#pragma once

#include <array>
#include <glm/glm.hpp>
//
#include "../../engine/renderer.h"
//
#include "colors.h"
#include "piece_data.h"

struct Piece {
    glm::vec2 position;
    glm::vec4 color;
    int type = 0;
    std::array<int, 16> piece;
    glm::vec4 bounds;
    int angle = 0;

    Piece() {}
    Piece(const Piece& other) { *this = other; }
    Piece& operator=(const Piece& other) {
        this->position = other.position;
        this->color = other.color;
        this->type = other.type;
        this->piece = other.piece;
        this->bounds = other.bounds;
        this->angle = other.angle;
        return *this;
    }

    void regen(glm::vec2 pos) {
        position = pos;
        type = rand() % 6;
        angle = 0;
        piece = type_to_rotated_array(type, angle);
        bounds = genBounds();
        color = piece_color(type);
    }

    glm::vec4 genBounds() {
        glm::vec4 results{5, 5, -1, -1};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (piece[j * 4 + i] == 1) {
                    results.x = fmin(results.x, i);
                    results.y = fmin(results.y, j);

                    results.z = fmax(results.z, i);
                    results.w = fmax(results.w, j);
                }
            }
        }
        return results;
    }

    bool collidesWithMap(int x, int y, std::array<int, 16> p) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (p[j * 4 + i] == 0) continue;
                if (grid[(y + j) * MAP_W + (x + i)].color.a < 0.4f) continue;
                return true;
            }
        }
        return false;
    }

    std::array<std::pair<int, int>, 4> getTests() {
        if (type == 0) {
            return long_boi_tests[angle];
        }
        return wall_kick_tests[angle];
    }

    void rotate() {
        // rotate the array
        std::array<int, 16> tmp = type_to_rotated_array(type, (angle + 1) % 4);

        // check if rotation fits,
        if (!collidesWithMap(position.x, position.y, tmp)) {
            // yep, lets change the angle and write to the piece
            angle = (angle + 1) % 4;
            piece = tmp;
            return;
        }

        // rotation didnt fit,
        // lets wall kick

        int x = position.x;
        int y = position.y;
        for (auto pair : getTests()) {
            if (collidesWithMap(x + pair.first, y + pair.second, tmp)) continue;
            // no collision we good
            position.x = x + pair.first;
            position.y = y + pair.second;
            angle = (angle + 1) % 4;
            piece = tmp;
            return;
        }
    }

    bool move(int a, int b) {
        int nx = position.x + a;
        int ny = position.y + b;
        if (collidesWithMap(nx, ny, piece)) return false;
        position.x = nx;
        position.y = ny;
        return true;
    }

    void render(bool ghost = false) {
        auto size = glm::vec2{TILESIZE};
        auto gridpos = position * (1.f * TILESIZE) + (TILESIZE * 0.1f);

        auto col = color;
        if (ghost) col.a = 0.3f;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (piece[j * 4 + i] == 1) {
                    Renderer::drawQuad(
                        (gridpos + (glm::vec2{i, j} * size)) + (size / 2.f),
                        size * 0.8f, col, "white");
                }
            }
        }
    }
};
