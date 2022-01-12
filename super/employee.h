
#pragma once

#include "job.h"
#include "movable_entities.h"

struct Employee : public Person {
    ItemGroup inventory;

    JobRange getJobRange() override {
        return {JobType::None, JobType::INVALID_Customer_Boundary};
    }

    bool workFill(const std::shared_ptr<Job>& j, const WorkInput& input) {
        (void)input;

        if (j->reg.find("amount") != j->reg.end()) {
            j->reg["amount"] = 0;
        }

        switch (j->jobStatus) {
            case 0:  // Just started the job
            {
                // walk to start...
                bool isAtStartLocation =
                    walkToLocation(j->startPosition, input);
                if (isAtStartLocation) {
                    // announce("got to start Location");
                    j->jobStatus = 1;  // grab something...
                }
            } break;
            case 1:  // Reached start
            {
                // announce("grab something");
                auto shelves = EntityHelper::getEntityInRangeWithItem<Storage>(
                    position, j->itemID, REACH_DIST);
                if (shelves.empty()) {
                    announce("no matching shelf");
                    // log_warn("no matching shelf, so uh what can we do");
                    j->jobStatus = 5;
                    return false;
                }
                // announce(fmt::format("trying to grab {} item{} from {}",
                // j->itemAmount, j->itemID,
                // (*shelves.begin())->contents));

                int handSize = 5;
                int amt = (*shelves.begin())
                              ->contents.removeItem(j->itemID, handSize);
                inventory.addItem(j->itemID, amt);
                j->jobStatus = 2;
            } break;
            case 2:  // Grabbed item
            {
                // walk to end...
                bool isAtEndLocation = walkToLocation(j->endPosition, input);
                if (isAtEndLocation) {
                    // announce("got to end Location");
                    j->jobStatus = 3;  // drop it off something...
                }
            } break;
            case 3:  // Got to End
            {
                // announce("drop it off ");
                auto shelves = EntityHelper::getEntitiesInRange<Shelf>(
                    position, REACH_DIST);
                // TODO need to support finding a shelf instead of
                // setting the start and end manually
                if (shelves.empty()) {
                    // log_warn("no matching shelf, so uh what can we do");
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
                // log_warn("Something bad happened and i couldnt finish");
                j->isComplete = true;
                return true;
            } break;
        }
        return false;
    }

    bool idleWalk(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        if (walkToLocation(j->endPosition, input)) {
            j->isComplete = true;
            return true;
        }
        return false;
    }

    bool directedWalk(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        if (walkToLocation(j->endPosition, input)) {
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

