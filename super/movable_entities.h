
#pragma once

#include "../engine/pch.hpp"
#include "../engine/thetastar.h"
#include "entity.h"
#include "job.h"

const float REACH_DIST = 0.2f;

bool isWalkable(const glm::vec2& size, const glm::vec2& pos) {
    // is valid location
    for (auto& e : entities) {
        auto s = dynamic_pointer_cast<Shelf>(e);
        if (!s) continue;
        // if (e->pointCollides(pos)) return false;
        {
            // collision x-axis?
            bool collisionX = pos.x + size.x >= e->position.x &&
                              e->position.x + e->size.x >= pos.x;
            // collision y-axis?
            bool collisionY = pos.y + size.y >= e->position.y &&
                              e->position.y + e->size.y >= pos.y;
            // collision only if on both axes
            if (collisionX && collisionY) {
                return false;
            }
        }
    }
    return true;
}

std::vector<glm::vec2> generateWalkablePath(  //
    int skipID,                               //
    float movement,                           //
    const glm::vec2& start,                   //
    const glm::vec2& end,                     //
    const glm::vec2& size                     //
) {
    (void)movement;
    log_info("starting theta");
    LazyTheta t(start, end, std::bind(isWalkable, size, std::placeholders::_1));
    auto a = t.go();
    std::reverse(a.begin(), a.end());
    for (auto i : a) {
        log_info(fmt::format("{}", i));
    }
    return a;
}

struct MovableEntity : public Entity {
    const glm::vec2 INVALID = {-99.f, -99.f};
    glm::vec2 last = glm::vec2(INVALID);
    std::vector<glm::vec2> path;
    float moveSpeed = 0.05f;

    bool walkToLocation(const glm::vec2 location) {
        prof(__PROFILE_FUNC__);
        // Have we reached the position yet?
        if (path.empty() && glm::distance(position, location) < REACH_DIST) {
            // our path should be empty but just in case
            path.clear();
            return true;
        }
        // Did we already generate a path?
        if (!path.empty()) {
            // first time we are moving, just set last to our current position
            if (last == INVALID) last = glm::vec2(position);
            // try to grab the next spot in the path
            auto target = path.begin();
            if (target == path.end()) return true;

            if (!isWalkable(this->size, *target)) {
                announce("my next target isnt walkable....");
            }

            // reached our local target, erase this
            // and next cycle we'll grab the next POI
            if (distance(position, *target) < REACH_DIST) {
                path.erase(target);
                announce(fmt::format(" reached local target, {} to go",
                                     path.size()));
                return path.empty() ? true : false;
            }

            position = lerp(position, *target, moveSpeed);
            return false;
        }
        announce(
            fmt::format(" distance to location end {}  (need to be within {})",
                        glm::distance(position, location), REACH_DIST));

        path =
            generateWalkablePath(id, moveSpeed, position, location, this->size);
        return false;
    }

    virtual void onUpdate(Time dt) {
        (void)dt;
        if (angle >= 360) angle -= 360;
        if (angle < 0) angle += 360;
    }

    virtual const char* typeString() const = 0;
};

struct Person : public MovableEntity {
    JobHandler handler;
    std::shared_ptr<Job> assignedJob;

    void startJob(const std::shared_ptr<Job> job) {
        assignedJob = job;
        if (!assignedJob) return;
        path.clear();
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
        if (walkToLocation(j->endPosition)) {
            j->isComplete = true;
            return true;
        }
        return false;
    }

    bool directedWalk(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        if (walkToLocation(j->endPosition)) {
            j->isComplete = true;
            return true;
        }
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
