
#pragma once

#include "../engine/pch.hpp"
#include "entity.h"
#include "job.h"
#include "vecutil.h"

glm::vec2 lerp(const glm::vec2& x, const glm::vec2& y, float t) {
    return x * (1.f - t) + y * t;
}

const float REACH_DIST = 1.f;
// TODO switch to a star
//
#include <queue>
struct AstarPQItem {
    glm::vec2 location;
    double score;
    bool operator<(const AstarPQItem& s) const { return this->score > s.score; }
};

std::vector<glm::vec2> reconstruct_path(
    const std::unordered_map<glm::vec2, glm::vec2, VectorHash>& cameFrom,
    glm::vec2 cur) {
    std::vector<glm::vec2> path;
    path.push_back(cur);
    while (cameFrom.find(cur) != cameFrom.end()) {
        cur = cameFrom.at(cur);
        path.push_back(cur);
    }
    return path;
}

std::vector<glm::vec2> generateWalkablePath(  //
    float movement,                           //
    const glm::vec2& start,                   //
    const glm::vec2& end                      //
) {
    // TODO currently only moving 1.f at a time
    (void)movement;

    prof p(__PROFILE_LOC__("astar"));
    std::vector<glm::vec2> path;
    glm::vec2 cur = glm::vec2(start);

    // astar
    auto heuristic = [](const glm::vec2& s, const glm::vec2& g) {
        return abs(s.x - g.x) + abs(s.y - g.y);
    };

    std::priority_queue<AstarPQItem> openSet;
    openSet.push(AstarPQItem{start, 0});

    std::unordered_map<glm::vec2, glm::vec2, VectorHash> cameFrom;
    std::unordered_map<glm::vec2, double, VectorHash> distTraveled;

    distTraveled[start] = 0;

    while (!openSet.empty()) {
        AstarPQItem qi = openSet.top();
        openSet.pop();
        auto cur = qi.location;
        if (glm::distance(cur, end) < REACH_DIST) break;

        const int x[] = {0, 0, 1, -1, -1, 1, -1, 1};
        const int y[] = {1, -1, 0, 0, -1, -1, 1, 1};

        for (int i = 0; i < 8; i++) {
            glm::vec2 neighbor = {cur.x + x[i], cur.y + y[i]};
            double newCost = distTraveled[cur] + distance(cur, neighbor);
            if (distTraveled.find(neighbor) == distTraveled.end() ||
                newCost < distTraveled[neighbor]) {
                distTraveled[neighbor] = newCost;
                double prio = newCost + heuristic(neighbor, end);
                openSet.push(AstarPQItem{.location = neighbor, .score = prio});
                cameFrom[neighbor] = cur;
            }
        }
    }
    return reconstruct_path(cameFrom, end);
}

std::vector<glm::vec2> generateWalkablePath(float movement,
                                            const glm::vec2& start,
                                            const glm::vec2& end,
                                            const glm::vec2& next) {
    std::vector<glm::vec2> pathToEnd =
        generateWalkablePath(movement, start, end);
    std::vector<glm::vec2> pathFromEndToNext =
        generateWalkablePath(movement, end, next);
    pathToEnd.insert(pathToEnd.end(), pathFromEndToNext.begin(),
                     pathFromEndToNext.end());
    return pathToEnd;
}

// Generates a walkable path, that hits all points of interest IN ORDER
template <typename... Rest>
std::vector<glm::vec2> generateWalkablePath(float movement,
                                            const glm::vec2& start,
                                            const glm::vec2& end,
                                            const glm::vec2& next,
                                            Rest... rest) {
    // base case
    std::vector<glm::vec2> path = generateWalkablePath(movement, start, end);

    // reducing the rest by one, as the first item in rest will become "next"
    std::vector<glm::vec2> path2 =
        generateWalkablePath(movement, end, next, rest...);
    path.insert(path.end(), path2.begin(), path2.end());
    return path;
}

struct MovableEntity : public Entity {
    const glm::vec2 INVALID = {-99.f, -99.f};
    glm::vec2 last = glm::vec2(INVALID);
    std::vector<glm::vec2> path;
    float moveSpeed = 0.1f;

