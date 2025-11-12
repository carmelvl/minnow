#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

class NetworkInterface
{
public:
  class OutputPort
  {
  public:
    virtual void transmit( const NetworkInterface& sender, const EthernetFrame& frame ) = 0;
    virtual ~OutputPort() = default;
  };

  NetworkInterface( std::string_view name,
                    std::shared_ptr<OutputPort> port,
                    const EthernetAddress& ethernet_address,
                    const Address& ip_address );

  void send_datagram( InternetDatagram dgram, const Address& next_hop );
  void recv_frame( EthernetFrame frame );
  void tick( size_t ms_since_last_tick );

  const std::string& name() const { return name_; }
  const OutputPort& output() const { return *port_; }
  OutputPort& output() { return *port_; }
  std::queue<InternetDatagram>& datagrams_received() { return datagrams_received_; }

private:
  std::string name_;
  std::shared_ptr<OutputPort> port_;
  void transmit( const EthernetFrame& frame ) const { port_->transmit( *this, frame ); }

  EthernetAddress ethernet_address_;
  Address ip_address_;

  std::queue<InternetDatagram> datagrams_received_ {};

  std::unordered_map<uint32_t, EthernetAddress> arp_address_map_;
  std::unordered_map<uint32_t, size_t> arp_expire_time_;
  std::unordered_map<uint32_t, std::vector<InternetDatagram>> waiting_queue_;
  std::unordered_map<uint32_t, size_t> last_arp_request_time_;
  size_t now_ms_ { 0 };
  static constexpr size_t ARP_TTL_MS  = 30000;
  static constexpr size_t ARP_HOLD_MS = 5000;
};
