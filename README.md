JuliusJS
====

> A speech recognition library for the web

### This is in progress and not fully implemented!

- You may need to read the code to use it in its current state
 - The README documents eventual behavior, and is not currently accurate
 - Emscripted code is not yet committed - you'll need to run `emscripten.sh`
 - After running the script, a use example can be found in `js/index.html`
- This project is in active development - expect __frequent breaking changes__

---

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
 - Use the provided vocabulary, or write your own
- 100% JavaScript implementation
 - All recognition is done in-browser through a `Worker`
 - Familiar event-inspired API
 - No external server calls

### Quickstart

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
  julius.onrecognition = function(sentence) {
    // ...
    console.log(sentence);
  };
  ```

- Your site now has real-time speech recognition baked in!

#### Configure your own vocabulary

### Motivation

### Advanced Use

#### Configuring the engine

#### Build from source

## Developers

#### Emscripten

### Implementation goals

### Contributing

- Contributions are welcome! See `CONTRIBUTING.md` for guidelines.


## In the wild

_If you use `JuliusJS` let me know, and I'll add your project to this list._

1. Coming soon...


---

*JuliusJS is a port of the "Large Vocabulary Continuous Speech Recognition Engine Julius" to JavaScript*