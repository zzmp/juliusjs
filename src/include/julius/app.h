#include <julius/juliuslib.h>

#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__MINGW32__)
#include <config-msvc-julius.h>
#else
#include "config.h"
#endif
#ifdef CHARACTER_CONVERSION
#include "charconv.h"
#endif

/**
 * Output file suffix for separate file output
 * 
 */
#define OUTPUT_FILE_SUFFIX ".out"

/* recogloop.c */
void main_recognition_stream_loop(Recog *recog);
void init_event_recognition_stream_loop(Recog *recog);
int main_event_recognition_stream_loop();
void end_event_recognition_stream_loop();

/* module.c */
int module_send(int sd, char *fmt, ...);
void module_add_option();
boolean is_module_mode();
void module_setup(Recog *recog, void *data);
void module_server();
void module_disconnect();

/* output_module.c */
void decode_output_selection(char *str);
void send_gram_info(RecogProcess *r);
void setup_output_msock(Recog *recog, void *data);

/* output_stdout.c */
void print_all_gram(Recog *recog);
void setup_output_tty(Recog *recog, void *data);

/* output_file.c */
void setup_output_file(Recog *recog, void *data);
void outfile_set_fname(char *input_filename);

/* record.c */
void record_add_option();
void record_setup(Recog *recog, void *data);

