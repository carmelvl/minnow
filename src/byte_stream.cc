#include "byte_stream.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}
// Push data to stream, but only as much as available capacity allows.
void Writer::push( string data )
{
  uint64_t avail = available_capacity();
  if ( avail == 0 ) { // if no capacity
    return;
  }
  if ( avail < data.size() ) { // if some capacity, but not all
    std::string toAdd = data.substr( 0, avail );
    // std::cout << toAdd << endl;
    pushed_ += toAdd.size();
    buf_.append( toAdd );
  } else if ( avail >= data.size() ) { // if can accommodate all
    pushed_ += data.size();
    buf_.append( data );
  }
}

// Signal that the stream has reached its ending. Nothing more will be written.
void Writer::close()
{
  // Make the stream closed
  c = true;
}

// Has the stream been closed?
bool Writer::is_closed() const
{
  return c; // will return true if closed, false if not closed
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const
{
  // Return the overall capacity minus number of bytes (chars) cur in the string
  return capacity_ - buf_.size();
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const
{
  return pushed_;
}

// Peek at the next bytes in the buffer -- ideally as many as possible.
// It's not required to return a string_view of the *whole* buffer, but
// if the peeked string_view is only one byte at a time, it will probably force
// the caller to do a lot of extra work.
string_view Reader::peek() const
{
  string_view v = buf_; // return a string view of the existing buffer
  return v;
}

// Remove `len` bytes from the buffer.
void Reader::pop( uint64_t len )
{
  buf_.erase( 0, len ); // remove bytes from the front
  popped_ += len;       // every time pop, record cumulative
}

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const
{
  if ( buf_.size() == 0 && c == true ) {
    return true;
  }
  return false;
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const
{
  return buf_.size(); // return length of string
}

// Total number of bytes cumulatively popped from stream
uint64_t Reader::bytes_popped() const
{
  return popped_;
}
