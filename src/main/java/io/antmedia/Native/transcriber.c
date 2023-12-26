
#include <stdio.h>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <strings.h>
#include <stdbool.h>
#include <jni.h>
#include "javaCallback.h"
#include <stdio.h>
static Callback callback;

typedef void(*onTranscriptionData)(char *streamId, char* data);

extern  bool transcription_data(GstElement *vosk, gchararray result, gpointer user_data) {
    printf("Recognized Speech: %s\n", result);
    
    void onTranscriptionData(streamid, data);

}

void registerCallback(Callback cb) {
    callback = cb;
}

void javaCallback(int data) {
    // Invoke the Java callback
    if (callback != NULL) {
        callback(data);
    }
}
