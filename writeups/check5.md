Checkpoint 5 Writeup
====================

My name: Carmel Limcaoco

My SUNet ID: cvlim

I collaborated with: aribarb, rliu25

I would like to thank/reward these classmates for their help: aribarb, rliu25

This checkpoint took me about [8] hours to do. I [did not] attend the lab session.

Program Structure and Design of the NetworkInterface [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]:

My implementation ended up being pretty simple. I used a few maps to keep
track of everything the interface needs: one map for the ARP cache, one for the
expiration times of each entry, another for any datagrams that are waiting on ARP resolution,
and one that records the last time I sent an ARP request for a given IP. This gave me
just enough state to match the expected behavior without overcomplicating things.

In send_datagram(), I check whether I already have a valid MAC address for the next hop.
If I do, I can just wrap the IPv4 datagram in an Ethernet frame and send it. If
I don’t, I queue the datagram and, if I haven’t sent an ARP request for that IP in
the last five seconds, I broadcast a new one. The receive side is similar in spirit.
If I get an IPv4 frame addressed to me or broadcast, I parse it and push it up. If it’s
an ARP message, I learn the sender’s IP-to-MAC mapping, reply immediately if they are
asking for my IP, and then check whether I have anything queued for that IP. I only
flush the queue if the ARP message arrived within five seconds of my last request;
otherwise I treat the waiting datagrams as stale and clear them. The tick() function
increments time, expires old ARP entries, and cleans up any empty queues that are
past their waiting window.

I briefly thought about combining the ARP data into a single struct instead of splitting
it across multiple maps, but keeping them separate made the code easier for me to read
and debug. The design isn’t necessarily the most efficient thing in the world, but
it’s easy to reason about and does exactly what the tests expect.

Implementation Challenges:
The biggest initial hiccup was understanding how the parsing worked.
I first tried to compare the result of parse() against a ParseResult,
but you’re supposed to create a Parser, pass it into parse(),
and then check p.has_error(). Once I switched, everything made
more sense. Another challenge was getting the ARP ordering correct. The tests
expect me to send the ARP reply right away and only then think about flushing
queued datagrams. Finally, handling “late” ARP replies required extra care;
I had to make sure I didn’t send queued datagrams if the ARP response arrived
more than five seconds after my request.

Remaining Bugs:
N/A

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

As in similar assignments, Chat helped me conceptually understand intricacies
of the assignment by clarifying concepts from class to fill in gaps.
I asked it questions like "in complete detail, how does the network
interface sit between IP and Ethernet, and how does it decide when to
actually send an ARP request?" As always, I kept my initial prompt setting it up
to be a "CS 144 head TA" and to "never send me any code, only explain conceptually,
and at worst only send English language-based pseudocode."

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
