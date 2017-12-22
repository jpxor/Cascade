
#pragma once

#include <mutex>

namespace Cascade {

    template<typename Type>
    auto Count = [](const int& count, const Type& val){ return count+1; };

    template<typename Type>
    auto Sum = [](const int& sum, const Type& val){ return sum+val; };

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


    template<typename Type>
    auto Max = [](const int& max, const Type& val){ 
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