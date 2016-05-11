JuliusJS
====

> A speech recognition library for the web

Try the [live demo.](https://zzmp.github.io/juliusjs/)

JuliusJS is an opinionated port of Julius to JavaScript. <br>
It __actively listens to the user to transcribe what they are saying__ through a callback.

```js
// bootstrap JuliusJS
var julius = new Julius();

julius.onrecognition = function(sentence) {
    console.log(sentence);
};

// say "Hello, world!"
// console logs: `> HELLO WORLD`
```

###### Features:

- Real-time transcription
 - Use the provided grammar, or write your own
- 100% JavaScript implementation
 - All recognition is done in-browser through a `Worker`
 - Familiar event-inspired API
 - No external server calls

## Quickstart

##### Using Express 4.0

1. Grab the latest version with bower
 - `bower install juliusjs --save`
1. Include `julius.js` in your html
 - `<script src="julius.js"></script>`
1. Make the scripts available to the client through your server
  ```js
  var express = require('express'),
      app     = express();
  
  app.use(express.static('path/to/dist'));
  ```
1. In your main script, bootstrap JuliusJS and register an event listener for recognition events
  ```js
  // bootstrap JuliusJS
  var julius = new Julius();
  
  // register listener
  julius.onrecognition = function(sentence, score) {
      // ...
      console.log(sentence);
  };
  ```

- Your site now has real-time speech recognition baked in!

### Configure your own recognition grammar

In order for JuliusJS to use it, your grammar must follow the [Julius grammar specification](http://julius.sourceforge.jp/en_index.php?q=en_grammar.html). The site includes a tutorial on writing grammars.<br>
By default, phonemes are defined in `voxforge/hmmdefs`, though you might find [other sites](http://www.boardman.k12.oh.us/bdms/phonological/44Phonemes.pdf) more useful as reference.

- Building your own grammar requires the `mkdfa.pl` script and associated binaries, distributed with Julius.
 - _On Mac OS X_
   - Use `./bin/mkdfa.pl`, included with this repo
 - _On other OS_
   - Run `emscripten.sh` to populate `bin` with the necessary files

1. Write a `yourGrammar.voca` file with words to be recognized
 - The `.voca` file defines "word candidates" and their pronunciations.
1. Write a `yourGrammar.grammar` file with phrases composed of those words
 - The `.grammar` file defines "category-level syntax, i.e. allowed connection of words by their category name."
1. Compile the grammar using `./bin/mkdfa.pl yourGrammar`
 - The `.voca` and `.grammar` must be prefixed with the same name
 - This will generate `yourGrammar.dfa` and `yourGrammar.dict`
1. Give the new `.dfa` and `.dict` files to the `Julius` constructor
  
  ```js
  // when bootstrapping JuliusJS
  var julius = new Julius('path/to/dfa', 'path/to/dict');
  ```

## Advanced Use

### Configuring the engine

The `Julius` constructor takes three arguments which can be used to tune the engine:

```js
new Julius('path/to/dfa', 'path/to/dict', options)
```

_Both 'path/to/dfa' and 'path/to/dict' must be set to use a custom grammar_

##### 'path/to/dfa'
- path to a valid `.dfa` file, generated as described [above](#configure-your-own-recognition-grammar)
- if left `null`, the default grammar will be used

##### 'path/to/dict'
- path to a valid `.dict` file, generated as described [above](#configure-your-own-recognition-grammar)
- if left `null`, the default grammar will be used

##### options
- `options.verbose` - _if `true`, JuliusJS will log to the console_
- `options.stripSilence` - _if `true`, silence phonemes will not be included in callbacks_
 - `true` by default
- `options.transfer` - _if `true`, captured microphone input will be piped to your speakers_
 - _this is mostly useful for debugging_
- `options.*`
 - Julius supports a wide range of options. Most of these are made available here, by specifying the flag name as a key. For example: `options.zc = 30` will lower the zero-crossing threshold to 30.<br> _Some of these options will break JuliusJS, so use with caution._
 - A reference to available options can be found in the [JuliusBook](http://julius.sourceforge.jp/juliusbook/en/).
 - Currently, the only supported hidden markov model is from voxforge. The `h` and `hlist` options are unsupported.

## Examples

### Voice Command

_Coming soon..._

### Keyword Spotting (e.g., API integration)

_Coming soon..._

### In the wild

_If you use `JuliusJS` let me know, and I'll add your project to this list (or issue a pull request yourself)._

1. _Coming soon..._

## Motivation

- Implement speech recognition in...
 - 100% JavaScript - no external dependencies
 - A familiar and _easy-to-use_ context
   - Follow standard eventing patterns (e.g., `onrecognition`)
- As far as accessibility, allow...
 - __Out-of-the-box use__
   - Minimal barrier to use
     - _This means commited sample files (e.g. commited emscripted library)_
   - Minimal configuration
     - Real-time (opinionated) use only
       - Hide mfcc/wav/rawfile configurations
 - Useful examples (_not so much motivation, as my motivating goals_)
   - Voice command
   - Keyword spotting

#### Future goals

- Better sample recognition grammar (_improves out-of-the-box usability_)
- Examples

## Developers

___Contributions are welcome! See `CONTRIBUTING.md` for guidelines.___

### Build from source

__You'll need [emscripten](http://kripken.github.io/emscripten-site/), the LLVM to JS compiler, to build this from the C source.__ Once you have that, run `./emscript.sh`. If you are missing other tools, the script will let you know.

As emscript.sh reloads and recompiles static libraries, `./reemscript.sh` is available once you've already run emscript.sh. reemscript.sh will only recompile to JavaScript based on your latest changes. This can also be run with `npm make`.

Additionally, tests <s>are set</s> will be made to run using `npm test`.<br> In the meantime,  a blank page with the JuliusJS library can be served using `npm start`.

### Codemap

##### emscript.sh / reemscript.sh

These scripts will compile/recompile Julius C source to JavaScript, as well as copy all other necessary files, to the **js** folder.

emscript.sh will also compile binaries, which you can use to create recognition grammars or compile grammars to smaller binary files. These are copied to the **bin** folder.

##### src

This is where the source for Julius will go once emscript.sh is run. emscript.sh will replace certain files in **src/julius4** with those in **src/include** in order to make **src/emscripted**, the files eventually compiled to JavaScript.

- src/include/julius/app.h - _the main application header_
- __src/include/julius/main.c__ - _the main application_
- __src/include/julius/recogloop.c__ - _a wrapper around the recognition loop_
- src/include/libjulius/src/adin_cut.c - _interactions with a microphone_
- src/include/libjulius/src/m_adin.c - _initialization to Web Audio_
- __src/include/libjulius/src/recogmain.c__ - _the main recognition loop_
- src/include/libsent/configure[.in] - _configuration to add Web Audio_
- src/include/libsent/src/adin/adin_mic_webaudio.c - _input on Web Audio_

_Files in bold were changed to replace a loop with eventing, to simulate multithreading in a Worker._

##### js

The home to the testing server run with `npm start`. Files are copied to this folder from **dist** with emscript.sh and reemscript.sh. If they are modified, they should be commited back to the **dist** folder.

##### dist

The home for committed copies of the compiled library, as well as the wrappers that make them work: julius.js and worker.js. **dist/listener/converter.js** is the file that actually pipes Web Audio to Julius (the compiled C program).

---

*JuliusJS is a port of the "Large Vocabulary Continuous Speech Recognition Engine Julius" to JavaScript*
