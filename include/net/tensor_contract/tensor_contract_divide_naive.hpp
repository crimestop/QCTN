#ifndef NET_TENSOR_CONTRACT_DIVIDE_NAIVE_HPP
#define NET_TENSOR_CONTRACT_DIVIDE_NAIVE_HPP
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
	
	struct kset_contract {
		template <typename NodeVal, typename EdgeKey, typename Comp>
		NodeVal operator()(const NodeVal & g1, const NodeVal & g2, const std::set<std::pair<EdgeKey, EdgeKey>, Comp> & inds) const {
			NodeVal res = g1;
			res.insert(g2.begin(), g2.end());
			return res;
		}
	};

	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::vector<std::set<NodeKey, typename Trait::nodekey_less>>
	Engine::divide(network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat, const std::set<NodeKey, typename Trait::nodekey_less> & part) {
		if (contract_trace_mode)
			std::cout << "in_divide \n";
		using KeySet = std::set<NodeKey, typename Trait::nodekey_less>;
		KeySet coarse_part = part;
		network<KeySet, int, NodeKey, EdgeKey> fakelat;
		fakelat = lat.template fmap<decltype(fakelat)>([](const NodeVal & tp) { return KeySet(); }, [](const int & m) { return m; });
		for (auto & n : fakelat)
			n.second.val.insert(n.first);

		//粗粒化
		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************1\n"; 	std::cout<<"finish **************1\n";

		if (contract_trace_mode)
			std::cout << "coarse_grain \n";
		combine_edges(fakelat, part);
		while (coarse_part.size() > coarse_grain_to) {
			std::set<std::tuple<int, NodeKey, NodeKey>, std::greater<std::tuple<int, NodeKey, NodeKey>>> ordered_bond; // weight, from, to
																																						  // we may replace std::greater
			// add bonds to ordered_bond
			for (auto & p : coarse_part) {
				auto & this_site = fakelat[p];
				for (auto & e : this_site.edges) {
					if (coarse_part.count(e.second.nbkey) > 0) {
						ordered_bond.insert({e.second.val, p, e.second.nbkey});
					}
				}
			}
			// do coarse grain
			KeySet treated_sites;
			for (auto & b : ordered_bond) {
				if (treated_sites.count(std::get<1>(b)) == 0 && treated_sites.count(std::get<2>(b)) == 0) {
					treated_sites.insert(std::get<1>(b));
					treated_sites.insert(std::get<2>(b));
					fakelat.absorb(std::get<1>(b), std::get<2>(b), no_absorb(), kset_contract());
					coarse_part.erase(std::get<2>(b));
					combine_edges(fakelat, {std::get<1>(b)});
				}
			}
		}
		// lat.draw(true);
		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************2\n"; 	std::cout<<"finish **************2\n";

		if (contract_trace_mode)
			std::cout << "initial \n";
		//初始分割，分成cut_part份
		KeySet treated_sites = {};
		KeySet final_sites = {};
		int size_limit;
		int treated_size = 0;
		bool decide2exit;
		for (int i = 0; i < cut_part; ++i) {
			size_limit = (part.size() - treated_size) / (cut_part - i);
			if (contract_test_mode)
				fakelat.draw("lat before initial_cut no. " + std::to_string(i), {part, final_sites}, true);
			// find site with max weight (within part-treated)
			int max_weight = 0;
			int this_weight;
			typename network<KeySet, int, NodeKey, EdgeKey>::IterNode max_weight_site_itr;
			typename network<KeySet, int, NodeKey, EdgeKey>::IterNode this_site_itr;
			for (auto p : coarse_part) {
				if (treated_sites.count(p) == 0) {
					this_site_itr = fakelat.find(p);
					this_weight = calc_weight(this_site_itr, coarse_part, treated_sites);
					if (contract_test_mode)
						std::cout << "test.divide.initial_cut.max_weight " << p << ' ' << this_weight << '\n';
					if (this_weight > max_weight) {
						max_weight = this_weight;
						max_weight_site_itr = this_site_itr;
					}
				}
			}
			if (contract_test_mode)
				std::cout << "test.divide.initial_cut.max_weight final " << max_weight_site_itr->first << ' ' << max_weight << '\n';
			// construct subpart
			treated_sites.insert(max_weight_site_itr->first);
			final_sites.insert(max_weight_site_itr->first);
			while (true) {
				// std::cout<<"a \n";
				// fakelat.consistency();
				combine_edges(fakelat, {max_weight_site_itr->first});
				// std::cout<<"b \n";
				// fakelat.consistency();
				std::set<std::pair<int, NodeKey>, std::greater<std::pair<int, NodeKey>>> ordered_nb;
				for (auto & e : max_weight_site_itr->second.edges) {
					if (coarse_part.count(e.second.nbkey) > 0 && treated_sites.count(e.second.nbkey) == 0) {
						ordered_nb.insert({e.second.val, e.second.nbkey});
					}
				}
				if (ordered_nb.size() == 0) // exit1: no neighbors
					break;
				decide2exit = false;
				for (auto & nb : ordered_nb) {
					if (max_weight_site_itr->second.val.size() + fakelat[nb.second].val.size() <= size_limit) {
						fakelat.absorb(max_weight_site_itr->first, nb.second, no_absorb(), kset_contract());
						treated_sites.insert(nb.second);
					} else {
						decide2exit = true; // exit2: reach limit
						break;
					}
				}
				if (decide2exit)
					break;
			}

			treated_size += max_weight_site_itr->second.val.size();
			// std::cout<<"c \n";
			// fakelat.consistency();
			combine_edges(fakelat, {max_weight_site_itr->first});
			// std::cout<<"d \n";
			// fakelat.consistency();
		}

		// if(contract_test_mode)
		// fakelat.draw("lat after initial_cut",{coarse_part,final_sites},true);
		if (contract_trace_mode)
			std::cout << "after_initial \n";
		//可能出现这种情况：由于连通性的原因，所有subpart搞完后，还剩下一些sites

		// int n=0;
		while (treated_sites.size() < coarse_part.size()) {
			// for(auto & e:fakelat.nodes.at("ten3_4").edges)
			// std::cout<<"hedddddre2 "<<e.second.nbkey<<'\n';
			// std::cout<<"diff"<<treated_sites.size()<<' '<<coarse_part.size()<<'\n';
			for (auto & s : final_sites) {
				// std::cout<<"here "<<s<<'\n';
				// auto & test_node=fakelat.nodes.at(s);
				// std::cout<<"there \n";
				// std::cout<<test_node.edges.size()<<"\n";
				// std::cout<<"there \n";
				for (auto & e : fakelat.at(s).edges) {
					// std::cout<<"here2
					// "<<e.second.nbkey<<coarse_part.count(e.second.nbkey)<<final_sites.count(e.second.nbkey)<<'\n';
					if (coarse_part.count(e.second.nbkey) > 0 && treated_sites.count(e.second.nbkey) == 0) {
						treated_sites.insert(e.second.nbkey);
						fakelat.absorb(s, e.second.nbkey, no_absorb(), kset_contract());
						// std::cout<<treated_sites.size()<<"success\n";
						// std::cout<<treated_sites.size()<<"success\n";
						break;
					}
				}
			}
			// n++;
			// if(n==10) std::exit(EXIT_FAILURE);
		}
		if (contract_test_mode)
			fakelat.draw("lat after adjustment", {part, final_sites}, true);

		if (contract_trace_mode)
			std::cout << "release \n";
		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************3\n"; 	std::cout<<"finish **************3\n";
		//释放
		std::vector<KeySet> subparts;
		for (auto & s : final_sites)
			subparts.push_back(fakelat[s].val);

		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************4\n"; 	std::cout<<"finish **************4\n";

		if (contract_trace_mode)
			std::cout << "out_divide \n";

		// adjust(lat,part,subparts,0.1);
		// refine(lat,part,subparts);
		adjust(lat, part, subparts, 0.3);
		// refine(lat,part,subparts);
		// adjust(lat,part,subparts,1);
		// refine(lat,part,subparts);
		if (contract_test_mode)
			lat.draw("lat after adjust", subparts, true);
		auto subparts2 = disconnect(lat, part, subparts);
		if (contract_test_mode)
			lat.draw("lat after disconnect", subparts2, true);
		return subparts2;
	}

	template <typename NodeType, typename SetType>
	bool calc_gain(int cut_part, net::rational & max_gain, NodeType & n, int & this_part, int & nb_part, const SetType & part) {
		std::vector<int> weights(cut_part, 1);
		for (auto & eg : n.edges)
			if (part.count(eg.second.nbkey) > 0)
				weights[std::get<1>(eg.second.nbitr->second.val)] *= eg.second.val;

		this_part = std::get<1>(n.val);
		nb_part = this_part;
		int this_weight = weights[this_part];
		int max_weight = this_weight;
		for (int i = 0; i < cut_part; i++) {
			if (weights[i] > max_weight) {
				max_weight = weights[i];
				nb_part = i;
			}
		}
		max_gain = net::rational(max_weight, this_weight);
		return (max_weight > this_weight);
	}

	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	double Engine::refine(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & subparts) {

		double uneven=0.;
		// if(contract_trace_mode) std::cout<<"in_refine \n";

		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************5\n"; 	std::cout<<"finish **************5\n";
		// if(contract_test_mode) lat.draw("lat before refinement",subparts,true);

		// set part label and  calculate part size
		std::vector<int> part_size(subparts.size(), 0);
		for (int i = 0; i < subparts.size(); ++i) {
			for (auto & s : subparts[i])
				std::get<1>(lat[s].val) = i;
			part_size[i] = subparts[i].size();
		}

		// calc gain sorted set
		std::map<std::pair<net::rational, NodeKey>, std::pair<int, int>, std::greater<std::pair<net::rational, NodeKey>>> gain_rec; // gain, from, to
		net::rational gain;
		double tot_gain = 1.;
		int this_part, nb_part;
		for (auto & p : part) {
			auto & n = lat[p];

			if (calc_gain(cut_part, gain, n, this_part, nb_part, part)) {
				gain_rec[{gain, p}] = {this_part, nb_part};
				std::get<2>(n.val) = gain;
			}
		}
		if (contract_test_mode) {
			std::cout << "test.refine.build_gain start\n";
			for (auto & i : gain_rec)
				std::cout << "test.refine.build_gain " << i.first.second << ' ' << i.first.first << ' ' << i.second.first << " -> " << i.second.second
							 << "\n";
			std::cout << "test.refine.build_gain finish\n";
		}

		// for(int i=0;i<subparts.size();++i){
		// 	std::cout<<"test.refine.run_gain subpart"<<i<<'\n';
		// 	for(auto & s:subparts[i]) std::cout<<"         "<<s<<'\n';
		// 	for(auto & s:subparts[i]) std::cout<<"         \n";
		// }

		int min_size = part.size() / cut_part * (1 - uneven);
		int max_size = part.size() / cut_part * (1 + uneven);
		for (int i = 0; i < refine_sweep; i++) {
			if (gain_rec.size() == 0)
				break;
			for (auto g_rec = gain_rec.begin(); g_rec != gain_rec.end(); ++g_rec) {
				if (part_size[g_rec->second.first] > min_size && part_size[g_rec->second.second] < max_size) {
					// std::cout<<"test.refine.run_gain "<<g_rec->first.second<<' '<<
					// 	g_rec->first.first<<' '<<g_rec->second.first<<" ->
					// "<<g_rec->second.second<<"\n";

					part_size[g_rec->second.first]--;
					part_size[g_rec->second.second]++;
					auto this_name = g_rec->first.second;
					auto & this_node = lat[this_name];
					std::get<1>(this_node.val) = g_rec->second.second;
					subparts[g_rec->second.first].erase(g_rec->first.second);
					subparts[g_rec->second.second].insert(g_rec->first.second);
					tot_gain *= g_rec->first.first.to_double();

					gain_rec.erase(g_rec);
					if (calc_gain(cut_part, gain, this_node, this_part, nb_part, part)) {
						gain_rec[{gain, this_name}] = {this_part, nb_part};
						std::get<2>(this_node.val) = gain;
					}
					for (auto & eg : this_node.edges) {
						if (part.count(eg.second.nbkey) > 0) {
							auto lookup_nb = gain_rec.find({std::get<2>(eg.second.nbitr->second.val), eg.second.nbkey});
							if (lookup_nb != gain_rec.end()) {
								gain_rec.erase(lookup_nb);
							}
							if (calc_gain(cut_part, gain, eg.second.nbitr->second, this_part, nb_part, part)) {
								gain_rec[{gain, eg.second.nbkey}] = {this_part, nb_part};
								std::get<2>(eg.second.nbitr->second.val) = gain;
							}
						}
					}
					break;
				}
			}
		}

		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************6\n"; 	std::cout<<"finish **************6\n";

		// if(contract_test_mode) lat.draw("lat after refinement",subparts,true);

		// if(contract_trace_mode) std::cout<<"out_refine \n";

		return tot_gain;
	}

	// calc weight of a node by part no.

	template <typename NodeType, typename SetType>
	void calc_weight(int cut_part, const NodeType & n, std::vector<int> & weight, const SetType & part) {
		weight = std::vector<int>(cut_part, 1);
		for (auto & eg : n.edges)
			if (part.count(eg.second.nbkey) > 0)
				weight[std::get<1>(eg.second.nbitr->second.val)] *= eg.second.val;
	}

	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void Engine::adjust(
			network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat,
			const std::set<NodeKey, typename Trait::nodekey_less> & part,
			std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & subparts,
			double alpha) {

		double uneven=0.;

		if (contract_trace_mode)
			std::cout << "in_adjust \n";
		std::vector<std::set<NodeKey, typename Trait::nodekey_less>> best_subparts = subparts, temp_subparts;
		std::vector<NodeKey> part_vec = {part.begin(), part.end()};

		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************5\n"; 	std::cout<<"finish **************5\n";
		if (contract_test_mode)
			lat.draw("lat before adjustment", subparts, true);

		// set part label and  calculate part size
		std::vector<int> part_size(subparts.size(), 0);
		for (int i = 0; i < subparts.size(); ++i) {
			for (auto & s : subparts[i])
				std::get<1>(lat[s].val) = i;
			part_size[i] = subparts[i].size();
		}

		// calc gain sorted set

		int min_size = part.size() / cut_part * (1 - 0.5 * uneven);
		int max_size = part.size() / cut_part * (1 + 0.5 * uneven);

		std::uniform_int_distribution<> site_dist(0, part.size() - 1);
		std::uniform_int_distribution<> part_dist(0, cut_part - 2);
		std::uniform_real_distribution<> monte_carlo;

		std::map<NodeKey, std::vector<int>, typename Trait::nodekey_less> weights;
		for (auto & p : part)
			calc_weight(cut_part, lat[p], weights[p], part);

		double cumu_gain = 1, max_gain = 1, further_gain;
		int i = 0;
		int j = 0;
		while (i < refine_sweep && j < 100 * refine_sweep) {
			j++;
			auto this_site = part_vec[site_dist(rand)];
			auto & this_node = lat[this_site];
			auto & this_weight = weights[this_site];
			int this_part = std::get<1>(this_node.val);
			int next_part = part_dist(rand);
			if (next_part >= this_part)
				next_part++;
			// std::cout<<std::pow(double(this_weight[next_part])/double(this_weight[this_part]),alpha)<<'\n';
			if (part_size[this_part] > min_size && part_size[next_part] < max_size &&
				 monte_carlo(rand) < std::pow(double(this_weight[next_part]) / double(this_weight[this_part]), alpha)) {
				// std::cout<<"success";
				cumu_gain *= double(this_weight[next_part]) / this_weight[this_part];

				part_size[this_part]--;
				part_size[next_part]++;
				std::get<1>(this_node.val) = next_part;
				subparts[this_part].erase(this_site);
				subparts[next_part].insert(this_site);
				calc_weight(cut_part, this_node, this_weight, part);
				for (auto & eg : this_node.edges)
					calc_weight(cut_part, lat[eg.second.nbkey], weights[eg.second.nbkey], part);

				temp_subparts = subparts;
				further_gain = refine(lat, part, temp_subparts);

				if (cumu_gain * further_gain > max_gain) {
					max_gain = cumu_gain * further_gain;
					best_subparts = temp_subparts;
				}
				++i;
			}
		}

		// for(auto & i: std::get<0>(lat["ten0_1"])->val ) std::cout<<i<<"
		// **************6\n"; 	std::cout<<"finish **************6\n";

		if (contract_test_mode)
			lat.draw("lat after adjustment", best_subparts, true);

		if (contract_trace_mode)
			std::cout << "out_adjust \n";

		subparts = best_subparts;
	}

} // namespace net
#endif
