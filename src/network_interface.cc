#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_(name)
  , port_(notnull( "OutputPort", move(port)))
  , ethernet_address_(ethernet_address)
  , ip_address_(ip_address)
  , datagrams_received_()
  , arp_address_map_()
  , arp_expire_time_()
  , waiting_queue_()
  , last_arp_request_time_()
  , now_ms_(0)
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ )
       << " and IP address " << ip_address.ip() << "\n";
}

// send one IP datagram toward next_hop by Ethernet
// if we do not know the MAC, queue and ARP at most once per 5s per IP
void NetworkInterface::send_datagram(InternetDatagram datagram, const Address& next_hop)
{
  const uint32_t next_hop_ip = next_hop.ipv4_numeric();
  // case 1: already have a valid mapping
  if (arp_address_map_.count(next_hop_ip) && arp_expire_time_.count(next_hop_ip) && arp_expire_time_.at(next_hop_ip) > now_ms_) {
    EthernetFrame frame_out;
    frame_out.header.src  = ethernet_address_;
    frame_out.header.dst  = arp_address_map_.at(next_hop_ip);
    frame_out.header.type = EthernetHeader::TYPE_IPv4;
    frame_out.payload     = serialize(datagram);
    transmit(frame_out);
    return;
  }
  // case 2: queue and maybe send ARP
  bool should_send_arp = false;
  if (!last_arp_request_time_.count(next_hop_ip) ||
      now_ms_ >= last_arp_request_time_.at(next_hop_ip) + ARP_HOLD_MS ) {
    waiting_queue_[next_hop_ip].clear(); // clear wait queue
    should_send_arp = true;
  }
  waiting_queue_[next_hop_ip].push_back(std::move(datagram));
  if (should_send_arp) {
    ARPMessage arp_msg;
    arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
    arp_msg.sender_ethernet_address = ethernet_address_;
    arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
    arp_msg.target_ethernet_address = {};
    arp_msg.target_ip_address = next_hop_ip;
    EthernetFrame frame_out;
    frame_out.header.src = ethernet_address_;
    frame_out.header.dst = ETHERNET_BROADCAST;
    frame_out.header.type = EthernetHeader::TYPE_ARP;
    frame_out.payload = serialize(arp_msg);
    transmit(frame_out);
    last_arp_request_time_[next_hop_ip] = now_ms_;
  }
}

void NetworkInterface::recv_frame(EthernetFrame frame)
{
  if (frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST) {
    return; // not for us
  }
  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
    InternetDatagram datagram_in;
    Parser p(frame.payload);
    datagram_in.parse(p);
    if (!p.has_error()) {
      datagrams_received_.push(std::move(datagram_in));
    }
    return;
  }
  if (frame.header.type == EthernetHeader::TYPE_ARP) {
    ARPMessage arp_msg;
    Parser p(frame.payload);
    arp_msg.parse(p);
    if (p.has_error()) {
      return;
    }
    const uint32_t sender_ip = arp_msg.sender_ip_address;
    const EthernetAddress sender_mac_address = arp_msg.sender_ethernet_address;
    if (arp_msg.opcode == ARPMessage::OPCODE_REQUEST && arp_msg.target_ip_address == ip_address_.ipv4_numeric() ) { // if request is for our IP, reply first
      ARPMessage reply_msg;
      reply_msg.opcode = ARPMessage::OPCODE_REPLY;
      reply_msg.sender_ethernet_address = ethernet_address_;
      reply_msg.sender_ip_address = ip_address_.ipv4_numeric();
      reply_msg.target_ethernet_address = arp_msg.sender_ethernet_address;
      reply_msg.target_ip_address = arp_msg.sender_ip_address;
      EthernetFrame frame_out;
      frame_out.header.src = ethernet_address_;
      frame_out.header.dst = arp_msg.sender_ethernet_address;
      frame_out.header.type = EthernetHeader::TYPE_ARP;
      frame_out.payload = serialize(reply_msg);
      transmit(frame_out);
    }
    arp_address_map_[sender_ip] = sender_mac_address;
    arp_expire_time_[sender_ip] = now_ms_ + ARP_TTL_MS; 
    if (waiting_queue_.count(sender_ip) && !waiting_queue_.at(sender_ip).empty()) { // if we were waiting on this IP
      const bool within_window = last_arp_request_time_.count(sender_ip) && now_ms_ < last_arp_request_time_.at(sender_ip) + ARP_HOLD_MS;

      if (within_window) {
        for (const auto& d: waiting_queue_.at(sender_ip)) {
          EthernetFrame frame_out;
          frame_out.header.src = ethernet_address_;
          frame_out.header.dst = sender_mac_address;
          frame_out.header.type = EthernetHeader::TYPE_IPv4;
          frame_out.payload = serialize(d);
          transmit(frame_out);
        }
      }
      waiting_queue_.erase(sender_ip); // clear either way to avoid dupe sends
    }
    return;
  }
}

void NetworkInterface::tick(const size_t ms_since_last_tick)
{
  now_ms_ += ms_since_last_tick; // advance
  vector<uint32_t> expired_ips; // expire ARP entries
  for (const auto& kv: arp_expire_time_) {
    if (kv.second <= now_ms_) expired_ips.push_back(kv.first);
  }
  for (const auto ip_to_remove: expired_ips) {
    arp_expire_time_.erase(ip_to_remove);
    arp_address_map_.erase(ip_to_remove);
  }
  vector<uint32_t> ips_to_clear; // drop empty wait lists after 5s cycle
  for (const auto& kv: waiting_queue_) {
    const uint32_t target_ip = kv.first;
    if (last_arp_request_time_.count(target_ip) && now_ms_ >= last_arp_request_time_.at(target_ip) + ARP_HOLD_MS && kv.second.empty()) {
      ips_to_clear.push_back(target_ip);
    }
  }
  for (const auto ip_to_remove: ips_to_clear) {
    waiting_queue_.erase(ip_to_remove);
  }
}
