#ifndef NET_GVIZ_THEMES
#define NET_GVIZ_THEMES
#include <nlohmann/json.hpp>
#include <string>
namespace net {
	// use default plot color in matplotlib
	// we use json lib https://github.com/nlohmann/json to parse json
	inline std::string theme_dark1 = R"({
	"global_bgcolor" : "transparent" ,
	"global_fontcolor" : "white" ,
	"global_fontname" : "Monaco" ,
	"node_strokecolor" : "white" ,
	"node_fontcolor" : "white" ,
	"node_fontname" : "Monaco" ,
	"edge_strokecolor" : "white" ,
	"edge_fontcolor" : "white" ,
	"edge_fontname" : "Monaco" ,
	"groupdef" : {},
	"group0" : {
		"node_strokecolor":"#ff7f0e" ,
		"edge_strokecolor":"#ff7f0e"
	},
	"group1" : {
		"node_strokecolor":"#1f77b4" ,
		"edge_strokecolor":"#1f77b4"
	},
	"group2" : {
		"node_strokecolor":"#2ca02c" ,
		"edge_strokecolor":"#2ca02c"
	},
	"group3" : {
		"node_strokecolor":"#d62728" ,
		"edge_strokecolor":"#d62728"
	},
	"group4" : {
		"node_strokecolor":"#9467bd" ,
		"edge_strokecolor":"#9467bd"
	},
	"group5" : {
		"node_strokecolor":"#8c564b" ,
		"edge_strokecolor":"#8c564b"
	},
	"group6" : {
		"node_strokecolor":"#e377c2" ,
		"edge_strokecolor":"#e377c2"
	},
	"group7" : {
		"node_strokecolor":"#7f7f7f" ,
		"edge_strokecolor":"#7f7f7f"
	},
	"group8" : {
		"node_strokecolor":"#bcbd22" ,
		"edge_strokecolor":"#bcbd22"
	},
	"group9" : {
		"node_strokecolor":"#17becf" ,
		"edge_strokecolor":"#17becf"
	}
})";

	inline std::string theme_light1 = R"({
	"global_bgcolor":"white",
	"global_fontcolor":"black",
	"global_fontname":"Monaco",
	"node_strokecolor":"black",
	"node_fontcolor":"black",
	"node_fontname":"Monaco",
	"edge_strokecolor":"black",
	"edge_fontcolor":"black",
	"edge_fontname":"Monaco",
	"groupdef":{
	},
	"group0" : {
		"node_strokecolor":"#ff7f0e" ,
		"edge_strokecolor":"#ff7f0e"
	},
	"group1" : {
		"node_strokecolor":"#1f77b4" ,
		"edge_strokecolor":"#1f77b4"
	},
	"group2" : {
		"node_strokecolor":"#2ca02c" ,
		"edge_strokecolor":"#2ca02c"
	},
	"group3" : {
		"node_strokecolor":"#d62728" ,
		"edge_strokecolor":"#d62728"
	},
	"group4" : {
		"node_strokecolor":"#9467bd" ,
		"edge_strokecolor":"#9467bd"
	},
	"group5" : {
		"node_strokecolor":"#8c564b" ,
		"edge_strokecolor":"#8c564b"
	},
	"group6" : {
		"node_strokecolor":"#e377c2" ,
		"edge_strokecolor":"#e377c2"
	},
	"group7" : {
		"node_strokecolor":"#7f7f7f" ,
		"edge_strokecolor":"#7f7f7f"
	},
	"group8" : {
		"node_strokecolor":"#bcbd22" ,
		"edge_strokecolor":"#bcbd22"
	},
	"group9" : {
		"node_strokecolor":"#17becf" ,
		"edge_strokecolor":"#17becf"
	}
})";

	inline std::string theme_light2 = R"({
	"global_bgcolor":"transparent",
	"global_fontcolor":"black",
	"global_fontname":"Monaco",
	"node_strokecolor":"black",
	"node_fontcolor":"black",
	"node_fontname":"Monaco",
	"edge_strokecolor":"black",
	"edge_fontcolor":"black",
	"edge_fontname":"Monaco",
	"groupdef":{
	},
	"group0" : {
		"node_strokecolor":"#ff7f0e" ,
		"edge_strokecolor":"#ff7f0e"
	},
	"group1" : {
		"node_strokecolor":"#1f77b4" ,
		"edge_strokecolor":"#1f77b4"
	},
	"group2" : {
		"node_strokecolor":"#2ca02c" ,
		"edge_strokecolor":"#2ca02c"
	},
	"group3" : {
		"node_strokecolor":"#d62728" ,
		"edge_strokecolor":"#d62728"
	},
	"group4" : {
		"node_strokecolor":"#9467bd" ,
		"edge_strokecolor":"#9467bd"
	},
	"group5" : {
		"node_strokecolor":"#8c564b" ,
		"edge_strokecolor":"#8c564b"
	},
	"group6" : {
		"node_strokecolor":"#e377c2" ,
		"edge_strokecolor":"#e377c2"
	},
	"group7" : {
		"node_strokecolor":"#7f7f7f" ,
		"edge_strokecolor":"#7f7f7f"
	},
	"group8" : {
		"node_strokecolor":"#bcbd22" ,
		"edge_strokecolor":"#bcbd22"
	},
	"group9" : {
		"node_strokecolor":"#17becf" ,
		"edge_strokecolor":"#17becf"
	}
})";

	inline auto gviz_theme_dark1 = nlohmann::json::parse(theme_dark1);
	inline auto gviz_theme_light1 = nlohmann::json::parse(theme_light1);
	inline auto gviz_theme_light2 = nlohmann::json::parse(theme_light2);

	inline auto gviz_theme = gviz_theme_dark1;
} // namespace net
#endif