    void move() {
        // first time we are moving, just set last to our current position
        if (last == INVALID) last = glm::vec2(position);

        // try to grab the next spot in the path
        auto target = path.begin();
        if (target == path.end()) return;

        // reached our local target, erase this
        // and next cycle we'll grab the next POI
        if (distance(position, *target) < REACH_DIST) {
            path.erase(target);
            // announce(
            // fmt::format(" reached local target, {} to go", path.size()));
            return;
        }

        position = lerp(position, *target, moveSpeed / 50);
    }

    virtual void onUpdate(Time dt) {
        (void)dt;
        if (angle >= 360) {
            angle -= 360;
        }
        if (angle < 0) {
            angle += 360;
        }
    }

    virtual const char* typeString() const = 0;
};

struct Person : public MovableEntity {
    JobHandler handler;
    std::shared_ptr<Job> assignedJob;

    void startJob(const std::shared_ptr<Job> job) {
        assignedJob = job;
        if (!assignedJob) return;

        assignedJob->isAssigned = true;
        announce(fmt::format("starting job {}", *job));
    }

    void workOrFindMore(Time dt) {
        if (!assignedJob) {
            // announce("finding new job");
            auto range = getJobRange();
            auto ptr = JobQueue::getNextInRange(range);
            startJob(ptr);
            return;
        }
        handler.handle(assignedJob, {dt});
        if (assignedJob->isComplete) {
            announce(fmt::format("finished with {}",
                                 jobTypeToString(assignedJob->type)));
            assignedJob.reset();
        }
    }

    virtual void onUpdate(Time dt) {
        if (handler.job_mapping.empty()) {
            registerJobHandlers();
        }
        workOrFindMore(dt);
    }
    virtual void registerJobHandlers() = 0;
    virtual JobRange getJobRange() { return {JobType::None, JobType::None}; }

    bool none(const std::shared_ptr<Job>& j, const WorkInput& input) {
        j->seconds = j->seconds - input.dt.s();
        if (j->seconds <= 0) {
            announce(fmt::format("completed job {}", jobTypeToString(j->type)));
            j->isComplete = true;
            return true;
        }
        return false;
    }

    virtual const char* typeString() const = 0;
};

struct Employee : public Person {
    ItemGroup inventory;

    JobRange getJobRange() override {
        return {JobType::None, JobType::INVALID_Customer_Boundary};
    }

    bool walkToLocation(const glm::vec2 location) {
        // Have we reached the position yet?
        if (path.empty() && glm::distance(position, location) < REACH_DIST) {
            // our path should be empty but just in case
            path.clear();
            return true;
        }
        // Did we already generate a path?
        if (!path.empty()) {
            move();
            return false;
        }
        announce(
            fmt::format(" distance to location end {}  (need to be within {})",
                        glm::distance(position, location), REACH_DIST));

        path = generateWalkablePath(  //
            moveSpeed,                //
            position,                 //
            location);
        return false;
    }

    bool workFill(const std::shared_ptr<Job>& j, WorkInput input) {
        if (j->reg.find("amount") != j->reg.end()) {
            j->reg["amount"] = 0;
        }

        switch (j->jobStatus) {
            case 0:  // Just started the job
            {
                // walk to start...
                bool isAtStartLocation = walkToLocation(j->startPosition);
                if (isAtStartLocation) {
                    announce("got to start Location");
                    j->jobStatus = 1;  // grab something...
                }
            } break;
            case 1:  // Reached start
            {
                announce("grab something");
                // TODO this should be  "Storage" instead of a shelf but shelf
                // is a little easier right now
                auto shelves = Shelf::getShelvesInRange(position, REACH_DIST);
                // announce(fmt::format("trying to grab {} item{} from {}",
                // j->itemAmount, j->itemID,
                // (*shelves.begin())->contents));
                // TODO need to support finding a shelf instead of
                // setting the start and end manually
                if (shelves.empty()) {
                    log_warn("no matching shelf, so uh what can we do");
                    j->jobStatus = 5;
                    return false;
                }

                int amt = (*shelves.begin())
                              ->contents.removeItem(j->itemID, j->itemAmount);
                inventory.addItem(j->itemID, amt);
                if (j->itemAmount - inventory[j->itemID] > 0) {
                    log_warn("shelf didnt have enough, so giving up");
                }
                // announce(fmt::format("tried to grab {} from {}, howd it go?
                // ", j->itemID, (*shelves.begin())->contents));
                j->jobStatus = 2;
            } break;
            case 2:  // Grabbed item
            {
                // walk to end...
                bool isAtEndLocation = walkToLocation(j->endPosition);
                if (isAtEndLocation) {
                    announce("got to end Location");
                    j->jobStatus = 3;  // drop it off something...
                }
            } break;
            case 3:  // Got to End
            {
                announce("drop it off ");
                auto shelves = Shelf::getShelvesInRange(position, REACH_DIST);
                // TODO need to support finding a shelf instead of
                // setting the start and end manually
                if (shelves.empty()) {
                    log_warn("no matching shelf, so uh what can we do");
                    j->jobStatus = 5;
                    return false;
                }
                (*shelves.begin())
                    ->contents.addItem(j->itemID, inventory[j->itemID]);
                inventory.removeItem(j->itemID, inventory[j->itemID]);
                j->jobStatus = 4;
            } break;
            case 4:  // Dropped off Item
            {
                j->isComplete = true;
                return true;
            } break;

            case 5:  // Something bad happened...
            {
                log_warn("Something bad happened and i couldnt finish");
                j->isComplete = true;
                return true;
            } break;
        }
        return false;
    }

