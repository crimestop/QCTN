#ifndef NET_NETWORK_DRAW_HPP
#define NET_NETWORK_DRAW_HPP

#include "error.hpp"
#include "gviz.hpp"
#include "network.hpp"
#include "node.hpp"
#include <cstdlib>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef NET_GRAPH_VIZ
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#ifdef NET_SHOW_FIG_USE_CLING
#include <unistd.h>
#endif
#endif

namespace net {

	std::string base64_encode(const std::string & in);

#ifdef NET_GRAPH_VIZ
	std::string render(std::string dot_content, const std::string & engine);
	void show_fig(const std::string & fig_content, bool tmux, bool st);

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void
	network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::draw_to_file(const std::string & filename, const std::string & title, const bool label_bond) const {
		draw(filename, {{}}, label_bond);
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::draw_to_file(
			const std::string & filename,
			const std::string & title,
			const std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & contains,
			const bool label_bond) const {
		std::ofstream fig_file(filename, std::ios::binary);
		if (this->size() > 0)
			fig_file << render(gviz(contains, label_bond), "sfdp");
	}

	inline std::string render(std::string dot_content, const std::string & engine) {
		GVC_t * gvc;
		Agraph_t * g;
		char * cres;
		std::string res;
		unsigned int len;
		gvc = gvContext();
		g = agmemread(dot_content.c_str());
		gvLayout(gvc, g, engine.c_str());
		gvRenderData(gvc, g, "png", &cres, &len);
		res = std::string(cres, len);
		gvFreeRenderData(cres);
		gvFreeLayout(gvc, g);
		agclose(g);
		return res;
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::draw(const std::string & title, const bool label_bond) const {
		draw(title, {{}}, label_bond);
	}

	template <typename NodeVal, typename EdgeVal, typename NodeKey, typename EdgeKey, typename Trait>
	void network<NodeVal, EdgeVal, NodeKey, EdgeKey, Trait>::draw(
			const std::string & title,
			const std::vector<std::set<NodeKey, typename Trait::nodekey_less>> & contains,
			const bool label_bond) const {
		consistency();
		if (this->size() == 0) {
			std::cout << "-----------------------------------------------\n";
			std::cout << "empty lattice\n";
			std::cout << "-----------------------------------------------\n";
		} else {
			show_fig(render(gviz(title, contains, label_bond), "sfdp"), false, false);
			show_fig(render(gviz_legend(contains), "neato"), false, true);
		}
	}

#ifdef NET_SHOW_FIG_USE_CLING

	extern int pipeToJupyterFD;
	// https://github.com/root-project/cling/blob/0c880f8472f5a313095203ccd35561b0097b13ec/tools/Jupyter/Kernel.cpp#L54
	inline bool show_mime(const std::map<std::string, std::string> & contentDict) {
		unsigned char sizeLong = sizeof(long);
		if (write(pipeToJupyterFD, &sizeLong, 1) != 1)
			return false;
		long dictSize = contentDict.size();
		if (write(pipeToJupyterFD, &dictSize, sizeof(long)) != sizeof(long))
			return false;

		for (auto iContent : contentDict) {
			const std::string & mimeType = iContent.first;
			long mimeTypeSize = (long)mimeType.size();
			if (write(pipeToJupyterFD, &mimeTypeSize, sizeof(long)) != sizeof(long))
				return false;
			if (write(pipeToJupyterFD, mimeType.c_str(), mimeType.size() + 1) != (long)(mimeType.size() + 1))
				return false;
			const std::string & mimeData = iContent.second;
			const long string_size = mimeData.size();
			if (write(pipeToJupyterFD, &string_size, sizeof(long)) != sizeof(long))
				return false;
			if (write(pipeToJupyterFD, mimeData.data(), string_size) != string_size)
				return false;
		}
		return true;
	}
#endif

	inline void show_fig(const std::string & fig_content, bool tmux, bool stop_tag) {
#if defined NET_SHOW_FIG_USE_ITERM
		if (tmux) {
			std::cout << "\033Ptmux;\033\033]";
		} else {
			std::cout << "\033]";
		}
		std::cout << "1337;File=;inline=1:";
		std::cout << base64_encode(fig_content);
		if (tmux) {
			std::cout << "\a\033\\";
		} else {
			std::cout << "\a";
		}
		std::cout << "\n";
		if (stop_tag) {
			std::cout << "\x1B[31mPaused.\x1B[0m Please press enter to continue.\n";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
#elif defined NET_SHOW_FIG_USE_CLING
		show_mime({{"text/html", "<img src='data:image/png;base64, " + base64_encode(fig_content) + "'></img>"}});
#elif defined NET_SHOW_FIG_USE_GWENVIEW
		std::ofstream("/tmp/net_tmp.png") << fig_content;
		std::system("gwenview /tmp/net_tmp.png || eog /tmp/net_tmp.png");
		std::cout << "\x1B[31mPaused.\x1B[0m Please press enter to continue.\n";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#else
		std::cout << "Method of showing the figure is invalid or unspecified!\n";
		std::cout << "\x1B[31mPaused.\x1B[0m Please press enter to continue.\n";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif
	}

#endif

	// base64 functions are copied from
	// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
	// originally by Manuel Martinez

	inline std::string base64_encode(const std::string & in) {
		std::string out;

		int val = 0, valb = -6;
		for (unsigned char c : in) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6)
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4)
			out.push_back('=');
		return out;
	}

	inline std::string base64_decode(const std::string & in) {
		std::string out;

		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++)
			T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

		int val = 0, valb = -8;
		for (unsigned char c : in) {
			if (T[c] == -1)
				break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
} // namespace net
#endif
