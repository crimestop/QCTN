#ifndef NET_TENSOR_CONTRACT_QUICKBB_HPP
#define NET_TENSOR_CONTRACT_QUICKBB_HPP
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

	// find neighbor with least contraction count of a node within given part
	// EdgeVal = int
	template <typename IterNode, typename NodeSet, typename EdgeKey, typename Trait>
	std::pair<std::pair<int,int>, IterNode> search_quick(IterNode & it1, const NodeSet & part) {
		std::map<EdgeKey, std::pair<std::pair<int,int>, IterNode>, typename Trait::edgekey_less> legs;
		for (auto & e : it1->second.edges) {
			if (part.count(e.second.nbkey) > 0) {
				if (legs.count(e.second.nbkey) == 0)
					legs[e.second.nbkey] = 
					{{std::get<1>(it1->second.val) * std::get<1>(e.second.nbitr->second.val) / e.second.val,
					std::get<1>(it1->second.val) * std::get<1>(e.second.nbitr->second.val) / e.second.val / e.second.val},
					e.second.nbitr};
				else{
					legs[e.second.nbkey].first.first /= e.second.val;
					legs[e.second.nbkey].first.second /= (e.second.val*e.second.val);
				}
			}
		}
		auto min_itr = std::min_element(
				legs.begin(),
				legs.end(),
				[](const std::pair<EdgeKey, std::pair<std::pair<int,int>, IterNode>> & a, const std::pair<EdgeKey, std::pair<std::pair<int,int>, IterNode>> & b) {
					return a.second.first < b.second.first;
				});
		return min_itr->second;
	}

	template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	NodeKey Engine::contract_quickbb(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {
		// if(contract_test_mode)
		//	lat.draw("lat before quickbb",{part},true);

		auto temp=lat;
		//std::cout<<lat.size()<<' '<<temp.size();
		std::set<NodeKey, typename Trait::nodekey_less> contracted_sites;

		if (contract_trace_mode)
			std::cout << "in_quickbb \n";
		if (part.size() == 1){
			return *(part.begin());
		}
		// else if(part.size() == 2){
		// 	lat.absorb(*(part.begin()), *(part.begin()++), absorb_fun, contract_fun);
		// 	return *(part.begin());
		// }

		// std::cout << "<test_part \n";
		// for(auto & p:part) std::cout<<p<<'\n';
		// std::cout << "test_part> \n";

		// lat.draw("part",{part},true);

		std::pair<int,int> count;
		std::pair<int,int> least_count = {-1,-1};
		using NodeItrType = typename network<NodeVal, int, NodeKey, EdgeKey, Trait>::IterNode;
		using KeySet = std::set<NodeKey, typename Trait::nodekey_less>;
		NodeItrType least_contract1, least_contract2, nb_itr;

		for (auto & p : part) {
			auto site_it = lat.find(p);
			std::get<1>(site_it->second.val) = calc_weight(site_it, lat, KeySet());
		}
		for (auto & p : part) {
			auto site_it = lat.find(p);
			std::tie(count, nb_itr) = search_quick<NodeItrType, KeySet, EdgeKey, Trait>(site_it, part);
			if (least_count < std::make_pair(0,0) || count < least_count) {
				least_count = count;
				least_contract1 = site_it;
				least_contract2 = nb_itr;
			}
		}
		int contract_size;
		auto root_itr = least_contract1;
		lat.absorb(root_itr, least_contract2, absorb_fun, contract_fun);
		contracted_sites.insert(root_itr->first);
		contracted_sites.insert(least_contract2->first);
		//temp.draw("quickbb",{contracted_sites},false);
		contract_size = 2;
		while (contract_size < part.size()) {
			std::tie(count, nb_itr) = search_quick<NodeItrType, KeySet, EdgeKey, Trait>(root_itr, part);
			lat.absorb(root_itr, nb_itr, absorb_fun, contract_fun);
			contracted_sites.insert(nb_itr->first);
			//temp.draw("quickbb",{contracted_sites},false);
			++contract_size;
		}

		if (contract_trace_mode)
			std::cout << "out_quickbb \n";
		return root_itr->first;
	}

} // namespace net
#endif
