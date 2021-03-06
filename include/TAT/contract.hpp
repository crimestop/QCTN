/**
 * \file contract.hpp
 *
 * Copyright (C) 2019-2021 Hao Zhang<zh970205@mail.ustc.edu.cn>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef TAT_CONTRACT_HPP
#define TAT_CONTRACT_HPP

#include "pmr_resource.hpp"
#include "tensor.hpp"
#include "timer.hpp"

#ifndef TAT_DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
void sgemm_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const float* alpha,
      const float* a,
      const int* lda,
      const float* b,
      const int* ldb,
      const float* beta,
      float* c,
      const int* ldc);
void dgemm_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const double* alpha,
      const double* a,
      const int* lda,
      const double* b,
      const int* ldb,
      const double* beta,
      double* c,
      const int* ldc);
void cgemm_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const std::complex<float>* alpha,
      const std::complex<float>* a,
      const int* lda,
      const std::complex<float>* b,
      const int* ldb,
      const std::complex<float>* beta,
      std::complex<float>* c,
      const int* ldc);
void zgemm_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const std::complex<double>* alpha,
      const std::complex<double>* a,
      const int* lda,
      const std::complex<double>* b,
      const int* ldb,
      const std::complex<double>* beta,
      std::complex<double>* c,
      const int* ldc);

#ifdef TAT_USE_MKL_GEMM_BATCH
void sgemm_batch_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const float* alpha,
      const float** a,
      const int* lda,
      const float** b,
      const int* ldb,
      const float* beta,
      float** c,
      const int* ldc,
      const int* group_count,
      const int* group_size);
void dgemm_batch_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const double* alpha,
      const double** a,
      const int* lda,
      const double** b,
      const int* ldb,
      const double* beta,
      double** c,
      const int* ldc,
      const int* group_count,
      const int* group_size);
void cgemm_batch_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const std::complex<float>* alpha,
      const std::complex<float>** a,
      const int* lda,
      const std::complex<float>** b,
      const int* ldb,
      const std::complex<float>* beta,
      std::complex<float>** c,
      const int* ldc,
      const int* group_count,
      const int* group_size);
void zgemm_batch_(
      const char* transpose_a,
      const char* transpose_b,
      const int* m,
      const int* n,
      const int* k,
      const std::complex<double>* alpha,
      const std::complex<double>** a,
      const int* lda,
      const std::complex<double>** b,
      const int* ldb,
      const std::complex<double>* beta,
      std::complex<double>** c,
      const int* ldc,
      const int* group_count,
      const int* group_size);
#endif
}
#endif

namespace TAT {
#ifndef TAT_DOXYGEN_SHOULD_SKIP_THIS
   template<typename ScalarType>
   constexpr void (*gemm)(
         const char* transpose_a,
         const char* transpose_b,
         const int* m,
         const int* n,
         const int* k,
         const ScalarType* alpha,
         const ScalarType* a,
         const int* lda,
         const ScalarType* b,
         const int* ldb,
         const ScalarType* beta,
         ScalarType* c,
         const int* ldc) = nullptr;

   template<>
   inline auto gemm<float> = sgemm_;
   template<>
   inline auto gemm<double> = dgemm_;
   template<>
   inline auto gemm<std::complex<float>> = cgemm_;
   template<>
   inline auto gemm<std::complex<double>> = zgemm_;

   template<typename ScalarType>
   constexpr void (*mkl_gemm_batch)(
         const char* transpose_a,
         const char* transpose_b,
         const int* m,
         const int* n,
         const int* k,
         const ScalarType* alpha,
         const ScalarType** a,
         const int* lda,
         const ScalarType** b,
         const int* ldb,
         const ScalarType* beta,
         ScalarType** c,
         const int* ldc,
         const int* group_count,
         const int* group_size) = nullptr;

#ifdef TAT_USE_MKL_GEMM_BATCH
   template<>
   inline auto mkl_gemm_batch<float> = sgemm_batch_;
   template<>
   inline auto mkl_gemm_batch<double> = dgemm_batch_;
   template<>
   inline auto mkl_gemm_batch<std::complex<float>> = cgemm_batch_;
   template<>
   inline auto mkl_gemm_batch<std::complex<double>> = zgemm_batch_;
#endif

   template<typename ScalarType, bool same_shape>
   void gemm_batch(
         const char* transpose_a,
         const char* transpose_b,
         const int* m,
         const int* n,
         const int* k,
         const ScalarType* alpha,
         const ScalarType** a,
         const int* lda,
         const ScalarType** b,
         const int* ldb,
         const ScalarType* beta,
         ScalarType** c,
         const int* ldc,
         const int& batch_size) {
      auto kernel_guard = contract_kernel_guard();
      if (batch_size == 1) {
         gemm<ScalarType>(transpose_a, transpose_b, m, n, k, alpha, a[0], lda, b[0], ldb, beta, c[0], ldc);
      } else {
#ifdef TAT_USE_MKL_GEMM_BATCH
         if constexpr (same_shape) {
            int group_count = 1;
            mkl_gemm_batch<ScalarType>(transpose_a, transpose_b, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc, &group_count, &batch_size);
         } else {
            pmr::vector<int> group_size(batch_size, 1);
            mkl_gemm_batch<ScalarType>(transpose_a, transpose_b, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc, &batch_size, group_size.data());
         }
#else
         if constexpr (same_shape) {
            for (auto i = 0; i < batch_size; i++) {
               gemm<ScalarType>(transpose_a, transpose_b, m, n, k, alpha, a[i], lda, b[i], ldb, beta, c[i], ldc);
            }
         } else {
            for (auto i = 0; i < batch_size; i++) {
               gemm<ScalarType>(
                     &transpose_a[i], &transpose_b[i], &m[i], &n[i], &k[i], &alpha[i], a[i], &lda[i], b[i], &ldb[i], &beta[i], c[i], &ldc[i]);
            }
         }
#endif
      }
   }

   template<int i, typename Name, typename SetNameAndName>
   auto find_in_contract_names(const SetNameAndName& contract_names, const Name& name) {
      auto iterator = contract_names.begin();
      for (; iterator != contract_names.end(); ++iterator) {
         if (std::get<i>(*iterator) == name) {
            return iterator;
         }
      }
      return iterator;
   }
#endif

   /// \private
   template<typename ScalarType, typename Name, typename SetNameAndName>
   Tensor<ScalarType, NoSymmetry, Name> contract_with_fuse(
         const Tensor<ScalarType, NoSymmetry, Name>& tensor_1,
         const Tensor<ScalarType, NoSymmetry, Name>& tensor_2,
         SetNameAndName contract_names);

   /// \private
   template<typename ScalarType, typename Symmetry, typename Name, typename SetNameAndName>
   Tensor<ScalarType, Symmetry, Name> contract_without_fuse(
         const Tensor<ScalarType, Symmetry, Name>& tensor_1,
         const Tensor<ScalarType, Symmetry, Name>& tensor_2,
         SetNameAndName contract_names);

   template<typename ScalarType, typename Symmetry, typename Name>
   template<typename SetNameAndName>
   Tensor<ScalarType, Symmetry, Name> Tensor<ScalarType, Symmetry, Name>::contract(
         const Tensor<ScalarType, Symmetry, Name>& tensor_1,
         const Tensor<ScalarType, Symmetry, Name>& tensor_2,
         SetNameAndName&& contract_names) {
      auto timer_guard = contract_guard();
      auto pmr_guard = scope_resource<>();
      if constexpr (std::is_same_v<Symmetry, NoSymmetry>) {
         return contract_with_fuse(tensor_1, tensor_2, std::forward<SetNameAndName>(contract_names));
      } else {
         return contract_without_fuse(tensor_1, tensor_2, std::forward<SetNameAndName>(contract_names));
      }
   }

   template<typename ScalarType, typename Symmetry, typename Name, typename SetNameAndName>
   Tensor<ScalarType, Symmetry, Name> contract_without_fuse(
         const Tensor<ScalarType, Symmetry, Name>& tensor_1,
         const Tensor<ScalarType, Symmetry, Name>& tensor_2,
         SetNameAndName contract_names) {
      constexpr bool is_fermi = is_fermi_symmetry_v<Symmetry>;
      constexpr bool is_no_symmetry = std::is_same_v<Symmetry, NoSymmetry>;
      // ?????????split?????????
      const Rank rank_1 = tensor_1.names.size();
      const Rank rank_2 = tensor_2.names.size();
      // ????????????????????????, ??????name tuple list????????????names???
      for (auto iterator = contract_names.begin(); iterator != contract_names.end();) {
         auto found_1 = tensor_1.name_to_index.find(std::get<0>(*iterator));
         auto found_2 = tensor_2.name_to_index.find(std::get<1>(*iterator));
         if (found_1 == tensor_1.name_to_index.end() || found_2 == tensor_2.name_to_index.end()) {
            iterator = contract_names.erase(iterator);
            TAT_warning_or_error_when_name_missing("Name missing in contract");
         } else {
            ++iterator;
         }
      }
      const auto common_rank = contract_names.size();
      pmr::set<Name> contract_names_1;
      pmr::set<Name> contract_names_2;
      for (const auto& [name_1, name_2] : contract_names) {
         contract_names_1.insert(name_1);
         contract_names_2.insert(name_2);
      }
      if (contract_names_1.size() != common_rank || contract_names_2.size() != common_rank) {
         TAT_error("Duplicated Contract Name");
      }
      // Rank contract_origin_rank = contract_names_1.size();
      // for (Rank i = 0; i < contract_origin_rank; i++) {
      // }
      // ??????????????? - + - -
      // ????????????????????????
      auto reversed_set_1 = pmr::set<Name>();           // ???????????????merge????????????
      auto reversed_set_2 = pmr::set<Name>();           // ???????????????merge????????????
      auto edge_result = std::vector<Edge<Symmetry>>(); // ??????????????????????????????split???????????????????????????edge
      if constexpr (is_no_symmetry) {
         edge_result.reserve(rank_1 + rank_2 - 2 * common_rank);
      }
      auto split_map_result = pmr::map<Name, pmr::vector<std::tuple<Name, BoseEdge<Symmetry>>>>(); // split??????
      auto reversed_set_result = pmr::set<Name>();                                                 // ??????split???????????????
      auto name_result = std::vector<Name>();                                                      // ??????split??????name
      name_result.reserve(rank_1 + rank_2 - 2 * common_rank);
      split_map_result[InternalName<Name>::Contract_1].reserve(rank_1 - common_rank);
      split_map_result[InternalName<Name>::Contract_2].reserve(rank_2 - common_rank);
      auto free_name_1 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      free_name_1.reserve(rank_1 - common_rank);
      for (Rank i = 0; i < rank_1; i++) {
         const auto& n = tensor_1.names[i];
         if (find_in_contract_names<0>(contract_names, n) == contract_names.end()) {
            // ???free name
            free_name_1.push_back(n);
            if constexpr (is_no_symmetry) {
               edge_result.push_back(tensor_1.core->edges[i]);
            } else {
               split_map_result.at(InternalName<Name>::Contract_1).push_back({n, {tensor_1.core->edges[i].map}});
            }
            name_result.push_back(n);
            if constexpr (is_fermi) {
               if (tensor_1.core->edges[i].arrow) {
                  reversed_set_1.insert(n);
                  reversed_set_result.insert(n);
               }
            }
         } else {
            // ??????contract???
            if constexpr (is_fermi) {
               if (!tensor_1.core->edges[i].arrow) {
                  reversed_set_1.insert(n);
               }
            }
         }
      }
      const auto free_rank_1 = free_name_1.size();
      auto free_name_2 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      free_name_2.reserve(rank_2 - common_rank);
      for (Rank i = 0; i < rank_2; i++) {
         const auto& n = tensor_2.names[i];
         if (find_in_contract_names<1>(contract_names, n) == contract_names.end()) {
            // ???free name
            free_name_2.push_back(n);
            if constexpr (is_no_symmetry) {
               edge_result.push_back(tensor_2.core->edges[i]);
            } else {
               split_map_result.at(InternalName<Name>::Contract_2).push_back({n, {tensor_2.core->edges[i].map}});
            }
            name_result.push_back(n);
            if constexpr (is_fermi) {
               if (tensor_2.core->edges[i].arrow) {
                  reversed_set_2.insert(n);
                  reversed_set_result.insert(n);
               }
            }
         } else {
            // ??????contract???
            if constexpr (is_fermi) {
               if (tensor_2.core->edges[i].arrow) {
                  reversed_set_2.insert(n);
               }
            }
         }
      }
      const auto free_rank_2 = free_name_2.size();
      // ??????????????????
      auto common_name_1 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      auto common_name_2 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      common_name_1.reserve(common_rank);
      common_name_2.reserve(common_rank);
      bool put_common_1_right;
      bool put_common_2_right;
      auto fit_tensor_1_common_edge = [&]() {
         for (const auto& n : tensor_1.names) {
            if (auto position = find_in_contract_names<0>(contract_names, n); position != contract_names.end()) {
               common_name_1.push_back(std::get<0>(*position));
               common_name_2.push_back(std::get<1>(*position));
            }
         }
      };
      auto fit_tensor_2_common_edge = [&]() {
         for (const auto& n : tensor_2.names) {
            if (auto position = find_in_contract_names<1>(contract_names, n); position != contract_names.end()) {
               common_name_1.push_back(std::get<0>(*position));
               common_name_2.push_back(std::get<1>(*position));
            }
         }
      };
      // ????????????
      if (free_rank_1 == 0) {
         put_common_1_right = true;
         fit_tensor_2_common_edge();
         put_common_2_right = common_name_2.empty() || common_name_2.back() == tensor_2.names.back();
      } else if (free_rank_2 == 0) {
         put_common_2_right = true;
         fit_tensor_1_common_edge();
         put_common_1_right = common_name_1.empty() || common_name_1.back() == tensor_1.names.back();
      } else if (free_name_1.back() != tensor_1.names.back()) {
         put_common_1_right = true;
         fit_tensor_1_common_edge();
         put_common_2_right = common_name_2.empty() || common_name_2.back() == tensor_2.names.back();
      } else if (free_name_2.back() != tensor_2.names.back()) {
         put_common_2_right = true;
         fit_tensor_2_common_edge();
         put_common_1_right = common_name_1.empty() || common_name_1.back() == tensor_1.names.back();
      } else {
         put_common_1_right = false;
         put_common_2_right = false;
         fit_tensor_2_common_edge();
         // ?????????????????????????????????
      }
      // ????????????????????????
      auto delete_1 = pmr::map<Name, pmr::map<Symmetry, Size>>();
      auto delete_2 = pmr::map<Name, pmr::map<Symmetry, Size>>();
      if constexpr (!is_no_symmetry) {
         for (Rank i = 0; i < common_rank; i++) {
            auto name_1 = common_name_1[i];
            auto name_2 = common_name_2[i];
            auto edge_1 = tensor_1.core->edges[tensor_1.name_to_index.at(name_1)];
            auto edge_2 = tensor_2.core->edges[tensor_2.name_to_index.at(name_2)];
            auto delete_unused_dimension = [](const auto& edge_this, const auto& edge_other, const auto& name_this, auto& delete_this) {
#ifndef _MSVC_LANG
               // 2020.10.27 ???????????????msvc??????constexpr????????????, ???????????????msvc???????????????, ???????????????????????????
               if constexpr (is_fermi) {
                  if (edge_this.arrow == edge_other.arrow) {
                     TAT_error("Different Fermi Arrow to Contract");
                  }
               }
#endif
               auto delete_map = pmr::map<Symmetry, Size>();
               for (const auto& [symmetry, dimension] : edge_this.map) {
                  auto found = edge_other.map.find(-symmetry);
                  if (found != edge_other.map.end()) {
                     // found
                     if (const auto dimension_other = found->second; dimension_other != dimension) {
                        TAT_error("Different Dimension to Contract");
                     }
                  } else {
                     // not found
                     delete_map[symmetry] = 0;
                     // ??????merge???cut, cut???0???????????????
                  }
               }
               if (!delete_map.empty()) {
                  delete_this[name_this] = std::move(delete_map);
               }
            };
            delete_unused_dimension(edge_1, edge_2, name_1, delete_1);
            delete_unused_dimension(edge_2, edge_1, name_2, delete_2);
         }
      }
      // merge
      // ????????????????????????????????????reverse???merge?????????
      auto common_name_1_set = pmr::set<Name>(common_name_1.begin(), common_name_1.end());
      auto tensor_1_merged = tensor_1.edge_operator(
            {},
            {},
            reversed_set_1,
            pmr::map<Name, pmr::vector<Name>>{
                  {InternalName<Name>::Contract_1, std::move(free_name_1)}, {InternalName<Name>::Contract_2, std::move(common_name_1)}},
            put_common_1_right ? pmr::vector<Name>{InternalName<Name>::Contract_1, InternalName<Name>::Contract_2} :
                                 pmr::vector<Name>{InternalName<Name>::Contract_2, InternalName<Name>::Contract_1},
            false,
            std::array<pmr::set<Name>, 4>{{{}, std::move(common_name_1_set), {}, {InternalName<Name>::Contract_2}}},
            delete_1);
      auto tensor_2_merged = tensor_2.edge_operator(
            {},
            {},
            reversed_set_2,
            pmr::map<Name, pmr::vector<Name>>{
                  {InternalName<Name>::Contract_2, std::move(free_name_2)}, {InternalName<Name>::Contract_1, std::move(common_name_2)}},
            put_common_2_right ? pmr::vector<Name>{InternalName<Name>::Contract_2, InternalName<Name>::Contract_1} :
                                 pmr::vector<Name>{InternalName<Name>::Contract_1, InternalName<Name>::Contract_2},
            false,
            std::array<pmr::set<Name>, 4>{{{}, {}, {}, {}}},
            delete_2);
      // calculate_product
      auto product_result = Tensor<ScalarType, Symmetry, Name>(
            {InternalName<Name>::Contract_1, InternalName<Name>::Contract_2},
            {std::move(tensor_1_merged.core->edges[!put_common_1_right]), std::move(tensor_2_merged.core->edges[!put_common_2_right])});
      // ?????????T1???T2???edge????????????????????????merge???????????????????????????
      auto common_edge = std::move(tensor_1_merged.core->edges[put_common_1_right]);

      auto max_batch_size = product_result.core->blocks.size();
      vector<char> transpose_a_list(max_batch_size), transpose_b_list(max_batch_size);
      vector<int> m_list(max_batch_size), n_list(max_batch_size), k_list(max_batch_size), lda_list(max_batch_size), ldb_list(max_batch_size),
            ldc_list(max_batch_size);
      vector<ScalarType> alpha_list(max_batch_size), beta_list(max_batch_size);
      vector<const ScalarType*> a_list(max_batch_size), b_list(max_batch_size);
      vector<ScalarType*> c_list(max_batch_size);
      int batch_size = 0;

      for (auto& [symmetries, data] : product_result.core->blocks) {
         // m k n
         auto symmetries_1 = put_common_1_right ? symmetries : decltype(symmetries){symmetries[1], symmetries[0]};
         auto symmetries_2 = put_common_2_right ? decltype(symmetries){symmetries[1], symmetries[0]} : symmetries;
         const auto& data_1 = tensor_1_merged.core->blocks.at(symmetries_1);
         const auto& data_2 = tensor_2_merged.core->blocks.at(symmetries_2);
         const int m = product_result.core->edges[0].map.at(symmetries[0]);
         const int n = product_result.core->edges[1].map.at(symmetries[1]);
         const int k = common_edge.map.at(symmetries[1]);
         ScalarType alpha = 1;
         if constexpr (is_fermi) {
            // ??????????????????- + - -???????????????
            if ((put_common_2_right ^ !put_common_1_right) && bool(symmetries[0].fermi % 2)) {
               alpha = -1;
            }
         }
         const ScalarType beta = 0;
         if (m && n && k) {
            transpose_a_list[batch_size] = put_common_2_right ? 'T' : 'N';
            transpose_b_list[batch_size] = put_common_1_right ? 'N' : 'T';
            m_list[batch_size] = n;
            n_list[batch_size] = m;
            k_list[batch_size] = k;
            alpha_list[batch_size] = alpha;
            a_list[batch_size] = data_2.data();
            lda_list[batch_size] = put_common_2_right ? k : n;
            b_list[batch_size] = data_1.data();
            ldb_list[batch_size] = put_common_1_right ? k : m;
            beta_list[batch_size] = beta;
            c_list[batch_size] = data.data();
            ldc_list[batch_size] = n;
            batch_size++;
         } else if (m && n) {
            std::fill(data.begin(), data.end(), 0);
         }
      }
      gemm_batch<ScalarType, false>(
            transpose_a_list.data(),
            transpose_b_list.data(),
            m_list.data(),
            n_list.data(),
            k_list.data(),
            alpha_list.data(),
            a_list.data(),
            lda_list.data(),
            b_list.data(),
            ldb_list.data(),
            beta_list.data(),
            c_list.data(),
            ldc_list.data(),
            batch_size);

      if constexpr (is_no_symmetry) {
         product_result.name_to_index = construct_name_to_index<decltype(product_result.name_to_index)>(name_result);
         product_result.names = std::move(name_result);
         product_result.core->edges = std::move(edge_result);
         return product_result;
      } else {
         auto result = product_result.edge_operator({}, split_map_result, reversed_set_result, {}, std::move(name_result));
         return result;
      }
   }

   template<typename ScalarType, typename Name, typename SetNameAndName>
   Tensor<ScalarType, NoSymmetry, Name> contract_with_fuse(
         const Tensor<ScalarType, NoSymmetry, Name>& tensor_1,
         const Tensor<ScalarType, NoSymmetry, Name>& tensor_2,
         SetNameAndName contract_names) {
      const Rank rank_1 = tensor_1.names.size();
      const Rank rank_2 = tensor_2.names.size();
      // ????????????????????????, ??????name tuple list????????????names???
      for (auto iterator = contract_names.begin(); iterator != contract_names.end();) {
         auto found_1 = tensor_1.name_to_index.find(std::get<0>(*iterator));
         auto found_2 = tensor_2.name_to_index.find(std::get<1>(*iterator));
         if (found_1 == tensor_1.name_to_index.end() || found_2 == tensor_2.name_to_index.end()) {
            iterator = contract_names.erase(iterator);
            TAT_warning_or_error_when_name_missing("Name missing in contract");
         } else {
            ++iterator;
         }
      }
      const auto common_rank = contract_names.size();
      pmr::set<Name> contract_names_1;
      pmr::set<Name> contract_names_2;
      for (const auto& [name_1, name_2] : contract_names) {
         contract_names_1.insert(name_1);
         contract_names_2.insert(name_2);
      }
      if (contract_names_1.size() != common_rank || contract_names_2.size() != common_rank) {
         TAT_error("Duplicated Contract Name");
      }
      // ??????fuse name?????????????????????
      pmr::set<Name> fuse_names;
      for (const auto& name : tensor_1.names) {
         const auto in_tensor_2 = tensor_2.name_to_index.find(name) != tensor_2.name_to_index.end();
         const auto in_contract_1 = contract_names_1.find(name) != contract_names_1.end();
         const auto in_contract_2 = contract_names_2.find(name) != contract_names_2.end();
         if (in_tensor_2 && !in_contract_1 && !in_contract_2) {
            fuse_names.insert(name);
         }
      }
      const auto fuse_rank = fuse_names.size();
      // ????????????
      auto edge_result = std::vector<Edge<NoSymmetry>>(); // ??????????????????????????????split???????????????????????????edge
      edge_result.reserve(rank_1 + rank_2 - 2 * common_rank - fuse_rank);
      auto name_result = std::vector<Name>(); // ??????split??????name
      name_result.reserve(rank_1 + rank_2 - 2 * common_rank - fuse_rank);

      // ????????????fuse name?????????????????????, ????????????????????????????????????????????????????????????????????????????????????
      pmr::vector<Name> fuse_names_list;
      fuse_names_list.reserve(fuse_rank);
      for (const auto& name : fuse_names) {
         name_result.push_back(name);
         fuse_names_list.push_back(name);
         const auto& edge_1 = tensor_1.core->edges[tensor_1.name_to_index.at(name)];
         const auto& edge_2 = tensor_2.core->edges[tensor_2.name_to_index.at(name)];
         if (!(edge_1 == edge_2)) {
            TAT_error("Cannot fuse two edge with different shape");
         }
         edge_result.push_back(edge_1);
      }

      auto free_name_1 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      free_name_1.reserve(rank_1 - common_rank - fuse_rank);
      for (Rank i = 0; i < rank_1; i++) {
         const auto& n = tensor_1.names[i];
         if (find_in_contract_names<0>(contract_names, n) == contract_names.end()) {
            // ???free name???fuse name
            if (fuse_names.find(n) == fuse_names.end()) {
               // ??????fuse name, ???free name
               free_name_1.push_back(n);
               edge_result.push_back(tensor_1.core->edges[i]);
               name_result.push_back(n);
            }
         }
      }
      const auto free_rank_1 = free_name_1.size();
      auto free_name_2 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      free_name_2.reserve(rank_2 - common_rank);
      for (Rank i = 0; i < rank_2; i++) {
         const auto& n = tensor_2.names[i];
         if (find_in_contract_names<1>(contract_names, n) == contract_names.end()) {
            // ???free name???fuse name
            if (fuse_names.find(n) == fuse_names.end()) {
               // ??????fuse name, ???free name
               free_name_2.push_back(n);
               edge_result.push_back(tensor_2.core->edges[i]);
               name_result.push_back(n);
            }
         }
      }
      const auto free_rank_2 = free_name_2.size();
      // ??????????????????
      auto common_name_1 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      auto common_name_2 = pmr::vector<Name>(); // ???????????????????????????, merge?????????
      common_name_1.reserve(common_rank);
      common_name_2.reserve(common_rank);
      bool put_common_1_right;
      bool put_common_2_right;
      auto fit_tensor_1_common_edge = [&]() {
         for (const auto& n : tensor_1.names) {
            if (auto position = find_in_contract_names<0>(contract_names, n); position != contract_names.end()) {
               // ???contract name
               common_name_1.push_back(std::get<0>(*position));
               common_name_2.push_back(std::get<1>(*position));
            }
         }
      };
      auto fit_tensor_2_common_edge = [&]() {
         for (const auto& n : tensor_2.names) {
            if (auto position = find_in_contract_names<1>(contract_names, n); position != contract_names.end()) {
               // ???contract name
               common_name_1.push_back(std::get<0>(*position));
               common_name_2.push_back(std::get<1>(*position));
            }
         }
      };
      // ????????????
      if (free_rank_1 == 0) {
         put_common_1_right = true;
         fit_tensor_2_common_edge();
         put_common_2_right = common_name_2.empty() || common_name_2.back() == tensor_2.names.back();
      } else if (free_rank_2 == 0) {
         put_common_2_right = true;
         fit_tensor_1_common_edge();
         put_common_1_right = common_name_1.empty() || common_name_1.back() == tensor_1.names.back();
      } else if (free_name_1.back() != tensor_1.names.back()) {
         put_common_1_right = true;
         fit_tensor_1_common_edge();
         put_common_2_right = common_name_2.empty() || common_name_2.back() == tensor_2.names.back();
      } else if (free_name_2.back() != tensor_2.names.back()) {
         put_common_2_right = true;
         fit_tensor_2_common_edge();
         put_common_1_right = common_name_1.empty() || common_name_1.back() == tensor_1.names.back();
      } else {
         put_common_1_right = false;
         put_common_2_right = false;
         fit_tensor_2_common_edge();
         // ?????????????????????????????????
      }

      // merge
      // ????????????????????????????????????reverse???merge?????????
      auto tensor_1_merged = tensor_1.edge_operator(
            {},
            {},
            {},
            pmr::map<Name, pmr::vector<Name>>{
                  {InternalName<Name>::Contract_1, std::move(free_name_1)},
                  {InternalName<Name>::Contract_2, std::move(common_name_1)},
                  {InternalName<Name>::Contract_0, fuse_names_list}},
            put_common_1_right ? pmr::vector<Name>{InternalName<Name>::Contract_0, InternalName<Name>::Contract_1, InternalName<Name>::Contract_2} :
                                 pmr::vector<Name>{InternalName<Name>::Contract_0, InternalName<Name>::Contract_2, InternalName<Name>::Contract_1});
      auto tensor_2_merged = tensor_2.edge_operator(
            {},
            {},
            {},
            pmr::map<Name, pmr::vector<Name>>{
                  {InternalName<Name>::Contract_2, std::move(free_name_2)},
                  {InternalName<Name>::Contract_1, std::move(common_name_2)},
                  {InternalName<Name>::Contract_0, std::move(fuse_names_list)}},
            put_common_2_right ? pmr::vector<Name>{InternalName<Name>::Contract_0, InternalName<Name>::Contract_2, InternalName<Name>::Contract_1} :
                                 pmr::vector<Name>{InternalName<Name>::Contract_0, InternalName<Name>::Contract_1, InternalName<Name>::Contract_2});
      // calculate_product
      auto product_result = Tensor<ScalarType, NoSymmetry, Name>(
            {InternalName<Name>::Contract_0, InternalName<Name>::Contract_1, InternalName<Name>::Contract_2},
            {std::move(tensor_1_merged.core->edges[0]),
             std::move(tensor_1_merged.core->edges[1 + !put_common_1_right]),
             std::move(tensor_2_merged.core->edges[1 + !put_common_2_right])});

      auto common_edge = std::move(tensor_1_merged.core->edges[1 + put_common_1_right]);

      const int l = product_result.core->edges[0].map.begin()->second;
      const int m = product_result.core->edges[1].map.begin()->second;
      const int n = product_result.core->edges[2].map.begin()->second;
      const int k = common_edge.map.begin()->second;
      const ScalarType alpha = 1;
      const ScalarType beta = 0;

      ScalarType* data = product_result.core->blocks.begin()->second.data();
      const ScalarType* data_1 = tensor_1_merged.core->blocks.begin()->second.data();
      const ScalarType* data_2 = tensor_2_merged.core->blocks.begin()->second.data();
      if (m && n && k) {
         pmr::vector<const ScalarType*> a_list(l), b_list(l);
         pmr::vector<ScalarType*> c_list(l);
         for (auto i = 0; i < l; i++) {
            a_list[i] = data_2 + k * n * i;
            b_list[i] = data_1 + m * k * i;
            c_list[i] = data + m * n * i;
         }
         gemm_batch<ScalarType, true>(
               put_common_2_right ? "T" : "N",
               put_common_1_right ? "N" : "T",
               &n,
               &m,
               &k,
               &alpha,
               a_list.data(),
               put_common_2_right ? &k : &n,
               b_list.data(),
               put_common_1_right ? &k : &m,
               &beta,
               c_list.data(),
               &n,
               l);
      } else if (m && n) {
         auto& result_vector = product_result.core->blocks.begin()->second;
         std::fill(result_vector.begin(), result_vector.end(), 0);
      }

      product_result.name_to_index = construct_name_to_index<decltype(product_result.name_to_index)>(name_result);
      product_result.names = std::move(name_result);
      product_result.core->edges = std::move(edge_result);
      return product_result;
   }
} // namespace TAT
#endif
