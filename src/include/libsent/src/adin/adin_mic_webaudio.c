/**
 * @file   adin_mic_sp.c
 * 
 * <EN>
 * @brief  Microphone input using spAudio library
 *
 * Low level I/O functions for microphone input using spAudio library.
 * To use, please specify "--with-mictype=sp" options to configure script.
 *
 * Julius does not alter any mixer device setting at all on Linux.  You should
 * configure the mixer for recording source (mic/line) and recording volume
 * correctly using other audio tool such as xmixer.
 *
 * This code has been contributed by Hideaki Banno.
 *
 * @sa http://www.sp.m.is.nagoya-u.ac.jp/people/banno/spLibs/index.html
 * 
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Sun Feb 13 19:16:43 2005
 *
 * $Revision: 1.5 $
 * 
 */
/* adin_mic_sp.c --- adin microphone library for spAudio
 * by Hideki Banno */

#include <sent/stddefs.h>
#include <sent/adin.h>

#include <emscripten.h>
// embind is unavailable in C

static int rate;    // < Sampling rate specified in adin_mic_standby()
SP16 *buffer;
static long limit = 150000; // About five seconds of buffer
long get_pos;
long set_pos;

int
get_rate()
{
  return rate;
}

void
fill_buffer(const void* audio_buf, unsigned int buffer_length)
{
  SP16* audio_buffer = (SP16*) audio_buf;
  buffer_length /= 2; // Guaranteed to be even (as it is PCM16)
  if (buffer_length + set_pos <= limit) {
    memcpy(&buffer[set_pos], audio_buffer, buffer_length);
    set_pos += buffer_length;
  } else {
    long remainder = buffer_length - set_pos;
    memcpy(&buffer[set_pos], audio_buffer, remainder);
    memcpy(buffer, &audio_buffer[remainder], buffer_length - remainder);
    set_pos = buffer_length - remainder;
  }
}

/** 
 * Device initialization: check device capability and open for recording.
 * 
 * @param sfreq [in] required sampling frequency.
 * @param dummy [in] a dummy data
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_standby(int sfreq, void *dummy)
{
  buffer = (SP16 *) malloc( sizeof(SP16) * limit );
  set_pos = 0;
  get_pos = 0;
  rate = sfreq;
  return TRUE;
}

/** 
 * Start recording.
 *
 * @param pathname [in] path name to open or NULL for default
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_begin(char *pathname)
{
  get_pos = 1;
  EM_ASM(
    window.adin.onaudioprocess = (function() {
      // Give the event listener access to functions through closure

      var rate = Module.ccall('get_rate') || 16000;
      var bufferSize = Math.floor(rate * 4096 / 44100);
      var byteSize = bufferSize * 2;
      // https://github.com/grantgalitz/XAudioJS/blob/master/resampler.js
      var resampler = new Resampler(44100, rate, 1, bufferSize, true);
      
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

      var fill_buffer = Module.cwrap('fill_buffer', 'number', ['number', 'number']);

      return function(e) {
        var inp, out;
        var ptr = Module._malloc(byteSize);
        var buffer = new Uint8Array(Module.HEAPU8.buffer, ptr, byteSize);
        inp = event.inputBuffer.getChannelData(0);
        out = event.outputBuffer.getChannelData(0);
        var l = resampler.resampler(inp);
        for (var i = 0; i < l; i++) {
          i16ToUTF8Array(f32Toi16(resampler.outputBuffer[i])).forEach(function(val, ind) {
            buffer[i * 2 + ind] = val;
          }); 
        }
        console.log(buffer, ptr, byteSize);
        fill_buffer(ptr, byteSize);
        Module._free(ptr);
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
  EM_ASM( window.adin.onaudioprocess = null; );
  return TRUE;
}

/**
 * @brief  Read samples from device
 * 
 * Try to read @a sampnum samples and returns actual number of recorded
 * samples currently available.  This function will block until
 * at least some samples are obtained.
 * 
 * @param buf [out] samples obtained in this function
 * @param sampnum [in] wanted number of samples to be read
 * 
 * @return actual number of read samples, -2 if an error occured.
 */
int
adin_mic_read(SP16 *buf, int sampnum)
{
  long nread;

  if (set_pos > get_pos) {
    nread = set_pos - get_pos >= sampnum ? sampnum : set_pos - get_pos;
    memcpy(buf, &buffer[get_pos], nread);
    get_pos += nread;
  } else if (set_pos < get_pos) {
    nread = limit - get_pos >= sampnum ? sampnum : limit - get_pos;
    memcpy(buf, &buffer[get_pos], nread);
    get_pos = 0;
    nread += adin_mic_read(&buf[nread], sampnum - nread);
  } else {
    return 0;
  }

  return nread;
}

/** 
 * Function to pause audio input (wait for buffer flush)
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_pause()
{
  return TRUE;
}

/** 
 * Function to terminate audio input (discard buffer)
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_terminate()
{
  return TRUE;
}

/** 
 * Function to resume the paused / terminated audio input
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_mic_resume()
{
  return TRUE;
}

/** 
 * 
 * Function to return current input source device name
 * 
 * @return string of current input device name.
 * 
 */
char *
adin_mic_input_name()
{
  return("JavaScript Web Audio API");
}
