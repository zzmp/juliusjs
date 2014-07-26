 // ### Expose AudioContext/Nodes
 // AudioContext nodemap: `source` -> `processor` -> `destination`
var audio = {
  // `AudioContext`
  context:   null,
  // `AudioSourceNode` from captured microphone input
  source:    null,
  // `ScriptProcessorNode` for julius
  processor: null,
  // Set to `true` to propogate audio buffers through `processor`
  // _if not set, an empty buffer will come out of the processor_
  transfer:  false
};

/**
 * ### Bootstrap juliusjs
 *
 * - `onrecog`: a function called when juliusjs recognizes a sentence
 *  - `function(sentence)`, where `sentence` is the recognized string
 * - `onfail`: a function called when juliusjs fails
 *  - `function(err)`, where `err` is an error message, if available
 */
var juliusjs = (function(AudioContext, getUserMedia, audio) {
  var context = audio.context = new AudioContext();
  var processor = audio.processor = context.createScriptProcessor(4096, 1, 1);

  // #### Julius recognizer
  // _offloaded to a separate thread to avoid slowing UI_
  var julius = new Worker('worker.js');

  var terminate = function(cb) {
    processor.onaudioprocess = null;
    julius.terminate();
    if (typeof cb === 'function')
      cb();
  };

  julius.onmessage = function(e) {
    if (e.data.type === 'begin') {
      processor.onaudioprocess = function(e) {
        var buffer = e.inputBuffer.getChannelData(0);
        
        if (audio.transfer) {
          var out = e.outputBuffer.getChannelData(0);

          for (var i = 0; i < 4096; i++) {
            out[i] = buffer[i];
          }
        }

        // Transfer audio to julius
        julius.postMessage(buffer);
      };

    } else if (e.data.type === 'error') {
      terminate();

    } else if (e.data.type === 'recog') {
      onrecog(e.data.sentence);

    } else if (e.data.error === true) {
      console.error(e.data);
      terminate();

    } else {
      console.info('Unexpected data received from julius:');
      console.info(e.data);
    }
  };

  return function(onrecog, onfail) {
    if (typeof onrecog !== 'function')
      throw Error('juliusjs: first argument not optional, must be a function');

    if (typeof onfail !== 'function')
      onfail = function(err) { /* noop */ };

    getUserMedia(
      { audio: true },
      function(stream) {
        audio.source = context.createMediaStreamSource(stream);
        audio.source.connect(processor);
        processor.connect(context.destination);

        // Bootstrap the recognizer
        julius.postMessage('begin');
      },
      function(err) {
        terminate(onfail.bind(null, err));
      }
    );
  };
}(window.AudioContext.bind(window),
  navigator.webkitGetUserMedia.bind(navigator),
  audio) );
