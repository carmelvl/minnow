#include "tcp_receiver.hh"
#include "debug.hh"
#include <string>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    reassembler_.reader().set_error();
  }
  if ( message.SYN ) { // use seqno. if syn, it's the first.
    if ( !zero_point ) {
      zero_point = message.seqno; // if we don't have a zero point yet, set it
    }
  }
  if ( !zero_point ) {
    return;
  }
  // Calculate values
  uint64_t checkpoint = reassembler_.writer().bytes_pushed(); // checkpoint is where we are in the stream.
  uint64_t start = message.seqno.unwrap( zero_point.value(), checkpoint ); // process Wrap32 seqno
  if ( message.SYN ) {
    start += 1;
  }                                 // if there is a SYN it actually starts 1 bit later
  uint64_t first_index = start - 1; // convert from abs seq no to stream index number

  std::string& data = message.payload;
  bool is_last_substring = message.FIN;
  reassembler_.insert( first_index, data, is_last_substring ); // push to reassembler
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage message {};
  message.RST = reassembler_.reader().has_error() || reassembler_.writer().has_error(); //
  // set ackno if there is a established ISN
  if ( zero_point ) {
    uint64_t ack
      = reassembler_.writer().bytes_pushed() + 1; // then our ack is where we are in the bytestream, +1 for SYN
    if ( reassembler_.writer().is_closed() ) {    // if the writer finished
      ack += 1;                                   // count FIN flag too
    }
    message.ackno = Wrap32::wrap( ack, zero_point.value() );
  }
  // Window size
  uint64_t win = reassembler_.writer().available_capacity();
  if ( win > UINT16_MAX )
    win = UINT16_MAX; // cap it to the max
  message.window_size = static_cast<uint16_t>( win );
  return message;
}
