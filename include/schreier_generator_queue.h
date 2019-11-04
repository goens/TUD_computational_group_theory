#ifndef _GUARD_SCHREIER_GENERATOR_QUEUE_H
#define _GUARD_SCHREIER_GENERATOR_QUEUE_H

#include <cassert>
#include <memory>
#include <vector>

#include "perm.h"
#include "perm_set.h"
#include "schreier_structure.h"

namespace cgtl
{

class SchreierGeneratorQueue
{
  using sg_type = PermSet;
  using sg_it_type = sg_type::const_iterator;

  using fo_type = std::vector<unsigned>;
  using fo_it_type = fo_type::const_iterator;

public:
  class iterator
  {
  public:
    iterator()
    : _queue(nullptr),
      _end(true)
    {}

    iterator(SchreierGeneratorQueue *queue)
    : _queue(queue),
      _end(false)
    {
      _queue->advance();
      _queue->mark_used();
    }

    iterator operator++() { _queue->advance(); return *this; }
    iterator operator++(int) { throw std::logic_error("not implemented"); }
    Perm const & operator*() const { return _queue->_schreier_generator; }
    Perm const * operator->() const { return &_queue->_schreier_generator; }

    bool operator==(iterator const &rhs) const
    { return end() && rhs.end(); }

    bool operator!=(iterator const &rhs) const
    { return !((*this) == rhs); }

  private:
    bool end() const
    { return _end || _queue->_exhausted; }

    SchreierGeneratorQueue *_queue;
    bool _end;
  };

  SchreierGeneratorQueue(sg_type const &strong_generators,
                         fo_type const &fundamental_orbit,
                         std::shared_ptr<SchreierStructure> schreier_structure)
  : _sg_it(strong_generators.begin()),
    _sg_begin(strong_generators.begin()),
    _sg_end(strong_generators.end()),
    _beta_it(fundamental_orbit.begin()),
    _beta_end(fundamental_orbit.end()),
    _schreier_structure(schreier_structure),
    _valid(false),
    _used(false),
    _exhausted(false),
    _u_beta(u_beta())
  {}

  void update(sg_type const &strong_generators,
              fo_type const &fundamental_orbit,
              std::shared_ptr<SchreierStructure> schreier_structure)
  {
    if (_valid)
      return;

    _sg_it = strong_generators.begin();
    _sg_begin = _sg_it;
    _sg_end = strong_generators.end();

    _beta_it = fundamental_orbit.begin();
    _beta_end = fundamental_orbit.end();

    _schreier_structure = schreier_structure;

    _u_beta = u_beta();

    _valid = true;
    _used = false;
    _exhausted = false;
  }

  void invalidate() { _valid = false; }

  iterator begin() { return iterator(this); }
  iterator end() { return iterator(); }

private:
  Perm u_beta()
  { return _schreier_structure->transversal(*_beta_it); }

  Perm u_beta_x()
  { return _schreier_structure->transversal((*_sg_it)[*_beta_it]); }

  void next_sg()
  {
    if (++_sg_it == _sg_end)
      next_beta();
  }

  void next_beta()
  {
    if (++_beta_it == _beta_end) {
      _exhausted = true;
    } else {
      _sg_it = _sg_begin;
      _u_beta = u_beta();
    }
  }

  void advance()
  {
    if (_used)
      next_sg();

    while (!_exhausted && _schreier_structure->incoming(*_beta_it, *_sg_it))
      next_sg();

    if (_exhausted)
      return;

    _schreier_generator = _u_beta * (*_sg_it) * ~u_beta_x();
  }

  void mark_used() { _used = true; }

  sg_it_type _sg_it;
  sg_it_type _sg_begin;
  sg_it_type _sg_end;

  fo_it_type _beta_it;
  fo_it_type _beta_end;

  std::shared_ptr<SchreierStructure> _schreier_structure;

  bool _valid;
  bool _used;
  bool _exhausted;

  Perm _u_beta;
  Perm _schreier_generator;
};

} // namespace cgtl

#endif // _GUARD_SCHREIER_GENERATOR_QUEUE_H
