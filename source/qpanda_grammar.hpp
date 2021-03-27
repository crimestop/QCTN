#ifndef QPANDA_GRAMMER_H
#define QPANDA_GRAMMER_H

#include<vector>
#include<string>
#include<regex>

namespace QCTN{
	namespace grammar{
		enum opetype{
			QINIT,CREG,DAGGER,ENDDAGGER,CONTROL,ENDCONTROL,MEASURE,ILLEGAL,
			H,X,Y,Z,S,T,X1,Y1,Z1,RX,RY,RZ,U1,U2,U3,U4,CNOT,CZ,CR,CU,SWAP,ISWAP,SQISWAP,ISWAPTHETA,TOFFOLI
		};

		inline const std::set<opetype> GATES({H,X,Y,Z,S,T,X1,Y1,Z1,RX,RY,RZ,U1,U2,U3,U4,
										CNOT,CZ,CR,CU,SWAP,ISWAP,SQISWAP,ISWAPTHETA,TOFFOLI});

		inline const std::map<std::string,opetype> opetype_map={{"QINIT",QINIT},{"CREG",CREG},{"DAGGER",DAGGER},
			{"ENDDAGGER",ENDDAGGER},{"CONTROL",CONTROL},{"ENDCONTROL",ENDCONTROL},{"MEASURE",MEASURE},
			{"H",H},{"X",X},{"Y",Y},{"Z",Z},{"S",S},{"T",T},{"X1",X1},{"Y1",Y1},{"Z1",Z1},
			{"RX",RX},{"RY",RY},{"RZ",RZ},{"U1",U1},{"U2",U2},{"U3",U3},{"U4",U4},
			{"CNOT",CNOT},{"CZ",CZ},{"CR",CR},{"CU",CU},
			{"SWAP",SWAP},{"SQISWAP",SQISWAP},{"ISWAPTHETA",ISWAPTHETA},{"TOFFOLI",TOFFOLI}};

		inline opetype opetype_cast(const std::string & opename){
			auto type = opetype_map.find(opename);
			if (type != opetype_map.end()) {
				return type->second;
			} else {
				return ILLEGAL;
			}
		}

		inline std::string get_operator(const std::string & expr){

			std::regex ope_regex(R"(\b\w+\b)");
			std::smatch ope_match;
			if(std::regex_search(expr, ope_match, ope_regex))
				return ope_match.str(0);
			else 
				return "";

		}

		inline int get_int(const std::string & expr){

			std::regex int_regex(R"(\d+)");
			std::smatch int_match;
			if(std::regex_search(expr, int_match, int_regex)){
				return std::stoi(int_match.str(0));
			}else return 0;
			
		}

		inline void get_operands(const std::string & expr, std::vector<int> & qbits, std::vector<int> & cbits, std::vector<double> & paras){

			qbits.clear();
			std::regex qbits_regex(R"(q\[\d+\])");
			for(auto i = std::sregex_iterator(expr.begin(),expr.end(),qbits_regex);i != std::sregex_iterator();++i){
				std::string dc_qbit=i->str(0);
				std::smatch qbit_match;
				std::regex qbit_regex(R"(\d+)");
				std::regex_search(dc_qbit, qbit_match, qbit_regex);
				qbits.push_back(std::stoi(qbit_match[0].str()));
			}

			cbits.clear();
			std::regex cbits_regex(R"(c\[\d+\])");
			for(auto i = std::sregex_iterator(expr.begin(),expr.end(),cbits_regex);i != std::sregex_iterator();++i){
				std::string dc_cbit=i->str(0);
				std::smatch cbit_match;
				std::regex cbit_regex(R"(\d+)");
				std::regex_search(dc_cbit, cbit_match, cbit_regex);
				cbits.push_back(std::stoi(cbit_match[0].str()));
			}

			paras.clear();
			std::regex paras_regex(R"(\(.+\))");
			std::regex para_regex(R"([^\)\(,]+)");
			for(auto i = std::sregex_iterator(expr.begin(), expr.end(),paras_regex);i != std::sregex_iterator();++i){
				std::string paras=i->str(0);
				for(auto j = std::sregex_iterator(paras.begin(), paras.end(),para_regex);j != std::sregex_iterator();++j)
					paras.push_back(std::stod(j->str(0)));
			}
			
		}
	}
}
#endif