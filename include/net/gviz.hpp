#ifndef NET_GVIZ_HPP
#define NET_GVIZ_HPP

#include "error.hpp"
#include "gviz_themes.hpp"
#include "network.hpp"
#include "node.hpp"
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace net {

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::string network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::gviz(
			const std::string & title,
			const std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & groups,
			const bool label_bond) const {
		std::stringstream dot_content;
		std::string grp;
		std::set<NodeKey, typename Trait::nodekey_less> drawn_nodes;

		dot_content << "digraph G {\n";
		dot_content << "  scale=0.6\n";
		dot_content << "  dpi=160\n";
		dot_content << "  bgcolor=" << gviz_theme["global_bgcolor"] << "\n";
		dot_content << "  fontcolor=" << gviz_theme["global_fontcolor"] << "\n";
		dot_content << "  fontname=" << gviz_theme["global_fontname"] << "\n";
		dot_content << "  label = \"" << title << "\"\n";

		for (auto & s_it : *this) {
			auto & nodekey1 = s_it.first;
			grp = "def";
			for (int i = 0; i < groups.size(); ++i)
				if (groups[i].count(nodekey1) == 1)
					grp = std::to_string(i);
			dot_content << "  " << Trait::nodekey_brief(nodekey1)
							<< " [ color=" << gviz_theme["group" + grp].value("node_strokecolor", gviz_theme["node_strokecolor"]) << ", label = \""
							<< Trait::nodekey_brief(nodekey1)
							<< ""
							<< "\", fontcolor=" << gviz_theme["group" + grp].value("node_fontcolor", gviz_theme["node_fontcolor"])
							<< ", fontname=" << gviz_theme["group" + grp].value("node_fontname", gviz_theme["node_fontname"]) << "]\n";
		}
		dot_content << "subgraph bond {\n";
		dot_content << "  edge[dir=none]\n";

		for (auto & s_it : *this) {
			auto & nodekey1 = s_it.first;
			for (auto & b_it : s_it.second.edges) {
				auto & ind1 = b_it.first;
				auto & nodekey2 = b_it.second.nbkey;
				auto & ind2 = b_it.second.nbind;
				if (drawn_nodes.count(nodekey2) == 0) {
					grp = "def";
					for (int i = 0; i < groups.size(); ++i)
						if (groups[i].count(nodekey1) == 1 && groups[i].count(nodekey2) == 1)
							grp = std::to_string(i);
					dot_content << "  " << Trait::nodekey_brief(nodekey1) << " -> " << Trait::nodekey_brief(nodekey2)
									<< " [fontcolor=" << gviz_theme["group" + grp].value("edge_fontcolor", gviz_theme["edge_fontcolor"])
									<< ", fontname=" << gviz_theme["group" + grp].value("edge_fontname", gviz_theme["edge_fontname"])
									<< ", color=" << gviz_theme["group" + grp].value("edge_strokecolor", gviz_theme["edge_strokecolor"]);
					if (label_bond)
						dot_content << ", taillabel = \"" << Trait::edgekey_brief(ind1) << "\",headlabel =\"" << Trait::edgekey_brief(ind2) << "\"";
					dot_content << ", len=10]\n";
				}
			}
			drawn_nodes.insert(nodekey1);
		}

		dot_content << "}\n}\n";

		// std::cout<<dot_content.str();
		return dot_content.str();
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	std::string
	network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::gviz_legend(const std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & groups) const {
		std::stringstream dot_content;
		std::string grp;
		std::set<NodeKey, typename Trait::nodekey_less> drawn_nodes;

		dot_content << "digraph G {\n";
		dot_content << "  scale=0.6\n";
		dot_content << "  dpi=160\n";
		dot_content << "  bgcolor=" << gviz_theme["global_bgcolor"] << "\n";

		int i = 0;
		grp = "def";
		dot_content << "  node" << std::to_string(i)
						<< " [ color=" << gviz_theme["group" + grp].value("node_strokecolor", gviz_theme["node_strokecolor"]) << ", label = \"group" << grp
						<< "\", fontcolor=" << gviz_theme["group" + grp].value("node_fontcolor", gviz_theme["node_fontcolor"])
						<< ", fontname=" << gviz_theme["group" + grp].value("node_fontname", gviz_theme["node_fontname"]) << ", pos=\""
						<< std::to_string(3 * i) << "," << std::to_string(-i / 6) << "!\"]\n";
		for (auto & g : groups) {
			for (auto & p : g) {
				if (this->count(p) > 0) {
					++i;
					grp = std::to_string(i - 1);
					dot_content << "  node" << std::to_string(i)
									<< " [ color=" << gviz_theme["group" + grp].value("node_strokecolor", gviz_theme["node_strokecolor"])
									<< ", label = \"group" << grp
									<< "\", fontcolor=" << gviz_theme["group" + grp].value("node_fontcolor", gviz_theme["node_fontcolor"])
									<< ", fontname=" << gviz_theme["group" + grp].value("node_fontname", gviz_theme["node_fontname"]) << ", pos=\""
									<< std::to_string(3 * i) << "," << std::to_string(-i / 6) << "!\"]\n";
					break;
				}
			}
		}

		dot_content << "}\n";

		// std::cout<<dot_content.str();
		return dot_content.str();
	}
} // namespace net
#endif