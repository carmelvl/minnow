// CALL OUT: This is the lab solution for reassembler.hh, because mine was too slow and made my 
// TCP receiver time out for the next checkpoint.
// My original submission and solution is below this code, commented out.

#pragma once
#include "byte_stream.hh"
#include <map>
#include <string>
#include <vector>
#include <optional>

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler(ByteStream&& output) // initialize all members
    : buf_()
    , occupancy_()
    , total_size_(std::nullopt)
    , output_(std::move(output)) 
  {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  // This function is for testing only; don't add extra state to support it.
  uint64_t count_bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  // Declare here in same order as the top!!
  std::string buf_;
  std::vector<bool> occupancy_;
  std::optional<uint64_t> total_size_;

  ByteStream output_;
};

// ORIGINAL SOLUTION: reassembler.hh

// #pragma once

// #include "byte_stream.hh"
// #include <map>
// #include <string>
// #include <vector>

// class Reassembler
// {
// public:
//   // Construct Reassembler to write into given ByteStream.
//   explicit Reassembler( ByteStream&& output ) // removed size_t capacity
//     : output_( std::move( output ) ), waitlist_(), last_idx( UINT64_MAX )
//   {}

//   /*
//    * Insert a new substring to be reassembled into a ByteStream.
//    *   `first_index`: the index of the first byte of the substring
//    *   `data`: the substring itself
//    *   `is_last_substring`: this substring represents the end of the stream
//    *   `output`: a mutable reference to the Writer
//    *
//    * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
//    * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
//    * learns the next byte in the stream, it should write it to the output.
//    *
//    * If the Reassembler learns about bytes that fit within the stream's available capacity
//    * but can't yet be written (because earlier bytes remain unknown), it should store them
//    * internally until the gaps are filled in.
//    *
//    * The Reassembler should discard any bytes that lie beyond the stream's available capacity
//    * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
//    *
//    * The Reassembler should close the stream after writing the last byte.
//    */
//   void insert( uint64_t first_index, std::string data, bool is_last_substring );

//   // How many bytes are stored in the Reassembler itself?
//   // This function is for testing only; don't add extra state to support it.
//   uint64_t count_bytes_pending() const;

//   // Access output stream reader
//   Reader& reader() { return output_.reader(); }
//   const Reader& reader() const { return output_.reader(); }

//   // Access output stream writer, but const-only (can't write from outside)
//   const Writer& writer() const { return output_.writer(); }

// private:
//   // Declare here in same order as the top!!
//   ByteStream output_;
//   uint64_t capacity_ = output_.writer().available_capacity();
//   std::map<uint64_t, char> waitlist_; // Reassembler waitlist is a map of chars
//   uint64_t last_idx;

//   // bytes pushed is the first unassembled index = next_needed
//   // use a string for the reassembler, because substrings can overlap , 0-a 1-bcd, 2-cde
// };
