#pragma once

#include "../concepts_compat.h"
#include "../graph.h"

#include <filesystem>
#include <string>
#include <type_traits>

namespace graaf::io {

#if __cplusplus >= 202002L && defined(__cpp_concepts)
namespace detail
{

template<typename T>
concept string_convertible = requires(T element)
{
    std::to_string(element);
};

template<typename T>
requires string_convertible<T> const auto default_vertex_writer{
    [](vertex_id_t vertex_id, const T &vertex) -> std::string
    {
        // TODO(bluppes): replace with std::format once Clang supports it
        return "label=\"" + std::to_string(vertex_id) + ": " + std::to_string(vertex) + "\"";
    }};

const auto default_edge_writer{[](const edge_id_t & /*edge_id*/, const auto &edge) -> std::string
                               {
                                   // TODO(bluppes): replace with std::format once Clang supports it
                                   return "label=\"" + std::to_string(get_weight(edge)) + "\"";
                               }};
} // namespace detail

/**
 * @brief Serializes a graph to dot format and writes the result to a file.
 *
 * @tparam V The vertex type of the graph.
 * @tparam E The edge type of the graph.
 * @param graph The graph we want to serialize.
 * @param vertex_writer Function used for serializing the vertices. Should
 * accept a vertex_id_t and a type V and serialize it to a string. Default
 * implementations are provided for primitive numeric types.
 * @param edge_writer Function used for serializing the edges. Should accept an
 * edge_id_t and a graph::edge_t and serialize it to a string. Default
 * implementations are provided for primitive numeric types.
 * @param path Path to the output dot file.
 */
template<typename V,
         typename E,
         graph_type T,
         typename VERTEX_WRITER_T = decltype(detail::default_vertex_writer<V>),
         typename EDGE_WRITER_T = decltype(detail::default_edge_writer)>
requires std::is_invocable_r_v<std::string, const VERTEX_WRITER_T &, vertex_id_t, const V &>
    &&std::is_invocable_r_v<std::string,
                            const EDGE_WRITER_T &,
                            const graaf::edge_id_t &,
                            const typename graph<V, E, T>::edge_t &> void
    to_dot(const graph<V, E, T> &graph,
           const std::filesystem::path &path,
           const VERTEX_WRITER_T &vertex_writer = detail::default_vertex_writer<V>,
           const EDGE_WRITER_T &edge_writer = detail::default_edge_writer);
#else
namespace detail
{
// C++17 replacement for the string_convertible concept
// Remove when no longer need support for Qt 5.15 and MinGW 8.1.
template<typename T, typename = void>
struct is_string_convertible : std::false_type
{
};

template<typename T>
struct is_string_convertible<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_string_convertible_v = is_string_convertible<T>::value;

// Default vertex writer (Constrained by string_convertible)
template<typename T>
struct default_vertex_writer_t
{
    auto operator()(vertex_id_t vertex_id, const T &vertex) const -> std::string
    {
        return "label=\"" + std::to_string(vertex_id) + ": " + std::to_string(vertex) + "\"";
    }
};

// Default edge writer
struct default_edge_writer_t
{
    template<typename E>
    auto operator()(const edge_id_t & /*edge_id*/, const E &edge) const -> std::string
    {
        return "label=\"" + std::to_string(get_weight(edge)) + "\"";
    }
};

} // namespace detail

/**
 * @brief Serializes a graph to dot format.
 */
template<typename V,
         typename E,
         graph_type T,
         typename VERTEX_WRITER_T = detail::default_vertex_writer_t<V>,
         typename EDGE_WRITER_T = detail::default_edge_writer_t,
         // SFINAE Constraints
         std::enable_if_t<std::is_invocable_r_v<std::string, const VERTEX_WRITER_T &, vertex_id_t, const V &>
                          && std::is_invocable_r_v<std::string,
                                                   const EDGE_WRITER_T &,
                                                   const graaf::edge_id_t &,
                                                   const typename graph<V, E, T>::edge_t &>> * = nullptr>
void to_dot(const graph<V, E, T> &graph,
            const std::filesystem::path &path,
            const VERTEX_WRITER_T &vertex_writer = VERTEX_WRITER_T{},
            const EDGE_WRITER_T &edge_writer = EDGE_WRITER_T{});

#endif

}  // namespace graaf::io

#include "dot.tpp"
