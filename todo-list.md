## the todo list
---

- audio decoding
  - maintain an internal buffer which will store output from stb vorbis
  - pipe that output into the stream
  - figure out how to do this efficiently "enough"
    - circular buffer with (say) 2048 samples
    - kickstart processes which will write to the end as we read from the start
    - (say 1024 sample "safe range?")
      - make a special thread safe class for it (something barebones)
      - allow the io to be spun off into another thread
      - storing like 3 seconds of float-encoded audio at 48Khz is 600KB or so
        which is a good chunk but not terrible for this purpose, alternatively
        we could keep it to like 1 second or less

- create a brief fastFFT library for shits
- transform a sample line based on sample input
  - first: offset vertices to represent sample data
  - second: transform the whole line while doing this
  - third: modify the offset direction programatically while transforming the line
  - fourth: break the line apart, do all sorts of wacky shit etc etc


- thread logic
  - callback is going to keep watch for a "low buffer size" warning and communicate it to thread.
  - create some mutex to ensure that we do not have any race conditions (if we're at the end of the buffer, wait around a bit, put some conditions on the lock so that it is associated with a particular size)
  - on the thread, we'll run in a brief loop and wait for a "low buffer size" signal as a means of repopulating the queue.
  - communication should have two channels: for instance, when we run out of "file" to read, we should send a message back to the callback thread notifying it that we can close the stream once the buffer runs dry.

  - put a lock on reading/altering the size of the buffer

  vorb manager should take care of the pa stream as well as the write thread.
  critical thread will therefore be sanctioned off and inaccessible, only to the buffer itself