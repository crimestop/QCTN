#ifndef NET_TENSOR_CONTRACT_HPP
#define NET_TENSOR_CONTRACT_HPP
#include "../network.hpp"
#include "../rational.hpp"
#include "../tensor_tools.hpp"
#include "../traits.hpp"
#include "../group.hpp"
#include "tensor_contract_engine.hpp"
#include "tensor_contract_qbb.hpp"
#include "tensor_contract_exact.hpp"
#include "tensor_contract_divide.hpp"
#include "tensor_contract_divide_naive.hpp"
#include <TAT/TAT.hpp>
#include <cstdlib>
#include <functional>
#include <random>
#include <variant>
#include <memory>
#include <vector>
#include <iostream>

namespace net {

	template <typename contract_type>
	struct lift_contract {
		contract_type contract_fun;
		lift_contract(contract_type cf) : contract_fun(cf){};
		template <typename NodeVal, typename NoUse>
		NodeVal operator()(const NodeVal & ten1, const NodeVal & ten2, const NoUse & inds) const {
			return std::make_tuple(contract_fun(std::get<0>(ten1), std::get<0>(ten2), inds), std::get<1>(ten1), std::get<2>(ten1));
		}
	};

	template <typename absorb_type>
	struct lift_absorb {
		absorb_type absorb_fun;
		lift_absorb(absorb_type af) : absorb_fun(af){};
		template <typename NodeVal, typename EdgeVal, typename NoUse>
		NodeVal operator()(const NodeVal & ten1, const EdgeVal & eg, const NoUse & ind) const {
			return std::make_tuple(absorb_fun(std::get<0>(ten1), eg, ind), std::get<1>(ten1), std::get<2>(ten1));
		}
	};

	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeVal Engine::contract(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {
		network<std::tuple<NodeVal, int, net::rational>, int, NodeKey, EdgeKey, Trait> temp;
		temp = lat.template fmap<decltype(temp)>(
				[](const NodeVal & ten) { return std::make_tuple(ten, 0, net::rational(0, 1)); }, [](const int & m) { return m; });
		std::string final_site = inner_contract(temp, part, lift_absorb(absorb_fun), lift_contract(contract_fun));
		return std::get<0>(temp[final_site].val);
	}
	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeVal Engine::contract_qbb(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {
		auto temp_small_part_size=small_part_size;
		auto temp_small_part_method=small_part_method;
		small_part_size=lat.size();
		small_part_method="quickbb";
		auto res=contract(lat,part,absorb_fun,contract_fun);
		small_part_size=temp_small_part_size;
		small_part_method=temp_small_part_method;
		return res;
	}
	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeVal Engine::contract_ect(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {
		auto temp_small_part_size=small_part_size;
		auto temp_small_part_method=small_part_method;
		small_part_size=lat.size();
		small_part_method="exact";
		auto res=contract(lat,part,absorb_fun,contract_fun);
		small_part_size=temp_small_part_size;
		small_part_method=temp_small_part_method;
		return res;
	}
	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeVal Engine::contract_naive(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {
		network<std::tuple<NodeVal, int, net::rational>, int, NodeKey, EdgeKey, Trait> temp;
		temp = lat.template fmap<decltype(temp)>(
				[](const NodeVal & ten) { return std::make_tuple(ten, 0, net::rational(0, 1)); }, [](const int & m) { return m; });
		return std::get<0>(temp.contract(part,lift_absorb(absorb_fun), lift_contract(contract_fun)));
	}

	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeKey Engine::inner_contract(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {

		auto parts=disconnect(lat,part,{part});
		for(auto & p:parts)
			connected_inner_contract(lat, p, absorb_fun, contract_fun,0.,false," ");
		// for(auto & s:lat){
		// 	std::cout<<s.first<<'\n';
		// }
		// 	std::cout<<"---\n";
		auto s0=lat.begin()->first;
		std::set<std::string> sites;
		for(auto & s:lat)
			if(s.first !=s0)
				sites.insert(s.first);
		for(auto & s:sites)
			lat.absorb(s0,s,absorb_fun, contract_fun);
		return lat.begin()->first;

	}

	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeKey Engine::connected_inner_contract(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun,double uneven, bool fix_uneven,std::string log) {

		//std::cout<<"in inner"<<std::endl;
		NodeKey contract_res1,contract_res2;
		
		double new_eneven,best_eneven,min_cost,this_cost;
		//network<NodeVal, int, NodeKey, EdgeKey, Trait>  temp2 = lat;

		bool failed;
		//std::cout<<max_quickbb_size<<','<<part.size()<<'\n';
		if (part.size() > small_part_size) {
			if(!fix_uneven){
				min_cost=-1.;
				for(int j=0;j<50;++j){
					NodeKey contract_res;
					network<NodeVal, int, NodeKey, EdgeKey, Trait>  temp = lat;
					new_eneven=0.02*j;
					//std::cout<<"part = "<<eg.cut_part<<" uneven = "<<eg.uneven<<'\n';
					std::vector<std::set<NodeKey, typename Trait::nodekey_less>> subparts = divide_kahypar(temp, part,new_eneven,failed);

					// std::cout<<"portal1 "<<new_eneven;
					// for (auto & p : subparts)
					// 	std::cout<<' '<<p.size();
					// std::cout<<std::endl;
					
					std::set<NodeKey, typename Trait::nodekey_less> new_part;
					for (auto & p : subparts){
						new_part.insert(connected_inner_contract(temp, p, absorb_fun, contract_fun,new_eneven,true," "));
					}
					contract_res=contract_quickbb(temp, new_part, absorb_fun, contract_fun);
					this_cost=std::get<0>(temp[contract_res].val)->val.contraction_cost;
					//std::cout<<new_eneven<<' '<<this_cost<<'\n';
					if(min_cost<0 || this_cost<min_cost){
						min_cost=this_cost;
						best_eneven=new_eneven;
					}
					if(failed) break;
				}
				if(verbose) std::cout<<log<<' '<<best_eneven<<' '<<min_cost<<'\n';
				std::vector<std::set<NodeKey, typename Trait::nodekey_less>> subparts = divide_kahypar(lat, part,best_eneven,failed);
				std::set<NodeKey, typename Trait::nodekey_less> new_part;
				int sp=0;
				for (auto & p : subparts){
					new_part.insert(connected_inner_contract(lat, p, absorb_fun, contract_fun,best_eneven,!rec_partition,log+std::to_string(sp)));
					sp++;
				}
				//std::cout<<"out inner"<<std::endl;
				return contract_quickbb(lat, new_part, absorb_fun, contract_fun);

			}else{
				std::vector<std::set<NodeKey, typename Trait::nodekey_less>> subparts = divide_kahypar(lat, part,uneven,failed);
				std::set<NodeKey, typename Trait::nodekey_less> new_part;

					// std::cout<<"portal2 "<<part.size()<<subparts.size()<<uneven;
					// for (auto & p : subparts)
					// 	std::cout<<' '<<p.size();
					// std::cout<<std::endl;
				for (auto & p : subparts)
					new_part.insert(connected_inner_contract(lat, p, absorb_fun, contract_fun,uneven,true," "));
				//std::cout<<"out inner"<<std::endl;
				return contract_quickbb(lat, new_part, absorb_fun, contract_fun);

			}

		} else {
			//std::cout<<"out inner"<<std::endl;
			if(!fix_uneven){
				if(small_part_method=="exact"){
					return contract_breadth_first(lat, part, absorb_fun, contract_fun);
				}
				else{
					return contract_quickbb(lat, part, absorb_fun, contract_fun);
				}
			}else{
				return contract_quickbb(lat, part, absorb_fun, contract_fun);
			}
		}
	}

	template <typename KeySetType>
	struct keyset {
		KeySetType node_set;
		static keyset<KeySetType> absorb(const keyset<KeySetType> & a, const int & b) {
			return a;
		};
		static keyset<KeySetType> contract(const keyset<KeySetType> & a, const keyset<KeySetType> & b) {
			keyset<KeySetType> r = a;
			r.node_set.insert(b.node_set.begin(), b.node_set.end());
			return r;
		}
		keyset() = default;
		template <typename NodeType>
		keyset(const typename KeySetType::key_type & k, const NodeType & n) {
			node_set.insert(k);
		}
		std::string show()const{
			return "";
		}
		keyset<KeySetType> forget_history(const typename KeySetType::key_type & k) {
			keyset<KeySetType> ci;
			ci.node_set.insert(k);
			return ci;
		}
	};

	template <typename KeySetType>
	struct contract_info {
		KeySetType node_set;
		long long int this_weight = 1;
		long long int hist_max_weight = 1;
		long long int contraction_cost = 1;
		long long int legs = 1;
		static contract_info<KeySetType> absorb(contract_info<KeySetType> & c, const int & d) {
			contract_info<KeySetType> r = c;
			r.legs *= d;
			return r;
		};
		static contract_info<KeySetType> contract(contract_info<KeySetType> & c, contract_info<KeySetType> & d) {
			contract_info<KeySetType> r;
			r.legs = 1;
			r.node_set = c.node_set;
			r.node_set.insert(d.node_set.begin(), d.node_set.end());
			r.this_weight = c.this_weight / c.legs / d.legs * d.this_weight / c.legs / d.legs;
			r.contraction_cost = c.contraction_cost + d.contraction_cost + c.this_weight / c.legs / d.legs * d.this_weight;
			r.hist_max_weight = std::max(std::max(c.this_weight, d.this_weight), r.this_weight);
			c.legs=1;
			d.legs=1;
			return r;
		}
		std::string show()const{
			std::ostringstream os;
			os.precision(4);
			os<<std::fixed<<"C "<<contraction_cost<<"\nW "<<this_weight<<"\nH "<<hist_max_weight;
			return os.str();
		}
		contract_info() = default;
		template <typename NodeType>
		contract_info(const typename KeySetType::key_type & k, const NodeType & n) {
			node_set.insert(k);
			this_weight = net::tensor::get_size(n);
			hist_max_weight = this_weight;
		}
		contract_info<KeySetType> forget_history(const typename KeySetType::key_type & k) {
			contract_info<KeySetType> ci;
			ci.node_set.insert(k);
			ci.this_weight = this_weight;
			ci.hist_max_weight = hist_max_weight;
			ci.contraction_cost = contraction_cost;
			ci.legs = legs;
			return ci;
		}
	};

	constexpr double exp_sum_log(const double & a,
										  const double & b) { // return log(exp(a)+exp(b))
		double ratio = 0.;
		if (a > b + 10) {
			ratio = std::pow(10,b - a);
			return a + ratio - ratio * ratio / 2. + ratio * ratio * ratio / 3.;
		} else if (b > a + 10) {
			ratio = std::pow(10,a - b);
			return b + ratio - ratio * ratio / 2. + ratio * ratio * ratio / 3.;
		} else
			return b + std::log10(1 + std::pow(10,a - b));
	}

	template <typename KeySetType>
	struct contract_info2 {
		KeySetType node_set;
		double this_weight = 0.;
		double hist_max_weight = 0.;
		double contraction_cost = 0.;
		double legs = 0.;
		static contract_info2<KeySetType> absorb(contract_info2<KeySetType> & c, const int & d) {
			contract_info2<KeySetType> r = c;
			r.legs += std::log10(double(d));
			return r;
		}
		static contract_info2<KeySetType> contract(contract_info2<KeySetType> & c, contract_info2<KeySetType> & d) {
			contract_info2<KeySetType> r;
			r.legs = 0.;
			r.node_set = c.node_set;
			// std::cout<<"before\n";
			// for (auto & test: d.node_set){
			// 	std::cout<<'\n';
			// 	std::cout<<test<<'\n';
			// } 
			r.node_set.insert(d.node_set.begin(), d.node_set.end());

			//std::cout<<"after\n";
			r.this_weight = c.this_weight + d.this_weight - 2 * c.legs - 2 * d.legs;
			r.contraction_cost = exp_sum_log(exp_sum_log(c.contraction_cost, d.contraction_cost), c.this_weight - c.legs - d.legs + d.this_weight);
			r.hist_max_weight = std::max(std::max(c.this_weight, d.this_weight), r.this_weight);
			c.legs=0.;
			d.legs=0.;
			return r;
		}
		std::string show()const{
			std::ostringstream os;
			os.precision(4);
			os<<std::fixed<<"C "<<contraction_cost<<"\nW "<<this_weight<<"\nH "<<hist_max_weight;
			return os.str();
		}
		contract_info2() = default;
		template <typename NodeType>
		contract_info2(const typename KeySetType::key_type & k, const NodeType & n) {
			node_set.insert(k);
			this_weight = std::log10(double(net::tensor::get_size(n)));
			hist_max_weight = this_weight;
		}
		contract_info2<KeySetType> forget_history(const typename KeySetType::key_type & k) {
			contract_info2<KeySetType> ci;
			ci.node_set.insert(k);
			ci.this_weight = this_weight;
			ci.hist_max_weight = hist_max_weight;
			ci.contraction_cost = contraction_cost;
			ci.legs = legs;
			return ci;
		}
	};

	template <template <typename> typename TreeVal, typename NetType>
	std::shared_ptr<net::tree<TreeVal<typename NetType::NodeKeySetType>>> get_contract_tree(const NetType & lat, Engine & eg, const std::string & method) {
		net::network<std::shared_ptr<net::tree<TreeVal<typename NetType::NodeKeySetType>>>, int, typename NetType::NodeKeyType, typename NetType::EdgeKeyType> temp;

		temp = lat.template gfmap<decltype(temp)>(
				[](const typename NetType::NodeKeyType & nodek, const typename NetType::NodeValType & nodev) {
					return std::make_shared<net::tree<TreeVal<typename NetType::NodeKeySetType>>>(TreeVal<typename NetType::NodeKeySetType>(nodek, nodev));
				},
				[](const typename NetType::NodeKeyType & nodek1,
					const typename NetType::NodeValType & nodev1,
					const typename NetType::NodeKeyType & nodek2,
					const typename NetType::NodeValType & nodev2,
					const typename NetType::EdgeKeyType & ind1,
					const typename NetType::EdgeKeyType & ind2,
					const typename NetType::EdgeValType & ev) { return net::tensor::get_dim(nodev1, ind1); });

		std::set<std::string> includes;
		for (auto & n : lat)
			includes.insert(n.first);
		if(method=="partition"){
			return eg.contract(
					temp,
					includes,
					net::Tree_act<TreeVal<typename NetType::NodeKeySetType>>(),
					net::Tree_combine<TreeVal<typename NetType::NodeKeySetType>>());
		}else if(method=="quickbb"){
			return eg.contract_qbb(
					temp,
					includes,
					net::Tree_act<TreeVal<typename NetType::NodeKeySetType>>(),
					net::Tree_combine<TreeVal<typename NetType::NodeKeySetType>>());
		}else if(method=="exact"){
			return eg.contract_ect(
					temp,
					includes,
					net::Tree_act<TreeVal<typename NetType::NodeKeySetType>>(),
					net::Tree_combine<TreeVal<typename NetType::NodeKeySetType>>());
		}else if(method=="naive"){
			return eg.contract_naive(
					temp,
					includes,
					net::Tree_act<TreeVal<typename NetType::NodeKeySetType>>(),
					net::Tree_combine<TreeVal<typename NetType::NodeKeySetType>>());
		}else{
			return std::shared_ptr<net::tree<TreeVal<typename NetType::NodeKeySetType>>>(); // null ptr
		}
	}


} // namespace net
#endif
