#ifndef _GUARD_ARCH_GRAPH_AUTOMORPHISMS_H
#define _GUARD_ARCH_GRAPH_AUTOMORPHISMS_H

#include <sstream>
#include <string>

#include "arch_graph_system.h"
#include "bsgs.h"
#include "perm_group.h"

namespace cgtl
{

class ArchGraphAutomorphisms : public ArchGraphSystem
{
public:
  ArchGraphAutomorphisms(PermGroup const &automorphisms)
  : _automorphisms(automorphisms)
  {}

  std::string to_gap() const override
  {
    std::stringstream ss;
    ss << "Group(" << _automorphisms.generators() << ")";
    return ss.str();
  }

private:
  PermGroup update_automorphisms(BSGS::Options const *) override
  { return _automorphisms; }

  PermGroup _automorphisms;
};

} // namespace cgtl

#endif // _GUARD_ARCH_GRAPH_AUTOMORPHISMS_H