(function() {
    var postBuffer = function() {
      var that = this;

      return function(e) {
        var buffer = e.inputBuffer.getChannelData(0);
        
        if (that.audio._transfer) {
          var out = e.outputBuffer.getChannelData(0);

          for (var i = 0; i < 4096; i++) {
            out[i] = buffer[i];
          }
        }

        // Transfer audio to the recognizer
        that.recognizer.postMessage(buffer);
      };
    };

    var initializeAudio = function(audio) {
      audio.context = new AudioContext();
      audio.processor = audio.context.createScriptProcessor(4096, 1, 1);
    };

    var bootstrap = function(pathToDfa, pathToDict, options) {
      var audio = this.audio;
      var recognizer = this.recognizer;
      var terminate = this.terminate;
      
      navigator.webkitGetUserMedia(
        { audio: true },
        function(stream) {
          audio.source = audio.context.createMediaStreamSource(stream);
          audio.source.connect(audio.processor);
          audio.processor.connect(audio.context.destination);

          // Bootstrap the recognizer
          recognizer.postMessage({
            type: 'begin',
            pathToDfa: pathToDfa,
            pathToDict: pathToDict,
            options: options
          });
        },
        function(err) {
          terminate();
          console.error('JuliusJS failed: could not capture microphone input.');
        }
      );
    };

    var Julius = function(pathToDfa, pathToDict, options) {
      var that = this;
      options = options || {};

      // The context's nodemap: `source` -> `processor` -> `destination`
      this.audio = {
        // `AudioContext`
        context:   null,
        // `AudioSourceNode` from captured microphone input
        source:    null,
        // `ScriptProcessorNode` for julius
        processor: null,
        _transfer:  options.transfer
      };

      // _Recognition is offloaded to a separate thread to avoid slowing UI_
      this.recognizer = new Worker('worker.js');

      this.recognizer.onmessage = function(e) {
        if (e.data.type === 'begin') {
          that.audio.processor.onaudioprocess = postBuffer.call(that);

        } else if (e.data.type === 'recog') {
          if (e.data.firstpass) {
            typeof that.onfirstpass === 'function' &&
              that.onfirstpass(e.data.sentence);
          } else
            typeof that.onrecognition === 'function' &&
              that.onrecognition(e.data.sentence);

        } else if (e.data.type === 'log') {
          typeof that.onlog === 'function' &&
            that.onlog(e.data.sentence);

        } else if (e.data.type === 'error') {
          console.error(e.data.error);
          that.terminate();

        } else {
          console.info('Unexpected data received from julius:');
          console.info(e.data);
        }
      };

      initializeAudio(this.audio);
      bootstrap.call(this, pathToDfa, pathToDict, options);
    };

    Julius.prototype.onfirstpass = function() { /* noop */ };
    Julius.prototype.onrecognition = function() { /* noop */ };
    Julius.prototype.onlog = function(obj) { console.log(obj); };
    Julius.prototype.onfail = function() { /* noop */ };
    Julius.prototype.terminate = function(cb) {
      this.audio.processor.onaudioprocess = null;
      this.recognizer.terminate();
      console.error('JuliusJS was terminated.');
      typeof this.onfail === 'function' && this.onfail();
    };

    window.Julius = Julius;
}() );