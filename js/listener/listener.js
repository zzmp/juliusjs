(function() {
  var context = new AudioContext();
  var source;
  window.adin = context.createScriptProcessor(4096, 1, 1);

  navigator.webkitGetUserMedia(
    { audio: true },
    function(stream) {
      source = context.createMediaStreamSource(stream);
      source.connect(window.adin);
      window.adin.connect(context.destination);

      window.setTimeout(function() {
        Module.callMain([
          '-dfa',   'voxforge/sample.dfa',
          '-v',     'voxforge/sample.dict',
          '-h',     'voxforge/hmmdefs',
          '-hlist', 'voxforge/tiedlist',
          '-input', 'mic',
          '-spsegment', '-pausemodels', 'sil',
          '-realtime', '-cutsilence'
        ]);
      }, 2000);
    },
    function(err) {}
  );
}() );
