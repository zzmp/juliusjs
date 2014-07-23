-[X] Simulate multithreading
 -[X] `main_recognition_stream_loop`
 -[X] `j_recognize_stream`/`j_recognize_stream_core`
 -[X] `ad_go`/`ad_cut`
-[X] Make the ring/cycle buffer explicit in `adin_mic_webaudio.c`
-[ ] Move audio to `Sink.js`
-[ ] Hardcode sane options in `julius/main.c`
 -[X] Suppress output through `j_log`
-[ ] Move to Web Workers
 -[ ] Remove option to terminate in `adin_mic_webaudio.c`
 1. Capture audio
 1. Receive and recognize captured audio (`onmessage`)
  - Pipe output to main thread (`postmessage`)
-[ ] Add dynamic volume normalization to audio Worker
-[ ] Create use cases
 - Separate repos?
 -[ ] Magic words

