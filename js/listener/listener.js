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

      Module.callMain([
        '-dfa',   'voxforge/sample.dfa',
        '-v',     'voxforge/sample.dict',
        '-h',     'voxforge/hmmdefs',
        '-hlist', 'voxforge/tiedlist',
        '-input', 'mic',
        '-1pass', '-realtime', //'-quiet'
      ]);
    },
    function(err) {}
  );
}() );
