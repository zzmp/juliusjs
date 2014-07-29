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
    var sentence;

    if (typeof str !== 'string') {
      if (console.verbose) master.postMessage({type: 'log', sentence: str});
      return;
    }

    if (score = str.match(scorePrefix)) {
      master.postMessage({type: 'recog', sentence: recog, score: score[1]});
    } else if (sentence = str.match(recogPrefix)) {
      recog = sentence[1];
      if (console.stripSilence)
        recog = recog.split(' ').slice(1, -1).join(' ');
    } else if (sentence = str.match(guessPrefix)) {
      master.postMessage({type: 'recog', sentence: sentence[1], firstpass: true});
    } else if (console.verbose)
      master.postMessage({type: 'log', sentence: str});
  };
}() );

console.error = function(err) { master.postMessage({type: 'error'}); };

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
      var dfa = 'julius.dfa';
      var dict = 'julius.dict';
      var options = [];

      console.verbose = e.data.options.verbose;
      console.stripSilence =
        e.data.options.stripSilence === undefined ?
          true : e.data.options.stripSilence;

      if (typeof e.data.pathToDfa === 'string' &&
          typeof e.data.pathToDict === 'string') {
        FS.createLazyFile('/', 'julius.dfa', e.data.pathToDfa, true, false);
        FS.createLazyFile('/', 'julius.dict', e.data.pathToDict, true, false);
      } else {
        dfa = 'voxforge/sample.dfa';
        dict = 'voxforge/sample.dict';
      }

      options = [
        '-dfa',   dfa,
        '-v',     dict,
        '-h',     'voxforge/hmmdefs',
        '-hlist', 'voxforge/tiedlist',
        '-input', 'mic',
        '-realtime'
      ];

      if (!console.verbose) {
        options.push('-nolog');
      }

      var bootstrap = function() {
        if (runDependencies) {
          setTimeout(bootstrap, 0);
          return;
        }
        try { Module.callMain(options); }
        catch (error) { master.postMessage({type: 'error', error: error}); }
      };
      bootstrap();

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
