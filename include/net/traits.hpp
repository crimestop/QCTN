#ifndef NET_TRAITS_HPP
#define NET_TRAITS_HPP
#include "default_bio.hpp"
#include "default_io.hpp"
#include <istream>
#include <ostream>
#include <set>
#include <string>
#include <utility>

namespace net {

	using stdEdgeKey = std::string;
	// using stdEdgeKey=std::pair<std::string,std::string>;

	template <typename single_less>
	struct double_less {
		template <typename double_type>
		bool operator()(const double_type & a, const double_type & b) {
			single_less comp;
			return comp(a.first, b.first) || (!comp(b.first, a.first) && comp(a.second, b.second));
		}
	};

	template <typename EdgeKey>
	struct base_edgekey_traits {
		static std::string edgekey_brief(const EdgeKey & edgekey) {
			std::stringstream a;
			default_write_text(a, edgekey);
			return a.str();
		}
		using edgekey_less = std::less<EdgeKey>;
		using edge2key_less = std::less<std::pair<EdgeKey, EdgeKey>>; // required by TAT
		static std::ostream & edgekey_write_text(std::ostream & os, const EdgeKey & edgekey) {
			return default_write_text(os, edgekey);
		}
		static std::ostream & edgekey_read_text(std::istream & is, EdgeKey & edgekey) {
			return default_read_text(is, edgekey);
		}
		static std::ostream & edgekey_write_bin(std::ostream & os, const EdgeKey & edgekey) {
			return default_write_bin(os, edgekey);
		}
		static std::ostream & edgekey_read_bin(std::istream & is, EdgeKey & edgekey) {
			return edgekey_read_bin(is, edgekey);
		}
	};

	template <typename NodeKey>
	struct base_nodekey_traits {
		static std::string nodekey_brief(const NodeKey & nodekey) {
			std::stringstream a;
			default_write_text(a, nodekey);
			return a.str();
		}
		using nodekey_less = std::less<NodeKey>;
		static std::ostream & nodekey_write_text(std::ostream & os, const NodeKey & nodekey) {
			return default_write_text(os, nodekey);
		}
		static std::ostream & nodekey_read_text(std::istream & is, NodeKey & nodekey) {
			return default_read_text(is, nodekey);
		}
		static std::ostream & nodekey_write_bin(std::ostream & os, const NodeKey & nodekey) {
			return default_write_bin(os, nodekey);
		}
		static std::ostream & nodekey_read_bin(std::istream & is, NodeKey & nodekey) {
			return edgekey_read_bin(is, nodekey);
		}
	};

	template <typename EdgeKey, typename NodeKey>
	struct base_bind_traits {
		static EdgeKey bind(const NodeKey & nodekey1, const NodeKey & nodekey2);
	};

	template <typename EdgeVal>
	struct base_edgeval_traits {
		static std::ostream & edgeval_write_text(std::ostream & os, const EdgeVal & edgeval) {
			return default_write_text(os, edgeval);
		}
		static std::ostream & edgeval_read_text(std::istream & is, EdgeVal & edgeval) {
			return default_read_text(is, edgeval);
		}
		static std::ostream & edgeval_write_bin(std::ostream & os, const EdgeVal & edgeval) {
			return default_write_bin(os, edgeval);
		}
		static std::ostream & edgeval_read_bin(std::istream & is, EdgeVal & edgeval) {
			return edgekey_read_bin(is, edgeval);
		}
	};

	template <typename NodeVal>
	struct base_nodeval_traits {
		static std::ostream & nodeval_write_text(std::ostream & os, const NodeVal & nodeval) {
			return default_write_text(os, nodeval);
		}
		static std::ostream & nodeval_read_text(std::istream & is, NodeVal & nodeval) {
			return default_read_text(is, nodeval);
		}
		static std::ostream & nodeval_write_bin(std::ostream & os, const NodeVal & nodeval) {
			return default_write_bin(os, nodeval);
		}
		static std::ostream & nodeval_read_bin(std::istream & is, NodeVal & nodeval) {
			return edgekey_read_bin(is, nodeval);
		}
	};

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey>
	struct base_traits :
			base_nodeval_traits<NodeVal>,
			base_edgeval_traits<EdgeVal>,
			base_nodekey_traits<NodeKey>,
			base_edgekey_traits<EdgeKey>,
			base_bind_traits<EdgeKey, NodeKey> {};

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey>
	struct default_traits : base_traits<NodeVal, EdgeVal, NodeKey, EdgeKey> {};

	template <typename NodeVal, typename EdgeVal>
	struct default_traits<NodeVal, EdgeVal, std::string, std::string> : base_traits<NodeVal, EdgeVal, std::string, std::string> {
		static std::string edgekey_brief(const std::string & e) {
			return e.substr(e.find('.') + 1);
		}
		static std::string bind(const std::string & e1, const std::string & e2) {
			return e1 + '.' + e2;
		}
	};

	template <typename NodeVal, typename EdgeVal>
	struct default_traits<NodeVal, EdgeVal, std::string, std::pair<std::string, std::string>> :
			base_traits<NodeVal, EdgeVal, std::string, std::pair<std::string, std::string>> {
		static std::string edgekey_brief(const std::pair<std::string, std::string> & e) {
			return e.second;
		}

		static std::ostream & edgekey_write_text(std::ostream & os, const std::pair<std::string, std::string> & edge) {
			return os << edge.first << '.' << edge.second;
		}

		static std::istream & edgekey_read_text(std::istream & is, std::pair<std::string, std::string> & edge) {
			std::string long_name;
			is >> long_name;
			auto pos = long_name.find('.');
			edge.first = long_name.substr(0, pos);
			edge.second = long_name.substr(pos + 1);
			return is;
		}
		static std::pair<std::string, std::string> bind(const std::string & e1, const std::string & e2) {
			return std::make_pair(e1, e2);
		}
	};

} // namespace net

#endif