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

/**
 * @file perm_group.hpp
 * @brief Defines `PermGroup`.
 *
 * @author Timo Nicolai
 */

namespace mpsym
{

namespace internal
{

class Orbit;
class OrbitPartition;

class BlockSystem;

/** A permutation group representation.
 *
 * This class provides a useful abstraction encapsulating several complex
 * algorithms and data structures used to efficiently represent a permutation
 * group defined by a set of generating permutations without the need to store
 * elements explicitely for very large groups.
 */
class PermGroup
{
  friend std::ostream &operator<<(std::ostream &os, PermGroup const &pg);

public:
  class const_iterator : std::iterator<std::forward_iterator_tag, Perm>
  {
  public:
    const_iterator() : _end(true) {};
    const_iterator(PermGroup const &pg);

    const_iterator & operator++()
    {
      next_state();
      return *this;
    }

    Perm const & operator*()
    {
      update_current_result();
      return _current_result;
    }

    Perm const * operator->()
    {
      update_current_result();
      return &_current_result;
    }

    PermSet const & factors() const
    { return _current_factors; }

    bool operator==(const_iterator const &rhs) const;

    bool operator!=(const_iterator const &rhs) const
    { return !((*this) == rhs); }

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

  /// TODO
  explicit PermGroup(unsigned degree = 1)
  : _bsgs(degree),
    _order(1)
  {}

  /// TODO
  explicit PermGroup(BSGS const &bsgs)
  : _bsgs(bsgs),
    _order(bsgs.order())
  {}

  /** Construct a permutation group.
   *
   * Constructs a permutation group representation from a given set of
   * generating permutations. The generators and group elements might not be
   * stored explicitly in the resulting object. Instead some variation of the
   * *Schreier-Sims* algorithm (see \cite holt05, chapter 4.4.2) might be used
   * to compute a *base* and *strong generating* set for the group which
   * describes the group completely and can be used to, among others, test
   * element membership and iterate through all group elements efficiently.
   *
   * \param degree the permutation group's *degree*, must be the same as the
   *               degree of all given generators (which in turn implies that
   *               they map from the set \f$\{1, \dots, degree\}\f$ to itself)
   *               otherwise this constructor's behaviour is undefined
   *
   * \param generators a generating set for the permutation group
   */
  PermGroup(unsigned degree, PermSet const &generators);

  /** Check two permutation groups for equality.
   *
   * Two permutation groups are equal exactly when they contain the same
   * elements. Although it it possible to use the PermGroup class to represent
   * permutation groups containing a very large number of elements, this
   * operation is guaranteed to be performed in \f$O(|bsgs.sgs()|)\f$ time. If
   * `(*this).degree() != rhs.degree()`, this function's behaviour is undefined.
   *
   * \return `true`, if `*this` and `rhs` describe permutation groups containing
   *         the same elements, else `false`
   */
  bool operator==(PermGroup const &rhs) const;

  /** Check two permutation groups for inequality.
   *
   * The result is always equal to `!(*this == rhs)`.
   *
   * \return `true`, if `*this` and `rhs` describe permutation groups not
   *         containing the same elements, else `false`
   */
  bool operator!=(PermGroup const &rhs) const;

  /** Construct a symmetric permutation group.
   *
   * \param degree
   *     degree \f$n\f$ of the resulting group, for `degree == 0u` this
   *     function's behaviour is undefined
   *
   * \return the symmetric group \f$S_n\f$
   */
  static PermGroup symmetric(unsigned degree);

  /** Construct a cyclic permutation group.
   *
   * \param degree
   *     degree \f$n\f$ of the resulting group, for `degree == 0u` this
   *     function's behaviour is undefined
   *
   * \return the cyclic group \f$C_n\f$
   */
  static PermGroup cyclic(unsigned degree);

  /** Construct an alternating permutation group.
   *
   * \param degree
   *     degree \f$n\f$ of the resulting group, for `degree == 0u` this
   *     function's behaviour is undefined
   *
   * \return the alternating group \f$A_n\f$
   */
  static PermGroup alternating(unsigned degree);

  /** Construct a dihedral permutation group.
   *
   * \param degree
   *     degree \f$n\f$ of the resulting group (except when `degree == 1u` or
   *     `degree == 2u`), for `degree == 0u` this function's behaviour is
   *     undefined
   *
   * \return the dihedral group \f$D_n\f$ (or \f$D_2n\f$ in a different notation)
   */
  static PermGroup dihedral(unsigned degree);

