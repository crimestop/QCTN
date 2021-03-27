#ifndef NET_TENSOR_TOOLS_HPP
#define NET_TENSOR_TOOLS_HPP

// #include "network.hpp"
#include "traits.hpp"
#include <TAT/TAT.hpp>
#include <functional>
#include <iostream>
#include <random>
#include <variant>

namespace net {
	namespace tensor {
		template <typename T, typename EdgeKey = stdEdgeKey>
		using Tensor = TAT::Tensor<T, TAT::NoSymmetry, EdgeKey>;

		template <typename TnType>
		int get_dim(TnType ten, int s) {
			return ten.core->edges[s].map[TAT::NoSymmetry()];
		}
		template <typename TnType>
		int get_size(TnType ten) {
			return ten.const_block().size();
		}
		template <typename TnType>
		int get_rank(TnType ten) {
			return ten.names.size();
		}

		template <typename T>
		std::set<std::string> inds_start_with(const TAT::Tensor<T> & ten, const std::string & head) {
			std::set<std::string> res;
			// std::cout<<"test inds_start_with\n";
			// std::cout<<head<<'\n';
			for (auto & s : ten.names)
				if (s.compare(0, head.size(), head) == 0) {
					// std::cout<<s<<'\n';
					res.insert(s);
				}
			return res;
		}

		template <typename TnType, typename EdgeKey>
		int get_dim(TnType ten, EdgeKey s) {
			return ten.core->edges[ten.name_to_index[s]].map[TAT::NoSymmetry()];
		}
		template <typename TnType>
		void diminfo(TnType ten, std::ostream & os) {
			for (auto & s : ten.names)
				os << s << ' ' << ten.core->edges[ten.name_to_index[s]].map[TAT::NoSymmetry()] << ' ';
			os << '\n';
		}

		/**
		 * \brief 得到tensor的对角元
		 */
		template <typename T, typename EdgeKey = stdEdgeKey>
		Tensor<T, EdgeKey> get_diag(const Tensor<T, EdgeKey> & ten, std::vector<std::pair<EdgeKey, EdgeKey>> names) {
			std::vector<EdgeKey> oldname;
			for (auto & n : names)
				oldname.push_back(n.first);
			for (auto & n : names)
				oldname.push_back(n.second);
			auto ten2 = ten.transpose(oldname);
			std::vector<EdgeKey> newname;
			std::vector<int> newdims;
			int dim, newsize = 1;
			for (int i = 0; i < names.size(); ++i) {
				newname.push_back(oldname[i]);
				dim = get_dim(ten2, i);
				newdims.push_back(dim);
				newsize *= dim;
			}
			Tensor<T, EdgeKey> result(newname, newdims);
			for (int i = 0; i < newsize; ++i) {
				result.block()[i] = ten2.const_block()[i * (newsize + 1)];
			}
			return result;
		}

	} // namespace tensor
} // namespace net

#endif
