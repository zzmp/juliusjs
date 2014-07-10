var log = [];
var context = new AudioContext();
var source;
var resampler = context.createScriptProcessor(8192, 1, 1);

navigator.webkitGetUserMedia({ audio: true },
                             function(stream) {
                               source = context.createMediaStreamSource(stream);
                               source.connect(resampler);
                               resampler.connect(context.destination);

                               window.setTimeout(function() {
                                 resampler.disconnect();
                                 end();
                                 console.log(TTY.ttys[1280].input);
                                 Module.callMain(['-dfa',   'voxforge/sample.dfa',
                                                  '-v',     'voxforge/sample.dict',
                                                  '-h',     'voxforge/hmmdefs',
                                                  '-hlist', 'voxforge/tiedlist',    
                                                  '-input', 'stdin',                                 
                                                  '-cutsilence']);
                                 }, 3500);
                             },
                             function(err) {});

resampler.onaudioprocess = (function() {
  // Give the event listener access to functions through closure

  // https://github.com/grantgalitz/XAudioJS/blob/master/resampler.js
  var resampler = new Resampler(44100, 16000, 1, 2972, true);
  
  function f32Toi16(float) {
    // Guard against overflow
    var s = Math.max(-1, Math.min(1, float));
    // Assume 2's complement representation
    return s < 0 ? 0xFFFF ^ Math.floor(-s * 0x7FFF) : Math.floor(s * 0x7FFF);
  };

  function i16ToUTF8Array(i16, littleEndian) {
    var l = i16 >> 8;
    var r = i16 - (r << 8);
    return littleEndian ? [r, l] : [l, r];
  };

  return function(e) {
    var inp, out;
    inp = event.inputBuffer.getChannelData(0);
    out = event.outputBuffer.getChannelData(0);
    var l = resampler.resampler(inp);
    for (var i = 0; i < l; i++) {
      i16ToUTF8Array(f32Toi16(resampler.outputBuffer[i])).forEach(function(val) {
        TTY.ttys[1280].input.push(val);
        log.push(val);
      });
    }
    for (var i = 0; i < 8192; i++) {
      out[i] = inp[i];
    }
  };
}());

function end() {
  window.prompt = function() { return null; };
}
