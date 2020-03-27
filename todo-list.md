## the todo list
---

- AudioBuffer is locked up -- no longer necessary to provide locks
- All calls, from either the manager or the readonly struct, are contained.

- the next step, then, is our vorbismanager
- check the notes you made yesterday on it :)

- track buffers w/ weak pointers still - no longer necessary to track locks as well!
- the write thread should make calls to "skip" on sub-buffers if necessary -- will need to grab the lock in this case

note on signaling:

- the write thread is started by the VM thread. a bit is flipped to confirm that the thread is moving.
- from here, the write thread is detached.
- the VM spins while the write thread sets up, waiting on a passed atomic flag (call it "thread_signal" or something like that)
- once that flag is cleared, VM has the go-ahead to allocate a TimeInfo* object. this ensures that we can get a somewhat more accurate representation of the current TimeInfo* state.

- from here, the write thread does its thing, reading from vorbis and writing that data to the thread.

- "thread_signal" communicates from the write thread to the VM thread. We should have a second means of communication, "VM_signal," which the VM can use to communicate to the thread.

- If this signal is ticked, or if the vorbis file runs dry, we break out of the `while` loop and perform the following actions:

- a third signal should exist through which the write thread can communicate with the portaudio playback. If we intend to play our audio to completion, we must ensure that the critical buffer is emptied completely (as it will be full once we write to it -- terminating early would be bad!)

- here, the write thread spins and waits for the callback to run the buffer dry. Once it does, it sends a signal to the buffer.
  - case to watch out for: what if the buffer empties prematurely?
    - we could have the write thread check the flag on each loop for diagnostic reasons, just in case something goes awry
    - would be especially good for denoting errors if we want to log that in the future!

- once the signal is genuinely received, the write thread stops the PA stream and terminates the PA instance. lastly, we clear our "thread_signal" flag to notify the VM thread that our operation is complete.
  - how do we avoid confusion between a start signal and an end signal?
  - if the thread is starting up, we prevent it from being stopped. however, we must also ensure that we do not start up multiple threads.
  - VM must track two factors of the thread state: 
    - whether the thread has been spun up, and
    - whether it is currently running
  - calls to StartWriteThread / StopWriteThread will be shut down if exactly one of these is true.
  - however, two calls to start could occur before the first flag is raised.
  - alternatively, since VM functions will not be called by multiple threads (only the buffers will be divvied out) we can be assured by the fact that start/stop or start/start will not be called successively
- consistent state:
  - our StartWriteThread will always intercept the first signal. However the second one may be missed if StopWriteThread is not called
  - therefore, Start/Stop should check for the signal and act accordingly.
  - Start: clear it, and set the thread running state to false. continue starting up the thread again
  - Stop: same thing. then do nothing.
  - also: add a "IsThreadRunning" function which will return whether or not the flag was cleared. Again, the first clear is intercepted by StartWriteThread, so if it is raised again, the thread has been stopped.