

#pragma once

#include <queue>
#include <set>

#include "pch.hpp"

struct ThetaPQ {
    typedef std::pair<glm::vec2, double> Qi;
    std::vector<Qi> Q;

    bool empty() { return Q.empty(); }

    Qi pop() {
        Qi i = *(Q.rbegin());
        Q.pop_back();
        return i;
    }

    void add(glm::vec2 loc, double prio) {
        for (auto& i : Q) {
            if (i.first == loc && prio < i.second) {
                i.second = prio;
                return;
            }
        }
        Q.push_back({loc, prio});
        sort(Q.begin(), Q.end(),
             [](const Qi& a, const Qi& b) { return a.second > b.second; });
    }

    bool contains(glm::vec2 location) {
        for (auto& i : Q) {
            if (i.first == location) return true;
        }
        return false;
    }

    void erase(glm::vec2 location) {
        for (auto it = Q.begin(); it != Q.end();) {
            if (it->first == location) {
                it = Q.erase(it);
            } else {
                ++it;
            }
        }
    }
};

struct Theta {
    const float EQUAL_RANGE = 0.25f;
    // TODO replace with float max?
    const float maxnum = 99999.f;
    const float x[8] = {0, 0, 1, -1, -1, 1, -1, 1};
    const float y[8] = {1, -1, 0, 0, -1, -1, 1, 1};
    const float dst = 0.25f;

    glm::vec2 start;
    glm::vec2 end;
    std::function<bool(const glm::vec2& pos)> isWalkable;

    std::unordered_map<glm::vec2, double, VectorHash> gScore;
    std::unordered_map<glm::vec2, glm::vec2, VectorHash> parent;
    // using a set as a priority queue
    // accessing the smallest element(use as min heap) *pq.begin();
    // accessing the largest element (use as max heap) *pq.rbegin();
    ThetaPQ openSet;
    ThetaPQ closedSet;

    Theta(const glm::vec2& s, const glm::vec2& e,
          std::function<bool(const glm::vec2& pos)> valid)
        : start(s), end(e), isWalkable(valid) {
        log_info(fmt::format("trying to find path from {} to {}", start, end));
        // gScore(node) is the current shortest distance from the start node to
        // node gScore(start) := 0;
        gScore[start] = 0;
        // parent(start) := start:
        // Initializing open and closed sets. The open set is initialized
        // with the start node and an initial cost
        // open := {};
        // open.insert(start, gScore(start) + heuristic(start, end));
        openSet.add(start, (gScore[start] + glm::distance(start, end)));
        // closed := {};
    }

    std::vector<glm::vec2> go() {
        while (!openSet.empty()) {
            ThetaPQ::Qi qi = openSet.pop();
            auto s = qi.first;
            // wow we got here already
            if (glm::distance(s, end) < EQUAL_RANGE) {
                return reconstruct_path(s);
            }
            closedSet.add(s, 0);
            // Loop through each immediate neighbor of s
            for (int i = 0; i < 8; i++) {
                glm::vec2 neighbor = {s.x + (x[i] * dst), s.y + (y[i] * dst)};
                if (!isWalkable(neighbor)) continue;
                // if (neighbor not in closed){
                if (!closedSet.contains(neighbor)) {
                    // if (neighbor not in open){
                    if (!openSet.contains(neighbor)) {
                        // Initialize values for neighbor if it is
                        // not already in the open list
                        // gScore(neighbor) := infinity;
                        // parent(neighbor) := Null;
                        gScore[neighbor] = maxnum;
                        // TODO do we need to set parent to null
                    }
                    update_vertex(s, neighbor);
                }
            }
        }
        log_warn("no path found to thing");
        return std::vector<glm::vec2>();
    }

    bool line_of_sight(const glm::vec2& parent, const glm::vec2& neighbor) {
        // technically we can see it but cant walk to it its in the wall?
        if (!isWalkable(neighbor)) {
            return false;
        }
        glm::vec2 loc(parent);
        while (distance(loc, neighbor) > 1.f) {
            loc = lerp(loc, neighbor, 0.25f);
            if (!isWalkable(loc)) {
                return false;
            }
        }
        return true;
    }

    void update_vertex(const glm::vec2& s, const glm::vec2& neighbor) {
        // This part of the algorithm is the main difference between A* and
        // Theta*
        if (line_of_sight(parent[s], neighbor)) {
            // If there is line-of-sight between parent(s) and neighbor
            // then ignore s and use the path from parent(s) to neighbor
            auto newPathScore =
                gScore[parent[s]] + glm::distance(parent[s], neighbor);
            if (newPathScore < gScore[neighbor]) {
                gScore[neighbor] = newPathScore;
                parent[neighbor] = parent[s];
                openSet.erase(neighbor);
                openSet.add(neighbor,
                            (gScore[neighbor] + glm::distance(neighbor, end)));
            }
        } else {
            // If the length of the path from start to s and from s to
            // neighbor is shorter than the shortest currently known distance
            // from start to neighbor, then update node with the new distance
            auto newPathScore = gScore[s] + glm::distance(s, neighbor);
            if (newPathScore < gScore[neighbor]) {
                gScore[neighbor] = newPathScore;
                parent[neighbor] = s;
                openSet.erase(neighbor);
                openSet.add(neighbor,
                            gScore[neighbor] + glm::distance(neighbor, end));
            }
        }
    }

    std::vector<glm::vec2> reconstruct_path(glm::vec2 cur) {
        std::vector<glm::vec2> path;
        path.push_back(cur);
        while (parent.find(cur) != parent.end() && cur != start) {
            cur = parent.at(cur);
            path.push_back(cur);
        }
        return path;
    }
};

