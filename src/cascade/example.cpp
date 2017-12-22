
#include "cascade/rstream.hpp"
#include <iostream>

template<typename Type>
void print_buffer(std::vector<Type> buf){
	std::cout << "[ ";
	for(Type t : buf){ std::cout << t << " "; }
	std::cout << "]" << std::endl;
}

int main(){

	using namespace Cascade;
	
	// A Cascade::rStream is a composition of segments, where inserted values
	// cascade down-stream, triggering reactions in each segment 
	auto stream = make_stream<int>();
	
	// Each call to the stream will create and add a new segment
	// References to down-stream segments are kept as a shared_ptr
	auto next_segment = stream->react( [](int i){ std::cout << "Hello Cascade!" << std::endl; } );

	// Values can be inserted into any segment
	next_segment->insert(0);

	// A chain of calls will build a chain of segments and they are executed in order
	next_segment->react( [](int i){ std::cout << "Chain"; } )
		->react( [](int i){ std::cout << " reactions"; } )
		->react( [](int i){ std::cout << "! val: " << i << std::endl; } );

	// Multiple calls to the same segment will fork the stream, and each new path 
	// is executed in parallel (std::async)
	auto delayed = next_segment->delay(200);
	delayed->react( [](int i){ std::cout << "a"; } );
	delayed->react( [](int i){ std::cout << "b"; } );
	delayed->react( [](int i){ std::cout << "c"; } );
	delayed->react( [](int i){ std::cout << "d"; } );
	delayed->react( [](int i){ std::cout << "e"; } );
	delayed->react( [](int i){ std::cout << "f"; } );
	delayed->react( [](int i){ std::cout << "g"; } );
	delayed->react( [](int i){ std::cout << "h"; } );
	delayed->react( [](int i){ std::cout << "i"; } );
	
	// Delay pauses execution, duration is specified in milliseconds
	auto delayed_longer = next_segment->delay(400)
	
		// React allows executing arbitrary code each time a new value is inserted
		->react( [](int i){ std::cout << "\n> react is akin to a container's apply" << std::endl; } );

	// Filter can be used to stop the flow from reaching down-stream segments
	delayed_longer->filter( [](int i){ return i%2 == 0; }  ) 

		// Map allows you to change the stream value or even type
		->map<float>( [](int i){ return 0.25f*(float)i; } )

		// Buffer will cause the flow to stop until a specified number of values are inserted
		// These buffered values are mapped into a vector before continuing
		->buffer(2)

		// Function pointers can be used as well
		->react(print_buffer<float>);

	//referencing an up-stream segments from within a lambda will result in a ptr cycle
	//and memory leaks. Create a weak pointer for this
	std::weak_ptr<rStream<int>> upstream_ref = delayed_longer;

	delayed_longer->filter([](int i){ return i < 10; })
		->react( 
		[upstream_ref](int i) { 
			if(auto upstream = upstream_ref.lock()){
				upstream->insert(i+1); 
			}
		});

	stream->insert(0);
	std::cout << "this is run after the inserted value is done cascading through the entire network" << std::endl;
}
