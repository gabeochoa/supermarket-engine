

#pragma once

#include "../engine/pch.hpp"

enum JobType {
    None = 0,
    // Employee job types
    Fill,
    Empty,
    IdleWalk,

    // delineates job group
    INVALID_Customer_Boundary,
    // Customer job types
    FindItem,
    GotoRegister,
    IdleShop,
    LeaveStore,

    // always last
    MAX_JOB_TYPE,
};

inline std::string jobTypeToString(JobType t) {
    switch (t) {
        case JobType::None:
            return "None";
        case JobType::Fill:
            return "Fill";
        default:
            log_warn(fmt::format("JobType {} has no toString handler", t));
            return std::to_string(t);
    }
}

struct Job {
    JobType type;
    bool isComplete;
    bool isAssigned;
    glm::vec2 startPosition;
    glm::vec2 endPosition;
    int seconds;

    // Job(JobType type_ = JobType::None, bool isComplete_ = false,
    // bool isAssigned_ = false, glm::vec2 startPosition_ = {0.f, 0.f},
    // glm::vec2 endPosition_ = {0.f, 0.f}, int seconds_ = -1)
    // : type(type_),
    // isComplete(isComplete_),
    // isAssigned(isAssigned_),
    // startPosition(startPosition_),
    // endPosition(endPosition_),
    // seconds(seconds_) {}

    // Job() = default;
    //
    // Job(const Job& other) {
    // type = other.type;
    // isComplete = other.isComplete;
    // isAssigned = other.isAssigned;
    // startPosition = other.startPosition;
    // endPosition = other.endPosition;
    // seconds = other.seconds;
    // }
};

struct JobRange {
    JobType start;
    JobType end;
};

static std::map<int, std::vector<std::shared_ptr<Job> > > jobs;
struct JobQueue {
    static void addJob(JobType t, const std::shared_ptr<Job>& j) {
        std::cout << "ss" << j->seconds << std::endl;
        jobs[(int)t].push_back(j);
    }

    static std::vector<std::shared_ptr<Job> >::iterator getNextMatching(
        JobType t) {
        auto js = jobs[(int)t];
        for (auto it = js.begin(); it != js.end(); it++) {
            if ((*it)->isAssigned || (*it)->isComplete) continue;
            return it;
        }
        return jobs[(int)t].end();
    }

    static std::shared_ptr<Job> getNextInRange(JobRange jr) {
        for (int i = (int)jr.start; i <= jr.end; i++) {
            auto js = jobs[i];
            for (auto it = js.begin(); it != js.end(); it++) {
                if ((*it)->isAssigned || (*it)->isComplete) continue;
                if ((*it)->type <= jr.end && (*it)->type >= jr.start)
                    return *it;
            }
        }
        return nullptr;
    }
};

struct WorkInput {
    Time dt;

    WorkInput(Time t) : dt(t) {}
};

typedef std::function<bool(const std::shared_ptr<Job>&, WorkInput input)>
    JobHandlerFn;

struct JobHandler {
    std::unordered_map<int, JobHandlerFn> job_mapping;

    JobHandler() {}

    void registerJobHandler(JobType jt, JobHandlerFn func) {
        log_info(
            fmt::format("Registering job handler for {}", jobTypeToString(jt)));
        job_mapping[(int)jt] = func;
    }

    void handle(const std::shared_ptr<Job>& j, const WorkInput& input) {
        if (job_mapping.find((int)j->type) == job_mapping.end()) {
            log_warn(
                fmt::format("Got job of type {} but dont have a handler for it",
                            jobTypeToString(j->type)));
            return;
        }

        job_mapping[(int)j->type](j, input);
    }
};
