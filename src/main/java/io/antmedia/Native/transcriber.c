
#include <stdio.h>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <strings.h>
#include <stdbool.h>
#include <jni.h>
#include "javaCallback.h"
#include <stdio.h>
Callback callback;
#define TRANSCRIPTION_DATA "caption"
//typedef void (*Callback)( char* streamId, char*  roomId , char* type , char* data );


extern  bool transcription_data(GstElement *vosk, gchararray result, gpointer user_data) {
    printf("Recognized Speech: %s\n", result);
    callback(TRANSCRIPTION_DATA, TRANSCRIPTION_DATA ,TRANSCRIPTION_DATA ,TRANSCRIPTION_DATA);
}

void registerCallback(Callback cb) {
    callback = cb;
}

