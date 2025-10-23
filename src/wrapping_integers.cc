#include "wrapping_integers.hh"
#include "debug.hh"
#include <iostream>
using namespace std;

static const uint64_t WRAP_MOD = 1ULL << 32; // save this val

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { zero_point + n % WRAP_MOD };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t offset = ( this->raw_value_ - zero_point.raw_value_ ) % WRAP_MOD;
  uint64_t base = ( ( checkpoint / WRAP_MOD ) * WRAP_MOD ) + uint64_t( offset );
  // Consider 3 candidates
  uint64_t below = base - WRAP_MOD; // one universe below
  uint64_t distance_below;
  if ( below > checkpoint ) {
    distance_below = below - checkpoint;
  } else {
    distance_below = checkpoint - below;
  }
  uint64_t same = base; // the same universe
  uint64_t distance_same;
  if ( same > checkpoint ) {
    distance_same = same - checkpoint;
  } else {
    distance_same = checkpoint - same;
  }
  uint64_t above = base + WRAP_MOD; // in the above universe
  uint64_t distance_above;
  if ( above > checkpoint ) {
    distance_above = above - checkpoint;
  } else {
    distance_above = checkpoint - above;
  }
  uint64_t best_so_far = below; // assume below is best and then compare to other two candidates
  uint64_t best_distance = distance_below;
  if ( distance_same < best_distance ) {
    best_distance = distance_same;
    best_so_far = same;
  }
  if ( distance_above < best_distance ) {
    best_distance = distance_above;
    best_so_far = above;
  }
  return uint64_t( best_so_far );
}
