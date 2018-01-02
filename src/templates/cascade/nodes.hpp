
#pragma once

#include <functional>
#include <memory>
#include <future>

namespace Cascade {

	template<typename InType>
	class Node {
	public:
		virtual void insert(const InType& val) = 0;
	};

	template<typename InType, typename OutType>
	class CascadeNode : public Node<InType> {
	public:
		std::vector<std::shared_ptr<Node<OutType>>> child_nodes;
		std::function<void(const InType&)> behaviour;

		CascadeNode(){}
		CascadeNode(const CascadeNode& other) = delete;
		CascadeNode(std::function<void(const InType&)> node_behaviour) : behaviour(node_behaviour) {}
		~CascadeNode(){}
		
		void dispatch(const OutType& out_val){
			if( child_nodes.size() == 1 ){
				child_nodes[0]->insert(out_val);
				return;
			}
			std::vector<std::future<void>> futures;
			for(auto& child : child_nodes){
				Node<OutType>* node = child.get();
				futures.emplace_back(std::async( std::launch::async, &Node<OutType>::insert, node, out_val ) );
			}
			for(auto& future : futures){
				future.get();
			}
		}

		void insert(const InType& val) {
			if(behaviour) {
				return behaviour(val);
			}
		}

		template<typename MType>
		std::shared_ptr<CascadeNode<OutType,MType>> map(std::function<MType(const OutType&)> mapper){
			auto chnode = std::make_shared<CascadeNode<OutType,MType>>();
			chnode->behaviour = [chnode, mapper](const OutType& val){
				chnode->dispatch(mapper(val)); 
			};
			child_nodes.push_back(chnode);
			return chnode;
		}

		template<typename RType>
		std::shared_ptr<CascadeNode<OutType,RType>> reduce(std::function<RType(const RType&, const OutType&)> reducer, RType default_val = 0){
			auto chnode = std::make_shared<CascadeNode<OutType,RType>>();
			chnode->behaviour = [chnode, reducer, default_val](const OutType& val){
				static RType reduced = default_val;
				reduced = reducer(reduced, val);
				chnode->dispatch(reduced); 
			};
			child_nodes.push_back(chnode);
			return chnode;
		}

		std::shared_ptr<CascadeNode<OutType,OutType>> react(std::function<void(const OutType&)> reaction){
			auto chnode = std::make_shared<CascadeNode<OutType,OutType>>();
			chnode->behaviour = [chnode, reaction](const OutType& val){
				reaction(val);
				chnode->dispatch(val); 
			};
			
			child_nodes.push_back(chnode);
			return chnode;
		}

		std::shared_ptr<CascadeNode<OutType,OutType>> filter(std::function<bool(const OutType&)> predicate){
			auto chnode = std::make_shared<CascadeNode<OutType,OutType>>();
			chnode->behaviour = [chnode, predicate](const OutType& val){
				if(predicate(val)){
					chnode->dispatch(val); 
				}
			};
			child_nodes.push_back(chnode);
			return chnode;
		}

		std::shared_ptr<CascadeNode<OutType,OutType>> delay(int ms){
			auto chnode = std::make_shared<CascadeNode<OutType,OutType>>();
			chnode->behaviour = [chnode, ms](const OutType& val){
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				chnode->dispatch(val); 
			};
			child_nodes.push_back(chnode);
			return chnode;
		}

		std::shared_ptr<CascadeNode<OutType,std::vector<OutType>>> buffer(int count){
			auto chnode = std::make_shared<CascadeNode<OutType,std::vector<OutType>>>();
			chnode->behaviour = [chnode, count](const OutType& val) {
				static std::vector<OutType> buffer;
				static std::mutex buf_mutex;

				std::vector<OutType    > tmp_buffer;
				{ //critical section
					std::lock_guard<std::mutex> guard(buf_mutex);
					buffer.push_back(val);

					// swap full static buffer with empty local buffer
					if (buffer.size() == count) {
						tmp_buffer.swap(buffer);
					}
				}
				// push full buffer downstream without blocking
				// new elements from being buffered
				if (tmp_buffer.size() == count) {
					chnode->dispatch(tmp_buffer);
				}

			};
			child_nodes.push_back(chnode);
			return chnode;
		}
		
	};

	template<typename Type>
	std::shared_ptr<CascadeNode<Type,Type>> make_node(){
		auto node = std::make_shared<CascadeNode<Type,Type>>();
		node->behaviour = [node](const Type& val){ node->dispatch(val); };
		return node;
	}

}