    bool idleWalk(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        // Have we reached the endPosition yet?
        if (path.empty() &&
            glm::distance(position, j->endPosition) < REACH_DIST) {
            // our path should be empty but just in case
            path.clear();
            j->isComplete = true;
            return true;
        }

        // Did we already generate a path?
        if (!path.empty()) {
            move();
            return false;
        }
        announce(fmt::format(" distance to job end {}  (need to be within {})",
                             glm::distance(position, j->endPosition),
                             REACH_DIST));

        path = generateWalkablePath(  //
            moveSpeed,                //
            position,                 //
            j->startPosition,         //
            j->endPosition);
        return false;
    }

    bool directedWalk(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        // Have we reached the endPosition yet?
        if (path.empty() &&
            glm::distance(position, j->endPosition) < REACH_DIST) {
            // our path should be empty but just in case
            path.clear();
            j->isComplete = true;
            return true;
        }

        // Did we already generate a path?
        if (!path.empty()) {
            move();
            return false;
        }
        announce(fmt::format(" distance to job end {}  (need to be within {})",
                             glm::distance(position, j->endPosition),
                             REACH_DIST));

        path = generateWalkablePath(  //
            moveSpeed,                //
            position,                 //
            j->endPosition);
        return false;
    }

    Employee() : Person() {}

    virtual void registerJobHandlers() override {
        handler.registerJobHandler(
            JobType::Fill,
            std::bind(&Employee::workFill, this, std::placeholders::_1,
                      std::placeholders::_2));

        handler.registerJobHandler(
            JobType::IdleWalk,
            std::bind(&Employee::idleWalk, this, std::placeholders::_1,
                      std::placeholders::_2));

        handler.registerJobHandler(
            JobType::DirectedWalk,
            std::bind(&Employee::directedWalk, this, std::placeholders::_1,
                      std::placeholders::_2));

        handler.registerJobHandler(
            JobType::None, std::bind(&Person::none, this, std::placeholders::_1,
                                     std::placeholders::_2));
    }

    virtual void onUpdate(Time dt) override {
        //
        Person::onUpdate(dt);
    }
    virtual const char* typeString() const override { return "Employee"; }
};

struct Customer : public Person {
    ItemGroup shoppingCart;
    ItemGroup shoppingList;

    virtual JobRange getJobRange() override {
        return {JobType::INVALID_Customer_Boundary, JobType::MAX_JOB_TYPE};
    }

    bool workFindItem(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)j;
        (void)input;
        log_info("workFindItem, ");
        return false;
    }

    Customer() : Person() {}

    virtual void registerJobHandlers() override {
        handler.registerJobHandler(
            JobType::FindItem,
            std::bind(&Customer::workFindItem, this, std::placeholders::_1,
                      std::placeholders::_2));
    }

    virtual void onUpdate(Time dt) override {
        //
        Person::onUpdate(dt);
    }

    virtual const char* typeString() const override { return "Customer"; }
};
