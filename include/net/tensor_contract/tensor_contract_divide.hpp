#ifndef NET_TENSOR_CONTRACT_DIVIDE_HPP
#define NET_TENSOR_CONTRACT_DIVIDE_HPP
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
#ifdef NET_USE_LIB_KAHYPAR
	#include <libkahypar.h>
#else
	#include <kahypar/application/command_line_options.h>
	#include <kahypar/definitions.h>
	#include <kahypar/io/hypergraph_io.h>
	#include <kahypar/partitioner_facade.h>
#endif


namespace net {

	template <typename IterNode, typename NodeSet1, typename NodeSet2>
	int calc_weight(const IterNode & it, const NodeSet1 & includes, const NodeSet2 & excludes) {
		int weight = 1;
		for (auto & e : it->second.edges)
			if (includes.count(e.second.nbkey) > 0 && excludes.count(e.second.nbkey) == 0)
				weight *= e.second.val;
		return weight;
	}

	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void combine_edges(network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat, const std::set<NodeKey, typename Trait::nodekey_less> & includes) {
		for (auto & i : includes) {
			auto & inode = lat[i];
			std::map<NodeKey, std::pair<EdgeKey, EdgeKey>, typename Trait::nodekey_less> nbkey2ind;
			for (auto iter = inode.edges.begin(); iter != inode.edges.end();) {
				// std::cout<<"combine "<<i<<' '<<iter->first<<'
				// '<<iter->second.nbkey<<'\n';
				if (nbkey2ind.count(iter->second.nbkey) == 0) {
					nbkey2ind.insert({iter->second.nbkey, {iter->first, iter->second.nbind}});
					++iter;
				} else {
					// std::cout<<"combine_erase "<<i<<' '<<iter->first<<'
					// '<<iter->second.nbkey<<'\n';
					inode.edges[nbkey2ind[iter->second.nbkey].first].val *= iter->second.val;
					iter->second.nbitr->second.edges[nbkey2ind[iter->second.nbkey].second].val *= iter->second.nbitr->second.edges[iter->second.nbind].val;
					iter->second.nbitr->second.edges.erase(iter->second.nbind);
					iter = inode.edges.erase(iter);
				}
			}
		}
	}

	template <typename NodeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::vector<std::set<NodeKey, typename Trait::nodekey_less>>
	Engine::divide_kahypar(network<NodeVal, int, NodeKey, EdgeKey, Trait> & lat, const std::set<NodeKey, typename Trait::nodekey_less> & part, double uneven, bool & failed) {

		const unsigned int num_vertices = part.size();
		unsigned int num_hyperedges = 0;

		std::vector<int> hyperedge_weights; // weight of edge

		std::vector<int> hypernode_weights(part.size(),0); // weight of node

		std::vector<size_t> hyperedge_indices; // start and end of vertice list for each edge

		std::vector<unsigned int> hyperedges; // vertice list

		std::set<NodeKey, typename Trait::nodekey_less> treated_nodes;

		std::map<NodeKey,int, typename Trait::nodekey_less> site_id_map; // node key from str to int
		std::vector<NodeKey> inv_site_id_map;

		int site_id=0;
		for (auto & p : part){
			inv_site_id_map.push_back(p);
			site_id_map[p] = site_id++;
		}

		int edge_id=0;
		hyperedge_indices.push_back(0);
		for (auto & s_it : lat) {
			auto & nodekey1 = s_it.first;
			if (part.count(nodekey1) == 1){
				for (auto & b_it : s_it.second.edges) {
					auto & nodekey2 = b_it.second.nbkey;
					if (part.count(nodekey2) == 1){
						hypernode_weights[site_id_map[nodekey1]] += std::log10(double(b_it.second.val))*40;
						if(treated_nodes.count(nodekey2) == 0){
							hyperedge_weights.push_back(std::log10(double(b_it.second.val))*40);
							// weight is int so we *40
							hyperedge_indices.push_back(2*(edge_id+1));
							hyperedges.push_back(site_id_map[nodekey1]);
							hyperedges.push_back(site_id_map[nodekey2]);
							edge_id++;
							num_hyperedges++;
						}
					}else
						hypernode_weights[site_id_map[nodekey1]] += std::log10(double(b_it.second.val))*40;
				}
				treated_nodes.insert(nodekey1);
			}
		}
			
		const int k = 2;
			
		int objective = 0;

		std::vector<int> partition(num_vertices, -1);

#ifdef NET_USE_LIB_KAHYPAR
		kahypar_context_t* context = kahypar_context_new();
		kahypar_configure_context_from_file(context, "km1_kKaHyPar_sea20.ini");
		kahypar_partition(num_vertices, num_hyperedges,
		   	            uneven, k,
		           	    hypernode_weights.data(), hyperedge_weights.data(),
		           	    hyperedge_indices.data(), hyperedges.data(),
		   	            &objective, context, partition.data());
		kahypar_context_free(context);
#else
		kahypar::Context context;
		kahypar::parseIniToContext(context,"km1_kKaHyPar_sea20.ini");
		ASSERT(!context.partition.use_individual_part_weights ||
		     !context.partition.max_part_weights.empty());
		ASSERT(partition != nullptr);

		context.partition.k = k;
		context.partition.epsilon = uneven;
		context.partition.write_partition_file = false;

		kahypar::Hypergraph hypergraph(num_vertices,
		                             num_hyperedges,
		                             hyperedge_indices.data(),
		                             hyperedges.data(),
		                             context.partition.k,
		                             hyperedge_weights.data(),
		                             hypernode_weights.data());

		if (context.partition.vcycle_refinement_for_input_partition) {
			for (const auto hn : hypergraph.nodes()) {
				hypergraph.setNodePart(hn, partition[hn]);
			}
		}

		kahypar::PartitionerFacade().partition(hypergraph, context);

		objective = kahypar::metrics::correctMetric(hypergraph, context);

		for (const auto hn : hypergraph.nodes()) {
			partition[hn] = hypergraph.partID(hn);
		}

		context.partition.perfect_balance_part_weights.clear();
		context.partition.max_part_weights.clear();
		context.evolutionary.communities.clear();
#endif

		std::vector<std::set<NodeKey, typename Trait::nodekey_less>> results(k);
		//std::cout<<"result\n";
		for(int i = 0; i != num_vertices; ++i){
		//	std::cout<<i<<' '<<partition[i]<<' '<<inv_site_id_map[i]<<'\n';
			results[partition[i]].insert(inv_site_id_map[i]);
		}
		//std::cout<<"part\n";
		//for(auto &p :part)
		//	std::cout<<p<<'\n';
		std::vector<std::set<NodeKey, typename Trait::nodekey_less>> disc_result=disconnect(lat, part, results);

		failed=(disc_result.size()==1);
		if(failed){ // there's chance that kahypar fails when epsilon ~ 1
			NodeKey max_key;
			int this_weight;
			int max_weight=-1;
			for(auto & p:part){
				this_weight=1;
				for (auto & eg : lat[p].edges)
					if (part.count(eg.second.nbkey) > 0)
						this_weight *= eg.second.val;
				if(max_weight<0 || this_weight>max_weight){
					max_weight=this_weight;
					max_key=p;
				}
			}
			disc_result[0].erase(max_key);
			disc_result.push_back({max_key});
		}
					// std::cout<<"root ";
					// for (auto & p : results)
					// 	std::cout<<' '<<p.size();
					// std::cout<<std::endl;

		//return results;
		return disc_result;
	}

} // namespace net
#endif
