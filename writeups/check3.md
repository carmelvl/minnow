Checkpoint 3 Writeup
====================

My name: Carmel Limcaoco

My SUNet ID: cvlim

I collaborated with: aribarb, rliu25

I would like to thank/reward these classmates for their help: Rachel Liu, Ari Barbella

This checkpoint took me about [14] hours to do. I [did] attend the lab session.

Program Structure and Design of the TCPSender [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 

For my private variables, I defined a total of 11. next_seqno tracks the next absolute
sequence number to be assigned to an outgoing segment, and represents the sender's total
progress in sequence space. It is also the foundation for wrapping and unwrapping ack numbers.
syn_sent_ and fin_sent_ are booleans that encode whether the SYN and FIN control flags
have already been transmitted; these flags are essential because each consumes one byte
in sequence space and must only be sent once. Storing separately allows push() to cleanly
decide which phase of the connection it is in, avoiding the accidental resending of SYN or
the early sending of FIN before data's been delivered. window_size_ stores the receiver's
most recently advertised window from the latest message. I initialized it to 1 so that before
the first ack arrives, the sender can still send the SYN.
To track segments that were sent but not yet acknowledged, I used outstanding_ a first in, first out
deque ordered by send time, making it easy to retransmit the oldest outstanding segment
on timeout and to pop from the front when cumulative acks arrive. Using a deque rather than a vector
or list keeps both popping from the front and appending at the back O(1) which fits TCP's FIFO
semantics. 
last_acked_ records the highest absolute seqno cumulatively acknowledged so far, and is critical for 
detecting forward progress, filtering duplicate or out of order acks, and knowing when to reset
the retransmission timer. In the assignment spec, it was said that only acks that advance the cumulative
ack point reset the timer, thus this is a critical variable to maintain. 
The next few variables implement the timer. timer_ counts how many milliseconds have elapsed since the last
reset, and running_ indicates whether it's currently active, which should be true only when there is 
at least one segment outstanding. cur_RTO_ms_ stores the current RTO in ms, starting from the initial RTO and 
doubling on consecutive timeouts as required. Finally, consecutive_retransmissions_ counts how many timeouts 
have occurred in a row without new acknowledgements. This provides a way to detect persistent failure or 
congestion, and is part of the spec's diagnostic interface. 
Now, when I implemented push(), I broke it down in a couple of different ways. 
I realized that the sender would keep pushing as long as it was able to, or as long as there is space in the
receiver's window, so it enters a loop to produce multiple back to back segments. At the top of each iteration,
it creates a new TCPSenderMessage and assigns its sequo by wrapping the current absolute next_seqno_. It
then computes how much room is left to send by taking the window_size_ (and treating 0 as an effective window of 1). It
then subtracts the number of seqnos already in flight to get the available space for this iteration. 
The function handles SYN, or connection setup first - if SYN's never been sent, it sets SYN = true and syn_sent_ true after
this segment. Because SYN consumes one seqno, it reserves one byte from available space. 
Then, it considers payload. It assesses, from the reader, how many bytes are currently buffered, then chooses a payload
length that is the minimum of three quantities: the buffered bytes, the available window after accounting for SYN, and the
MAX_PAYLOAD_SIZE as required by the spec. It builds the payload by copying from the peek() view and commits the transfer
by popping that many bytes from the reader. 
After payload sizing, it checks whether or not it should attach FIN. The rule is that if the stream is finished, and 
FIN was not sent before, and there is at least one byte that can hold FIN, then we set FIN true and fin_sent_ true. 
Before transmitting, it lastly performs one last guard - if the segment has no SYN, payload, or FIN, there's nothing
meaningful to send, so it breaks the loop. Otherwise, it commits to the segment, sends it. It also mirrors any errors
by setting RST on the segment through set_error() on the reader side. 
It then transmits the segment via (transmit) and appends it to the outstanding_ deque. 
Finally, it initializes the timer_, if it's not already running. Then, the timer starts when the first outstanding
segment exists, with the clock set at 0. 

receive()
On receive(), I treat each inbound TCPReceiverMessage as the single source of truth for both window and progress.
I immediately record the new window_size_ so even duplicate or out-of-order ACKs can widen a previously tight
window. If the peer sets RST, I flip the ByteStream into error and return. If there is no ackno,
I also return since cumulative progress cannot be determined. Otherwise I unwrap the ackno against
isn_ using next_seqno_ as the checkpoint to get an absolute acked. ACKs that point beyond next_seqno_
are ignored as impossible. ACKs at or behind last_acked_ are ignored as duplicates. When acked is strictly ahead,
I make forward progress: set last_acked_ = acked, pop fully covered segments from the front of
outstanding_ by computing each segment’s absolute start and end and removing those with seg_end <=
acked, then reset the retransmission machinery to its optimistic state by zeroing timer_, restoring
cur_RTO_ms_ to the initial RTO, and clearing consecutive_retransmissions_. If popping empties the deque,
I turn the timer off.

Implementation Challenges:
I initially struggled with correctly handling the FIN flag and window edge cases. Initially, I attempted to send
FIN immediately after reading EOF from the stream without checking the available window. This caused tests to fail when
the window was full, because even if the stream had ended, FIN still occupies one byte in sequence space. I fixed
this by adding the explicit space > 0 condition. A related challenge was properly updating fin_sent_ - originally I 
toggled it too early, before confirming the segment actually got sent, which caused later pushes to skip  FIN even
though it hadn't left yet. I moved the update to occur only after transmission, which solved it. This suite of problems - 
related to the semi-finnicky ordering of updating flags and checking information, was a problem area for a while. 
I also ran into a bunch of off by one errors with wrapping sequence numbers, especially around the SYN and FIN bytes, since both
occupy a seqno.

The timer definitely also posed a challenge to me. Early on, my implementation reset the timer after any ack, even duplicates, making
the timeout tests fail because the sender never doubled the RTO. I then realized that the timer should reset only when an ack makes
forward progress, and adjusted the condition accordingly.

Lastly, there was some trial and error in play with implementing push() in a loop - initially, I didn't consider that it should
push in a loop if there was space to accommodate. I adjusted this by adding the while(true) loop and breaking on the condition
that there was not enough space. This was one of my last big hurdles (the last being forgetting to accommodate for RST) but after
I ironed those two out, all my tests passed :')

RESPONSES FOR 4.2 - Sending info to a friend

Sizes of two files: 
• Both were 1,000,000 bytes (as created)

Sender output:
-rw-rw-r-- 1 cs144 cs144 1000000 Oct 22 21:33 /tmp/big.txt 

Receiver output:
-rw-rw-r-- 1 cs144 cs144 1000000 Oct 22 21:35 /tmp/big-received.txt

Results:

sha256sum /tmp/big.txt
• 7a18b716189ae527fb1a7c0e5194336327dbc669be55f7d07b57905d7a1d9895  /tmp/big.txt

sha256sum /tmp/big-received.txt
• 7a18b716189ae527fb1a7c0e5194336327dbc669be55f7d07b57905d7a1d9895  /tmp/big-received.txt

Remaining Bugs:
[]

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

  ChatGPT was useful in helping me think about breaking down the task into the different sections. I used the same prompt from last time (you are an expert TA, never send me code, etc.) and asked it clarifying questions the way I would a TA in office hours. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [submit as GitHub PR
  and include URL here]
