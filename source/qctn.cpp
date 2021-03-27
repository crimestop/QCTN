#include <istream>
#include <string>
#include <vector>
#include <set>
#include <complex>
#include <random>
#include "qctn_tools.hpp"
#include <net/net.hpp>
#include "qctn.hpp"
#include "qpanda_grammar.hpp"

#define str std::to_string
#define vec std::vector

namespace QCTN{

	void qctn::initialize(int len,const std::vector<int> & dims,const std::vector<int> & vals){

		length=len;
		for(int i=0;i<length;++i){
			view[{0,i}]=circuit.add("init_"+str(i));
			circuit["init_"+str(i)]=init_ten(dims[i],vals[i]).edge_rename
				({{"ten.out0","ten0.out"+str(i)}});
			last_pos.push_back({0,i});
		}
		current_depth=1;
	}

	int qctn::get_length(){
		return length;
	}

	int qctn::get_clength(){
		return clength;
	}

	void qctn::dagger(){
		dagger_tag =(!dagger_tag);
	}

	void qctn::swap(int pos1,int pos2){

		auto & ten1=view[last_pos[pos1]]->second.val;
		auto & ten2=view[last_pos[pos2]]->second.val;
		ten1=ten1.edge_rename({{"ten"+str(last_pos[pos1].first)+".out"+str(pos1),"tensor.temp_leg"}});
		ten2=ten2.edge_rename({{"ten"+str(last_pos[pos2].first)+".out"+str(pos2),"ten"+str(last_pos[pos1].first)+".out"+str(pos1)}});
		ten1=ten1.edge_rename({{"tensor.temp_leg","ten"+str(last_pos[pos2].first)+".out"+str(pos2)}});

		std::iter_swap(last_pos.begin()+pos1,last_pos.begin()+pos2);

	}

	void qctn::evolve(const std::string & name, const std::vector<int> & pos,const Tensor &ope){

		view[{current_depth,pos[0]}]=circuit.add(name);
		auto & ten=circuit[name].val;
		if(dagger_tag)
			ten=ope.conjugate();
		else
			ten=ope;	
		std::vector<int> tot_pos=pos;
		if(controlled.size()>0){
			ten=add_ctr_ope<std::complex<double>>(ten,controlled.back().size());
			tot_pos.insert(tot_pos.end(),controlled.back().begin(),controlled.back().end());
		}
		for(int i=0;i<tot_pos.size();++i){
			ten=ten.edge_rename({{"ten.in"+str(i),"ten"+str(current_depth)+".in"+str(tot_pos[i])},
				{"ten.out"+str(i),"ten"+str(current_depth)+".out"+str(tot_pos[i])}});
			circuit.set_edge(view[{current_depth,pos[0]}],view[last_pos[tot_pos[i]]],
				"ten"+str(current_depth)+".in"+str(tot_pos[i]),"ten"+str(last_pos[tot_pos[i]].first)
				+".out"+str(tot_pos[i]));
		}
		for(int i:tot_pos)
			last_pos[i]={current_depth,pos[0]};
	}

	void qctn::next(){

		current_depth++;
	}


	void qctn::measure_tn(const std::vector<int> &vals){

		int d;
		for(int i=0;i< length;++i){
			auto & old_ten=view[last_pos[i]]->second.val;
			d=net::tensor::get_dim(old_ten,"ten"+str(last_pos[i].first)+".out"+str(i));
			view[{current_depth,i}]=circuit.add("measure_"+str(i));
			view[{current_depth,i}]->second.val=
				init_ten(d,vals[i]).edge_rename({{"ten.out0","ten"+str(current_depth)+".in"+str(i)}});
			circuit.set_edge(view[{current_depth,i}],view[last_pos[i]],
				"ten"+str(current_depth)+".in"+str(i),"ten"+str(last_pos[i].first)+".out"+str(i));
		}
		
	}

