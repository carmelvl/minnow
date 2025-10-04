#include "reassembler.hh"
#include "debug.hh"
#include <iostream>
#include <map>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( is_last_substring ) { // if the last one, save last idx accordingly
    last_idx = first_index + data.size();
  }
  // next needed - track the amount of bytes you've already pushed, to know the next.
  uint64_t next_needed = output_.writer().bytes_pushed();
  // set cap.
  uint64_t capacity_limit = next_needed + output_.writer().available_capacity();
  // insert each char of data in the map.
  if ( first_index < capacity_limit ) {
    for ( size_t i = 0; i < data.size(); i++ ) {
      uint64_t index = first_index + i;
      // if it already exists, skip it (a dupe push)
      if ( index < next_needed ) {
        continue;
      }
      // if its beyond capacity, stop adding
      if ( index >= capacity_limit ) {
        break;
      }
      // insert the char at the right idx
      waitlist_[index] = data[i];
    }
  }
  uint64_t i = next_needed;
  while ( waitlist_.find( i ) != waitlist_.end() ) { // push everything you need
    output_.writer().push( string( 1, waitlist_[i] ) );
    waitlist_.erase( i );
    i++;
  }
  if ( last_idx == output_.writer().bytes_pushed() ) {
    output_.writer().close();
    cout << "Closed the stream \n" << endl;
  }
}

uint64_t Reassembler::count_bytes_pending() const
{
  return waitlist_.size();
}