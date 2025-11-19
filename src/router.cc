#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.

// adds a route to the routing table.
// add data structure as private member in the Router class to store this info.
// this method needs to save the route for later use.

// PARTS OF A ROUTE:
// "match action" rule - tells router, if a datagram is headed for a particular network
// and if route is chosen as most specific route, then router should forward datagram to
// particular next hop on a particular interface

// MATCH: is it headed for this network?
// route_prefix & prefix_length specify range of IP addys, a network. that might include dest.
// route_prefix - 32 bit numeric IP address
// prefix_length is # between 0 and 32, inclusive, tells router how many bits are significant
// ACTION: what to do if the route matches, and is chosen

// interface_num: index of the router’s NetworkInterface that should use to send the datagram to the
// next hop. You can access this interface with the interface(interface num)
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  RouteEntry new_entry = { route_prefix, prefix_length, next_hop, interface_num };
  routing_table_.push_back( new_entry );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  for ( size_t i = 0; i < interfaces_.size(); i++ ) {         // go through all the interfaces.
    while ( !interfaces_[i]->datagrams_received().empty() ) { // while there are still datagrams here
      auto& queue = interfaces_[i]->datagrams_received();
      InternetDatagram cur = queue.front();
      queue.pop();
      // edge: if TTL is 0 or is 1 away from 0, drop the datagram
      if ( cur.header.ttl <= 1 ) {
        continue; // drop datagram
      }
      cur.header.ttl--;
      cur.header.compute_checksum();
      uint32_t cur_dest = cur.header.dst;
      int max_length = -1; // find a match!!!
      int best_index = -1; // track idx of best entry
      for ( size_t j = 0; j < routing_table_.size(); j++ ) {
        const RouteEntry& entry = routing_table_[j];
        uint32_t mask;
        if ( entry.prefix_length == 0 ) {
          mask = 0;
        } else {
          mask = 0xFFFFFFFFu << ( 32 - entry.prefix_length );
        }
        uint32_t masked_dst = cur_dest & mask;           // mask the cur dst
        uint32_t masked_rte = entry.route_prefix & mask; // mask the intended route
        if ( masked_dst == masked_rte && entry.prefix_length > max_length ) {
          max_length = entry.prefix_length; // update max length
          best_index = j;
        }
      }
      if ( best_index == -1 ) { // if there was no matched route, drop
        continue;
      }
      const RouteEntry& best_entry = routing_table_[best_index];
      // decide the next hop address
      Address next_hop_addr
        = best_entry.next_hop.has_value() ? best_entry.next_hop.value() : Address::from_ipv4_numeric( cur_dest );
      interface( best_entry.interface_num )->send_datagram( cur, next_hop_addr );
    }
  }
}
