Checkpoint 1 Writeup
====================

My name: Carmel Limcaoco

My SUNet ID: cvlim

I collaborated with: aribarb, rliu25

I would like to thank/reward these classmates for their help: rliu25, aribarb

This lab took me about [8] hours to do. I [did] attend the lab session.

I was surprised by or edified to learn that: [the implementation could be as simple and clean as it is!]

Report from the hands-on component of the lab checkpoint: [include
information from 2.1(4), and report on your experience in 2.2]

The average round trip delay between my and Rachel's VMs 20.502 ms. 
After 1100 pings, the delivery rate was 100%, and the loss rate was 0%.
I did not see any duplicated datagrams. 
We then compared the same datagram in Wireshark, one that she had sent to me.
We noticed that the only two fields that changed were TTL and checksum - TTL 
had decremented from 64 to 63, because one router hop occurred for this datagram
to get to me. The header checksum has to then change to reflect this change.
It was cool to see this in action!

Describe Reassembler structure and design. 

For my reassembler, I ended up using a map of chars. The way it works is that
it takes in an input string into insert.
If it's the last string (because of the bool), it calculates the last index,
using the first index of that string and its length. Then, it saves this as a
global variable to be preserved.
Then, I calculate the next_needed index - the index at which I want to fill the
next part of the bytestream. I also calculate my overall capacity by adding
the next_needed - empty reassembler - and the capacity of the bytestream that is
unpopped. I then go in and process my data string by adding each of its chars
to my map at its respective index.
Then, if the string I just received is able to be added (first index is in capacity), I process it
and add each of its chars to my map at the appropriate index. I skip it if it's already
in the map, and then if the index goes above capacity as I iterate, I stop.
The last part then iterates from the value of next_needed through the map.
Starting from the value with the next_needed key (the next char to be added to
the bytestream), I push data from the map until there is no key value pair directly
after the one I am on. This way, I push a contiguous block of data until it's no longer
contiguous. I delete the pairs I push as I go along.
Finally, once I've reached the last index that I initially saved - meaning I'ved pushed
as many bytes as are in the whole bytestream - I close the stream. Voila.

For my first attempt, I tried a vector of strings, and then after lab, learned
that this was not a good approach because it would cause overlapping strings to 
be stored. Then, I spent a long time trying to make a string work as the 
reassembler data structure, because operations on it char-wise are idempotent. I then realized
this wasn't that efficient because in order to store substrings at high indices - 
for example, if I had a string "a" at 0, and then a string "potato" at 1029101, I
would have to somehow represent all that blank space in the string and also resize it
every single time. This is costly and inefficient. However, I liked the idea that
chars in a string were idempotent and easily accessible via index.
So, I turned to a map. I decided on a map because a map has a flexible size, unlike
a string, which has to have a predetermined size that you then have to manually
change with every addition of an element beyond the size, which would have been 
more costly.
I first tried using a map of substrings, which I actually implemented almost all the 
way to the end before feeling like it was getting really complicated, especially when 
some of the more nitpicky tests came up. For example, if I had received 
the string abc, and had next_needed be 3, then later on, received 
bcde, I realized I would have to go in using next_needed and somehow identify if I had
the letter at that index number by inspecting nearby substrings, as the next_needed
index wasn't necessarily the first_index of any given input string.
Another annoying thing about the substring method was that if two of them overlapped,
I had an overcomplicated merge section that would check if the added substring had 
to be merged with the previous one. I realized Keith would definitely call me dumb, not in
a good way, for doing this, because it was processing my final string implicitly over
and over again, rather than in one fell swoop at the end.
So, I implemented everything I had already done with a map of chars. Instead of 
the merging thing, I just iterated over every inputted data substring, and put each of the
characters in a the map. Then, I pushed everything from my next_needed index to the data
stream after it was all said and done. The resulting design is simple and quick, and easy to
understand - first, you process your input string, then go through your database of chars, and
push them out to the datastream in order until you don't have the next one ready.
This wasn't too difficult to do given I had basically done everything required for it logically.
So I was elated when it passed. 

[Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

Implementation Challenges:
I discussed most of my implementation challenges above!! As I said above,
navigating between all the different options and truly giving each of them
a genuine go was time consuming, especially when I realized where one's shortcomings
were solved by another. Once I realized which data structure was best fit for the job,
I actually didn't have any significant implementation challenges- but before that,
figuring out which was the best was the biggest challenge in itself, in my opinion.

Remaining Bugs:
N/A

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]
    I asked Chat to help with writing debugging code to make it a little faster, like "write me a print statement in C++" or
    "can you give me a for loop to print out entries in a map in c++". Nothing towards the actual function of the code, just
    to see the variables in my code as I went along.

- Optional: I had unexpected difficulty with: [N/A]

- Optional: I think you could make this lab better by: [N/A]

- Optional: I'm not sure about: [N/A]
