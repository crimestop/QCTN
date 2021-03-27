#include <cmath>
#include <vector>
#include <complex>
#include <iostream>
#include <random>
#include "qctn_tools.hpp"
#include <net/tensor_network.hpp>
#include <TAT/TAT.hpp>
#include "qpanda_grammar.hpp"

namespace QCTN{

	using namespace std;
	const std::complex<double> I(0, 1);

	std::vector<int> to_bits(const int & i,const int & n){
		std::vector<int> result;
		for (int j=n-1;j>=0;--j)
			result.push_back((i>>j)&1);
		return result;
	}

	std::vector<int> to_bits(const std::map<int,int> & res){
		std::vector<int> result;
		for (auto it=res.rbegin();it!=res.rend();++it)
			result.push_back(it->second);
		return result;
	}

	std::ostream & write_bits(std::ostream & os,const int & i,const int & n){
		for (int j=n-1;j>=0;--j)
			os<<((i>>j)&1);
		return os;
	}

	std::ostream & write_bits(std::ostream & os,const std::map<int,int> & res){
		for (auto it=res.rbegin();it!=res.rend();++it)
			os<<it->second;
		return os;
	}

	Tensor init_ten(int dim, int val){
		Tensor T({"ten.out0"},{dim});
		T.zero();
		T.at({{"ten.out0",val}})=1;
		return T;
	}

	Tensor get_ope(grammar::opetype type,const std::vector<double> & p){
		switch(type){
			case grammar::H: return hardmard_ope(p);break;
			case grammar::X: return X_ope(p);break;
			case grammar::Y: return Y_ope(p);break;
			case grammar::Z: return Z_ope(p);break;
			case grammar::S: return S_ope(p);break;
			case grammar::T: return T_ope(p);break;
			case grammar::X1: return X1_ope(p);break;
			case grammar::Y1: return Y1_ope(p);break;
			case grammar::Z1: return Z1_ope(p);break;
			case grammar::RX: return RX_ope(p);break;
			case grammar::RY: return RY_ope(p);break;
			case grammar::RZ: return RZ_ope(p);break;
			case grammar::U1: return U1_ope(p);break;
			case grammar::U2: return U2_ope(p);break;
			case grammar::U3: return U3_ope(p);break;
			case grammar::U4: return U4_ope(p);break;
			case grammar::CNOT: return CNOT_ope(p);break;
			case grammar::CZ: return CZ_ope(p);break;
			case grammar::CR: return CR_ope(p);break;
			case grammar::CU: return CU_ope(p);break;
			case grammar::SWAP: return SWAP_ope(p);break;
			case grammar::ISWAP: return ISWAP_ope(p);break;
			case grammar::SQISWAP: return SQISWAP_ope(p);break;
			case grammar::ISWAPTHETA: return ISWAPTHETA_ope(p);break;
			case grammar::TOFFOLI: return TOFFOLI_ope(p);break;
			default: return Tensor();break;
		}
	}

