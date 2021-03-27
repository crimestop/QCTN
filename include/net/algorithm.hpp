#ifndef NET_ALGORITHM_HPP
#define NET_ALGORITHM_HPP

#include "network.hpp"
#include <vector>
#include <set>
#include <memory>
#include <random>
#include <utility>
#include <iterator>
#include <valarray>
#include <iostream>

namespace net{

	bool is_connected(const int & N, const std::set<std::pair<int,int>> & edge){ // check if a graph is connected
		std::vector<std::shared_ptr<std::set<int>>> connected_componets(N);
		for(int i=0;i<N;++i)
			connected_componets[i]=std::make_shared<std::set<int>>(std::set<int>{i});
		for(auto & e:edge){
			//std::cout<<"   ---"<<e.first<<' '<<e.second<<'\n';
			if(connected_componets[e.first]!=connected_componets[e.second]){
				connected_componets[e.first]->insert(connected_componets[e.second]->begin(),connected_componets[e.second]->end());
				connected_componets[e.second]=connected_componets[e.first];
			}
		}
		//std::cout<<connected_componets[0]->size()<<' '<<N<<'\n';
		return (connected_componets[0]->size()==N);
	}

	std::set<std::pair<int,int>> random_edges(const int & N, const int & d, std::default_random_engine & R){
	// reference: Generating random regular graphs quickly, A. STEGER and N. C. WORMALD
	// reference: Efficient and Simple Generation of Random Simple Connected Graphs with Prescribed Degree Sequence, Fabien Viger and Matthieu Latapy

		std::valarray<int> avail(N);
		auto distribution = std::uniform_real_distribution<double>(0., 1.);
		std::set<std::pair<int,int>> edges;
		int tot_avail;
		int ind1,ind2,site1,site2;
		bool found;
		do{
			tot_avail=N*d;
			for(int i=0;i<N;++i)
				avail[i]=d;
			edges.clear();

			for(int i=0;i<N;++i){
				edges.insert({i,(i+1)%N});
				avail[i]--;
				avail[(i+1)%N]--;
				tot_avail-=2;
			}
			found = true;
			while(tot_avail>0){
				//std::cout<<tot_avail<<'\n';
				if(!found){ // if there's no available edge, delete d edges
					for(int i=0;i<d;++i){
						if(edges.size()>0){
							auto it=edges.begin();
							std::advance(it,int(edges.size()*distribution(R)));
							avail[it->first]++;
							avail[it->second]++;
							edges.erase(it);
							tot_avail+=2;
						}
					}
					// for(auto s:avail)
					// 	std::cout<<s<<' ';
					// std::cout<<std::endl;
				}

				found = false;
				for(int i=0;i<1000;i++){
					ind1=tot_avail*distribution(R);
					ind2=tot_avail*distribution(R);
					//std::cout<<' '<<ind1<<' '<<ind2<<' '<<tot_avail<<'\n';
					site1=-1;
					for(int site_cumu=-1;site_cumu<ind1;site_cumu+=avail[++site1]){
						//std::cout<<' '<<site1<<' '<<avail[site1]<<site_cumu<<'\n';
					}
					//std::cout<<' '<<site1<<'\n';
					site2=-1;
					for(int site_cumu=-1;site_cumu<ind2;site_cumu+=avail[++site2]);
					//std::cout<<' '<<site1<<' '<<site2<<'\n';
					if(site1 != site2 && edges.count({site1,site2})==0 && edges.count({site2,site1}) ==0){
														 // avoid double edge and loop on one site
						found=true;
						edges.insert({site1,site2});
						avail[site1]--;
						avail[site2]--;
						//std::cout<<site1<<' '<<site2<<' '<<N<<'\n';
						tot_avail-=2;
						break;
					}
				} 
			}
		}while(!is_connected(N,edges)); // do until the network is connected

		int pos1,pos2,sf1,sf2,sf3,sf4; // shuffle edges
		std::pair<int,int> edge1,edge2;
		for(int i=0;i<2000;i++){
			//std::cout<< i<<'\n';
			do{
				pos1=N*d/2*distribution(R);
				pos2=(N*d/2-1)*distribution(R);
				if(pos2==pos1) pos2++;

				auto it=edges.begin();
				std::advance(it,pos1);
				edge1=*it;
				it=edges.begin();
				std::advance(it,pos2);
				edge2=*it;

				sf1=edge1.first;
				sf2=edge1.second;
				if(distribution(R)<0.5){
					sf3=edge2.first;
					sf4=edge2.second;
				}else{
					sf3=edge2.second;
					sf4=edge2.first;
				}
				// delete sf1-sf2 and sf3-sf4, add sf1-sf3 and sf2-sf4
			}while(sf1==sf3 || sf2==sf4 || edges.count({sf1,sf3})>0 || edges.count({sf3,sf1})>0
			 || edges.count({sf2,sf4})>0 || edges.count({sf4,sf2})>0);
			edges.erase(edge1);
			edges.erase(edge2);
			edges.insert({sf1,sf3});
			edges.insert({sf2,sf4});
			if(!is_connected(N,edges)){
				edges.insert(edge1);
				edges.insert(edge2);
				edges.erase({sf1,sf3});
				edges.erase({sf2,sf4});	
			}
		}

		return edges;
	}

	template<typename NetType>
	void generate_random_regular_network(NetType & N, const int & d, std::default_random_engine & R){

		auto edges = random_edges(N.size(),d,R);
		std::vector<typename NetType::NodeKeyType> keys;
		for(auto & s:N)
			keys.push_back(s.first);
		for(auto & e:edges){
			//std::cout<<e.first<<' '<<e.second<<'\n';
			N.set_edge(keys[e.first],keys[e.second]);
		}
	}

}
#endif