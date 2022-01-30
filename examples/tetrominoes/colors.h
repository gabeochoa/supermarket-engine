
#pragma once
#include <glm/glm.hpp>

const glm::vec4 BLACK{0.2f};
const glm::vec4 GRAY{0.5, 0.5f, 0.5f, 1.f};
const glm::vec4 WHITE{1.f};
const glm::vec4 CYAN{0.f, 1.f, 1.f, 1.f};
const glm::vec4 YELLOW{1.f, 1.f, 0.f, 1.f};
const glm::vec4 PURPLE{0.5f, 0.f, 0.5f, 1.f};
const glm::vec4 GREEN{0.f, 1.f, 0.f, 1.f};
const glm::vec4 RED{1.f, 0.f, 0.f, 1.f};
const glm::vec4 BLUE{0.f, 0.f, 1.f, 1.f};
const glm::vec4 ORANGE{0.5f, 0.5f, 0.5f, 1.f};

inline glm::vec4 piece_color(int type) {
    if (type == 0) return CYAN;
    if (type == 1) return YELLOW;
    if (type == 2) return PURPLE;
    if (type == 3) return GREEN;
    if (type == 4) return RED;
    if (type == 5) return BLUE;
    if (type == 6) return ORANGE;
    return WHITE;
}

