#pragma once

#include "concepts_compat.h"
#include <type_traits>

namespace graaf {

/**
 * @brief Interface for a weighted edge.
 *
 * This is what is stored internally and returned from a weighted graph in
 * order to make sure each edge in a weighted graph has a common interface to
 * extract the weight.
 *
 * @tparam WEIGHT_T The type of the weight.
 */
template <typename WEIGHT_T = int>
class weighted_edge {
 public:
  using weight_t = WEIGHT_T;

  virtual ~weighted_edge() = default;

  [[nodiscard]] virtual auto get_weight() const noexcept -> WEIGHT_T = 0;
};

#if __cplusplus >= 202002L && defined(__cpp_concepts)
template<typename derived>
concept derived_from_weighted_edge = std::is_base_of_v<weighted_edge<typename derived::weight_t>, derived>;
#endif

// C++17 Replacement for the 'derived_from_weighted_edge' concept
// We use a helper trait to safely check for the nested weight_t
// Remove when no longer need support for Qt 5.15 and MinGW 8.1.
template<typename T, typename = void>
struct is_weighted_edge : std::false_type
{
};

template<typename T>
struct is_weighted_edge<T, std::void_t<typename T::weight_t>> : std::is_base_of<weighted_edge<typename T::weight_t>, T>
{
};

template<typename T>
inline constexpr bool is_weighted_edge_v = is_weighted_edge<T>::value;

/**
 * Overload set to get the weight from an edge
 */
template<typename WEIGHTED_EDGE_T>
#if __cplusplus >= 202002L && defined(__cpp_concepts)
GRAAF_REQUIRES(derived_from_weighted_edge<WEIGHTED_EDGE_T>)
#endif
[[nodiscard]] auto get_weight(const WEIGHTED_EDGE_T &edge)
    -> GRAAF_RETTYPE(typename WEIGHTED_EDGE_T::weight_t, is_weighted_edge_v<WEIGHTED_EDGE_T>);

template<typename EDGE_T>
GRAAF_REQUIRES(std::is_arithmetic_v<EDGE_T>)
[[nodiscard]] auto get_weight(const EDGE_T &edge) -> GRAAF_RETTYPE(EDGE_T, std::is_arithmetic_v<EDGE_T>);

template<typename EDGE_T>
[[nodiscard]] auto get_weight(const EDGE_T & /*edge*/)
    -> GRAAF_RETTYPE(int, !is_weighted_edge_v<EDGE_T> && !std::is_arithmetic_v<EDGE_T>);

}  // namespace graaf

#include "edge.tpp"
