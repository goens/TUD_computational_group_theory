#ifndef GUARD_PERM_GROUP_H
#define GUARD_PERM_GROUP_H

#include <cassert>
#include <iterator>
#include <map>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>

#include "bsgs.hpp"
#include "perm.hpp"
#include "perm_set.hpp"

namespace mpsym
{

namespace internal
{

class Orbit;
class OrbitPartition;

class BlockSystem;

class PermGroup
{
  friend std::ostream &operator<<(std::ostream &os, PermGroup const &pg);

public:
  class const_iterator
  {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Perm;
    using pointer = Perm const *;
    using reference = Perm const &;

    const_iterator() : _end(true) {};
    const_iterator(PermGroup const &pg);

    const_iterator const & operator++()
    {
      next_state();
      return *this;
    }

    value_type operator*()
    {
      update_current_result();
      return _current_result;
    }

    pointer operator->()
    {
      update_current_result();
      return &_current_result;
    }

    bool operator==(const_iterator const &rhs) const;

    bool operator!=(const_iterator const &rhs) const
    { return !((*this) == rhs); }

    PermSet const & factors() const
    { return _current_factors; }

  private:
    void next_state();
    void update_current_result();

    std::vector<unsigned> _state;
    bool _trivial;
    bool _end;

    std::vector<PermSet> _transversals;
    PermSet _current_factors;
    Perm _current_result;
    bool _current_result_valid;
  };

  explicit PermGroup(unsigned degree = 1)
  : _bsgs(degree),
    _order(1)
  {}

  explicit PermGroup(BSGS const &bsgs)
  : _bsgs(bsgs),
    _order(bsgs.order())
  {}

  PermGroup(unsigned degree, PermSet const &generators);

  static PermGroup symmetric(unsigned degree);
  static PermGroup cyclic(unsigned degree);
  static PermGroup alternating(unsigned degree);
  static PermGroup dihedral(unsigned degree);

  template<typename IT>
  static PermGroup direct_product(IT first,
                                  IT last,
                                  BSGSOptions const *bsgs_options = nullptr)
  {
    assert(std::distance(first, last) > 0);

    unsigned current_degree = 0u;
    unsigned total_degree = 0u;

    for (auto it = first; it != last; ++it)
      total_degree += it->degree();

    PermSet generators;
    for (auto it = first; it != last; ++it) {
      for (Perm const &perm : it->generators())
        generators.insert(perm.shifted(current_degree).extended(total_degree));

      current_degree += it->degree();
    }

    return PermGroup(BSGS(total_degree, generators, bsgs_options));
  }

  static PermGroup wreath_product(PermGroup const &lhs,
                                  PermGroup const &rhs,
                                  BSGSOptions const *bsgs_options = nullptr);

  bool operator==(PermGroup const &rhs) const;
  bool operator!=(PermGroup const &rhs) const;

  const_iterator begin() const { return const_iterator(*this); }
  const_iterator end() const { return const_iterator(); }

  PermSet generators() const { return _bsgs.strong_generators(); }

  BSGS &bsgs() { return _bsgs; }
  BSGS const &bsgs() const { return _bsgs; }

  unsigned degree() const { return _bsgs.degree(); }
  BSGS::order_type order() const { return _order; }

  unsigned smallest_moved_point() const
  { return generators().smallest_moved_point(); }

  unsigned largest_moved_point() const
  { return generators().largest_moved_point(); }

  bool is_trivial() const { return _bsgs.base_empty(); }
  bool is_symmetric() const;
  bool is_shifted_symmetric() const;
  bool is_alternating() const;
  bool is_shifted_alternating() const;
  bool is_transitive() const;

  bool contains_element(Perm const &perm) const;
  Perm random_element() const;

  std::vector<PermGroup> disjoint_decomposition(
    bool complete = true, bool disjoint_orbit_optimization = false) const;

  std::vector<PermGroup> wreath_decomposition() const;

private:
  static boost::multiprecision::cpp_int symmetric_order(unsigned deg)
  {
    boost::multiprecision::cpp_int ret(1);
    for (unsigned i = deg; i > 0u; --i)
      ret *= i;

    return ret;
  }

  static boost::multiprecision::cpp_int alternating_order(unsigned deg)
  {
    boost::multiprecision::cpp_int ret(1);
    for (unsigned i = deg; i > 2u; --i)
      ret *= i;

    return ret;
  }

  // complete disjoint decomposition
  bool disjoint_decomp_orbits_dependent(
    Orbit const &orbit1,
    Orbit const &orbit2) const;

  void disjoint_decomp_generate_dependency_classes(
    OrbitPartition &orbits) const;

  static bool disjoint_decomp_restricted_subgroups(
    OrbitPartition const &orbit_split,
    PermGroup const &perm_group,
    std::pair<PermGroup, PermGroup> &restricted_subgroups);

  static std::vector<PermGroup> disjoint_decomp_join_results(
    std::vector<PermGroup> const &res1,
    std::vector<PermGroup> const &res2);

  static std::vector<PermGroup> disjoint_decomp_complete_recursive(
    OrbitPartition const &orbits,
    PermGroup const &perm_group);

  std::vector<PermGroup> disjoint_decomp_complete(
    bool disjoint_orbit_optimization = true) const;

  // incomplete disjoint decomposition
  struct MovedSet : public std::vector<unsigned>
  {
    void init(Perm const &perm);
    bool equivalent(MovedSet const &other) const;
    void extend(MovedSet const &other);
  };

  struct EquivalenceClass
  {
    EquivalenceClass(Perm const &init, MovedSet const &moved)
    : generators({init}),
      moved(moved),
      merged(false)
    {}

    PermSet generators;
    MovedSet moved;
    bool merged;
  };

  std::vector<EquivalenceClass> disjoint_decomp_find_equivalence_classes() const;

  void disjoint_decomp_merge_equivalence_classes(
    std::vector<EquivalenceClass> &equivalence_classes) const;

  std::vector<PermGroup> disjoint_decomp_incomplete() const;

  // wreath decomposition
  std::vector<PermGroup> wreath_decomp_find_stabilizers(
    BlockSystem const &block_system,
    PermGroup const &block_permuter) const;

  PermSet wreath_decomp_construct_block_permuter_image(
    BlockSystem const &block_system,
    PermGroup const &block_permuter) const;

  bool wreath_decomp_reconstruct_block_permuter(
    BlockSystem const &block_system,
    PermGroup const &block_permuter,
    PermSet const &block_permuter_image) const;

  BSGS _bsgs;
  BSGS::order_type _order;
};

std::ostream &operator<<(std::ostream &os, PermGroup const &pg);

} // namespace internal

} // namespace mpsym

#endif // GUARD_PERM_GROUP_H
