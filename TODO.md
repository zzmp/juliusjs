-[X] Simulate multithreading
 -[X] `main_recognition_stream_loop`
 -[X] `j_recognize_stream`/`j_recognize_stream_core`
 -[X] `ad_go`/`ad_cut`
-[X] Make the ring/cycle buffer explicit in `adin_mic_webaudio.c`
-[X] <s>Move audio to `Sink.js`</s> Silence audio in ScriptProcessorNode
-[X] Move to Web Workers
 -[X] Remove option to terminate in `adin_mic_webaudio.c`
 1. Capture audio
  - _Main thread, as Web Audio is not available in worker context_
 1. Receive and recognize captured audio (`onmessage`)
  - _Worker thread_
  - _Pipe output to main thread (`postmessage`)_
   - [X] Capture sentences to send to callback (not just logging)
-[ ] Add dynamic volume normalization to audio
-[ ] Create use cases
 - Separate repos?
 -[ ] Magic words
