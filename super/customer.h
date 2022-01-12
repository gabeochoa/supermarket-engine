
#pragma once

#include "job.h"
#include "movable_entities.h"

struct Customer : public Person {
    float totalWallet;
    float totalSpendToday;
    // TODO are customers going to be
    float totalSpendLifetime;
    ItemGroup shoppingCart;
    ItemGroup shoppingList;

    float timeBetweenChecks = 20.f;
    float timeShopping;
    std::map<int, float> avgPricePaid;

    void refresh() {
        timeShopping = timeBetweenChecks;
        //  schedule our jobs
        //  -- first schedule all find jobs
        for (auto ig : shoppingList) {
            scheduleFindItemJob(ig.first, ig.second);
            // if (randIn(0, 2) > 1) {
            // scheduleIdleShop();
            // }
        }
        if (shoppingList.empty()) {
            int totalNumItems = 0;
            for (auto ig : shoppingCart) totalNumItems += ig.second;
            announce(fmt::format(
                "I finished shopping. I have {} items ({}) in my cart",
                shoppingCart.size(), totalNumItems));
        }
    }

    void init() {
        // decide what to get
        // TODO maybe shouldnt be linear but normal distribution..
        int numToGet = randIn(1, 1);
        for (int i = 0; i < numToGet; i++) {
            shoppingList.addItem(
                // itemid
                // TODO probably dont use items_ directly...
                randIn(0, items_.size() - 1),
                // amount
                randIn(1, 1));
        }

        // decide how much money to bring
        estimateCartSpend();

        refresh();
    }

    void scheduleIdleShop() {
        // TODO
    }

    void scheduleFindItemJob(int itemID, int itemAmount) {
        auto shelves =
            EntityHelper::getEntityInRangeWithItem<Shelf>(position, itemID, -1);
        if (shelves.empty()) {
            announce(
                fmt::format("tried to schedule a FindItem for {} but store "
                            "doesnt have it",
                            itemID));
            // TODO idk...
            return;
        }
        auto shelfPos = shelves.front()->position;
        announce(fmt::format(
            "scheduling a job for myself, for item {} at shelf {}, ", itemID,
            shelfPos));

        JobQueue::addJob(
            JobType::FindItem,
            std::make_shared<Job>(Job({.type = JobType::FindItem,
                                       .reserved = id,
                                       .startPosition = shelfPos,
                                       .itemID = itemID,
                                       .itemAmount = itemAmount})));
    }

    void estimateCartSpend() {
        float possibleSpend = 0;
        auto im = GLOBALS.get<ItemManager>("item_manager");
        for (auto ig : shoppingList) {
            float global_price = im.get_avg_price(ig.first);
            float my_price = map_get_or_default(avgPricePaid, id, global_price);
            // I believe myself more than the world
            float price = 0.6 * my_price + 0.4 * global_price;
            // TODO randomize this a bit (UP?)
            possibleSpend += (price * ig.second);
        }
        // TODO randomize this a bit
        totalWallet = possibleSpend * 1.5f;
    }

    virtual JobRange getJobRange() override {
        return {JobType::INVALID_Customer_Boundary, JobType::MAX_JOB_TYPE};
    }

    Customer() : Person() { init(); }

    virtual void registerJobHandlers() override {
        handler.registerJobHandler(
            JobType::FindItem,
            std::bind(&Customer::workFindItem, this, std::placeholders::_1,
                      std::placeholders::_2));
        handler.registerJobHandler(
            JobType::IdleShop,
            std::bind(&Customer::idleShop, this, std::placeholders::_1,
                      std::placeholders::_2));
    }

    virtual void onUpdate(Time dt) override {
        //
        Person::onUpdate(dt);
        timeShopping -= dt.s();
        if (timeShopping <= 0) {
            refresh();
            timeShopping = timeBetweenChecks;
        }
    }

    virtual const char* typeString() const override { return "Customer"; }

    // TODO move into Person() and use the same thing
    // for Employee
    bool grabFromNearbyShelf(int itemID, int itemAmount) {
        auto shelves = EntityHelper::getEntityInRangeWithItem<Shelf>(
            position, itemID, REACH_DIST);
        if (shelves.empty()) {
            announce("no matching shelf");
            log_warn("no matching shelf, so uh what can we do");
            return false;
        }
        announce(fmt::format("trying to grab {} item{} from {}", itemAmount,
                             itemID, (*shelves.begin())->contents));
        int amt = (*shelves.begin())->contents.removeItem(itemID, itemAmount);
        shoppingCart.addItem(itemID, amt);
        shoppingList.removeItem(itemID, amt);
        return true;
    }

    bool workFindItem(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        log_trace("workFindItem, ");
        if (j->reg.find("amount") != j->reg.end()) {
            j->reg["amount"] = 0;
        }
        switch (j->jobStatus) {
            case 0:  // Just started the job
            {
                // announce("walking to start");
                // walk to start...
                bool isAtStartLocation =
                    walkToLocation(j->startPosition, input);
                j->jobStatus = isAtStartLocation ? 1 : 0;
            } break;
            case 1:  // Reached start
            {
                // announce("grab something");
                bool success = grabFromNearbyShelf(j->itemID, j->itemAmount);
                j->jobStatus = success ? 2 : -1;
            } break;
            case 2:  // Grabbed item
            {
                // announce(fmt::format("finished finding item, {}, ",
                // j->itemID));
                j->isComplete = true;
                return true;
            } break;
            case -1:  // Something bad happened...
            {
                // log_warn("Something bad happened and i couldnt finish");
                j->isComplete = true;
                return true;
            } break;
        }
        return false;
    }

    bool idleShop(const std::shared_ptr<Job>& j, WorkInput input) {
        (void)input;
        if (walkToLocation(j->endPosition, input)) {
            j->isComplete = true;
            return true;
        }
        return false;
    }
};
