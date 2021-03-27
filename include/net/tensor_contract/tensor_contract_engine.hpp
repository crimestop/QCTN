#ifndef NET_TENSOR_CONTRACT_ENGINE_HPP
#define NET_TENSOR_CONTRACT_ENGINE_HPP
#include "../network.hpp"
#include "../rational.hpp"
#include "../tensor_tools.hpp"
#include "../traits.hpp"
#include "../group.hpp"
#include <TAT/TAT.hpp>
#include <cstdlib>
#include <functional>
#include <random>
#include <variant>
#include <memory>
#include <vector>
#include <iostream>

namespace net {
	inline bool contract_test_mode = false;
	inline bool contract_trace_mode = false;

	class Engine {
	public:
		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeVal contract(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);
		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeVal contract_qbb(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);
		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeVal contract_ect(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);
		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeVal contract_naive(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);

		int coarse_grain_to = 800;
		int cut_part = 2;
		int refine_sweep = 10000;
		int small_part_size = 20;
		bool rec_partition=false;
		bool verbose=false;
		std::string small_part_method="quickbb";

		//double uneven = 0.2;
		std::default_random_engine rand = std::default_random_engine(0);
		// std::default_random_engine
		// rand=std::default_random_engine(std::random_device()());

		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeKey contract_breadth_first(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);
	private:
		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeKey inner_contract(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);

		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeKey connected_inner_contract(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &,
				double,bool,std::string);

		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeKey contract_quickbb(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);

		template <typename contract_type, typename absorb_type, typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		NodeKey contract_exact(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				const absorb_type &,
				const contract_type &);

		template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		std::vector<std::set<NodeKey, typename Trait::nodekey_less>>
		divide(network<NodeVal, int, NodeKey, EdgeKey, Trait> &, const std::set<NodeKey, typename Trait::nodekey_less> &);

		template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		std::vector<std::set<NodeKey, typename Trait::nodekey_less>>
		divide_kahypar(network<NodeVal, int, NodeKey, EdgeKey, Trait> &, const std::set<NodeKey, typename Trait::nodekey_less> &,double,bool&);

		template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		double refine(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				std::vector<std::set<NodeKey, typename Trait::nodekey_less>> &);

		template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
		void adjust(
				network<NodeVal, int, NodeKey, EdgeKey, Trait> &,
				const std::set<NodeKey, typename Trait::nodekey_less> &,
				std::vector<std::set<NodeKey, typename Trait::nodekey_less>> &,
				double);
	};


	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void get_component(
			node<NodeVal, int, NodeKey, EdgeKey, Trait> & n,
			const NodeKey & p,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			std::set<NodeKey, typename Trait::nodekey_less> & treated,
			std::set<NodeKey, typename Trait::nodekey_less> & component) {
		treated.insert(p);
		component.insert(p);
		for (auto & eg : n.edges)
			if (part.count(eg.second.nbkey) > 0 && treated.count(eg.second.nbkey) == 0 && std::get<1>(n.val) == std::get<1>(eg.second.nbitr->second.val))
				get_component(eg.second.nbitr->second, eg.second.nbkey, part, treated, component);
	}

	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::vector<std::set<NodeKey, typename Trait::nodekey_less>> disconnect(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			const std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & subparts) {
		for (int i = 0; i < subparts.size(); ++i)
			for (auto & s : subparts[i])
				std::get<1>(lat[s].val) = i;

		std::set<NodeKey, typename Trait::nodekey_less> treated, newsubpart;
		std::vector<std::set<NodeKey, typename Trait::nodekey_less>> newsubparts;
		while (treated.size() < part.size())
			for (auto & p : part)
				if (treated.count(p) == 0) {
					newsubparts.push_back({});
					get_component(lat[p], p, part, treated, newsubparts.back());
				}

		return newsubparts;
	}

} // namespace net
#endif
