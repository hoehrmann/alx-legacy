#include <set>
#include <functional>

#if 0

#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <list>
#include <vector>
#include <iterator>
#include <algorithm>
#include <random>
#include <chrono>
#include <queue>
#include <bitset>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <climits>
#include <cassert>

#include <boost/algorithm/cxx11/copy_if.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/assign.hpp>

#endif

extern "C" {
  #include "alx.h" 
};


struct alx_set {
  std::set< std::tuple<
    alx_graph_node_ix,
    alx_graph_node_ix,
    alx_graph_node_ix,
    alx_graph_node_ix
  > > set;
};

extern "C" {

struct alx_set *
alx_set_create(
  struct alx_parser_state const* const p
) {

  return new alx_set();

}

void
alx_set_insert(
  struct alx_set * const set,
  struct alx_graph_quad const* const quad
) {

#if 1
  auto const& pair =
    set->set.insert(std::make_tuple(
      quad->v1,
      quad->v2,
      quad->v3,
      quad->v4
    ));

  if (!pair.second)
    return;
#endif

#if 1
  // Insert otherwise impossible quad to abuse this structure
  // for queries that check only whether there is anything in
  // there set that has certain exterior vertices.

  set->set.insert(std::make_tuple(
    quad->v1,
    0,
    quad->v4,
    quad->v4
  ));
#endif

}

bool
alx_set_has(
  struct alx_set * const set,
  struct alx_graph_quad const* const quad
) {

  auto result = set->set.find(std::make_tuple(
    quad->v1,
    quad->v2,
    quad->v3,
    quad->v4
  ));

  return result != set->set.end();

}

bool
alx_set_has_exteriors(
  struct alx_set * const set,
  struct alx_graph_quad const* const quad
) {

  auto result = set->set.find(std::make_tuple(
    quad->v1,
    0,
    quad->v4,
    quad->v4
  ));

  return result != set->set.end();

}

void
alx_set_clear(
  struct alx_set * const set
) {

  set->set.clear();

}

void
alx_set_destroy(
  struct alx_set * const set
) {

  delete set;

}

}; // extern "C"