  /// TODO
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

  /** Compute the wreath product of two permutation groups.
   *
   * \param lhs left hand side permutation group operand
   *
   * \param rhs right hand side permutation group operand
   *
   * \return the permutation group wreath product of `lhs` and `rhs`
   */
  static PermGroup wreath_product(PermGroup const &lhs,
                                  PermGroup const &rhs,
                                  BSGSOptions const *bsgs_options = nullptr);

  /** Obtain a constant iterator iterating over this group's elements.
   *
   * Note that the permutation group elements might not be stored explicitly
   * and could instead be constructed by the iterator "on the fly" which means
   * that storing references or pointers to the partial permutation pointed to
   * by any iterator iterating over a permutation group will result in
   * undefined behaviour. The elements are not guaranteed to be returned in any
   * particular order but every element will be returned exactly once.The
   * elements will also be returned in the same order on subsequent iterations
   * through the permutation group. This last point is not necessarily true for
   * other equal (in the sense of `operator==`) permutation groups.
   *
   * Iterating through a permutation group could thus look like this:
   *
   * ~~~~~{.cpp}
   * PermGroup pg(degree, generators);
   *
   * for (auto const &perm : pg)
   *   std::cout << perm;
   *
   * for (auto const it = pg.begin(); it != pg.end(); ++i)
   *   std::cout << *it;
   * ~~~~~
   *
   * \return a constant iterator pointing to some element in this permutation
   *         group, incrementing it will yield all other group members exactly
   *         once (in no particular order) until end() is reached.
   */
  const_iterator begin() const { return const_iterator(*this); }

  /** Obtain a contant iterator signifying a permutation group's "end".
   *
   * \return a contant iterator signifying a permutation group's "end"
   */
  const_iterator end() const { return const_iterator(); }

  /** Obtain a permutation group's *degree*.
   *
   * A permutation group's degree is the positive integer \f$n\f$ such that
   * all its elements map from the set \f$\{1, \dots, n\}\f$ to itself.
   *
   * \return this permutation group's degree
   */
  unsigned degree() const { return _bsgs.degree(); }

  /** Obtain a permutation group's *order*.
   *
   * A permutation group \f$G\f$ 's order is equal to the number of elements it
   * contains, i.e. \f$|G|\f$. Note that every permutation group must at least
   * contain an identity and thus it is always true that `order() > 0u`.
   *
   * \return this permutation group's order
   */
  BSGS::order_type order() const { return _order; }

  /** Obtain permutation group generators.
   *
   * \return a generating set for this group.
   */
  PermSet generators() const { return _bsgs.strong_generators(); }

  // TODO
  unsigned smallest_moved_point() const
  { return generators().smallest_moved_point(); }

  // TODO
  unsigned largest_moved_point() const
  { return generators().largest_moved_point(); }

  /** Obtain a permutation group's base and strong generating set.
   *
   * This function is only meaningful is the permutation group's elements are
   * not stored explicitely, which can be determined via TODO.
   *
   * \return a BSGS object representing this permutation group's base and strong
   *         generating set
   */
  BSGS &bsgs() { return _bsgs; }

  /// TODO
  BSGS const &bsgs() const { return _bsgs; }

  /** Check whether a permutation group is *trivial*.
   *
   * A permutation group is trivial by definition if it only contains an
   * identity permutation on the set \f$\{1, \dots, n\}\f$ (where \f$n\f$ is the
   * group's degree()). This is equivalent to testing whether `order() == 1u`.
   *
   * \return `true` if this permutation group is trivial, else `false`
   */
  bool is_trivial() const { return _bsgs.base_empty(); }

  /** Check whether a permutation group is symmetric.
   *
   * \return `true` if the permutation group \f$G \leq Sym(\Omega)\f$
   *          represented by this object is in fact equal to \f$Sym(\Omega)\f$,
   *          else `false`
   */
  bool is_symmetric() const;

  // TODO
  bool is_shifted_symmetric() const;

  /** Check whether a permutation group is symmetric.
   *
   * \return `true` if the permutation group \f$G \leq S_n\f$ represented by
   *          this object is the alternating group \f$A_n\f$, else `false`
   */
  bool is_alternating() const;

  // TODO
  bool is_shifted_alternating() const;

  /** Check whether a permutation group is *transitive*.
   *
   * A permutation group acting on a set \f$\Omega$\f is transitive by
   * definition if the group orbit of any \f$x \in \Omega$\f,
   * \f$G(x) = \{g \cdot x \in \{1, \dots, n\} : g \in G\} = \{1, \dots, n\}\f$
   * (where \f$n\f$ is the group's degree()).
   *
   * \return `true` if this permutation group is transitive, else `false`.
   */
  bool is_transitive() const;

