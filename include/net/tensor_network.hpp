#ifndef NET_TENSOR_NETWORK_HPP
#define NET_TENSOR_NETWORK_HPP

#include "network.hpp"
#include "tensor_contract/tensor_contract.hpp"
#include "tensor_tools.hpp"
#include "traits.hpp"
#include <TAT/TAT.hpp>
#include <functional>
#include <random>
#include <type_traits>
#include <variant>

namespace net {

	namespace tensor {
		// template <typename T,typename EdgeKey=stdEdgeKey>
		// using Tensor=TAT::Tensor<T,TAT::NoSymmetry,EdgeKey>;

		template <typename T, typename SiteKey = std::string, typename EdgeKey = stdEdgeKey>
		using TensorNetworkEnv = network<
				Tensor<T, EdgeKey>,
				Tensor<T, EdgeKey>,
				SiteKey,
				EdgeKey,
				default_traits<Tensor<T, EdgeKey>, Tensor<T, EdgeKey>, SiteKey, EdgeKey>>;
		template <typename T, typename SiteKey = std::string, typename EdgeKey = stdEdgeKey>
		using TensorNetworkNoEnv =
				network<Tensor<T, EdgeKey>, std::monostate, SiteKey, EdgeKey, default_traits<Tensor<T, EdgeKey>, std::monostate, SiteKey, EdgeKey>>;

		template <typename EdgeKey = stdEdgeKey>
		Tensor<double, EdgeKey> init_node_rand(
				const std::vector<EdgeKey> & str_inds,
				const unsigned int D,
				const double min,
				const double max,
				std::default_random_engine & R) {
			auto distribution = std::uniform_real_distribution<double>(min, max);
			std::vector<unsigned int> dims(str_inds.size(), D);
			Tensor<double, EdgeKey> result(str_inds, {dims.begin(), dims.end()});
			return result.set([&distribution, &R]() { return distribution(R); });
		}

		template <typename NetType>
		typename NetType::NodeValType
		init_node_rand_phy(const typename NetType::IterNode & itr, const unsigned int D, const unsigned int dphy, std::default_random_engine & R) {
			auto distribution = std::uniform_real_distribution<double>(-1., 1.);
			std::vector<typename NetType::EdgeKeyType> inds;
			for (auto & b : itr->second.edges) {
				inds.push_back(b.first);
			}
			std::vector<unsigned int> dims(inds.size(), D);
			inds.push_back(itr->first + ".phy");
			dims.push_back(dphy);
			typename NetType::NodeValType result(inds, {dims.begin(), dims.end()});
			if constexpr (std::is_same_v<typename NetType::NodeValType::scalar_t, double>)
				result.set([&distribution, &R]() { return distribution(R); });
			else if constexpr (std::is_same_v<typename NetType::NodeValType::scalar_t, std::complex<double>>)
				result.set([&distribution, &R]() { return std::complex<double>(distribution(R), distribution(R)); });
			return result;
		}

		template <typename T, typename EdgeKey = stdEdgeKey>
		Tensor<T, EdgeKey> init_edge_one(const unsigned int D, const EdgeKey & edge1, const EdgeKey & edge2) {
			Tensor<T, EdgeKey> result({edge1, edge2}, {D, D});
			result.zero();
			for (int i = 0; i < D; ++i) {
				result.block()[i * (D + 1)] = 1.;
			}
			return result;
		}

		struct default_dec {
			template <typename TensorType, typename EdgeKey, typename EdgeKeySet, typename EdgeVal>
			void operator()(
					const TensorType & ten1,
					TensorType & ten2,
					TensorType & ten3,
					const EdgeKeySet & inds,
					const EdgeKey & ind1,
					const EdgeKey & ind2,
					EdgeVal & env) const {
				ten2 = ten1;
				ten3 = ten1;
			}
		};

		struct qr {
			template <typename TensorType, typename EdgeKey, typename EdgeKeySet, typename EdgeVal>
			void operator()(
					const TensorType & ten1,
					TensorType & ten2,
					TensorType & ten3,
					const EdgeKeySet & inds,
					const EdgeKey & ind1,
					const EdgeKey & ind2,
					EdgeVal & env) const {
				auto qr_res = ten1.qr('R', inds, ind1, ind2);
				ten2 = qr_res.Q;
				ten3 = qr_res.R;
			}
		};

		struct svd {
			int Dc = -1;
			svd() = default;
			svd(int d) : Dc(d){};
			template <typename TensorType, typename EdgeKey, typename EdgeKeySet, typename EdgeVal>
			void operator()(
					const TensorType & ten1,
					TensorType & ten2,
					TensorType & ten3,
					const EdgeKeySet & inds,
					const EdgeKey & ind1,
					const EdgeKey & ind2,
					EdgeVal & env) const {
				auto svd_res = ten1.svd(inds, ind2, ind1, Dc);
				ten3 = svd_res.U;
				ten2 = svd_res.V;
				env = svd_res.S;
			}
		};

