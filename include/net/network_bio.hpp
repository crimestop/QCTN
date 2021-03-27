#ifndef NET_NETWOTK_BIO_HPP
#define NET_NETWOTK_BIO_HPP

#include "network.hpp"
#include "node.hpp"
#include "type.hpp"
#include <complex>
#include <map>
#include <vector>

namespace net {
	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	class network;

	template <typename T, std::enable_if_t<is_scalar_v<T>, int> = 0>
	std::ostream & output_bin(std::ostream & os, const T & m) {
		return os.write(reinterpret_cast<const char *>(&m), sizeof m);
	}

	inline std::ostream & output_bin(std::ostream & os, const std::string & m) {
		unsigned int len = m.size();
		output_bin(os, len);
		return os.write(&m[0], len);
	}

	template <typename T, std::enable_if_t<is_scalar_v<T>, int> = 0>
	std::istream & input_bin(std::istream & is, T & m) {
		return is.read(reinterpret_cast<char *>(&m), sizeof m);
	}

	inline std::istream & input_bin(std::istream & is, std::string & m) {
		unsigned int len;
		input_bin(is, len);
		m.resize(len);
		return is.read(&m[0], len);
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::ostream & operator<(std::ostream & os, const network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & n) {
		output_bin(os, n.size());
		for (auto & i : n) {
			Trait::nodekey_write_bin(os, i.first);
			output_node_bin(os, i.second);
		}
		return os;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::ostream & output_node_bin(std::ostream & os, const node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & s) {
		output_bin(os, s.edges.size());
		for (auto & i : s.edges) {
			Trait::edgekey_write_bin(os, i.first);
			output_edge_bin(os, i.second);
		}
		Trait::nodeval_write_bin(os, s.val);
		return os;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::ostream & output_edge_bin(std::ostream & os, const edge<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & b) {
		Trait::nodekey_write_bin(os, b.nbkey);
		Trait::edgekey_write_bin(os, b.nbind);
		Trait::edgeval_write_bin(os, b.val);
		return os;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::istream & operator>(std::istream & is, network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & n) {
		unsigned int len;
		input_bin(is, len);
		NodeKey a;
		n.clear();
		for (int i = 0; i < len; ++i) {
			Trait::nodekey_read_bin(is, a);
			input_node_bin(is, n[a]);
		}
		for (auto & s : n)
			s.relink(n);
		return is;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::istream & input_node_bin(std::istream & is, node<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & s) {
		unsigned int len;
		input_bin(is, len);
		EdgeKey a;
		s.edges.clear();
		for (int i = 0; i < len; ++i) {
			Trait::edgekey_read_bin(is, a);
			input_edge_bin(is, s.edges[a]);
		}
		Trait::nodeval_read_bin(is, s.val);
		return is;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::istream & input_edge_bin(std::istream & is, edge<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait> & b) {
		Trait::nodekey_read_bin(is, b.nbkey);
		Trait::edgekey_read_bin(is, b.nbind);
		Trait::edgeval_read_bin(is, b.val);
		return is;
	}

} // namespace net
#endif