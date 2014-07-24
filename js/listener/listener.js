var adin;

(function() {
  var context = new AudioContext();
  var source;
  adin = context.createScriptProcessor(4096, 1, 1);

  navigator.webkitGetUserMedia(
    { audio: true },
    function(stream) {
      source = context.createMediaStreamSource(stream);
      source.connect(adin);
      adin.connect(context.destination);
    },
    function(err) {}
  );
}() );

var listener = new Worker('../julius.js');

listener.onmessage = function(e) {
  if (e.data.name == 'rate') {
    adin.onaudioprocess = (function() {
      // Give the event listener access to functions through closure

      var rate = e.data.rate || 16000;
      var bufferSize = Math.floor(rate * 4096 / 44100);
      var byteSize = bufferSize * 2;
      // https://github.com/grantgalitz/XAudioJS/blob/master/resampler.js
      importScripts('resampler.js');
      var resampler = new Resampler(44100, rate, 1, bufferSize, true);
         
      // Helper functions to convert to PCM16
      function f32Toi16(float) {
        // Guard against overflow
        var s = Math.max(-1, Math.min(1, float));
        // Assume 2's complement representation
        return s < 0 ? 0xFFFF ^ Math.floor(-s * 0x7FFF) : Math.floor(s * 0x7FFF);
      }
      function i16ToUTF8Array(i16, littleEndian) {
        var l = i16 >> 8;
        var r = i16 - (l << 8); 
        return littleEndian ? [r, l] : [l, r];
      }

      return function(e) {
        var inp, out, i;
        var buffer = new ArrayBuffer(byteSize);
        // Use Uint8Array to enforce endianness
        // TODO: use Int16Array TypedArray to enforce system endianness
        var view = new Uint8Array(buffer);
        inp = event.inputBuffer.getChannelData(0);
        out = event.outputBuffer.getChannelData(0);
        var l = resampler.resampler(inp);
        for (i = 0; i < l; i++) {
          i16ToUTF8Array(f32Toi16(resampler.outputBuffer[i]), true).forEach(function(val, ind) {
            view[i * 2 + ind] = val;
          });
        }

        // Transfer buffer to recognizer
        self.postMessage(buffer);
        
        // Ensure output so audio runs
        // TODO: replace with audio sink
        for (i = 0; i < 4096; i++) {
          out[i] = inp[i];
        }
      };
    }() );
  } else {
    console.log(e.data);
  }
};
