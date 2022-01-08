#pragma once

#include <algorithm>
#include <cstdlib>

#include "../engine/edit.h"

// https://www.geeksforgeeks.org/dynamic-convex-hull-adding-points-existing-convex-hull/?ref=rp

struct Polygon {
    std::vector<glm::vec2> points;
    std::vector<glm::vec2> hull;

    // checks whether the point crosses the convex hull
    // or not
    int orientation(glm::vec2 a, glm::vec2 b, glm::vec2 c) const {
        int res = (b.y - a.y) * (c.x - b.x) - (c.y - b.y) * (b.x - a.x);
        if (res == 0) return 0;
        if (res > 0) return 1;
        return -1;
    }

    // Returns the square of distance between two input points
    int sqDist(glm::vec2 p1, glm::vec2 p2) const {
        return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
    }

    // Checks whether the point is inside the convex hull or not
    bool inside(glm::vec2 p) {
        // Initialize the centroid of the convex hull
        glm::vec2 mid = {0, 0};

        std::vector<glm::vec2> hullcopy(hull);

        int n = hullcopy.size();

        // Multiplying with n to avoid floating point
        // arithmetic.
        p.x *= n;
        p.y *= n;
        for (int i = 0; i < n; i++) {
            mid.x += hullcopy[i].x;
            mid.y += hullcopy[i].y;
            hullcopy[i].x *= n;
            hullcopy[i].y *= n;
        }

        // if the mid and the given point lies always
        // on the same side w.r.t every edge of the
        // convex hullcopy, then the point lies inside
        // the convex hullcopy
        for (int i = 0, j; i < n; i++) {
            j = (i + 1) % n;
            int x1 = hullcopy[i].x, x2 = hullcopy[j].x;
            int y1 = hullcopy[i].y, y2 = hullcopy[j].y;
            int a1 = y1 - y2;
            int b1 = x2 - x1;
            int c1 = x1 * y2 - y1 * x2;
            int for_mid = a1 * mid.x + b1 * mid.y + c1;
            int for_p = a1 * p.x + b1 * p.y + c1;
            if (for_mid * for_p < 0) return false;
        }

        return true;
    }

    // TODO split work into add / compute
    // so that we can add 4 points at a time (at least)

    // Adds a point p to given convex hull a[]
    void add(glm::vec2 p) {
        if (hull.size() < 3) {
            hull.push_back(p);
            return;
        }

        // If point is inside p
        if (inside(p)) return;

        // point having minimum distance from the point p
        int ind = 0;
        int n = hull.size();
        for (int i = 1; i < n; i++)
            if (sqDist(p, hull[i]) < sqDist(p, hull[ind])) ind = i;

        // Find the upper tangent
        int up = ind;
        while (orientation(p, hull[up], hull[(up + 1) % n]) >= 0)
            up = (up + 1) % n;

        // Find the lower tangent
        int low = ind;
        while (orientation(p, hull[low], hull[(n + low - 1) % n]) <= 0)
            low = (n + low - 1) % n;

        // Initialize result
        std::vector<glm::vec2> ret;

        // making the final hull by traversing points
        // from up to low of given convex hull.
        int curr = up;
        ret.push_back(hull[curr]);
        while (curr != low) {
            curr = (curr + 1) % n;
            ret.push_back(hull[curr]);
        }

        // Modify the original vector
        ret.push_back(p);
        hull.clear();
        for (int i = 0; i < (int)ret.size(); i++) hull.push_back(ret[i]);
    }
};

struct NavMesh {
    Polygon shape;

    NavMesh() { GLOBALS.set("navmesh", this); }
};
static NavMesh __navmesh___DO_NOT_USE_DIRECTLY;

inline void test_polygon_hull() {
    Polygon polygon;
    glm::vec2 points[] = {{0, 3}, {1, 1}, {2, 2}, {4, 4},
                          {0, 0}, {1, 2}, {3, 1}, {3, 3}};
    std::vector<glm::vec2> hull;

    for (auto aa : points) {
        polygon.add(aa);
        // Print the modified Convex Hull
        // for (auto e : polygon.hull)
        // std::cout << "(" << e.x << ", " << e.y << ") ";
        // std::cout << std::endl;
    }

    M_ASSERT(polygon.hull[0].x == 4, "yo");
    M_ASSERT(polygon.hull[1].x == 0, "yo");
    M_ASSERT(polygon.hull[2].x == 0, "yo");
    M_ASSERT(polygon.hull[3].x == 3, "yo");
}
