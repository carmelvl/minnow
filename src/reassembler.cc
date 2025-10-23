// CALL OUT: This is the lab solution for reassembler.cc, because mine was too slow and made my
// TCP receiver time out for the next checkpoint.
// My original submission and solution is below this code, commented out.

#include "reassembler.hh"
#include "debug.hh"
#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( is_last_substring ) {
    total_size_.emplace( first_index + data.size() );
  }

  buf_.resize( writer().available_capacity() );
  occupancy_.resize( writer().available_capacity() );

  const auto first_unassembled = writer().bytes_pushed();
  const auto first_unacceptable = first_unassembled + writer().available_capacity();

  const auto left_insert = max( first_unassembled, first_index );
  const auto right_insert = min( first_unacceptable, first_index + data.size() );

  if ( left_insert < right_insert ) {
    copy( data.begin() + ( left_insert - first_index ),
          data.begin() + ( right_insert - first_index ),
          buf_.begin() + ( left_insert - first_unassembled ) );

    fill( occupancy_.begin() + ( left_insert - first_unassembled ),
          occupancy_.begin() + ( right_insert - first_unassembled ),
          true );
  }

  const uint64_t count = std::ranges::find( occupancy_, false ) - occupancy_.begin();

  output_.writer().push( buf_.substr( 0, count ) );
  buf_.erase( 0, count );
  occupancy_.erase( occupancy_.begin(), occupancy_.begin() + count );

  // close if we’re done
  if ( total_size_.has_value() && total_size_.value() == writer().bytes_pushed() ) {
    output_.writer().close();
  }
}

uint64_t Reassembler::count_bytes_pending() const
{
  return accumulate( occupancy_.begin(), occupancy_.end(), uint64_t { 0 } ); // <-- 64-bit zero
}

//  ORIGINAL SOLUTION: reassembler.cc

// #include "reassembler.hh"
// #include "debug.hh"
// #include <iostream>
// #include <map>

// using namespace std;

// void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
// {
//   if ( is_last_substring ) { // if the last one, save last idx accordingly
//     last_idx = first_index + data.size();
//   }
//   // next needed - track the amount of bytes you've already pushed, to know the next.
//   uint64_t next_needed = output_.writer().bytes_pushed();
//   // set cap.
//   uint64_t capacity_limit = next_needed + output_.writer().available_capacity();
//   // insert each char of data in the map.
//   if ( first_index < capacity_limit ) {
//     for ( size_t i = 0; i < data.size(); i++ ) {
//       uint64_t index = first_index + i;
//       // if it already exists, skip it (a dupe push)
//       if ( index < next_needed ) {
//         continue;
//       }
//       // if its beyond capacity, stop adding
//       if ( index >= capacity_limit ) {
//         break;
//       }
//       // insert the char at the right idx
//       waitlist_[index] = data[i];
//     }
//   }
//   uint64_t i = next_needed;
//   while ( waitlist_.find( i ) != waitlist_.end() ) { // push everything you need
//     output_.writer().push( string( 1, waitlist_[i] ) );
//     waitlist_.erase( i );
//     i++;
//   }
//   if ( last_idx == output_.writer().bytes_pushed() ) {
//     output_.writer().close();
//     cout << "Closed the stream \n" << endl;
//   }
// }

// uint64_t Reassembler::count_bytes_pending() const
// {
//   return waitlist_.size();
// }