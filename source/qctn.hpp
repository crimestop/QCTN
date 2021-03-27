#ifndef QCTN_H
#define QCTN_H
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

	class qctn{
	public:
		qctn()=default;
		//copy constructor
		qctn(const qctn&)=default;
		//copy assignment
		qctn& operator=(const qctn&)=default;
		//move constructor
		qctn(qctn&&)=default;
		//move assignment
		qctn& operator=(qctn&&)=default;

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
		void measure_tn(const std::vector<int> &vals);
		std::map<int,int> measure(const std::string &method,std::default_random_engine & R);
		std::map<int,int> measure(const std::set<int> &qbits,const std::string &method,std::default_random_engine & R);
		net::tensor::TensorNetworkNoEnv<std::complex<double>> circuit;
		net::Engine eg;
	private:
		std::map<std::pair<int,int>, typename net::tensor::TensorNetworkNoEnv<std::complex<double>>::IterNode> view;
		int length=0;
		int clength=0;
		int current_depth=0;
		int measure_limit=2;
		bool dagger_tag=false;
		std::vector<std::pair<int,int>> last_pos;
		std::map<int,int> cmap;
		std::vector<std::set<int>> controlled;
		void inner_measure(std::map<int,int> & measure_result,const std::set<int>& to_measure, 
			const net::tensor::TensorNetworkNoEnv<std::complex<double>>& circuit,
			const std::map<std::pair<int,int>, typename net::tensor::TensorNetworkNoEnv<std::complex<double>>::IterNode>& view,
			const std::string &method,std::default_random_engine & R);
	};
	std::istream & operator>>(std::istream &, qctn &);
}
#endif