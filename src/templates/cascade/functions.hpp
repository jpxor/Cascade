
#pragma once

#include <mutex>
#include <math.h>
#include <algorithm>

namespace Cascade {

    template<typename Type>
    auto Count = [](const int& count, const Type& val){ return count+1; };

    template<typename Type>
    auto Sum = [](const int& sum, const Type& val){ return sum+val; };

    template<typename Type>
    auto Max = [](const Type& red, const Type& val){ return std::max(red, val); };

    template<typename Type>
    auto Min = [](const Type& red, const Type& val){ return std::min(red, val); };

    template<typename Type>
    auto Sigmoid = [](const Type& val){ return 1.f/( 1.f + exp(-val) ); };

    template<typename Type>
    auto Average = [](const int& avg, const Type& val){ 
        static std::mutex mutex;
        static int count = 0;
        static Type sum = 0;
        {//critical section
            std::lock_guard<std::mutex> guard(mutex);
            ++count;
            sum += val;
            return sum/count; 
        }
    };

}
