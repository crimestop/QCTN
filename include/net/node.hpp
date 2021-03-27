#ifndef NET_NODE_HPP
#define NET_NODE_HPP

#include "error.hpp"
#include "network.hpp"
#include "traits.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace net {
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	class node;
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	class network;
	template <typename T>
	std::string to_string(const T & m);

	/**
	 * \brief 描述着连接格点的边上信息，实际上这是一个半边而不是是一个完整的边
	 *
	 * \see node
	 */
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	struct edge {
		/**
		 * \brief 所指向的格点的名称
		 */
		NodeKey nbkey;
		/**
		 * \brief 所指向格点连接自身的边的名称
		 */
		EdgeKey nbind;
		/**
		 * \brief 所指向的格点的指针
		 */
		typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode nbitr;
		/**
		 * \brief 边上的附着信息
		 */
		EdgeVal val;

		typename std::map<EdgeKey, edge<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>, typename Trait::edgekey_less>::iterator nbegitr;

		edge() = default;
		edge(const NodeKey & s1, const EdgeKey & s2, typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode s) :
				nbkey(s1), nbind(s2), nbitr(s){};
		edge(const NodeKey & s1, const EdgeKey & s2, typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode s, const EdgeVal & E) :
				nbkey(s1), nbind(s2), nbitr(s), val(E){};
		edge(const NodeKey & s1, const EdgeKey & s2, const EdgeVal & E) : nbkey(s1), nbind(s2), val(E){};
		edge(const edge<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> &) = default;
	};

	/**
	 * \brief 格点存储了网络中点内的信息
	 *
	 * 每个格点中有一个中心元素val, 和边的信息
	 * 每个边拥有一个名字和一个edge对象
	 * \see network
	 * \see edge
	 */
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	class node {
		template <typename NodeVal1, typename EdgeVal1, typename NodeKey1, typename EdgeKey1, typename Trait1>
		friend std::ostream & output_node_text(std::ostream &, const node<NodeVal1, EdgeVal1, NodeKey1, EdgeKey1, Trait1> &);

		template <typename NodeVal1, typename EdgeVal1, typename NodeKey1, typename EdgeKey1, typename Trait1>
		friend std::istream & input_node_text(std::istream &, node<NodeVal1, EdgeVal1, NodeKey1, EdgeKey1, Trait1> &);

		template <typename NodeVal1, typename EdgeVal1, typename NodeKey1, typename EdgeKey1, typename Trait1>
		friend std::ostream & output_node_bin(std::ostream &, const node<NodeVal1, EdgeVal1, NodeKey1, EdgeKey1, Trait1> &);

		template <typename NodeVal1, typename EdgeVal1, typename NodeKey1, typename EdgeKey1, typename Trait1>
		friend std::istream & input_node_bin(std::istream &, node<NodeVal1, EdgeVal1, NodeKey1, EdgeKey1, Trait1> &);

	public:
		// constructor
		node() = default;
		node(const NodeVal & s) : val(s){};
		// copy constructor
		node(const node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> &) = default;
		// copy assignment
		node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & operator=(const node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> &) = default;
		// move constructor
		node(node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> &&) = default;
		// move assignment
		node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & operator=(node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> &&) = default;

		using EdgeType = edge<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>;
		using NodeKeyType = NodeKey;
		using NodeValType = NodeVal;
		using EdgeKeyType = EdgeKey;
		using EdgeValType = EdgeVal;
		using TraitType = Trait;

		void clean();

		template <typename Condition>
		void delete_edge(Condition &&);
		void delete_nbedge();

		void reset_nbkey_of_nb(const NodeKey &);

		template <typename absorb_type, typename contract_type>
		void absorb_nb(const NodeKey &, const NodeVal &, const absorb_type &, const contract_type &);
		template <typename absorb_type, typename Condition>
		void
		harmless_absorb_nb(NodeVal &, std::set<std::pair<EdgeKey, EdgeKey>, typename Trait::edge2key_less> &, const absorb_type &, Condition &&) const;

		void transfer_edge(const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode &);
		template <typename Condition>
		void transfer_edge(const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode &, Condition &&);
		template <typename Condition>
		void transfer_edge(
				const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode &,
				const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode &,
				Condition &&);

		// void set_edge(
		// 		const EdgeKey & ind,
		// 		const NodeKey & nbkey,
		// 		const EdgeKey & nbind,
		// 		typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode nbitr,
		// 		const EdgeVal & edgev);

		void relink(std::map<NodeKey, node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>, typename Trait::nodekey_less> &);

		bool consistency(const std::map<NodeKey, node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>, typename Trait::nodekey_less> &, std::ostream &)const;

		void fope(std::function<NodeVal(const NodeVal &)> f1, std::function<EdgeVal(const EdgeVal &)> f2);

		template <typename NodeType2>
		NodeType2
		fmap(std::function<typename NodeType2::NodeValType(const NodeVal &)> f1,
			  std::function<typename NodeType2::EdgeValType(const EdgeVal &)> f2) const;

		template <typename NodeType2>
		NodeType2
		fmap(std::function<typename NodeType2::NodeValType(const NodeVal &)> f1,
			  std::function<typename NodeType2::EdgeValType(const EdgeVal &)> f2,
			  std::function<typename NodeType2::NodeKeyType(const NodeKey &)> f3,
			  std::function<typename NodeType2::EdgeKeyType(const EdgeKey &)> f4) const;

		template <typename NodeType2>
		NodeType2
		gfmap(const NodeKey & thiskey,
				std::function<typename NodeType2::NodeValType(const NodeKey &, const NodeVal &)> f1,
				std::function<typename NodeType2::EdgeValType(
						const NodeKey &,
						const NodeVal &,
						const NodeKey &,
						const NodeVal &,
						const EdgeKey &,
						const EdgeKey &,
						const EdgeVal &)> f2) const;
		/**
		 * \brief 格点所附着的信息
		 */
		NodeVal val;
		/**
		 * \brief 格点所相连的边, 存储了另一测的指针等信息
		 * \see edge
		 */
		std::map<EdgeKey, edge<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>, typename Trait::edgekey_less> edges;
	};

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename Condition>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::delete_edge(Condition && cond) {
		for (auto edge_itr = edges.begin(); edge_itr != edges.end();) {
			if (cond(edge_itr)) {
				edge_itr->second.nbitr->second.edges.erase(edge_itr->second.nbegitr);
				edge_itr = edges.erase(edge_itr);
			} else {
				++edge_itr;
			}
		}
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::delete_nbedge() {
		for (auto & b : edges) {
			b.second.nbitr->second.edges.erase(b.second.nbegitr);
		}
	}

	/*
	 * \brief 对格点的每一条边，更新邻居对应的边的nbkey为newkey
	 */
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::reset_nbkey_of_nb(const NodeKey & newkey) {
		for (auto & b : edges) {
			//b.second.nbitr->second.edges[b.second.nbind].nbkey = newkey;
			b.second.nbegitr->second.nbkey = newkey;
		}
	}

	/*
	 * \brief
	 * 对格点的指定nbkey每一条边，记录这条边的(ind,nbind)，吸收边的val到格点的val，然后删掉这条边。
	 *
	 *
	 *  A(this) <----> B(nb) <----> C   =>  A <---- B <----> C
	 */
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename absorb_type, typename contract_type>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::absorb_nb(
			const NodeKey & nbkey,
			const NodeVal & nbval,
			const absorb_type & absorb_fun,
			const contract_type & contract_fun) {
		std::set<std::pair<EdgeKey, EdgeKey>, typename Trait::edge2key_less> ind_pairs;

		// set ind_pairs and erase iterator in node1
		for (auto iter = edges.begin(); iter != edges.end();) {
			if (iter->second.nbkey == nbkey) {
				ind_pairs.insert({iter->first, iter->second.nbind});
				val = absorb_fun(val, iter->second.val, iter->first);
				iter = edges.erase(iter);
			} else {
				++iter;
			}
		}
		val = contract_fun(val, nbval, ind_pairs);
	}

	/*
	 * \brief
	 * 对格点的指定nbkey每一条边，如果符合条件，记录这条边的(ind,nbind)，吸收边的val到给定的的val，将ind-pair填入ind_pairs
	 *
	 */
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename absorb_type, typename Condition>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::harmless_absorb_nb(
			NodeVal & thisval,
			std::set<std::pair<EdgeKey, EdgeKey>, typename Trait::edge2key_less> & ind_pairs,
			const absorb_type & absorb_fun,
			Condition && cond) const {
		for (auto & b : edges) {
			// std::cout<<"edge"<<b.first<<' '<<b.second.nbkey<<'\n';
			if (cond(b)) {
				// std::cout<<"edgehere"<<b.first<<' '<<b.second.nbkey<<'\n';
				ind_pairs.insert({b.first, b.second.nbind});
				thisval = absorb_fun(thisval, b.second.val, b.first);
			}
		}
	}

	/*
	 * \brief 对格点的每一条边，将它和邻居的这条边转移为格点newit和邻居的边
	 *
	 */

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::transfer_edge(
			const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode & newit) {
			while(edges.size()>0) {
				auto nh = edges.extract(edges.begin());
				auto status = newit->second.edges.insert(std::move(nh));
				status.position->second.nbegitr->second.nbkey = newit->first;
				status.position->second.nbegitr->second.nbitr = newit;
				status.position->second.nbegitr->second.nbegitr = status.position;
			}
	}

	/*
	 * \brief
	 * 对格点的每一条边，如果条件成立，将它和邻居的这条边转移为格点newit和邻居的边
	 *
	 */

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename Condition>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::transfer_edge(
			const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode & newit,
			Condition && cond) {
		for (auto iter = edges.begin(); iter != edges.end();) {
			if (cond(iter)) {
				auto nh = edges.extract(iter++);
				auto status = newit->second.edges.insert(std::move(nh));
				status.position->second.nbegitr->second.nbkey = newit->first;
				status.position->second.nbegitr->second.nbitr = newit;
				status.position->second.nbegitr->second.nbegitr = status.position;
			} else {
				++iter;
			}
		}
	}

	/*
	 * \brief
	 * 对格点的每一条边，如果条件成立，将它和邻居的这条边转移为格点(newkey,newnode)和邻居的边
	 *                       否则，将它和邻居的这条边转移为格点(newkey2,newnode2)和邻居的边
	 *
	 */
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename Condition>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::transfer_edge(
			const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode & newit,
			const typename network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::IterNode & newit2,
			Condition && cond) {
		for (auto iter = edges.begin(); iter != edges.end();) {
			auto nh = edges.extract(iter++);
			if (cond(iter)) {
				auto status = newit->second.edges.insert(std::move(nh));
				status.position->second.nbegitr->second.nbkey = newit->first;
				status.position->second.nbegitr->second.nbitr = newit;
				status.position->second.nbegitr->second.nbegitr = status.position;
			} else {
				auto status = newit2->second.edges.insert(std::move(nh));
				status.position->second.nbegitr->second.nbkey = newit2->first;
				status.position->second.nbegitr->second.nbitr = newit2;
				status.position->second.nbegitr->second.nbegitr = status.position;
			}
		}
	}

	template <typename IterNode, typename EdgeKey, typename EdgeVal>
	void set_edge_node(IterNode it1, IterNode it2, const EdgeKey & ind1,const EdgeKey & ind2, const EdgeVal & edgev) {

		auto [egit1, succ1] = it1->second.edges.insert({ind1, typename decltype(it1->second.edges)::mapped_type(it2->first, ind2, it2, edgev)});
		if (!succ1)
			throw key_exist_error("In node.add_edge, ind " + to_string(ind1) + " already linked!");
		auto [egit2, succ2] = it2->second.edges.insert({ind2, typename decltype(it2->second.edges)::mapped_type(it1->first, ind1, it1, edgev)});
		if (!succ2)
			throw key_exist_error("In node.add_edge, ind " + to_string(ind2) + " already linked!");
		egit1->second.nbegitr=egit2;
		egit2->second.nbegitr=egit1;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::relink(
			std::map<NodeKey, node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>, typename Trait::nodekey_less> & nodes) {
		for (auto & b : edges){
			b.second.nbitr = nodes.find(b.second.nbkey);
			b.second.nbegitr = b.second.nbitr->second.edges.find(b.second.nbind);
		}
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename NodeType2>
	NodeType2 node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::gfmap(
			const NodeKey & thiskey,
			std::function<typename NodeType2::NodeValType(const NodeKey &, const NodeVal &)> f1,
			std::function<typename NodeType2::EdgeValType(
					const NodeKey &,
					const NodeVal &,
					const NodeKey &,
					const NodeVal &,
					const EdgeKey &,
					const EdgeKey &,
					const EdgeVal &)> f2) const {
		NodeType2 result;
		result.val = f1(thiskey, val);
		for (auto & b : edges)
			result.edges[b.first] = typename NodeType2::EdgeType(
					b.second.nbkey, b.second.nbind, f2(thiskey, val, b.second.nbkey, b.second.nbitr->second.val, b.first, b.second.nbind, b.second.val));
		return result;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename NodeType2>
	NodeType2 node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::fmap(
			std::function<typename NodeType2::NodeValType(const NodeVal &)> f1,
			std::function<typename NodeType2::EdgeValType(const EdgeVal &)> f2) const {
		NodeType2 result;
		result.val = f1(val);
		for (auto & b : edges)
			result.edges[b.first] = typename NodeType2::EdgeType(b.second.nbkey, b.second.nbind, f2(b.second.val));
		return result;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	template <typename NodeType2>
	NodeType2 node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::fmap(
			std::function<typename NodeType2::NodeValType(const NodeVal &)> f1,
			std::function<typename NodeType2::EdgeValType(const EdgeVal &)> f2,
			std::function<typename NodeType2::NodeKeyType(const NodeKey &)> f3,
			std::function<typename NodeType2::EdgeKeyType(const EdgeKey &)> f4) const {
		NodeType2 result;
		result.val = f1(val);
		for (auto & b : edges)
			result.edges[f4(b.first)] = typename NodeType2::EdgeType(f3(b.second.nbkey), f4(b.second.nbind), f2(b.second.val));
		return result;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void
	node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::fope(std::function<NodeVal(const NodeVal &)> f1, std::function<EdgeVal(const EdgeVal &)> f2) {
		val = f1(val);
		for (auto & b : edges)
			b.second.val = f2(b.second.val);
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	bool node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::consistency(
			const std::map<NodeKey, node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>, typename Trait::nodekey_less> & nodes,
			std::ostream & diagnosis) const {
		for (auto & b : edges) { // check if b is consistent
			if (nodes.count(b.second.nbkey) == 0) {
				diagnosis << "Network is not consistent, neighbor " + to_string(b.second.nbkey) + " is not found!\n";
				return false;
			} else if (nodes.at(b.second.nbkey).edges.count(b.second.nbind) == 0) {
				diagnosis << "Network is not consistent, neighbor " + to_string(b.second.nbkey) + " has no index named " + to_string(b.second.nbind) +
										 " !\n";
				return false;
			} else if (b.second.nbitr != nodes.find(b.second.nbkey)) {
				diagnosis << "Network is not consistent, pointer to neighbor " + to_string(b.second.nbkey) + " is not correctly pointed !\n";
				return false;
			} else if (b.second.nbegitr != b.second.nbitr->second.edges.find(b.second.nbind)) {
				diagnosis << "Network is not consistent, pointer to neighbor edge " + to_string(b.second.nbind) + " is not correctly pointed !\n";
				return false;
			}
		}

		return true;
	}
} // namespace net

#endif
