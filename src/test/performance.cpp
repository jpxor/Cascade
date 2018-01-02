
#include "cascade/nodes.hpp"
#include "cascade/functions.hpp"
#include <iostream>
#include <stdio.h>
#include <chrono>

class Timer {
private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
public:
    Timer() : beg_(clock_::now()) {;;}
    void reset() { 
        beg_ = clock_::now(); 
    }
    double elapsed() const { 
        return std::chrono::duration_cast<second_>(clock_::now() - beg_).count(); 
    }
};

// multiple implementations of the streams were tested
template<typename Type> 
void build_stream( std::shared_ptr<Type> stream, int depth ){
    stream->react([](float val){std::cout << "Testing " << val << std::endl;});
    
    auto tmp = stream->map<int>([](float val){return (int)(31*val);})
        ->map<float>([](int val){return (float)(val%7);});

    for(int i = 0; i < depth; ++i){
        tmp = tmp->map<int>([](float val){return (int)(31*val);})
            ->map<float>([](int val){return (float)(val%7);});
    }
    // tmp->react([](float val){ printf("final value: %f", val); });
    tmp->react([](float val){std::cout << "final value: " << val << std::endl;});
}

int main(){

	using namespace Cascade;
    
	auto node = make_node<float>();
    build_stream<CascadeNode<float,float>>(node, 300);

    Timer timer;
    try{
        timer.reset();
        node->insert(2.0);
        std::cout << "warmup: "  << timer.elapsed() << std::endl;

        timer.reset();
        node->insert(3.0);
        std::cout << "second run: "  << timer.elapsed() << std::endl;
    }
    catch (const std::exception& e) { std::cout << e.what() << std::endl; }
    
}