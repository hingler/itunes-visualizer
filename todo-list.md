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

- create a library which will accept audio input and convert it to samples
- create a brief fastFFT library for shits
- transform a sample line based on sample input
  - first: offset vertices to represent sample data
  - second: transform the whole line while doing this
  - third: modify the offset direction programatically while transforming the line
  - fourth: break the line apart, do all sorts of wacky shit etc etc
