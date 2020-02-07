#ifndef TIMER_H_
#define TIMER_H_
#include <chrono>

namespace Timer {
    /**
     * Returns the amount of time passed since execution began.
     * 
     * Returns:
     *  - number of milliseconds since execution began.
     */ 
    double GetGlobalTime();

    /**
     * Returns the amount of time passed since `getDelta` was last called.
     * 
     * Returns:
     *  - Number of milliseconds since last call. 
     */ 
    double GetDelta();

    /**
     * Returns amount of time passed since `getDelta` was last called
     * while preserving last delta change.
     */ 
    double PollDelta();

    template <typename T = std::milli>
    class TimerInstance {
     private:
      std::chrono::time_point<std::chrono::high_resolution_clock> timerStart;
     public:
      TimerInstance<T>() : timerStart(std::chrono::high_resolution_clock::now()) {};

      TimerInstance<T>(const TimerInstance<T> &ti) : timerStart(ti.timerStart) {};
        
      double GetDelta() {
        std::chrono::high_resolution_clock::time_point newDelta = 
          std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, T> dur(newDelta - timerStart);
        return dur.count();
      }

      void ResetTimer() {
        timerStart = std::chrono::high_resolution_clock::now();
      }
    };  // TimerInstance
};
#endif  // TIMER_H_