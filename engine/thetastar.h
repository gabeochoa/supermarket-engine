

#pragma once

#include <limits>
#include <queue>
#include <set>

#include "pch.hpp"

///////// ///////// ///////// ///////// ///////// ///////// ///////// /////////
//
//
// How to use:
//
// Theta t(start, end, isWalkable);
// auto path = t.go();
//
// where isWalkable is a function that takes a glm::vec2 and returns whether
// or not the path can include it
//
///////// ///////// ///////// ///////// ///////// ///////// ///////// /////////

typedef std::pair<glm::vec2, double> Qi;
struct CompareQi {
    bool operator()(Qi a, Qi b) { return a.second > b.second; }
};
typedef std::priority_queue<Qi, std::vector<Qi>, CompareQi> ThetaPQ;

template <class T, class S, class C>
S& UnderlyingContainer(std::priority_queue<T, S, C>& q) {
    struct HackedQueue : private std::priority_queue<T, S, C> {
        static S& UnderlyingContainer(std::priority_queue<T, S, C>& q) {
            return q.*&HackedQueue::c;
        }
    };
    return HackedQueue::UnderlyingContainer(q);
}

struct Theta {
    struct Options {
        bool isLazy;
        bool moveDiagonally;
        float step;
        bool disableLineOfSight;
    } options;

    const float EQUAL_RANGE = 0.5f;
    const float maxnum = std::numeric_limits<float>::max();

    // Make sure the first 4 are the cardinal directions
    const float x[8] = {0, 0, 1, -1, -1, 1, -1, 1};
    const float y[8] = {1, -1, 0, 0, -1, -1, 1, 1};

    glm::vec2 start;
    glm::vec2 end;
    std::function<bool(const glm::vec2& pos)> canVisit;

    std::unordered_map<glm::vec2, double, VectorHash> gScore;
    std::unordered_map<glm::vec2, glm::vec2, VectorHash> parent;
    // using a set as a priority queue
    // accessing the smallest element(use as min heap) *pq.begin();
    // accessing the largest element (use as max heap) *pq.rbegin();
    ThetaPQ openSet;
    ThetaPQ closedSet;

    Theta(const glm::vec2 start, const glm::vec2 end,
          std::function<bool(const glm::vec2& pos)> isWalkable,
          Options options =
              {
                  .isLazy = false,
                  .moveDiagonally = false,
                  .disableLineOfSight = false,
                  .step = 1.f,
              })
        : options(options), start(start), end(end), canVisit(isWalkable) {
        log_trace("trying to find path from {} to {}", start, end);
        // if the goal isnt reachable, then we have to change the goal for now
        if (!canVisit(end)) {
            // uses the closedSet
            this->end = expandUntilWalkable(end);
            while (!closedSet.empty()) closedSet.pop();
            log_trace("couldnt get to end so end is now {}", end);
        } else {
            log_trace("goal is reachable {}", end);
        }

        // init vars
        gScore[start] = 0;
        openSet.push(
            Qi{start, (gScore[start] + glm::distance(start, this->end))});
    }

    glm::vec2 expandUntilWalkable(glm::vec2 n) {
        closedSet.push(Qi{n, 0});
        while (true) {
            Qi qi = closedSet.top();
            closedSet.pop();
            n = qi.first;
            int neighbors = options.moveDiagonally ? 8 : 4;
            for (int i = 0; i < neighbors; i++) {
                glm::vec2 neighbor = {n.x + (x[i] * options.step),
                                      n.y + (y[i] * options.step)};
                if (canVisit(neighbor)) return neighbor;
                closedSet.push(Qi{neighbor, qi.second + 1});
            }
        }
    }

    bool contains(ThetaPQ pq, glm::vec2 pos) {
        auto c = UnderlyingContainer(pq);
        for (auto it = c.begin(); it != c.end(); it++) {
            if (it->first == pos) return true;
        }
        return false;
    }

    void erase(ThetaPQ pq, glm::vec2 pos) {
        auto c = UnderlyingContainer(pq);
        for (auto it = c.begin(); it != c.end(); it++) {
            if (it->first == pos) {
                c.erase(it);
                return;
            }
        }
    }

    std::vector<glm::vec2> go() {
        prof give_me_a_name(__PROFILE_FUNC__);
        int neighbors = options.moveDiagonally ? 8 : 4;
        while (!openSet.empty()) {
            Qi qi = openSet.top();
            openSet.pop();
            auto s = qi.first;
            if (options.isLazy) {
                set_vertex(s);
            }
            // wow we got here already
            if (glm::distance(s, end) < (options.step / 2.f)) {
                return reconstruct_path(s);
            }
            closedSet.push(Qi{s, 0});
            // Loop through each immediate neighbor of s
            for (int i = 0; i < neighbors; i++) {
                glm::vec2 neighbor = {s.x + (x[i] * options.step),
                                      s.y + (y[i] * options.step)};
                if (!canVisit(neighbor)) continue;
                // if (neighbor not in closed){
                if (!contains(closedSet, neighbor)) {
                    // if (neighbor not in open){
                    if (!contains(openSet, neighbor)) {
                        // Initialize values for neighbor if it is
                        // not already in the open list
                        // gScore(neighbor) := infinity;
                        // parent(neighbor) := Null;
                        gScore[neighbor] = maxnum;
                        parent[neighbor] = {};
                    }
                    update_vertex(s, neighbor);
                }
            }
        }
        log_trace("no path found to thing");
        return std::vector<glm::vec2>();
    }

