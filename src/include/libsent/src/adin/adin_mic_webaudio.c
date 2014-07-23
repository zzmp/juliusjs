/**
 * @file   adin_mic_webaudio.c
 *
 * <EN>
 * @brief  Microphone input on JavaScript's Web Audio API.
 *
 * Low level I/O functions for microphone input on an emscripten port.
 * This relies on other alterations inherent in the port.
 * See the attached script (`emscripten.sh`) for details.
 *
 * For more details, see https://github.com/zzmp/juliusjs
 *
 * Tested on Chrome 35.0.1916.153 for OS X.
 * </EN>
 *
 * @author Zachary POMERANTZ
 * @date   Wed Jul 16 13:06:00 2014
 *
 * $Revision: 1.00 $
 * 
 */
/*
 * Copyright (c) 2014 Zachary Pomerantz, @zzmp
 * Using the MIT License
 */

#include <sent/stddefs.h>
#include <sent/adin.h>

#include <emscripten.h>

static int rate;    // < Sampling rate specified in adin_mic_standby()
static long limit = 320000; // About 20 seconds of buffer
SP16 *buffer;
long get_pos = 0;
long set_pos = 0;

// Expose the rate to JavaScript
int  get_rate()   { return rate; }

/**
 * Fill the microphone ring buffer from the Web Audio API
 *
 * @param audio_buf [in] buffer to copy, must be valid monaural PCM16.
 * @param buffer_length [in] length of buffer to copy (in SP16)
 */
void
fill_buffer(const SP16* audio_buf, unsigned int buffer_length)
{
  if (buffer_length + set_pos <= limit) {
    memcpy(buffer + set_pos, audio_buf, sizeof(SP16) * buffer_length);
    set_pos += buffer_length;
  } else {
    long tail = limit - set_pos;
    long head = buffer_length - tail;
    memcpy(buffer + set_pos, audio_buf, sizeof(SP16) * tail);
    memcpy(buffer, audio_buf + tail, sizeof(SP16) * head);
    set_pos = head;
  }
}

/** 
 * Device initialization: check device capability and open for recording.
 * 
 * @param sfreq [in] required sampling frequency.
 * @param dummy [in] dummy data
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_standby(int sfreq, void *dummy)
{
  buffer = (SP16 *) malloc( sizeof(SP16) * limit );
  rate = sfreq;
  return TRUE;
}

/** 
 * Start recording.
 *
 * This will always prepare monaural PCM16 audio data.
 * The endianness is system-defined, to avoid conflict with the port.
 *
 * @param pathname [in] ignored for Web Audio.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_begin(char *pathname)
{
  EM_ASM(
    adin.onaudioprocess = (function() {
      // Give the event listener access to functions through closure

      var rate = Module.ccall('get_rate') || 16000;
      var bufferSize = Math.floor(rate * 4096 / 44100);
      var byteSize = bufferSize * 2;
      // https://github.com/grantgalitz/XAudioJS/blob/master/resampler.js
      var resampler = new Resampler(44100, rate, 1, bufferSize, true);
      
      // Helper functions to convert to PCM16=
      function f32Toi16(float) {
        // Guard against overflow
        var s = Math.max(-1, Math.min(1, float));
        // Assume 2's complement representation
        return s < 0 ? 0xFFFF ^ Math.floor(-s * 0x7FFF) : Math.floor(s * 0x7FFF);
      };
      function i16ToUTF8Array(i16, littleEndian) {
        var l = i16 >> 8;
        var r = i16 - (l << 8);
        return littleEndian ? [r, l] : [l, r];
      };

      // Fill the microphone buffer in C
      var fill_buffer = Module.cwrap('fill_buffer', 'number', ['number', 'number']);

      return function(e) {
        var inp, out;
        var ptr = Module._malloc(byteSize);
        // Use Uint8Array to enforce endianness
        // TODO: use Int16Array TypedArray to enforce system endianness
        var buffer = new Uint8Array(Module.HEAPU16.buffer, ptr, byteSize);
        inp = event.inputBuffer.getChannelData(0);
        out = event.outputBuffer.getChannelData(0);
        var l = resampler.resampler(inp);
        for (var i = 0; i < l; i++) {
          i16ToUTF8Array(f32Toi16(resampler.outputBuffer[i]), true).forEach(function(val, ind) {
            buffer[i * 2 + ind] = val;
          });
        }
        fill_buffer(ptr, bufferSize);
        Module._free(ptr);
        // Ensure output so audio runs
        // TODO: replace with audio sink
        for (var i = 0; i < 4096; i++) {
          out[i] = inp[i];
        }
      };
    }() );
  );
    
  return TRUE;
}

/** 
 * Stop recording.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_end()
{
  return TRUE;
}

/**
 * @brief  Read samples from device.
 * 
 * Try to read @a sampnum samples and returns actual number of recorded
 * samples currently available.  This function will block until
 * at least some samples are obtained.
 * 
 * @param buf [out] samples obtained in this function.
 * @param sampnum [in] wanted number of samples to be read.
 * 
 * @return actual number of read samples, -2 if an error occured.
 */
int
adin_mic_read(SP16 *buf, int sampnum)
{
  long nread = 0;

  if (set_pos > get_pos) {
    nread = (set_pos - get_pos >= sampnum) ? sampnum : set_pos - get_pos;
    memcpy(buf, buffer + get_pos, sizeof(SP16) * nread);
    get_pos += nread;
  } else if (set_pos < get_pos) {
    nread = (limit - get_pos >= sampnum) ? sampnum : limit - get_pos;
    memcpy(buf, buffer + get_pos, sizeof(SP16) * nread);
    get_pos = 0;
    nread += adin_mic_read(buf + nread, sampnum - nread);
  }

  return nread;
}

/** 
 * Tiny function to pause audio input (wait for buffer flush).
 *
 * Unused by Web Audio.
 *
 * @return TRUE on success, FALSE on failure.
 */
boolean adin_mic_pause() { return TRUE; }

/**
 * Tiny function to terminate audio input (discard buffer).
 *
 * Unused by Web Audio.
 *
 * @return TRUE on success, FALSE on failure.
 */
boolean adin_mic_terminate() { return TRUE; }

/** 
 * Tiny function to resume paused / terminated audio input.
 *
 * Unused by Web Audio.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean adin_mic_resume() { return TRUE; }

/** 
 * Tiny function to return current input source device name.
 * 
 * @return string of current input device name.
 */
char * adin_mic_input_name() { return("JavaScript Web Audio API"); }
