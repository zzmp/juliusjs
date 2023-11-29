(function(window, navigator, undefined) {
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
        audio.context = new(window.AudioContext || window.webkitAudioContext)();
        audio.processor = audio.context.createScriptProcessor(4096, 1, 1);
    };

    var bootstrap = function(pathToDfa, pathToDict, options) {
        var audio = this.audio;
        var recognizer = this.recognizer;
        var terminate = this.terminate;

        // Compatibility
        navigator.getUserMedia =
            navigator.getUserMedia ||
            navigator.webkitGetUserMedia ||
            navigator.mozGetUserMedia ||
            navigator.msGetUserMedia;

        navigator.getUserMedia({ audio: true },
            function(stream) {
                audio.source = audio.context.createMediaStreamSource(stream);
                audio.source.connect(audio.processor);
                audio.processor.connect(audio.context.destination);

                // Bootstrap the recognizer
                recognizer.postMessage({
                    type: "begin",
                    pathToDfa: pathToDfa,
                    pathToDict: pathToDict,
                    options: options,
                });
            },
            function(err) {
                terminate();
                console.error("JuliusJS failed: could not capture microphone input.");
            }
        );
    };

    //This bootstrapToStream() function implements direct stream channeling and allows developers to have more control over the bootstrap function and avoid accessing the microphone twice
    //which could be inefficient and annoying on mobile devices when trying to recognize and record at the same time

    var bootstrapToStream = function(pathToDfa, pathToDict, options, stream) {
        //check if stream object exists
        if (!stream) {
            terminate();
            return console.error("JuliusJS failed: Stream Object is undefined.");
        }

        //check if the stream accepts audio i.e has an audio track
        const hasAudioTrack = stream.getAudioTracks().length > 0;

        if (!hasAudioTrack) {
            terminate();
            return console.error(
                "JuliusJS failed: Stream Object provided is not an audio stream"
            );
        }

        var audio = this.audio;
        var recognizer = this.recognizer;
        var terminate = this.terminate;

        try {
            audio.source = audio.context.createMediaStreamSource(stream);
            audio.source.connect(audio.processor);
            audio.processor.connect(audio.context.destination);

            // Bootstrap the recognizer
            recognizer.postMessage({
                type: "begin",
                pathToDfa: pathToDfa,
                pathToDict: pathToDict,
                options: options,
            });
        } catch (err) {
            terminate();
            console.error("JuliusJS failed: an unexpected error occured.", err);
        }
    };

    var Julius = function(pathToDfa, pathToDict, options, stream) {
        var that = this;
        options = options || {};

        // The context's nodemap: `source` -> `processor` -> `destination`
        this.audio = {
            // `AudioContext`
            context: null,
            // `AudioSourceNode` from captured microphone input
            source: null,
            // `ScriptProcessorNode` for julius
            processor: null,
            _transfer: options.transfer,
        };

        // Do not pollute the object
        delete options.transfer;

        // _Recognition is offloaded to a separate thread to avoid slowing UI_
        this.recognizer = new Worker(options.pathToWorker || "worker.js");

        this.recognizer.onmessage = function(e) {
            if (e.data.type === "begin") {
                that.audio.processor.onaudioprocess = postBuffer.call(that);
            } else if (e.data.type === "recog") {
                if (e.data.firstpass) {
                    typeof that.onfirstpass === "function" &&
                        that.onfirstpass(e.data.sentence, e.data.score);
                } else
                    typeof that.onrecognition === "function" &&
                    that.onrecognition(e.data.sentence);
            } else if (e.data.type === "log") {
                typeof that.onlog === "function" && that.onlog(e.data.sentence);
            } else if (e.data.type === "error") {
                console.error(e.data.error);
                that.terminate();
            } else {
                console.info("Unexpected data received from julius:");
                console.info(e.data);
            }
        };

        initializeAudio(this.audio);

        stream
            ?
            bootstrapToStream.call(this, pathToDfa, pathToDict, options, stream) :
            bootstrap.call(this, pathToDfa, pathToDict, options);
    };

    Julius.prototype.onfirstpass = function(sentence) {
        /* noop */
    };
    Julius.prototype.onrecognition = function(sentence, score) {
        /* noop */
    };
    Julius.prototype.onlog = function(obj) {
        console.log(obj);
    };
    Julius.prototype.onfail = function() {
        /* noop */
    };
    Julius.prototype.terminate = function(cb) {
        this.audio.processor.onaudioprocess = null;
        this.recognizer.terminate();
        console.error("JuliusJS was terminated.");
        typeof this.onfail === "function" && this.onfail();
    };

    window.Julius = Julius;
})(window, window.navigator);