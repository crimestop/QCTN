#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <random>
#include "source/qctn.hpp"
#include "source/qct.hpp"

int main(){
	QCTN::qct qc2;
	std::ifstream("input.dat")>>qc2; //测试概率
	auto possi2=qc2.possibility_all();


	QCTN::qctn qc;
	std::ifstream("input.dat")>>qc; //传入脚本

	std::cout<<"print all possibility:\n";
	auto possi=qc.possibility_all();  //得到所有概率，为std::vector<double>，第i个值为所有qbit转化为十进制取值为i的概率
	for(int i=0;i<possi.size();++i)
		QCTN::write_bits(std::cout,i,qc.get_length())<<" : "<<possi[i]<<" , "<<possi2[i]<<'\n';
		//QCTN::write_bits 将十进制数字转化为二进制按照低位在右输出

	std::cout<<"\nmeasure 1000 times:\n";
	int seed =std::random_device()();
	std::cout<<"seed = "<<seed<<'\n';
	seed=869250713;
	std::default_random_engine random_engine(seed);
	std::map<std::vector<int>,int> mcount;
	for(int i=0;i<1000;++i){
	 	auto result=QCTN::to_bits(qc.measure("quickbb",random_engine));
	 	//qc.measure得到测量结果，为std::map<int,int>，为对应cbit的值
	 	//QCTN::to_bits将结果转化为std::vector<int>，是二进制的测量值，低位在右
		//可以用 QCTN::write_bits(std::cout,qc.measure(random_engine))<<'\n'; 将结果转化为二进制按照低位在右输出
		if(mcount.count(result)==0)
			mcount[result]=1;
		else
			mcount[result]++;
	}
	for(auto & i:mcount){
		for(auto b: i.first) std::cout<<b;
		std::cout<<" : "<<double(i.second)/100<<'\n';
	}

	return 0;
}

