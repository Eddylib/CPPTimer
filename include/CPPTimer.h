//
// Created by libaoyu on 2021/4/8.
//

#ifndef UTILS_CPP_TIMER_H
#define UTILS_CPP_TIMER_H
#pragma once

#include <csignal>
#include <cassert>
#include <cstring>
#include <atomic>
#include <mutex>
#include <functional>


class CPPTimer {
    using callback_type = void(sigval);
public:

    struct Config {
        enum class TimerType {
            WALL_CLOCK, // system wide clock
            STEADY_CLOCK,   // monotonically increasing clock
            HR_CLOCK    // currently same as wall clock
        };

        uint32_t run_time_ = 1;
        uint64_t period_ms_ = 1000;
        bool start_now_ = false;
        TimerType clock_type_ = TimerType::STEADY_CLOCK;

        Config(
                uint32_t run_time = 1,
                uint64_t period_ms = 1000,
                bool start_now = false,
                TimerType clock_type = TimerType::STEADY_CLOCK) :
                run_time_(run_time), period_ms_(period_ms), start_now_(start_now), clock_type_(clock_type) {}
    };

    template<class Object, typename Res, typename ... Args>
    CPPTimer(Res ( Object::*func )(Args ...),
             Object *const object,
             const Config &config, Args ...args):
            curr_time_(0), config_(config), object_(object) {

        func_wrapper_ = [this, args..., func](sigval v) {
            if (object_) {
                (((Object *) object_)->*func)(args...);
            }
        };

        sigev_.sigev_value.sival_ptr = (void *) this;
        sigev_.sigev_notify = SIGEV_THREAD;
        sigev_.sigev_notify_function = &CPPTimer::timer_func_exec;

        auto timer_type = CLOCK_MONOTONIC;
        switch (config.clock_type_) {
            case Config::TimerType::WALL_CLOCK:
                timer_type = CLOCK_REALTIME;
                break;
            case Config::TimerType::STEADY_CLOCK:
                timer_type = CLOCK_MONOTONIC;
                break;
            case Config::TimerType::HR_CLOCK:
                break;
        }

        if (timer_create(timer_type, &sigev_, &timer_id_) != 0) {
            std::cout << strerror(errno) << std::endl;
            return;
        }

        if (config.start_now_) {
            start();
        }
    }

    template<typename Res, typename ... Args>
    CPPTimer(Res (*func )(Args ...),
             const Config &config, Args ...args):
            curr_time_(0), config_(config), object_(nullptr) {

        func_wrapper_ = [this, args..., func](sigval v) {
            (*func)(args...);
        };

        sigev_.sigev_value.sival_ptr = (void *) this;
        sigev_.sigev_notify = SIGEV_THREAD;
        sigev_.sigev_notify_function = &CPPTimer::timer_func_exec;

        auto timer_type = CLOCK_MONOTONIC;
        switch (config.clock_type_) {
            case Config::TimerType::WALL_CLOCK:
                timer_type = CLOCK_REALTIME;
                break;
            case Config::TimerType::STEADY_CLOCK:
                timer_type = CLOCK_MONOTONIC;
                break;
            case Config::TimerType::HR_CLOCK:
                break;
        }

        if (timer_create(timer_type, &sigev_, &timer_id_) != 0) {
            std::cout << strerror(errno) << std::endl;
            return;
        }

        if (config.start_now_) {
            start();
        }
    }

    void start() {
        struct itimerspec its;
        its.it_value.tv_sec = config_.period_ms_ / 1000;
        its.it_value.tv_nsec = (config_.period_ms_ % 1000) * 1000 * 1000;
        its.it_interval = its.it_value;

        curr_time_ = 0;
        timer_settime(timer_id_, 0, &its, NULL);
    }

    void stop() {
        struct itimerspec its{0};
        timer_settime(timer_id_, 0, &its, NULL);
    }

private:

    void check_and_run(const sigval &v) {
        bool need_run = true;
        {
            std::lock_guard<std::mutex> lk(mut_cnt_);
            curr_time_++;
            need_run = curr_time_ <= config_.run_time_;
        }

        if (need_run) {
            func_wrapper_(v);
        } else {
            stop();
        }
    }

    static void timer_func_exec(sigval v) {
        auto object_ptr = (CPPTimer *) v.sival_ptr;
        if (object_ptr) {
            object_ptr->check_and_run(v);
        }
    }

    timer_t timer_id_;
    struct sigevent sigev_{0};
    std::function<callback_type> func_wrapper_;
    std::mutex mut_cnt_;
    const Config config_;
    std::atomic<uint32_t> curr_time_ = {0};
    const void *object_;

};

#ifndef linux
static_assert(false && "Just support linux");
#endif

#endif //UTILS_CPP_TIMER_