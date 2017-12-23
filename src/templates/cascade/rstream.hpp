
#pragma once

#include <functional>
#include <memory>
#include <future>

namespace Cascade {
	
	template<typename Type>
	class rStream {
	public:	
		rStream(){}
		~rStream(){}
		rStream(const rStream& other) = delete;
		
		std::vector<std::function<void(const Type&)>> reactions;
		
		void insert(const Type& val){
			if( reactions.size() == 1 ){
				reactions[0](val);
				return;
			}
			std::vector<std::future<void>> futures;
			for(auto reaction : reactions){
				futures.emplace_back(std::async( std::launch::async, reaction, val ));
			}
			for(auto& future : futures){
				future.get();
			}
		}
		
		std::shared_ptr<rStream<Type>> react(std::function<void(const Type&)> reaction){
			auto next_seg = std::make_shared<rStream<Type>>();
			reactions.push_back(
				[reaction, next_seg](const Type& val){
					reaction(val);
					next_seg->insert(val); 
				}
			);
			return next_seg;
		}
		
		std::shared_ptr<rStream<Type>> delay(int ms){
			auto delayed_stream = std::make_shared<rStream<Type>>();
			reactions.push_back(
				[ms, delayed_stream](const Type& val){
					std::this_thread::sleep_for(std::chrono::milliseconds(ms));
					delayed_stream->insert(val); 
				}
			);
			return delayed_stream;
		}
		
		std::shared_ptr<rStream<Type>> filter(std::function<bool(const Type&)> predicate){
			auto conditional_stream = std::make_shared<rStream<Type>>();
			reactions.push_back(
				[predicate, conditional_stream](const Type& val){
					if( predicate(val) ){
						conditional_stream->insert(val); 
					}
				}
			);
			return conditional_stream;
		}
		
		template<typename MType>
		std::shared_ptr<rStream<MType>> map(std::function<MType(const Type&)> mapping){
			auto mapped_stream = std::make_shared<rStream<MType>>();
			reactions.push_back(
				[mapping, mapped_stream](const Type& val){
					mapped_stream->insert(mapping(val));
				}
			);
			return mapped_stream;
		}

		template<typename RType>
		std::shared_ptr<rStream<RType>> reduce(std::function<RType(RType&, const Type&)> reduction, RType init_value = 0){
			auto reduced_stream = std::make_shared<rStream<RType>>();
			reactions.push_back(
				[reduction, reduced_stream, init_value](const Type& val){
					static std::mutex red_mutex;
					static RType reduced_val = init_value;
					RType reduced_copy;
					{//critical section
						std::lock_guard<std::mutex> guard(red_mutex);
						reduced_val = reduction(reduced_val, val);
						reduced_copy = reduced_val;
					}
					reduced_stream->insert(reduced_copy);
				}
			);
			return reduced_stream;
		}
		
		std::shared_ptr<rStream<std::vector<Type>>> buffer(unsigned int count){
			auto buffered_stream = std::make_shared<rStream<std::vector<Type>>>();
			reactions.push_back(
				[count, buffered_stream](const Type& val){
					static std::vector<Type> buffer; 
					static std::mutex buf_mutex;

					std::vector<Type> tmp_buffer;
					{//critical section
						std::lock_guard<std::mutex> guard(buf_mutex);
						buffer.push_back(val);

						// swap full static buffer with empty local buffer
						if(buffer.size() == count){
							tmp_buffer.swap(buffer);
						}
					}
					// push full buffer downstream without blocking
					// new elements from being buffered
					if(tmp_buffer.size() == count){
						buffered_stream->insert(tmp_buffer);
					}
				}
			);
			return buffered_stream;
		}
	};

	template<typename Type>
	std::shared_ptr<rStream<Type>> make_stream(){
		return std::make_shared<rStream<Type>>();
	}

}
