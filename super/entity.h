
#pragma once

#include "../engine/input.h"
#include "../engine/pch.hpp"

struct Entity {
    glm::vec2 position;
    glm::vec2 size;
    float angle;
    glm::vec4 color;
    std::string textureName;

    Entity()
        : position({0.f, 0.f}),
          size({1.f, 1.f}),
          angle(0.f),
          color({1.f, 1.f, 1.f, 1.f}),
          textureName("white") {}

    Entity(const glm::vec2& position_, const glm::vec2& size_, float angle_,
           const glm::vec4& color_, const std::string& textureName_)
        : position(position_),
          size(size_),
          angle(angle_),
          color(color_),
          textureName(textureName_) {}

    virtual ~Entity() {}

    virtual void onUpdate(Time dt) {
        (void)dt;
        if (angle >= 360) {
            angle -= 360;
        }
        if (angle < 0) {
            angle += 360;
        }
    }

    virtual void render() {
        // computing angle transforms are expensive so
        // if the angle is under thresh, just render it square
        if (angle <= 5.f) {
            Renderer::drawQuad(position, size, color, textureName);
        } else {
            Renderer::drawQuadRotated(position, size, glm::radians(angle),
                                      color, textureName);
        }
    }
};

struct Billboard : public Entity {
    // Billboard is a textured ent that never moves
    Billboard(const glm::vec2& position, const glm::vec2& size, float angle,
              const glm::vec4& color,
              // TODO somehow i broke colored textures...
              // so for now we will use a definite undefinied tex
              // and it will trigger the flat shader
              const std::string& textureName = "__INVALID__")
        : Entity(position, size, angle, color, textureName) {}

    virtual ~Billboard() {}
};

#include "job.h"

struct Item : public Entity {
    const char* name;
    const double price;

    Item(const char* n, const double p) : name(n), price(p) {}
};

struct ItemGroup {
    std::unordered_map<std::string, int> group;

    void addItem(const std::string& name, int amount) {
        if (group.find(name) == group.end()) {
            group[name] = 0;
        }
        group[name] += amount;
    }

    int removeItem(const std::string& name, int amount) {
        auto it = group.find(name);
        if (it == group.end()) {
            log_warn(fmt::format(
                "Trying to remove {} of {} but this ItemGroup doesnt have that",
                amount, name));
            return 0;
        }
        if (it->second >= amount) {
            group[name] -= amount;
            return amount;
        }
        int has = group[name];
        group[name] = 0;
        return has;
    }
};

struct Person : public Entity {
    JobHandler handler;
    std::shared_ptr<Job> assignedJob;

    void workOrFindMore(Time dt) {
        if (!assignedJob) {
            auto range = getJobRange();
            auto ptr = JobQueue::getNextInRange(range);
            assignedJob = ptr;
            if (assignedJob) assignedJob->isAssigned = true;

            return;
        }
        // if (assignedJob) {
        handler.handle(assignedJob, {dt});
        if (assignedJob->isComplete) {
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
        log_info(
            fmt::format("working the none job a little, i have {} seconds left",
                        j->seconds));
        j->seconds = j->seconds - input.dt.s();
        if (j->seconds <= 0) {
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

    Employee() : Person() {}

    virtual void registerJobHandlers() override {
        handler.registerJobHandler(
            JobType::Fill,
            std::bind(&Employee::workFill, this, std::placeholders::_1,
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
