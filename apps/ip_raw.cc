#include "socket.hh"

using namespace std;

int main()
{
  // // Internet Datagram
  // string d;
  // d += 0b0100'0101;
  // d += string( 7, 0 );

  // d += 64;
  // d += 5; // protocol : 5
  // d += string(6, 0); // padding

  // // 10.144.0.94
  // d += 10; // dest address
  // d += 144u;
  // d += string( 1, 0 );
  // d += 94u;

  // d += "rawr type";

  // RawSocket {}.send( d1, Address { "10.144.0.94"} );

  // User Datagram
  string d;
  d += 0b0100'0101;
  d += string( 7, 0 );

  d += 64; // time to live
  d += 17; // protocol : 17

  d += string( 6, 0 ); // padding

  // 10.144.0.94
  d += 10; // dest address
  d += 144u;
  d += string( 1, 0 );
  d += 94u;

  d += 4; // source port
  d += 1;

  d += 4; // destination port (user id)
  d += char( 0 );

  string message = "rawr type.";

  d += char( 0 );
  d += char( message.length() + 8 );

  d += string( 2, 0 );

  d += message;

  RawSocket {}.send( d, Address { "10.144.0.94" } );
}