    glm::vec2 moveToward(const glm::vec2& me, const glm::vec2& you) {
        // already there
        if (you.y == me.y && you.x == me.x) return me;

        glm::vec2 next(me);
        bool movedInX = false;

        if (you.x != me.x) {
            movedInX = true;
            next.x += you.x > me.x ? options.step : -options.step;
        }

        // Can only move in one dir at a time
        if (movedInX && !options.moveDiagonally) return next;

        if (you.y != me.y) {
            next.y += you.y > me.y ? options.step : -options.step;
        }

        return next;
    }

    bool line_of_sight(const glm::vec2& parentNode, const glm::vec2& neighbor) {
        // technically we can see it but cant walk to it its in the wall?
        if (options.disableLineOfSight || !canVisit(neighbor)) {
            return false;
        }
        glm::vec2 loc(parentNode);
        float halfstep = options.step / 2.f;
        while (distance(loc, neighbor) > halfstep) {
            loc = moveToward(loc, neighbor);
            if (!canVisit(loc)) {
                return false;
            }
        }
        return true;
    }

    std::vector<glm::vec2> reconstruct_path(glm::vec2 e) {
        std::vector<glm::vec2> path;
        path.push_back(e);
        glm::vec2 cur = e;
        while (parent.find(cur) != parent.end() && cur != start) {
            path.push_back(cur);
            cur = parent.at(cur);
        }
        return path;
    }

    void set_vertex(const glm::vec2& s) {
        int neighbors = options.moveDiagonally ? 8 : 4;
        if (!line_of_sight(parent[s], s)) {
            glm::vec2 minN;
            double minS = -1;
            for (int i = 0; i < neighbors; i++) {
                glm::vec2 neighbor = {s.x + (x[i] * options.step),
                                      s.y + (y[i] * options.step)};
                if (!canVisit(neighbor)) continue;
                if (contains(closedSet, neighbor)) {
                    if (close(minS, -1) || minS > gScore[neighbor]) {
                        minN = neighbor;
                        minS = gScore[neighbor];
                    }
                }
            }
            parent[s] = minN;
            gScore[s] = minS;
        }
    }

    // This part is the main difference between A* and Theta*
    void update_vertex(const glm::vec2& s, const glm::vec2& neighbor) {
        double oldScore = gScore[neighbor];

        if (options.isLazy) {
            // If there is line-of-sight between parent(s) and neighbor
            // then ignore s and use the path from parent(s) to neighbor
            auto sparent = parent[s];
            auto newPathScore =
                gScore[sparent] + glm::distance(sparent, neighbor);
            if (newPathScore < gScore[neighbor]) {
                parent[neighbor] = parent[s];
                gScore[neighbor] = newPathScore;
            }
            if (gScore[neighbor] < oldScore) {
                erase(openSet, neighbor);
                openSet.push(Qi{neighbor, (gScore[neighbor] +
                                           glm::distance(neighbor, end))});
            }
            return;
        }

        // Non lazy version

        if (line_of_sight(parent[s], neighbor)) {
            // If there is line-of-sight between parent(s) and neighbor
            // then ignore s and use the path from parent(s) to neighbor
            auto sparent = parent[s];
            auto newPathScore =
                gScore[sparent] + glm::distance(sparent, neighbor);
            if (newPathScore < gScore[neighbor]) {
                parent[neighbor] = parent[s];
                gScore[neighbor] = newPathScore;
            }
            if (gScore[neighbor] < oldScore) {
                erase(openSet, neighbor);
                openSet.push(Qi{neighbor, (gScore[neighbor] +
                                           glm::distance(neighbor, end))});
            }
        } else {
            // If the length of the path from start to s and from s to
            // neighbor is shorter than the shortest currently known distance
            // from start to neighbor, then update node with the new distance
            auto newPathScore = gScore[s] + glm::distance(s, neighbor);
            if (newPathScore < gScore[neighbor]) {
                parent[neighbor] = s;
                gScore[neighbor] = newPathScore;
            }
            if (gScore[neighbor] < oldScore) {
                erase(openSet, neighbor);
                openSet.push(Qi{neighbor, (gScore[neighbor] +
                                           glm::distance(neighbor, end))});
            }
        }
    }
};