	std::complex<double> qctn::amplitude(const std::vector<int> &vals){

		net::Engine eg_inner;
		auto temp=circuit;
		auto viewtemp=view;
		for(auto & v:viewtemp) v.second=temp.find(v.second->first);
		int d;
		for(int i=0;i< length;++i){
			auto & old_ten=view[last_pos[i]]->second.val;
			d=net::tensor::get_dim(old_ten,"ten"+str(last_pos[i].first)+".out"+str(i));
			viewtemp[{current_depth,i}]=temp.add("measure_"+str(i));
			viewtemp[{current_depth,i}]->second.val=
				init_ten(d,vals[i]).edge_rename({{"ten.out0","ten"+str(current_depth)+".in"+str(i)}});
			temp.set_edge(viewtemp[{current_depth,i}],viewtemp[last_pos[i]],
				"ten"+str(current_depth)+".in"+str(i),"ten"+str(last_pos[i].first)+".out"+str(i));
		}

		//return std::complex<double>(temp.contract<net::no_absorb,net::tensor::tensor_contract>());
		return net::tensor::contract_tn(temp,eg_inner,"quickbb");
	}

	double qctn::possibility(const std::vector<int> &vals){
		return std::norm(amplitude(vals));
	}

	std::vector<std::complex<double>> qctn::amplitude_all(){

		net::Engine eg_inner;
		//auto ten = circuit.contract<net::no_absorb,net::tensor::tensor_contract>();
		auto ten = net::tensor::contract_tn(circuit,eg_inner,"quickbb");
		std::vector<std::string> names;
		std::string name;
		for(int i=0;i<length;++i){
			name=ten.names[i];
			ten=ten.edge_rename({{name,"ten."+name.substr(name.find('.')+1)}});
			names.push_back("ten.out"+str(length-i-1));
		}
		ten=ten.transpose(names);
		// std::cout<<ten.at({{"ten.out0",0},{"ten.out1",0},{"ten.out2",0}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",0},{"ten.out1",0},{"ten.out2",1}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",0},{"ten.out1",1},{"ten.out2",0}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",0},{"ten.out1",1},{"ten.out2",1}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",1},{"ten.out1",0},{"ten.out2",0}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",1},{"ten.out1",0},{"ten.out2",1}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",1},{"ten.out1",1},{"ten.out2",0}})<<'\n';
		// std::cout<<ten.at({{"ten.out0",1},{"ten.out1",1},{"ten.out2",1}})<<'\n';
		return {ten.block().begin(),ten.block().end()};
	}
	std::vector<double> qctn::possibility_all(){

		std::vector<std::complex<double>> amp;
		amp=amplitude_all();
		std::vector<double> res;
		for(auto &a:amp) res.push_back(std::norm(a));
		return res;
	}

	std::map<int,int> qctn::measure(const std::string & method,std::default_random_engine & R){

		std::map<int,int> creg;
		std::set<int> qbits;
		for (auto & q:cmap)
			qbits.insert(q.second);
		std::map<int,int> qbits_val=measure(qbits,method,R);
		for (auto & q:cmap)
			creg[q.first]=qbits_val[q.second];
		return creg;
	}

	std::map<int,int> qctn::measure(const std::set<int> &qbits,const std::string & method,std::default_random_engine & R){

		std::vector<int> result;
		std::map<int,int> measure_result;
		std::set<int> to_measure;
		for (auto & q:qbits){
			to_measure.insert(q);
			if(to_measure.size()>measure_limit){
				inner_measure(measure_result,to_measure,circuit,view,method,R);
				to_measure.clear();
			}
		}
		if(to_measure.size()>0)
			inner_measure(measure_result,to_measure,circuit,view,method,R);
		return measure_result;
	}

