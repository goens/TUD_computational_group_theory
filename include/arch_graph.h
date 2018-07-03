#ifndef _GUARD_ARCH_GRAPH_H
#define _GUARD_ARCH_GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <ostream>
#include <string>

namespace cgtl
{

class ArchGraph
{
  friend std::ostream& operator<<(std::ostream&, const ArchGraph &);

  typedef boost::vecS vertex_selector;
  typedef boost::vecS edge_selector;
  typedef boost::property<boost::vertex_name_t, unsigned> vertex_property;
  typedef boost::property<boost::edge_name_t, unsigned> edge_property;

public:
  typedef std::vector<std::string>::size_type ProcessorType;
  typedef std::vector<std::string>::size_type ChannelType;

  typedef boost::adjacency_list<vertex_selector, edge_selector,
      boost::bidirectionalS, unsigned, unsigned>::vertex_descriptor Processor;

  ProcessorType new_processor_type(std::string label);
  ChannelType new_channel_type(std::string label);

  Processor add_processor(ProcessorType pe);
  void add_channel(ProcessorType pe1, ProcessorType pe2, ChannelType ch);

private:
  boost::adjacency_list<vertex_selector, edge_selector, boost::bidirectionalS,
                        vertex_property, edge_property> _adj;

  std::vector<std::string> _processor_types;
  std::vector<std::string> _channel_types;
};

std::ostream& operator<<(std::ostream& stream, ArchGraph const &arch_graph);

}

#endif // _GUARD_ARCH_GRAPH
