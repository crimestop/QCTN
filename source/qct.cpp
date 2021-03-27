#include <istream>
#include <string>
#include <vector>
#include <set>
#include <complex>
#include <random>
#include "qctn_tools.hpp"
#include <net/net.hpp>
#include "qct.hpp"
#include "qpanda_grammar.hpp"

#define str std::to_string
#define vec std::vector

namespace QCTN{

	void qct::initialize(int len,const std::vector<int> & dims,const std::vector<int> & vals){

		length=len;
		// for(int i=0;i<length;++i){
		// 	if(i==0){
		// 		state=init_ten(dims[i],vals[i]).edge_rename({{"ten.out0","ten.out"+str(i)}});
		// 	}else{
		// 		state=state.contract(init_ten(dims[i],vals[i]).edge_rename({{"ten.out0","ten.out"+str(i)}}),{});
		// 	}
		// }
		state=std::complex<double>(1.);
		for(int i=0;i<length;++i)
			state=state.contract(init_ten(dims[i],vals[i]).edge_rename({{"ten.out0","ten.out"+str(i)}}),{});
		current_depth=1;
	}

	int qct::get_length(){
		return length;
	}

	int qct::get_clength(){
		return clength;
	}

	void qct::dagger(){
		dagger_tag =(!dagger_tag);
	}

	void qct::swap(int pos1,int pos2){

		state=state.edge_rename({{"ten.out"+str(pos1),"ten.out"+str(pos2)},{"ten.out"+str(pos2),"ten.out"+str(pos1)}});

	}

	void qct::evolve(const std::string & name, const std::vector<int> & pos,const Tensor &ope){

		Tensor ten;
		if(dagger_tag)
			ten=ope.conjugate();
		else
			ten=ope;	
		std::vector<int> tot_pos=pos;
		if(controlled.size()>0){
			ten=add_ctr_ope_simp<std::complex<double>>(ten,controlled.back().size());
			tot_pos.insert(tot_pos.end(),controlled.back().begin(),controlled.back().end());
		}

		std::map<std::string,std::string> rnames;
		std::set<std::pair<std::string,std::string>> cnames;
		for(int i=0;i<tot_pos.size();++i){
			rnames.insert({"ten.in"+str(i),"ten.in"+str(tot_pos[i])});
			rnames.insert({"ten.out"+str(i),"ten.out"+str(tot_pos[i])});
			cnames.insert({"ten.out"+str(tot_pos[i]),"ten.in"+str(tot_pos[i])});
		}
		//std::cout<<ten;
		state=state.contract(ten.edge_rename(rnames),cnames);
	}

	void qct::next(){

		current_depth++;
	}

	std::complex<double> qct::amplitude(const std::vector<int> &vals){

		std::map<std::string,int> elem_pos;
		for(int i=0;i< length;++i){
			elem_pos.insert({"ten.out"+str(i),vals[i]});
		}

		return state.const_at(elem_pos);
	}

	double qct::possibility(const std::vector<int> &vals){
		return std::norm(amplitude(vals));
	}

	std::vector<std::complex<double>> qct::amplitude_all(){

		std::vector<std::string> names;
		for(int i=0;i<length;++i){
			names.push_back("ten.out"+str(length-i-1));
		}
		state=state.transpose(names);
		return {state.block().begin(),state.block().end()};
	}
	std::vector<double> qct::possibility_all(){

		std::vector<std::complex<double>> amp;
		amp=amplitude_all();
		std::vector<double> res;
		for(auto &a:amp) res.push_back(std::norm(a));
		return res;
	}

	void qct::control(const std::set<int> ctr_qbits){
		if(controlled.size()==0)
			controlled.push_back(ctr_qbits);
		else{
			controlled.push_back(controlled.back());
			controlled.back().insert(ctr_qbits.begin(),ctr_qbits.end());
		}
	}

	void qct::endcontrol(){
		controlled.pop_back();
	}

	void qct::init_creg(int l){
		clength = l;
	}
	void qct::set_cmap(const std::vector<int>& qbits,const std::vector<int>& cbits){
		for(int i=0;i<qbits.size();++i)
			cmap[cbits[i]]=qbits[i];
	}

	std::istream & operator>>(std::istream & is,qct & qc){

		std::vector<int> qbits;
		std::vector<int> cbits;
		std::vector<double> paras;
		int op_order=0;
		for(std::string line;std::getline(is,line);){
			std::string type_name=grammar::get_operator(line);
			grammar::opetype type=grammar::opetype_cast(type_name);
			switch(type){
				case grammar::QINIT:
					{int qbit_num = grammar::get_int(line);
					qc.initialize(qbit_num,std::vector<int>(qbit_num,2),std::vector<int>(qbit_num,0));}
					break;
				case grammar::CREG:
					qc.init_creg(grammar::get_int(line));
					break;
				case grammar::DAGGER:
				case grammar::ENDDAGGER:
					qc.dagger();
					break;
				case grammar::CONTROL:
					grammar::get_operands(line,qbits,cbits,paras);
					qc.control(std::set<int>(qbits.begin(),qbits.end()));
					break;
				case grammar::ENDCONTROL:
					qc.endcontrol();
					break;
				case grammar::MEASURE:
					grammar::get_operands(line,qbits,cbits,paras);
					qc.set_cmap(qbits,cbits);
					break;
				default :
					break;
			}
			if(grammar::GATES.count(type)==1){
				grammar::get_operands(line,qbits,cbits,paras);
				qc.evolve(type_name+'_'+str(op_order),qbits,get_ope(type,paras));
				op_order++;
				qc.next();
			}
		}
		return is;
	}
}