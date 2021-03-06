/**
 * \file edge_miscellaneous.hpp
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
#ifndef TAT_EDGE_MISCELLANEOUS_HPP
#define TAT_EDGE_MISCELLANEOUS_HPP

#include "tensor.hpp"

namespace TAT {
   template<typename ScalarType, typename Symmetry, typename Name>
   template<typename MapNameName>
   auto Tensor<ScalarType, Symmetry, Name>::edge_rename(const MapNameName& dictionary) const {
      // too easy so not use edge_operator
      using ResultName = typename MapNameName::mapped_type;
      auto result = Tensor<ScalarType, Symmetry, ResultName>{};
      result.core = core;
      result.names.reserve(names.size());
      std::transform(names.begin(), names.end(), std::back_inserter(result.names), [&dictionary](const Name& name) {
         if (auto position = dictionary.find(name); position == dictionary.end()) {
            if constexpr (std::is_same_v<ResultName, Name>) {
               return name;
            } else {
               TAT_error("New names not found in edge_rename which change type of name");
            }
         } else {
            return position->second;
         }
      });
      result.name_to_index = construct_name_to_index<decltype(result.name_to_index)>(result.names);
      return result;
   }

   template<typename ScalarType, typename Symmetry, typename Name>
   template<typename VectorName>
   Tensor<ScalarType, Symmetry, Name> Tensor<ScalarType, Symmetry, Name>::transpose(const VectorName& target_names) const {
      return edge_operator({}, {}, {}, {}, target_names);
   }

   template<typename ScalarType, typename Symmetry, typename Name>
   template<typename SetName1, typename SetName2>
   Tensor<ScalarType, Symmetry, Name>
   Tensor<ScalarType, Symmetry, Name>::reverse_edge(const SetName1& reversed_name, const bool apply_parity, SetName2&& parity_exclude_name) const {
      return edge_operator(
            {}, {}, reversed_name, {}, names, apply_parity, std::array<SetName2, 4>{{{}, std::forward<SetName2>(parity_exclude_name), {}, {}}});
   }

   template<typename ScalarType, typename Symmetry, typename Name>
   template<typename MapNameVectorName, typename SetName>
   Tensor<ScalarType, Symmetry, Name> Tensor<ScalarType, Symmetry, Name>::merge_edge(
         MapNameVectorName merge,
         const bool apply_parity,
         SetName&& parity_exclude_name_merge,
         SetName&& parity_exclude_name_reverse) const {
      auto pmr_guard = scope_resource<1 << 10>();
      // delete edge from names_before_merge if not exist
      for (auto& [name_after_merge, names_before_merge] : merge) {
         auto new_names_before_merge = decltype(names_before_merge)();
         new_names_before_merge.reserve(names_before_merge.size());
         for (const auto& i : names_before_merge) {
            if (auto found = name_to_index.find(i); found != name_to_index.end()) {
               new_names_before_merge.push_back(i);
            }
         }
         names_before_merge.swap(new_names_before_merge);
         // if len(names_before_merge) == 0, it create a trivial edge?
         // ???, ?????????target_name???, ?????????merge??????????????????????????????target_name
         // ??????edge_operator??????????????????????????????????????????merge
      }
      pmr::vector<Name> target_name;
      target_name.reserve(names.size());
      for (auto iterator = names.rbegin(); iterator != names.rend(); ++iterator) {
         // ??????????????? -> ????????????
         // ??????????????? -> ?????????
         // ????????? -> ??????
         auto found_in_merge = false;
         for (const auto& [name_after_merge, names_before_merge] : merge) {
            if (auto position_in_group = std::find(names_before_merge.begin(), names_before_merge.end(), *iterator);
                position_in_group != names_before_merge.end()) {
               if (*iterator == names_before_merge.back()) {
                  target_name.push_back(name_after_merge);
               }
               found_in_merge = true;
               break;
            }
         }
         if (!found_in_merge) {
            target_name.push_back(*iterator);
         }
      }
      // ??????target_name
      for (const auto& [name_after_merge, names_before_merge] : merge) {
         if (names_before_merge.empty()) {
            target_name.push_back(name_after_merge);
         }
      }
      std::reverse(target_name.begin(), target_name.end());
      return edge_operator(
            {},
            {},
            {},
            merge,
            target_name,
            apply_parity,
            std::array<SetName, 4>{{{}, {}, std::forward<SetName>(parity_exclude_name_reverse), std::forward<SetName>(parity_exclude_name_merge)}});
   }

   template<typename ScalarType, typename Symmetry, typename Name>
   template<typename MapNameVectorNameAndEdge, typename SetName>
   Tensor<ScalarType, Symmetry, Name>
   Tensor<ScalarType, Symmetry, Name>::split_edge(MapNameVectorNameAndEdge split, const bool apply_parity, SetName&& parity_exclude_name_split)
         const {
      auto pmr_guard = scope_resource<1 << 10>();
      // ?????????????????????
      // ??????edge_operator????????????, ?????????????????????, ??????names???????????????????????????
      for (auto iterator = split.begin(); iterator != split.end();) {
         if (auto found = name_to_index.find(iterator->first); found == name_to_index.end()) {
            iterator = split.erase(iterator);
         } else {
            ++iterator;
         }
      }
      // ??????target_name
      pmr::vector<Name> target_name;
      target_name.reserve(names.size()); // ??????, ??????????????????new?????????
      for (const auto& n : names) {
         if (auto found = split.find(n); found != split.end()) {
            for (const auto& edge_after_split : found->second) {
               target_name.push_back(std::get<0>(edge_after_split));
            }
         } else {
            target_name.push_back(n);
         }
      }
      return edge_operator(
            {}, split, {}, {}, target_name, apply_parity, std::array<SetName, 4>{{std::forward<SetName>(parity_exclude_name_split), {}, {}, {}}});
   }
} // namespace TAT
#endif
