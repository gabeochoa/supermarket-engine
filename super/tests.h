
#pragma once

void theta_test() {
    // Walk straight through
    //  - i expect it to just walk around
    {
        auto shelf = std::make_shared<Shelf>(
            glm::vec2{1.f, 0.f}, glm::vec2{1.f, 1.f}, 0.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "box");
        entities.push_back(shelf);

        auto shelf2 = std::make_shared<Shelf>(
            glm::vec2{3.f, 0.f}, glm::vec2{1.f, 1.f}, 0.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "box");
        entities.push_back(shelf2);

        glm::vec2 start = {0.f, 0.f};
        glm::vec2 end = {6.f, 0.f};

        auto emp = Employee();
        emp.position = start;
        emp.size = {0.6f, 0.6f};
        entities.push_back(std::make_shared<Employee>(emp));

        LazyTheta t(start, end,
                    std::bind(isWalkable, emp.size, std::placeholders::_1));
        auto result = t.go();
        std::reverse(result.begin(), result.end());
        for (auto i : result) {
            log_info(fmt::format("{}", i));
        }
        M_ASSERT(result.size(), "Path is empty but shouldnt be");
        entities.clear();
    }
    return;
}

void point_collision_test() {
    auto shelf =
        std::make_shared<Shelf>(glm::vec2{0.f, 0.f}, glm::vec2{2.f, 2.f}, 0.f,
                                glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "box");

    M_ASSERT(shelf->pointCollides(glm::vec2{0.f, 0.f}) == true, "00");
    M_ASSERT(shelf->pointCollides(glm::vec2{1.f, 0.f}) == true, "10");
    M_ASSERT(shelf->pointCollides(glm::vec2{0.f, 1.f}) == true, "01");
    M_ASSERT(shelf->pointCollides(glm::vec2{1.f, 1.f}) == true, "11");
    M_ASSERT(shelf->pointCollides(glm::vec2{3.f, 3.f}) == false, "33");
    M_ASSERT(shelf->pointCollides(glm::vec2{01.f, 0.f}) == true, "010");
    M_ASSERT(shelf->pointCollides(glm::vec2{1.9f, 0.f}) == true, "110");
    M_ASSERT(shelf->pointCollides(glm::vec2{01.f, 1.f}) == true, "011");
    M_ASSERT(shelf->pointCollides(glm::vec2{1.1f, 1.f}) == true, "111");
    M_ASSERT(shelf->pointCollides(glm::vec2{2.0001f, 3.f}) == false, "200013");
}

