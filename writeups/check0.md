Checkpoint 0 Writeup
====================

My name: Carmel Limcaoco

My SUNet ID: cvlim

I collaborated with: Rachel Liu, Ari Barbella

I would like to credit/thank these classmates for their help: Rachel Liu, Ari Barbella

This lab took me about [3] hours to do. I did attend the lab session.

My secret code from section 2.1 was: 498009

I was surprised by or edified to learn that: Bytestreams can be implemented extremely simply and elegantly! I didn't expect using a string to work.

Describe ByteStream implementation. 

I used a string as my bytestream, which made my implementation super simple.
This was actually my initial idea, but then during lab I thought Keith called it dumb (haha).
So I considered a couple other designs, firstly a queue, then a buffer or vector of chars. 
I realized a queue wasn't optimal because you can't see a contiguous block which was
a requirement of the peek() function. For a buffer or vector of chars, this felt a little
too convoluted - and honestly I wanted to avoid using using pointers or overcomplicating with
index numbers. I think my solution is elegant and very simple as compared to these other 
alternatives, and because of that, is able to run quite quickly. It also was pretty quick to implement.

Implementation Challenges:
Had an issue with a minor edge case, but that's about it.

Remaining Bugs:
N/A

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: [provide Pull Request URL]