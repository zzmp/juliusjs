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

static int rate;		///< Sampling rate specified in adin_mic_standby()
//int [] buffer;

int
get_rate()
{
  return rate;
}

void
fill_buffer()
{
  //buffer = audio_buffer;
}

//EMSCRIPTEN_BINDINGS(julius) {
//  function("fill_buffer", &fill_buffer);
//}

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
  EM_ASM(
    window.adin.onaudioprocess = (function() {
      // Give the event listener access to functions through closure

      var bufferSize = rate * 4096 / 44100;
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
          view.setInt16(f32Toi16(resampler.outputBuffer[i])
          i16ToUTF8Array(f32Toi16(resampler.outputBuffer[i])).forEach(function(val, ind) {
            buffer[i * 2 + ind] = val;
          }); 
        }   
        fill_buffer(ptr, byteSize);
        Module._free(ptr);
        for (var i = 0; i < 4096; i++) {
          out[i] = inp[i];
        }
      }; 
    }());
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
 * @return actural number of read samples, -2 if an error occured.
 */
int
adin_mic_read(SP16 *buf, int sampnum)
{
  long nread;

  //if (sampnum <= buffer_length) {
  //    nread = sampnum;
  //} else {
  //    nread = buffer_length;
  //}

  // FILL THAT BUFFER
  nread = sampnum;
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
