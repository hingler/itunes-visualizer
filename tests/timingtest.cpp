#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "timing/timing.hpp"

// should probably get rid of these
#define ITR_COUNT 100
#define MS_JUMP 25
#define RL_EPS 0.05

#define CHR std::chrono

typedef std::chrono::high_resolution_clock HRC;


TEST(EnterTheFunkoverse, EvalGlobalTimerAccuracy) {
    HRC::time_point init_time = HRC::now();
    double init_lib = Timer::GetGlobalTime();

    double libDiff;
    double expDiff;
    double hrcDiff;

    double thisDelta;
    double lastDelta = 0.0;
    double lastLib = 0.0;

    double eps_calc;

    Timer::GetDelta();

    for (int i = 1; i <= ITR_COUNT; i++) {
        std::this_thread::sleep_for(CHR::milliseconds(MS_JUMP));
        // after jump, evaluate accuracy bounds, check if close enough

        libDiff = Timer::GetGlobalTime() - init_lib;
        hrcDiff = CHR::duration<double, std::milli>(HRC::now() - init_time).count();
        expDiff = (double)MS_JUMP * i;

        thisDelta = Timer::GetDelta();
        lastDelta = libDiff - lastLib;

        eps_calc = i * RL_EPS;

        printf("expected duration: %.3f ms\n", (double)MS_JUMP * i);
        printf("chrono duration: %.3f ms\n", hrcDiff);
        printf("timing lib duration: %.3f ms\n\n", libDiff);

        ASSERT_NEAR(libDiff, hrcDiff, eps_calc);
        ASSERT_NEAR(thisDelta, lastDelta, RL_EPS);

        lastLib = libDiff;
    }
}

TEST(EnterTheFunkoverse, EvalTimerClassAccuracy) {
    HRC::time_point init_time = HRC::now();

    Timer::TimerInstance<std::milli> timer;

    double init_lib = timer.GetDelta();

    double hrcDiff;
    double libDiff;
    double hrcDelta;
    double lastLib = 0.0;
    double libDelta;

    double eps_calc;

    timer.ResetTimer();

    Timer::TimerInstance<std::milli> timerDelta(timer);
    timerDelta.ResetTimer();
    for (int i = 1; i <= ITR_COUNT; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_JUMP));

        hrcDiff = CHR::duration<double, std::milli>(HRC::now() - init_time).count();
        libDiff = timer.GetDelta();
        libDelta = timerDelta.GetDelta();
        hrcDelta = hrcDiff - lastLib;
        lastLib = hrcDiff;
        
        timerDelta.ResetTimer();

        eps_calc = i * RL_EPS;

        printf("chrono duration: %.3f ms\n", hrcDiff);
        printf("lib duration: %.3f ms\n", libDiff);
        // something weird is going on with the deltas
        printf("chrono delta: %.3f ms\n", hrcDelta);
        printf("lib delta: %.3f ms\n\n", libDelta);

        ASSERT_NEAR(libDiff, hrcDiff, eps_calc);
        ASSERT_NEAR(libDelta, hrcDelta, RL_EPS);
    }
}