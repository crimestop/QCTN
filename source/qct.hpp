#ifndef QCT_H
#define QCT_H
#include <istream>
#include <string>
#include <vector>
#include <set>
#include <complex>
#include <random>
#include "qctn_tools.hpp"
#include <net/net.hpp>
#include "qpanda_grammar.hpp"

namespace QCTN{

	class qct{
	public:
		qct()=default;
		//copy constructor
		qct(const qct&)=default;
		//copy assignment
		qct& operator=(const qct&)=default;
		//move constructor
		qct(qct&&)=default;
		//move assignment
		qct& operator=(qct&&)=default;

		void initialize(int len,const std::vector<int> & dims,const std::vector<int> & vals);
		void init_creg(int);
		void set_cmap(const std::vector<int>&,const std::vector<int>&);
		int get_length();
		int get_clength();
		void swap(int pos1,int pos2);
		void dagger();
		void next();
		void evolve(const std::string & name, const std::vector<int> & pos,const Tensor &ope);
		
		std::complex<double> amplitude(const std::vector<int> &vals);
		double possibility(const std::vector<int> &vals);
		std::vector<std::complex<double>> amplitude_all();
		std::vector<double> possibility_all();
		void control(const std::set<int> ctr_qbits);
		void endcontrol();
		Tensor state;
	private:
		int length=0;
		int clength=0;
		int current_depth=0;
		int measure_limit=2;
		bool dagger_tag=false;
		std::vector<std::pair<int,int>> last_pos;
		std::map<int,int> cmap;
		std::vector<std::set<int>> controlled;
	};
	std::istream & operator>>(std::istream &, qct &);
}
#endif