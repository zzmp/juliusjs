var master = self;

// Functions exposed to libsent/src/adin_mic_webaudio.c
var setRate;
var begin = function() { master.postMessage({type: 'begin'}); };

// console polyfill for emscripted Module
var console = {};

importScripts('recognizer.js', 'listener/resampler.js', 'listener/converter.js');

console.log = (function() {
  // The designation used by julius for recognition
  var recogPrefix = /^sentence[0-9]+: (.*)/;
  var guessPrefix = /^pass[0-9]+_best: (.*)/;
  var scorePrefix = /^score[0-9]+: (.*)/;
  var recog;

  return function(str) {
    var score;
    var sentence

    if (score = str.match(scorePrefix)) {
      master.postMessage({type: 'recog', sentence: recog, score: score[1]});
    } else if (sentence = str.match(recogPrefix)) {
      recog = sentence[1];
    } else if (sentence = str.match(guessPrefix)) {
      master.postMessage({type: 'recog', sentence: sentence[1], firstpass: true});
    } else if (console.verbose)
      master.postMessage({type: 'log', sentence: str});
  };
}() );

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
    if (e.data.type === 'begin') {
      console.verbose = e.data.verbose;

      Module.callMain([
        '-dfa',   'voxforge/sample.dfa',
        '-v',     'voxforge/sample.dict',
        '-h',     'voxforge/hmmdefs',
        '-hlist', 'voxforge/tiedlist',
        '-input', 'mic',
        '-realtime', '-nolog'//, '-quiet'
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