	void qctn::inner_measure(std::map<int,int> & measure_result,const std::set<int> & to_measure, 
		const net::tensor::TensorNetworkNoEnv<std::complex<double>> & circuit,
		const std::map<std::pair<int,int>, typename net::tensor::TensorNetworkNoEnv<std::complex<double>>::IterNode>& view,
		const std::string & method,
		std::default_random_engine & R){

		// std::cout<<"measure result\n";
		// for(auto s:measure_result){
		// 	std::cout<<s.first<<' '<<s.second<<'\n';
		// }
		// std::cout<<"to measure\n";
		// for(auto s:to_measure){
		// 	std::cout<<s<<'\n';
		// }



		// set temp variable, rename edge
		auto circuit_temp=circuit;
		auto view_temp=view;
		//std::cout<<"view temp\n";
		for(auto & v:view_temp){
			v.second=circuit_temp.find(v.second->first);
			//std::cout<<v.first.first<<' '<<v.first.second<<' '<<v.second->first<<'\n';
		}
		for(int i=0;i< length;++i)
			view_temp[last_pos[i]]->second.val=view_temp[last_pos[i]]->second.val.edge_rename({{"ten"+str(last_pos[i].first)+".out"+str(i),"ten.final"+str(i)}});

		// set measured value
		int i,qval;
		for(auto & res:measure_result){
			i=res.first;
			qval=res.second;
			auto & old_ten=view_temp.at(last_pos[i])->second.val;
			int d=net::tensor::get_dim(old_ten,"ten.final"+str(i));
			view_temp[{current_depth,i}]=circuit_temp.add("measure_"+str(i));
			view_temp[{current_depth,i}]->second.val=
				init_ten(d,qval).edge_rename({{"ten.out0","ten.measure"+str(i)}});
			//std::cout<<i<<'\n';
			//circuit_temp.draw("test",true);
			circuit_temp.set_edge(view_temp[{current_depth,i}],view_temp[last_pos[i]],
				"ten.measure"+str(i),"ten.final"+str(i));
		}
		//std::cout<<"finish\n";
		//std::cout<<"current_depth"<<current_depth<<'\n';

		// std::cout<<"view temp test2\n";
		// for(auto & v:view_temp){
		// 	std::cout<<v.first.first<<' '<<v.first.second<<' '<<v.second->first<<'\n';
		// }
		Tensor measure_ten;

		if(to_measure.size()+measure_result.size()==length){
			measure_ten = net::tensor::contract_tn(circuit_temp,eg,method);
			std::vector<std::string> names;
			for(auto s:to_measure) names.push_back({"ten.final"+str(s)});
			measure_ten=measure_ten.transpose(names);

		}else{
			// set measure network
			auto db_circuit_temp=net::tensor::double_tnnoenv(circuit_temp);
			for(int i=0;i< length;++i){
				if(to_measure.count(i)==0 && measure_result.count(i)==0){
					//std::cout<<"check measure \n";
					//std::cout<<i<<' '<<last_pos[i].first<<' '<<last_pos[i].second<<'\n';
					auto name_to_link=view_temp.at(last_pos[i])->first;
					//std::cout<<name_to_link<<'\n';
					db_circuit_temp.set_edge(name_to_link,"conjg_"+name_to_link,"ten.final"+str(i),"conjg_ten.final"+str(i));
				}
			}
				//circuit_temp.draw("test",true);

			//contract measure network and get diagonal element
			//auto measure_ten = circuit_temp.contract<net::no_absorb,net::tensor::tensor_contract>();
			measure_ten = net::tensor::contract_tn(db_circuit_temp,eg,method);
			std::vector<std::pair<std::string,std::string>> names;
			for(auto s:to_measure) names.push_back({"ten.final"+str(s),"conjg_ten.final"+str(s)});
			//std::cout<<measure_ten;
			measure_ten=net::tensor::get_diag(measure_ten,names);
		}

		//randomly measure
		measure_ten=measure_ten/measure_ten.norm<1>();
		int measue_pos=random_choose(measure_ten.block(),R);
		std::map<std::string,int> measure_coord = get_coord(measure_ten,measue_pos);
		for(auto s:to_measure) measure_result[s]=measure_coord["ten.final"+str(s)];
	}

	void qctn::control(const std::set<int> ctr_qbits){
		if(controlled.size()==0)
			controlled.push_back(ctr_qbits);
		else{
			controlled.push_back(controlled.back());
			controlled.back().insert(ctr_qbits.begin(),ctr_qbits.end());
		}
	}

	void qctn::endcontrol(){
		controlled.pop_back();
	}

	void qctn::init_creg(int l){
		clength = l;
	}
	void qctn::set_cmap(const std::vector<int>& qbits,const std::vector<int>& cbits){
		for(int i=0;i<qbits.size();++i)
			cmap[cbits[i]]=qbits[i];
	}

	std::istream & operator>>(std::istream & is,qctn & qc){

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