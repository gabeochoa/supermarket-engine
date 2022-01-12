

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
// where isWalkable is a function that takes a glm::vec2 and returns wether
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

/*
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
*/

struct Theta {
    const float EQUAL_RANGE = 0.5f;
    const float maxnum = std::numeric_limits<float>::max();
    const float x[8] = {0, 0, 1, -1, -1, 1, -1, 1};
    const float y[8] = {1, -1, 0, 0, -1, -1, 1, 1};
    const float dst = 0.25f;
    const int LOOP_LIMIT = 10000;

    glm::vec2 start;
    glm::vec2 end;
    glm::vec4 bounds;
    std::function<bool(const glm::vec2& pos)> isWalkable;
    bool lazy;

    std::unordered_map<glm::vec2, double, VectorHash> gScore;
    std::unordered_map<glm::vec2, glm::vec2, VectorHash> parent;
    // using a set as a priority queue
    // accessing the smallest element(use as min heap) *pq.begin();
    // accessing the largest element (use as max heap) *pq.rbegin();
    ThetaPQ openSet;
    ThetaPQ closedSet;

    Theta(const glm::vec2& s, const glm::vec2& e, const glm::vec4& b,
          std::function<bool(const glm::vec2& pos)> valid, bool isLazy = false)
        : start(s), end(e), bounds(b), isWalkable(valid), lazy(isLazy) {
        log_trace("trying to find path from {} to {}", start, end);

        // if the goal isnt reachable, then we have to change the goal for now
        if (!canVisit(end)) {
            // uses the closedSet
            end = expandUntilWalkable(end);
            while (!closedSet.empty()) closedSet.pop();
            log_trace("couldnt get to end so end is now {}", end);
        } else {
            log_trace("goal is reachable {}", end);
        }

        // init vars
        gScore[start] = 0;
        openSet.push(Qi{start, (gScore[start] + glm::distance(start, end))});
    }

    bool canVisit(glm::vec2 p) {
        auto l = p.x >= bounds.x;
        auto r = p.x <= bounds.z;
        auto t = p.y >= bounds.y;
        auto b = p.y <= bounds.w;
        if (l && r && t && b) return this->isWalkable(p);
        return false;
    }

    glm::vec2 expandUntilWalkable(glm::vec2 n) {
        closedSet.push(Qi{n, 0});
        while (true) {
            Qi qi = closedSet.top();
            closedSet.pop();
            n = qi.first;
            for (int i = 0; i < 8; i++) {
                glm::vec2 neighbor = {n.x + (x[i] * dst), n.y + (y[i] * dst)};
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
        if (!canVisit(end)) {
            // for now just clear it,
            // otherwise itll inf loop
            while (!openSet.empty()) openSet.pop();
        }

        prof give_me_a_name(__PROFILE_FUNC__);
        int loop_iter = 0;
        while (loop_iter < LOOP_LIMIT && !openSet.empty()) {
            Qi qi = openSet.top();
            openSet.pop();
            auto s = qi.first;
            if (lazy) {
                set_vertex(s);
            }
            // wow we got here already
            if (glm::distance(s, end) < EQUAL_RANGE) {
                return reconstruct_path(s);
            }
            closedSet.push(Qi{s, 0});
            // Loop through each immediate neighbor of s
            for (int i = 0; i < 8; i++) {
                glm::vec2 neighbor = {s.x + (x[i] * dst), s.y + (y[i] * dst)};
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
                    }
                    update_vertex(s, neighbor);
                }
            }
            loop_iter++;
        }
        if (loop_iter >= LOOP_LIMIT) {
            log_trace("hit loop limit and quit early");
        }
        log_trace("no path found to thing");
        return std::vector<glm::vec2>();
    }

    bool line_of_sight(const glm::vec2& parentNode, const glm::vec2& neighbor) {
        // technically we can see it but cant walk to it its in the wall?
        if (!canVisit(neighbor)) {
            return false;
        }
        glm::vec2 loc(parentNode);
        while (distance(loc, neighbor) > 0.5f) {
            loc = lerp(loc, neighbor, 0.25f);
            if (!canVisit(loc)) {
                return false;
            }
        }
        return true;
    }

    std::vector<glm::vec2> reconstruct_path(glm::vec2 cur) {
        std::vector<glm::vec2> path;
        path.push_back(end);
        path.push_back(cur);
        while (parent.find(cur) != parent.end() && cur != start) {
            cur = parent.at(cur);
            if (cur.x == 0.f && cur.y == 0.f) continue;
            path.push_back(cur);
        }
        return path;
    }

    void set_vertex(const glm::vec2& s) {
        if (!line_of_sight(parent[s], s)) {
            glm::vec2 minN;
            double minS = -1;
            for (int i = 0; i < 8; i++) {
                glm::vec2 neighbor = {s.x + (x[i] * dst), s.y + (y[i] * dst)};
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

    void update_vertex(const glm::vec2& s, const glm::vec2& neighbor) {
        double oldScore = gScore[neighbor];
        // This part of the algorithm is the main difference between A* and
        // Theta*
        if (this->lazy) {
            // If there is line-of-sight between parent(s) and neighbor
            // then ignore s and use the path from parent(s) to neighbor
            auto newPathScore =
                gScore[parent[s]] + glm::distance(parent[s], neighbor);
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
            auto newPathScore =
                gScore[parent[s]] + glm::distance(parent[s], neighbor);
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

struct LazyTheta : public Theta {
    LazyTheta(const glm::vec2& s, const glm::vec2& e, const glm::vec4& b,
              std::function<bool(const glm::vec2& pos)> valid)
        : Theta(s, e, b, valid, true) {}
};
