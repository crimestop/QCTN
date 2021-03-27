#ifndef QCTN_TOOLS_H
#define QCTN_TOOLS_H
#include <vector>
#include <string>
#include <net/tensor_network.hpp>
#include <net/tensor_tools.hpp>
#include <TAT/TAT.hpp>
#include "qpanda_grammar.hpp"
#define str std::to_string

namespace QCTN{

	using Tensor = TAT::Tensor<std::complex<double>>;
	Tensor init_ten(int dim, int val);
	Tensor get_ope(grammar::opetype type,const std::vector<double> & p);
	Tensor ID_ope(const std::vector<double> & p);
	Tensor Proj0_ope(const std::vector<double> & p);
	Tensor Proj1_ope(const std::vector<double> & p);
	Tensor hardmard_ope(const std::vector<double> & p);
	Tensor X_ope(const std::vector<double> & p);
	Tensor Y_ope(const std::vector<double> & p);
	Tensor Z_ope(const std::vector<double> & p);
	Tensor S_ope(const std::vector<double> & p);
	Tensor T_ope(const std::vector<double> & p);
	Tensor X1_ope(const std::vector<double> & p);
	Tensor Y1_ope(const std::vector<double> & p);
	Tensor Z1_ope(const std::vector<double> & p);
	Tensor RX_ope(const std::vector<double> & p);
	Tensor RY_ope(const std::vector<double> & p);
	Tensor RZ_ope(const std::vector<double> & p);
	Tensor U1_ope(const std::vector<double> & p);
	Tensor U2_ope(const std::vector<double> & p);
	Tensor U3_ope(const std::vector<double> & p);
	Tensor U4_ope(const std::vector<double> & p);
	Tensor CNOT_ope(const std::vector<double> & p);
	Tensor CZ_ope(const std::vector<double> & p);
	Tensor CR_ope(const std::vector<double> p);
	Tensor ISWAP_ope(const std::vector<double> & p);
	Tensor SQISWAP_ope(const std::vector<double> & p);
	Tensor ISWAPTHETA_ope(const std::vector<double> & p);
	Tensor CU_ope(const std::vector<double> & p);
	Tensor SWAP_ope(const std::vector<double> & p);
	Tensor TOFFOLI_ope(const std::vector<double> & p);
	std::ostream & write_bits(std::ostream &,const int &,const int &);
	std::ostream & write_bits(std::ostream &,const std::map<int,int> &);
	std::vector<int> to_bits(const int & i,const int & n);
	std::vector<int> to_bits(const std::map<int,int> & res);
	int random_choose(std::vector<std::complex<double>>,std::default_random_engine & R);
	std::map<std::string,int> get_coord(const Tensor & ten, int pos);
	std::vector<int> get_coord_simp(const Tensor & ten, int pos);

	template <typename T>
	TAT::Tensor<T> add_ctr_ope(TAT::Tensor<T> &tin,int nctr){
		// add nctr 2D qbits at tail
		// tout =tin⊗I-tin⊗|1><1|+I⊗|1><1|

		TAT::Tensor<T> ileft=tin.same_shape();
		ileft.zero();
		auto rank=ileft.names.size();
		std::vector<std::string> nameleft;
		for(int i=0;i<rank/2;++i){
			nameleft.push_back("ten.out"+std::to_string(i));
		}
		for(int i=0;i<rank/2;++i){
			nameleft.push_back("ten.in"+std::to_string(i));
		}
		ileft=ileft.transpose(nameleft);
		int totdleft=1;
		for(int i=0;i<rank/2;++i){
			totdleft *= net::tensor::get_dim(ileft,i);
		}
		for(int i=0;i<totdleft;++i){
			ileft.block()[i*(totdleft+1)]=1;
		}


		std::vector<std::string> nameright;
		for(int i=0;i<nctr;++i){
			nameright.push_back("ten.out"+std::to_string(rank/2+i));
		}
		for(int i=0;i<nctr;++i){
			nameright.push_back("ten.in"+std::to_string(rank/2+i));
		}
		TAT::Tensor<T> iright(nameright,std::vector<TAT::Size>(2*nctr,2));
		iright.zero();
		for(int i=0;i<(std::pow(2,nctr));++i){
			iright.block()[i*(std::pow(2,nctr)+1)]=1;
		}


		TAT::Tensor<T> oneright=iright.same_shape();
		oneright.zero();
		iright.block()[std::pow(2,2*nctr)-1]=1;

		return TAT::Tensor<T>::contract(tin,iright,{})-TAT::Tensor<T>::contract(tin,oneright,{})+TAT::Tensor<T>::contract(ileft,oneright,{});
	}

	template <typename T>
	TAT::Tensor<T> add_ctr_ope_simp(TAT::Tensor<T> &tin,int nctr){
		// add nctr qbits at tail
		// tout =tin⊗I-tin⊗|1><1|+I⊗|1><1|

		TAT::Tensor<T> ileft=T(1.),iright=T(1.),oneright=T(1.);
		auto rank=tin.names.size();
		for(int i=0;i<rank/2;++i)
			ileft=ileft.contract(ID_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}}),{});
		for(int i=rank/2;i<rank/2+nctr;++i){
			iright=iright.contract(ID_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}}),{});
			oneright=oneright.contract(Proj1_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}}),{});
		}
		// for(int i=0;i<rank/2;++i){
		// 	if(i==0){
		// 		ileft=ID_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}});
		// 	}else{
		// 		ileft=ileft.contract(ID_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}}),{});
		// 	}
		// }

		// for(int i=rank/2;i<rank/2+nctr;++i){
		// 	if(i==rank/2){
		// 		iright=ID_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}});
		// 		oneright=Proj1_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}});
		// 	}else{
		// 		iright=iright.contract(ID_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}}),{});
		// 		oneright=oneright.contract(Proj1_ope({}).edge_rename({{"ten.in0","ten.in"+str(i)},{"ten.out0","ten.out"+str(i)}}),{});
		// 	}
		// }

		return TAT::Tensor<T>::contract(tin,iright,{})-TAT::Tensor<T>::contract(tin,oneright,{})+TAT::Tensor<T>::contract(ileft,oneright,{});
	}
}
#endif