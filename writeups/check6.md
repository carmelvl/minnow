Checkpoint 6 Writeup
====================

My name: Carmel Limcaoco

My SUNet ID: cvlim

I collaborated with: rliu25, aribarb

I would like to thank/reward these classmates for their help: rliu25, aribarb

This checkpoint took me about [4] hours to do. I [did not] attend the lab session.

Program Structure and Design of the Router [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 

For my router, the data structure I chose to use was a vector of RouteEntry
structs. The RouteEntry struct I defined to have four fields which were the
input fields of the add_route() function- specifically, the route_prefix, the prefix_length, the next_hop, and the interface_num. Implementing add_route was simple - I just made a new RouteEntry struct, filled in all of the fields with the input information, and added it to my vector.

For route(), I firstly iterated over all of the interfaces I had stored in my 
interfaces_ vector. Then, I posed a while loop that continued while there 
were still queued datagrams. For each datagram, I first checked the TTL, and
if it was less than or equal to 1, I dropped the datagram, and then adjusted
TTL, decrementing by one and then computed the checksum of the header again
to update it overall.

Then comes the logic for finding the route match. I initialized max_length 
at -1 to ensure a length of 0 could still be accommodated to send to the 
default route, which is when prefix_length = 0. Then, I initialized a var
best_index at -1 to track the idx of which entry has the best match. Then
I iterate over the whole routing table and for each entry, create a bitmask
to capture the most significant bits of the route using the prefix_length 
value. I then apply this mask to both the destination of the datagram
and the route of the RouteEntry. Then, I check if they are the same, and if 
the current prefix length is larger than the max_length value that I have 
saved. If it is, I update max_length and best_index to match the values for
this specific RouteEntry. 

Then, I check if best_index was ever changed; if it was still -1, I 
skip this datagram because it means there's no match. 

Finally, I grab the best entry from the routing table using the best_index
value that I had saved. From this I grab the next hop address, which is the
next_hop value of the pulled RouteEntry. If it's nullopt, that means the
network is directly attached, so in this case the next_hop will be the
final destination address itself.

Lastly, I send off the datagram using the appropriate outgoing interface!

Another option I considered which would probably have been faster, but I didn't
have enough time to figure out was to store the routes in 33 separate buckets,
one for each possible prefix length from 0 through 32. During routing,
the router would check the bucket for the longest prefixes first
and work downward until it finds a match. This structure naturally
enforces longest-prefix-match and would work faster than a single flat
vector, since the router only scans the small set of routes that share
the same prefix length instead of searching through the entire table.

Although that bucketed design would likely have been faster, I realized it a bit late,
and ultimately chose not to implement it because the routing tables
in this lab are small enough that a single flat vector is sufficient. Given the scope
of the assignment and my time limit, simplicity and readability mattered
more than optimizing for scale, and the straightforward vector-based approach
was easier to implement, debug, and reason about.

Implementation Challenges:

A challenge I faced was making sure that the edge case of sending to the 
default route was handled correctly. I initially set max_length to 0, which 
made it impossible for a /0 route to ever be selected, so I had to change the
initial value to −1. That bug took me a second to track down because
everything else looked correct. I also struggled a bit with the syntax
and layering of the interface tools, since there are several abstractions
in play.

Another challenge was working through the longest-prefix-match logic itself.
I had to verify that my mask computation, signed comparisons, and prefix-length
checks were all behaving as expected. Debugging this involved printing
intermediate masked values to confirm that matches were actually occurring.
I also had to remember to correctly decrement the TTL and recompute the
checksum before forwarding, because forgetting either of those caused
packets to be dropped silently during the test harness.

Overall, the biggest difficulty was keeping track of the order of operations
in the routing loop: draining the interface queues, updating the header,
finding the best route, and then forwarding through the correct network interface.


Remaining Bugs:
[]

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