  /** Check whether a permutation group contains a given permutation.
   *
   * Note that the group's elements may not be stored explicitly so while
   * efficient, this operation is not trivial. if `perm.degree() !=
   * (*this).degree()` this function's behaviour is undefined.   *
   * \return `true` if this permutation group contains the permutation `perm`,
   *         else `false`
   */
  bool contains_element(Perm const &perm) const;

  /** Construct a random group element.
   *
   * This function makes no guarantees about the returned elements distribution
   * of cryptographic security. Repeated calls to this function may not be very
   * efficient, use TODO instead.
   *
   * \return some element \f$x\f$ of this permutation group
   */
  Perm random_element() const;

  /** Find a *disjoint subgroup decomposition* of a permutation group
   *
   * This function's implementation is based on \cite donaldson09.
   *
   * A disjoint subgroup decomposition of a permutation group \f$G\f$ is a set
   * \f$\{H_1, \dots, H_k\}\f$ where \f$H_1, \dots, H_k\f$ are subgroups of
   * \f$G\f$ such that \f$G = \{\alpha_1 \cdot \alpha_2 \dots \alpha_k :
   * \alpha_i \in H_i, (1 \leq i \leq k)\}\f$ and \f$\forall (i, j) \in \{1,
   * \dots, k\}^2 : i \neq j \Rightarrow moved(H_i) \cap moved(H_j) =
   * \emptyset\f$.  Here, \f$moved(H_i)\f$ is the set \f$\{x : (x \in \{1,
   * \dots, n\} \land \exists \alpha \in H_i : \alpha(x) \neq x)\}\f$.
   *
   * \param complete
   *    if this is `true` the function will always return the *finest* possible
   *    disjoint subgroup decomposition (i.e. one in which no subgroup can be
   *    further decomposed into disjoint subgroups), this might be more
   *    computationally expensive
   *
   * \param disjoint_orbit_optimization
   *    if this is `true`, the optimization described in \cite donaldson09,
   *    chapter 5.2.1 is applied, this is only meaningful if `complete = true`,
   *    otherwise this argument is ignored.
   *
   * \return a disjoint subgroup decomposition of this permutation group given
   *         as a vector of subgroups as described above, in no particular
   *         order, if no disjoint subgroup partition could be found, the vector
   *         contains the group itself as its only element.
   */
  std::vector<PermGroup> disjoint_decomposition(
    bool complete = true, bool disjoint_orbit_optimization = false) const;

  /** Find a *wreath product decomposition* of a permutation group
   *
   * This function's implementation is based on \cite donaldson09.
   *
   * A wreath product decomposition of a permutation group \f$G \leq S_n\f$ is
   * (using the notation in \cite donaldson09) a triple \f$(H, K,
   * \mathcal{X})\f$. Here\f$H \leq S_m\f$, \f$K \leq S_d\f$ and \f$n = m \cdot
   * d\f$, \f$\mathcal{X}\f$ is a set \f$\{X_1, \dots, X_d\}\f$ of sets
   * \f$X_i\f$ (with \f$|X_i| = m\f$) which partition the set \f$X = \{1,
   * \dots, n\}\f$ on which \f$G\f$ operates and \f$G = \{\sigma(\beta)\
   * \sigma_1(\alpha_1) \dots \sigma_d(\alpha_d) : \beta \in K, \alpha_i \in H
   * \ (1 \leq i \leq d)\}\f$. \f$\sigma\f$ is the permutation representation
   * of the action of \f$K\f$ on \f$\mathcal{X}\f$ which permutes the sets
   * \f$X_i\f$ "among themselves" and the \f$\sigma_i\f$ are the permutation
   * representations of the obvious actions of \f$K\f$ on the sets \f$X_i\f$.
   * This is conventionally written as: \f$G = H \wr K\f$.
   *
   * \return a wreath product decomposition of \f$G\f$ as described above in
   *         the form of the vector of permutation groups \f$[\sigma(K),
   *         \sigma_1(H_1), \dots, \sigma_d(H_d)]\f$, if no wreath product
   *         decomposition could be found (either because none exists or the
   *         algorithm is unable to determine a valid decomposition, which is
   *         currently possible due to limitations in the implementation) an
   *         empty vector is returned
   */
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