	Tensor ID_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,0.,
					0.,1.};
		return T;
	}
	Tensor Proj0_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,0.,
					0.,0.};
		return T;
	}
	Tensor Proj1_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	0.,0.,
					0.,1.};
		return T;
	}

	Tensor hardmard_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,1.,
					1.,-1.};
		return T/sqrt(2.);
	}

	Tensor X_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	0.,1.,
					1.,0.};
		return T;
	}

	Tensor Y_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	0.,-I,
					I,0.};
		return T;
	}

	Tensor Z_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,0.,
					0.,-1.};
		return T;
	}

	Tensor S_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,0.,
					0.,I};
		return T;
	}

	Tensor T_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,0.,
					0.,(1.+I)/sqrt(2.)};
		return T;
	}

	Tensor X1_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,-I,
					-I,1.};
		return T;
	}

	Tensor Y1_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,-1.,
					1.,1.};
		return T;
	}

	Tensor Z1_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.-I,0.,
					0.,1.+I};
		return T;
	}

	Tensor RX_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	cos(p[0]/2.),-sin(p[0]/2.)*I,
					-sin(p[0]/2.)*I,cos(p[0]/2.)};
		return T;
	}

	Tensor RY_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	cos(p[0]/2.),-sin(p[0]/2.),
					sin(p[0]/2.),cos(p[0]/2.)};
		return T;
	}

	Tensor RZ_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	exp(-p[0]/2.*I),0.,
					0.,exp(p[0]/2.*I)};
		return T;
	}

	Tensor U1_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,0.,
					0.,exp(p[0]*I)};
		return T;
	}

	Tensor U2_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	1.,-exp(p[1]*I),
					exp(p[0]*I),exp((p[0]+p[1])*I)};
		return T/sqrt(2.);
	}

	Tensor U3_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.in0"},{2,2});
		T.block()={	cos(p[0]/2.),-exp(p[2]*I)*sin(p[0]/2.),
					exp(p[1]*I)*sin(p[0]/2.),exp((p[1]+p[2])*I)*cos(p[0]/2.)};
		return T;
	}

	Tensor U4_ope(const std::vector<double> & p){
		return U3_ope({p[2],p[1],p[3]})*(exp((p[0]-p[1]/2.-p[3]/2.)*I));
	}

	Tensor CNOT_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,1.,0.,0.,
					0.,0.,0.,1.,
					0.,0.,1.,0.,};
		return T;
	}

	Tensor CZ_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,1.,0.,0.,
					0.,0.,1.,0.,
					0.,0.,0.,-1.,};
		return T;
	}

	Tensor CR_ope(const std::vector<double> p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,1.,0.,0.,
					0.,0.,1.,0.,
					0.,0.,0.,exp(p[0]*I)};
		return T;
	}

	Tensor ISWAP_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,0.,-I,0.,
					0.,-I,0.,0.,
					0.,0.,0.,1.};
		return T;
	}

	Tensor SQISWAP_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,1./sqrt(2.),-I/sqrt(2.),0.,
					0.,-I/sqrt(2.),1./sqrt(2.),0.,
					0.,0.,0.,1.};
		return T;
	}

	Tensor ISWAPTHETA_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,cos(p[1]),-sin(p[1])*I,0.,
					0.,-sin(p[1])*I,cos(p[1]),0.,
					0.,0.,0.,1.};
		return T;
	}

	Tensor CU_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,1.,0.,0.,
					0.,1.,exp(p[1]-p[2]/2.-p[4]/2.)*cos(p[3]/2),-exp(p[1]-p[2]/2.+p[4]/2.)*sin(p[3]/2),
					0.,1.,-exp(p[1]+p[2]/2.-p[4]/2.)*sin(p[3]/2),exp(p[1]+p[2]/2.+p[4]/2.)*cos(p[3]/2)};
		return T;
	}

	Tensor SWAP_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.in0","ten.in1"},{2,2,2,2});
		T.block()={	1.,0.,0.,0.,
					0.,0.,1.,0.,
					0.,1.,0.,0.,
					0.,0.,0.,1.};
		return T;
	}

	Tensor TOFFOLI_ope(const std::vector<double> & p){
		Tensor T({"ten.out0","ten.out1","ten.out2","ten.in0","ten.in1","ten.in2"},{2,2,2,2,2,2});
		T.block()={	1.,0.,0.,0.,0.,0.,0.,0.,
					0.,1.,0.,0.,0.,0.,0.,0.,
					0.,0.,1.,0.,0.,0.,0.,0.,
					0.,0.,0.,1.,0.,0.,0.,0.,
					0.,0.,0.,0.,1.,0.,0.,0.,
					0.,0.,0.,0.,0.,1.,0.,0.,
					0.,0.,0.,0.,0.,0.,0.,1.,
					0.,0.,0.,0.,0.,0.,1.,0.};
		// for(int i=0;i<2;++i)
		// for(int j=0;j<2;++j)
		// for(int k=0;k<2;++k)
		// for(int l=0;l<2;++l)
		// for(int m=0;m<2;++m)
		// for(int n=0;n<2;++n)
		// 	std::cout<<i<<' '<<j<<' '<<k<<' '<<l<<' '<<m<<' '<<n<<' '<<T.at({{"ten.out0",i},{"ten.out1",j},{"ten.out2",k},{"ten.in0",l},{"ten.in1",m},{"ten.in2",n}})<<'\n';

		return T;
	}
	
	int random_choose(std::vector<std::complex<double>> measure_val,std::default_random_engine & R){
		// choose an integer randomly according to a probability amplitude
		auto distribution = std::uniform_real_distribution<double>(0.,1.);
		int find;
		double cumu_possi=0.;
		double possi=distribution(R);
		for(find=0;(cumu_possi+=std::abs(measure_val[find]))<possi; ++find);
		return find;
	}
	
	std::map<std::string,int> get_coord(const Tensor & ten, int pos){
		int rank=ten.names.size();
		std::map<std::string,int> result;
		for(int i=rank-1;i>=0;--i){
			result[ten.names[i]]=pos % net::tensor::get_dim(ten,i);
			pos=pos/net::tensor::get_dim(ten,i);
		}
		return result;
	}
}