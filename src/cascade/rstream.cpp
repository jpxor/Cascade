
#include "react/react.hpp"
#include <iostream>


bool equals10(float f){
	return f == 10;
}

void print_equals10(float f){
	std::cout << "Y: equals 10: " << f << " recursing!" << std::endl;
}

template<typename Type>
void print_vector(std::vector<Type> v){
	std::cout << "[ ";
	for(Type t : v){
		std::cout << t << " ";
	}
	std::cout << "]" << std::endl;
}

int main(){
	
	//creating a stream: 
	//streams must be referenced via shared_ptr
	auto stream = React::make_stream<float>();
	//OR
	auto another_stream = std::make_shared<React::rStream<float>>();
	
	//react is equivalent to a container's apply function,
	//it will execute for each value inserted into the stream,
	auto stream_ptr = stream->react( [](float f){ std::cout << "streaming: " << f << std::endl; } );
	
	//react returns a shared_ptr to the calling stream segment
	if( stream.get() == stream_ptr.get() ){
		std::cout << "they point to the same stream segment" << std::endl;
	}
	
	//all other functions create and return a new stream fragment,
	//you can store a pointer to any segment of the stream and inject (insert) values
	auto X = stream->map<int>( [](float f){ return 2*(int)f; } );
	X-> react( [](float f){ std::cout << "X: " << f << std::endl; } );
	X->insert(21);
		
	//references to down-stream segments of the stream are kept in 
	//a shared_ptr and so these are not deleted when scope exits
	{
		//inserted values are const& and so cannot be changed by the stream, 
		//map allows you to map to a new value or even type
		auto Y = stream->map<float>( [](float f){ return 0.25f*f; } )
			->map<int>( [](float f){ return ((int)(97*f))%17; } )
			->react( [](int f){ std::cout << "Y: filtering: " << f << std::endl; } );
			
		//filter is a conditional to either stop or allow a value from cascading to down-stream segments
		Y->filter([](int f)->bool{ return f < 10; })
			->react( [](int f){ std::cout << "Y: Less than 10: " << f << " STOPPING!" << std::endl; } );
			
		//referencing an up-stream segment from within a lambda will result in a ptr cycle
		//and memory leaks. Create a weak pointer for this,
		std::weak_ptr<React::rStream<float>> upstream_ref = stream;
			
		//streams can be forked to create multiple paths of execution
		Y->filter([](int f)->bool{ return f > 10; })
			->react( [](int f){ std::cout << "Y: Greater than 10: " << f << " recursing!" << std::endl; } )
			->react( [upstream_ref](int f) { 
					if(auto upstream = upstream_ref.lock()){
						upstream->insert(f*3); 
					}
				});

		//can use lambdas or function pointers
		Y->filter(equals10)
			->react(print_equals10)
			->react( [upstream_ref](int f) { 
					if(auto upstream = upstream_ref.lock()){
						upstream->insert(f/2); 
					}
				});
			
	}

	//add a delay in the stream,
	//Note that by default the streams are SERIAL and not PARALLEL! 
	X->delay(1000)
		//gather stream values into a vector buffer (maps type to vector<type>) 
		->buffer(3)
		->react(print_vector<int>);
		
	stream->insert(3.36);
	std::cout << "this is run after the inserted value is done cascading through the entire network" << std::endl;
}
