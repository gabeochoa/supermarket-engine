
#pragma once

#include "../engine/pch.hpp"
#include "entity.h"
#include "job.h"

glm::vec2 lerp(const glm::vec2& x, const glm::vec2& y, float t) {
    return x * (1.f - t) + y * t;
}

std::vector<glm::vec2> generateWalkablePath(  //
    float movement,                           //
    const glm::vec2& start,                   //
    const glm::vec2& end                      //
) {
    std::vector<glm::vec2> path;
    glm::vec2 cur = glm::vec2(start);
    while (glm::distance(cur, end) > movement) {
        path.push_back(cur);
        cur = lerp(cur, end, movement);
    }
    return path;
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
    const float REACH_DIST = 1.f;
    glm::vec2 last = glm::vec2(INVALID);
    std::vector<glm::vec2> path;
    float moveSpeed = 0.01f;

    void move() {
        // first time we are moving, just set last to our current position
        if (last == INVALID) last = glm::vec2(position);

        // try to grab the next spot in the path
        auto target = path.begin();
        if (target == path.end()) return;

        // reached our local target, erase this
        // and next cycle we'll grab the next POI
        if (distance(position, *target) < REACH_DIST) {
            position = *target;
            path.erase(target);
            // announce(
            // fmt::format(" reached local target, {} to go", path.size()));
            return;
        }

        position = lerp(position, *target, moveSpeed / 100);
    }
};

struct Person : public MovableEntity {
    JobHandler handler;
    std::shared_ptr<Job> assignedJob;

    void startJob(const std::shared_ptr<Job> job) {
        assignedJob = job;
        if (!assignedJob) return;

        assignedJob->isAssigned = true;
        announce(fmt::format("starting job {}", jobTypeToString(job->type)));
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
};

struct Employee : public Person {
    ItemGroup inventory;

    JobRange getJobRange() override {
        return {JobType::None, JobType::INVALID_Customer_Boundary};
    }

    bool workFill(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)j;
        (void)input;
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
            JobType::None, std::bind(&Person::none, this, std::placeholders::_1,
                                     std::placeholders::_2));
    }

    virtual void onUpdate(Time dt) override {
        //
        Person::onUpdate(dt);
    }
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
};