		struct svd2 {
			int Dc = -1;
			svd2() = default;
			svd2(int d) : Dc(d){};
			template <typename TensorType, typename EdgeKey, typename EdgeKeySet, typename EdgeVal>
			void operator()(
					const TensorType & ten1,
					TensorType & ten2,
					TensorType & ten3,
					const EdgeKeySet & inds,
					const EdgeKey & ind1,
					const EdgeKey & ind2,
					EdgeVal & env) const {
				auto svd_res = ten1.svd(inds, ind2, ind1, Dc, ind2, ind1);
				ten3 = svd_res.U;
				ten2 = svd_res.V;
				// std::cout<<"<this test\n";
				// std::cout<<"ten1\n";
				// diminfo(ten1,std::cout);
				// std::cout<<"inds\n";
				// for(auto a: inds) std::cout<<a<<'\n';
				// std::cout<<"ind1\n";
				// std::cout<<ind1<<'\n';
				// std::cout<<"ind2\n";
				// std::cout<<ind2<<'\n';
				// std::cout<<"ten3\n";
				// diminfo(ten3,std::cout);
				// std::cout<<"ten2\n";
				// diminfo(ten2,std::cout);
				env = svd_res.S;
				env /= env.template norm<-1>();
				// std::cout<<'\n';
				// std::cout<<env<<"\n";
				// std::cout<<ten2<<"\n";
				// std::cout<<ten3<<"\n";
				ten2 = ten2.contract(env, {{ind1, ind2}});
				ten3 = ten3.contract(env, {{ind2, ind1}});

				// std::cout<<ten2<<"\n";
				// std::cout<<ten3<<"\n";
				int D = get_dim(env, 0);
				for (int i = 0; i < D * D; i += D + 1)
					env.block()[i] = 1. / env.block()[i];
				// std::cout<<env<<"\n";
				// std::cout<<"this test>\n";
			}
		};

		struct absorb {
			template <typename TensorType, typename EdgeKey>
			TensorType operator()(const TensorType & ten1, const TensorType & ten2, const EdgeKey & ind) const {
				return ten1.contract(ten2, {{ind, ten2.names[0]}}).edge_rename({{ten2.names[1], ind}});
			}
		};

		struct contract {
			template <typename TensorType, typename IndType>
			TensorType operator()(const TensorType & ten1, const TensorType & ten2, const IndType & inds) const {
				return ten1.contract(ten2, {inds.begin(), inds.end()});
			}
		};

		template <typename T, typename EdgeKey = stdEdgeKey>
		std::monostate zero_map(const Tensor<T, EdgeKey> & ten) {
			return std::monostate();
		}

		inline std::string conjugate_string(const std::string & s) {
			return "conjg_" + s;
		}
		inline std::function<std::string(const std::string &)> conjugate_string_fun = conjugate_string;

		inline std::monostate conjugate_mono(const std::monostate & m) {
			return m;
		}
		inline std::function<std::monostate(const std::monostate &)> conjugate_mono_fun = conjugate_mono;

		template <typename T>
		Tensor<T> conjugate_tensor(const Tensor<T> & t) {
			std::map<std::string, std::string> name_map;
			for (auto & m : t.names) {
				name_map[m] = conjugate_string(m);
			}
			return t.conjugate().edge_rename(name_map);
		}
		template <typename T>
		std::function<Tensor<T>(const Tensor<T> &)> conjugate_tensor_fun = conjugate_tensor<T>;

		template <typename T>
		TensorNetworkEnv<T> conjugate_tnenv(const TensorNetworkEnv<T> & t) {
			return t.template fmap<TensorNetworkEnv<T>>(conjugate_tensor_fun<T>, conjugate_tensor_fun<T>, conjugate_string_fun, conjugate_string_fun);
		}
		template <typename T>
		TensorNetworkNoEnv<T> conjugate_tnnoenv(const TensorNetworkNoEnv<T> & t) {
			return t.template fmap<TensorNetworkNoEnv<T>>(conjugate_tensor_fun<T>, conjugate_mono_fun, conjugate_string_fun, conjugate_string_fun);
		}
		template <typename T>
		TensorNetworkEnv<T> double_tnenv(const TensorNetworkEnv<T> & t) {
			TensorNetworkEnv<T> result = conjugate_tnenv(t);
			result.add(t);
			return result;
		}
		template <typename T>
		TensorNetworkNoEnv<T> double_tnnoenv(const TensorNetworkNoEnv<T> & t) {
			TensorNetworkNoEnv<T> result = conjugate_tnnoenv(t);
			result.add(t);
			return result;
		}

		template <typename Network>
		typename Network::NodeValType contract_tn(const Network & n,Engine & eg, const std::string & method) {
			typename Network::NodeValType result;
			auto ctree = get_contract_tree<contract_info2>(n, eg,method);
			result = n.template contract_tree(ctree, no_absorb(), contract());

			//std::cout<<result;
			return result;
		}

	} // namespace tensor
} // namespace net

#endif
