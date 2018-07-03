#include "arch_graph.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <cassert>
#include <ostream>
#include <string>

#include "arch_graph.h"

namespace cgtl
{

ArchGraph::ProcessorType ArchGraph::new_processor_type(std::string label)
{
  auto id = _processor_types.size();
  _processor_types.push_back(label);

  return id;
}

ArchGraph::ChannelType ArchGraph::new_channel_type(std::string label)
{
  auto id = _channel_types.size();
  _channel_types.push_back(label);

  return id;
}

ArchGraph::Processor ArchGraph::add_processor(ArchGraph::ProcessorType pt)
{
  return boost::add_vertex(pt, _adj);
}

void ArchGraph::add_channel(ArchGraph::Processor from, ArchGraph::Processor to,
                            ArchGraph::ChannelType cht)
{
  boost::add_edge(from, to, cht, _adj);
}

std::ostream& operator<<(std::ostream& stream, ArchGraph const &arch_graph)
{
  static char const * const COLORSCHEME = "accent";
  static const unsigned COLORS = 8;
  static char const * const NODESTYLE = "filled";
  static const unsigned LINEWIDTH = 2;

  assert(arch_graph._processor_types.size() < COLORS
         && "Distinguishably many processor types in dot output");

  assert(arch_graph._channel_types.size() < COLORS
         && "Distinguishably many channel types in dot output");

  // construct dotfile...
  stream << "graph {\n";

  // add vertices
  auto vertex_labels = boost::get(boost::vertex_name, arch_graph._adj);

  for (auto v : boost::make_iterator_range(boost::vertices(arch_graph._adj))) {
    stream << v << " [label=" << "PE" << v << ",style=" << NODESTYLE
           << ",colorscheme=" << COLORSCHEME << COLORS << ",fillcolor="
           << vertex_labels[v] + 1 << "]\n";
  }

  // add edges
  auto edge_labels = boost::get(boost::edge_name, arch_graph._adj);

  for (auto e : boost::make_iterator_range(boost::edges(arch_graph._adj))) {
    auto source = boost::source(e, arch_graph._adj);
    auto target = boost::target(e, arch_graph._adj);

    stream << source << " -- " << target << " [penwidth=" << LINEWIDTH
           << ",colorscheme=" << COLORSCHEME << COLORS << ",color="
           << edge_labels[e] + 1 << "]\n";
  }

  stream << "}\n";

  return stream;
}

}
