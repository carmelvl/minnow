#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// how many sequence numbers are outstanding?
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  uint64_t total = 0; // Sum up the number of bytes in the deque
  for ( size_t i = 0; i < outstanding_.size(); i++ ) {
    total += outstanding_[i].sequence_length();
  }
  return total;
}

// how many consecutive retransmissions have happened?
uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  while ( true ) {
    TCPSenderMessage to_send {};                       // create message
    to_send.seqno = Wrap32::wrap( next_seqno_, isn_ ); // create seqno.

    uint64_t in_flight = sequence_numbers_in_flight();

    uint64_t effective_window = ( window_size_ == 0 ) ? 1 : window_size_;
    uint64_t avail_space = ( effective_window > in_flight ) ? ( effective_window - in_flight ) : 0;

    if ( !syn_sent_ ) { // set up connection if SYN was never sent.
      to_send.SYN = true;
      syn_sent_ = true;                                            // set syn flag to be true
      avail_space = ( avail_space > 0 ) ? ( avail_space - 1 ) : 0; 
    }

    uint64_t bytes_to_send = input_.reader().bytes_buffered();
    uint64_t max_val = TCPConfig::MAX_PAYLOAD_SIZE;

    // cap to the smallest of (available window, max payload, bytes buffered)
    bytes_to_send = std::min<uint64_t>( bytes_to_send, std::min<uint64_t>( avail_space, max_val ) );
    // set payload with string

    std::string data = std::string( input_.reader().peek() ); // cast string view into a string
    to_send.payload = data.substr( 0, bytes_to_send );
    input_.reader().pop( bytes_to_send ); // remove those from the bytestream

    // handle fin, if bytestream is closed, didn't fin yet, and there's space
    if ( input_.reader().is_finished() && !fin_sent_ && ( avail_space >= bytes_to_send + 1 ) ) {
      to_send.FIN = true;
      fin_sent_ = true;
    }
    if ( bytes_to_send == 0 && !to_send.SYN && !to_send.FIN ) { // guard against sending empty segment
      break;                                                    // nothing to send, break out of loop
    }
    next_seqno_ += to_send.sequence_length();

    to_send.RST = input_.reader().has_error();
    transmit( to_send );               // send it out
    outstanding_.push_back( to_send ); // track that we sent it

    if ( !running_ ) {
      timer_ = 0;
      running_ = true;
    }
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage to_send {};
  to_send.RST = input_.reader().has_error();
  to_send.seqno = Wrap32::wrap( next_seqno_, isn_ );
  return to_send;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Receive the data
  window_size_ = msg.window_size;
  if ( msg.RST ) {
    input_.set_error();
  }
  if ( !msg.ackno.has_value() ) {
    return;
  }
  uint64_t acked = msg.ackno->unwrap( isn_, next_seqno_ );
  if ( acked > next_seqno_ ) { // if the ack is for something we haven't sent, ignore it.
    return;
  } else if ( acked <= last_acked_ ) { // if the ack is for something already acked, ignore
    return;                            // but keep window size?
  } // we have made progress w/ this seqno
  last_acked_ = acked;
  // reset timer
  timer_ = 0;
  cur_RTO_ms_ = initial_RTO_ms_;
  consecutive_retransmissions_ = 0;
  while ( !outstanding_.empty() ) {
    TCPSenderMessage front_seg = outstanding_.front();
    // Assess the seqnos at the start and end
    uint64_t seg_start = front_seg.seqno.unwrap( isn_, next_seqno_ );
    uint64_t seg_end = seg_start + front_seg.sequence_length();
    if ( seg_end <= last_acked_ ) {
      outstanding_.pop_front();
    } else {
      break;
    }
  }
  if ( outstanding_.empty() ) { // if nothing is waiting to push, turn off the timer
    running_ = false;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if ( !running_ ) {
    return;
  }
  timer_ += ms_since_last_tick;

  // check if timer expired and if there are outstanding segments.
  if ( timer_ >= cur_RTO_ms_ && !outstanding_.empty() ) {
    transmit( outstanding_.front() );
    if ( window_size_ > 0 ) {
      cur_RTO_ms_ *= 2; // exponential backoff
      consecutive_retransmissions_++;
    }
    timer_ = 0;
  }
}
