#include <sstream>

#include "arch_graph.h"
#include "gmock/gmock.h"

#include "test_main.cc"

class ArchGraphTest : public ::testing::Test
{
protected:
  void SetUp() {
    auto a7 = arch_graph.new_processor_type("A7");
    auto a15 = arch_graph.new_processor_type("A15");

    auto l1 = arch_graph.new_channel_type("L1");
    auto l2 = arch_graph.new_channel_type("L2");
    auto sram = arch_graph.new_channel_type("SRAM");

    auto pe0 = arch_graph.add_processor(a7);
    auto pe1 = arch_graph.add_processor(a7);
    auto pe2 = arch_graph.add_processor(a7);
    auto pe3 = arch_graph.add_processor(a7);
    auto pe4 = arch_graph.add_processor(a15);
    auto pe5 = arch_graph.add_processor(a15);
    auto pe6 = arch_graph.add_processor(a15);
    auto pe7 = arch_graph.add_processor(a15);

    std::vector<cgtl::ArchGraph::Processor> all_pes {
      pe0, pe1, pe2, pe3, pe4, pe5, pe6, pe7
    };

    for (auto pe : all_pes) {
      arch_graph.add_channel(pe, pe, l1);
      arch_graph.add_channel(pe, pe, l2);
      arch_graph.add_channel(pe, pe, sram);

      for (auto other : all_pes) {
        if (other == pe)
          continue;

        arch_graph.add_channel(pe, other, sram);
      }
    }

    arch_graph.add_channel(pe0, pe1, l2);
    arch_graph.add_channel(pe0, pe2, l2);
    arch_graph.add_channel(pe0, pe3, l2);
    arch_graph.add_channel(pe1, pe2, l2);
    arch_graph.add_channel(pe1, pe3, l2);
    arch_graph.add_channel(pe2, pe3, l2);

    arch_graph.add_channel(pe4, pe5, l2);
    arch_graph.add_channel(pe4, pe6, l2);
    arch_graph.add_channel(pe4, pe7, l2);
    arch_graph.add_channel(pe5, pe6, l2);
    arch_graph.add_channel(pe5, pe7, l2);
    arch_graph.add_channel(pe6, pe7, l2);
  }

  cgtl::ArchGraph arch_graph;
};
