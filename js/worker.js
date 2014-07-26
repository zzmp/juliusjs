var master = self;

// Functions exposed to libsent/src/adin_mic_webaudio.c
var setRate;
var begin = function() { master.postMessage({type: 'begin'}); };

importScripts('recognizer.js', 'listener/resampler.js', 'listener/converter.js');

master.onmessage = (function() {
  var converter;
  var bufferSize;
  var byteSize;

  setRate = function(rate) {
    rate = rate || 16000;
    bufferSize = Math.floor(rate * 4096 / 44100);
    byteSize = bufferSize * 2;
    converter = new Converter(rate, bufferSize, byteSize);
  };

  var fillBuffer = Module.cwrap('fill_buffer', 'number', ['number', 'number']);

  return function(e) {
    if (e.data === 'begin') {
      Module.callMain([
        '-dfa',   'voxforge/sample.dfa',
        '-v',     'voxforge/sample.dict',
        '-h',     'voxforge/hmmdefs',
        '-hlist', 'voxforge/tiedlist',
        '-input', 'mic',
        '-realtime', '-nolog', '-quiet'
      ]);

    } else {
      var ptr = Module._malloc(byteSize);
      // Convert to .raw format
      converter.convert(e.data, Module.HEAPU16.buffer, ptr);
      // Copy to ring buffer (see libsent/src/adin_mic_webaudio.c)
      fillBuffer(ptr, bufferSize);
      Module._free(ptr);
    }
  };
}() );
