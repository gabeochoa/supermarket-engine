

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

    Job(JobType t)
        : type(t),
          isComplete(false),
          isAssigned(false),
          startPosition({0.f, 0.f}),
          endPosition({0.f, 0.f}),
          seconds(-1) {}

    //
    // internal use
    bool handled;
};

struct JobRange {
    JobType start;
    JobType end;
};

static std::map<int, std::vector<std::shared_ptr<Job> > > jobs;
struct JobQueue {
    static void addJob(JobType t, const std::shared_ptr<Job>& j) {
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
