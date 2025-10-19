Checkpoint 2 Writeup
====================

My name: Carmel Limcaoco

My SUNet ID: cvlim

I collaborated with: Ari Barbella, Rachel Liu

I would like to thank/reward these classmates for their help: N/A

This lab took me about [6] hours to do. I [did] attend the lab session.

Describe Wrap32 and TCPReceiver structure and design. 

For Wrap32, the way the wrap function works is that it takes in a uint64_t and wraps
it into the 32 bit "universe," by completing the modulo operation with 2^32 and returning
the remainder. Unwrap was more complex because you're essentially doing the opposite,
meaning that this "remainder" can exist in any of the 2^32 universes. So, the way
I implemented this was first calculating the offset of the uint32_t value from the zero_point
by finding the difference and then modding it by 2^32 just to get the remainder value 
I'm looking for. Then, I placed this value in the base universe (the one we were currently in,
aka closest to the checkpoint), by finding the universe the checkpoint was in by dividing and then
multiplying it by 2^32 (this eliminates the lower 32 bits). I then add the offset to this.

I then compared it to one degree of 2^32 above and below. These are my three
candidates to inspect (including the base universe). So, I defined the three potential index
numbers by adding and subtracting 2^32. I then calculated the differences in distances and 
then compared them to each other one by one while tracking which distance was the smallest.
Then, I return the index number corresponding to the smallest difference.

For the TCPReceiver, let's start with receive(). First, I handled the RST flag, by setting an error
via the reader (can't through the writer, because it's const) on the ByteStream. 
Next, I handled the SYN flag. If the flag was true, I then set my zero point (a private var I created)
to be the seqno of the message. 
Then, if there was no zero point set, I would just return, because I cannot push anything to the 
reassembler if I don't know where it's even intended to be.

Then, I needed to calculate the values to input into my insert() function for the reassembler. First,
I needed checkpoint, the value that tells me where in the ByteStream we are going to be. For this,
I used bytes_pushed(), because that is literally where we are in the stream. 
Next, I needed to calculate first_index. The diagram in the assignment spec was useful for this because
of the different schemes for index numbers, i.e. seqno vs absolute seqno vs stream index.
First I calculated the seqno by unwrapping my value. Then, if it was the first (SYN on) I added 1 to the
index because the data starts one later. Then, this gives me my absolute seqno, which I then had to
convert to the stream index number by subtracting 1.

I then preserved my data and is_last_substring vars from both payload and FIN. Then, I push it to the
reassembler.

For send, I also broke down the message by component.
Firstly, I handled RST by just checking if the reassembler reader or writer had an error.
Then, I set ackno by first checking if there was a value for zero_point established, because otherwise 
I can't have a valid ackno. Then, I got my ackno first version by getting where we were in the bytestream,
which is bytes_pushed() + 1. Then, I handle the case where the bytestream was closed, meaning we received the
FIN flag and therefore we need to add another +1. Lastly, I wrap this as a Wrap32 using the function I wrote
previously, using the ISN I have saved as a global var.

Next, for window size, I got the writer's available capacity, and then capped it at the max - UINT16_MAX.
I then cast it back to a uint16_t and put it in my message. 
That wraps up (haha get it) the three components of my TCP sender message.

Implementation Challenges:
A challenge I faced in writing unwrap was underflow, because when I would subtract uint64_ts,
if it underflowed it would result in an extremely large number due to how unsigned
arithmetic wraps around instead of going negative. Unsigned integers can’t represent negative values
so an operation like 90 - 100 doesn’t give -10, it actually wraps around to 2^64 - 10, which breaks
distance comparisons. To fix this, I added a conditional that checks which value is greater before 
performing the subtraction. This may have resulted in my code being longer, but it was worthwhile
to make sure my comparisons were correct.

Another challenge I faced in implementing receive was figuring out the index number conversions
between seqno, absolute seqno and stream index. It took me a bit of time to figure out where 
the -1 subtraction went for the last conversion because I didn't fully understand the differences.

Lastly, my solution kept timing out even after I passed most of my tests, and I realized this was because
my reassembler implementation was not efficient - that had almost not passed its own tests in the first place.
I then implemented the reassembler walked through during lab session to update my overall cumulative implementation
to make sure that in the future, my building blocks stack upon each other nicely and don't hinder
the efficacy of future blocks. After doing this, my solution passed quickly.

Remaining Bugs:
N/A 

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]
    Chat helped me conceptually understand the differences between the index numbers, as well as 
    the use of optional global variables. I also had a long prompt that explicitly said not
    to show any code, which was:

    you are my expert teaching assistant, but you will never tell me explicitly how to do it, just be a good teacher, don't show me how to do it either (vague pseudocode is sometimes but not usually valid, but nothing else, and DEFINITELY no real code.)
    
    Other prompts I used to clarify were:

    conceptually, if my tcp receiver is sending a message, can it create an ackno if there is no ISN established?
    conceptually, can you explain the difference between seqno, absolute seqno, and index number in TCP?
    can you explain underflow with uint64_ts and subtraction?

  
- Optional: I had unexpected difficulty with: mathematical operations. and I found it hard sometimes to think about how to use
  pieces from check 1 and check 0 in this assignment. 

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [submit as GitHub PR
  and include URL here]
