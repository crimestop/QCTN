#ifndef NET_TENSOR_CONTRACT_EXACT_HPP
#define NET_TENSOR_CONTRACT_EXACT_HPP
#include "../network.hpp"
#include "../rational.hpp"
#include "../tensor_tools.hpp"
#include "../traits.hpp"
#include "../group.hpp"
#include "tensor_contract_engine.hpp"
#include <TAT/TAT.hpp>
#include <cstdlib>
#include <functional>
#include <random>
#include <variant>
#include <memory>
#include <vector>
#include <iostream>

namespace net {

	struct all_combination{
		std::vector<bool> elems;
		all_combination(unsigned int s):elems(std::vector<bool>(s,false)),size(s){};
		unsigned int size;
		bool finish;
		bool reversed=false;
		void begin(unsigned int s){
			if(s>=0 && s<=size){
				finish = false;
				if(s>size/2){
					reversed=true;
					for(int i=0;i<size;++i)
						elems[i]=(i<size-s);
				}else{
					reversed=false;
					for(int i=0;i<size;++i)
						elems[i]=(i<s);
				}
			}else
				finish=true;
		}
		void next(){
			int true_num=0;
			int i;
			for(i=0;i<size-1 && !(elems[i] && ! elems[i+1]);++i){
				if(elems[i]) true_num++;
			}
			//std::cout<<i<<' '<<size<<'\n';
			if(i==size-1)
				finish=true;
			else{
				if(true_num<i)
					for(int j=0;j<i;j++)       // true for 0 ... true_num-1
						elems[j]=(j<true_num); // false for true_num ... i-1
				elems[i]=false;
				elems[i+1]=true;
			}

		}
		template<typename ValSet>
		std::pair<ValSet,ValSet> generate(const ValSet & V){
			std::pair<ValSet,ValSet> result;
			int i=0;
			if(reversed){
				for(auto & v:V)
					elems[i++] ? result.second.insert(v): result.first.insert(v);
			}else{
				for(auto & v:V)
					elems[i++] ? result.first.insert(v): result.second.insert(v);
			}
			return result;
		}
	};

	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeKey Engine::contract_breadth_first(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {

		if(part.size()==2){
			auto it=part.begin();
			auto it2=part.begin();
			++it2;
			lat.absorb(*it,*it2,absorb_fun,contract_fun);
			return *it;
		}

		std::map<std::set<NodeKey, typename Trait::nodekey_less>,group<NodeVal, int, NodeKey, EdgeKey, Trait>> solution;
		
		for(auto & p:part){
			//std::cout<<p<<"==="<<std::endl;
		}
		for(auto & p:part){
			//std::cout<<p<<"---"<<std::endl;
			auto x =std::set<NodeKey, typename Trait::nodekey_less>({p});
			auto y=group<NodeVal, int, NodeKey, EdgeKey, Trait>(lat);
			y.absorb(p,absorb_fun,contract_fun);
			std::get<0>(y.val)=std::make_shared<typename std::tuple_element_t<0,decltype(y.val)>::element_type>(tree<typename std::tuple_element_t<0,decltype(y.val)>::element_type::DataType>(std::get<0>(y.val)->val.forget_history(p)));
			solution.insert({x,y});
		}

		all_combination ac(part.size());
		for(int s=2; s<=part.size();++s){
			std::cout<<s<<' '<<part.size()<<"bfirst\n";
			for(ac.begin(s);!ac.finish;ac.next()){
				auto [P,N] = ac.generate(part); // divide part into P and N, P.size()=s
			// std::cout<<"P [ ";
			// for(auto & p: P) std::cout<<p<<' ';
			// std::cout<<"]\n";
			// std::cout<<"N [ ";
			// for(auto & p: N) std::cout<<p<<' ';
			// std::cout<<"]\n";
				all_combination ac2(P.size());
				for(int s2=1;s2<=P.size()/2;++s2){
					for(ac2.begin(s2);!ac2.finish;ac2.next()){
						auto [L,R] = ac2.generate(P);  // divide part into P and N, L.size()=s2 <= R.size()
			// std::cout<<"L [ ";
			// for(auto & p: L) std::cout<<p<<' ';
			// std::cout<<"]\n";
			// std::cout<<"R [ ";
			// for(auto & p: R) std::cout<<p<<' ';
			// std::cout<<"]\n";
						if(P.size()%2==0 && s2==P.size()/2 && L>R) // if L.size() == R.size(), require L <= R
							continue;
						group<NodeVal, int, NodeKey, EdgeKey, Trait> G=net::contract<NodeVal, int, NodeKey, EdgeKey, Trait>(solution[L],solution[R],absorb_fun,contract_fun);
						auto it=solution.find(P);
						if(it==solution.end()){
							solution.insert({P,G});
						}else{
							//std::cout<<std::get<0>(G.val)->val.contraction_cost<<'\n';
							//std::cout<<std::get<0>(it->second.val)->val.contraction_cost<<'\n';
							if(std::get<0>(G.val)->val.contraction_cost < std::get<0>(it->second.val)->val.contraction_cost){
								it->second=G;
							}
						}
					}
				}
			}
		}
		//std::get<0>(solution[part].val)->draw();
		NodeKey res=lat.absorb_tree(std::get<0>(solution[part].val),absorb_fun,contract_fun);
		//std::cout<<res<<"=result="<<std::endl;
		return res;
	}


} // namespace net
#endif
