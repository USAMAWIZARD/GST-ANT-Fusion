
#include <stdio.h>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <strings.h>
#include <stdbool.h>
#include <jni.h>
#include <stdio.h>
#include <json-glib/json-glib.h>

typedef void (*Callback)( char* streamId, char*  roomId , char* data );
void registerCallback(Callback cb);

Callback callback;
#define TRANSCRIPTION_DATA "caption"

//typedef void (*Callback)( char* streamId, char*  roomId  , char* data );
static gchar *get_string_from_json_object(JsonObject *object)
{
    JsonNode *root;
    JsonGenerator *generator;
    gchar *text;
    /* Make it the root node */
    root = json_node_init_object(json_node_alloc(), object);
    generator = json_generator_new();
    json_generator_set_root(generator, root);
    text = json_generator_to_data(generator, NULL);
    /* Release everything */
    g_object_unref(generator);
    json_node_free(root);
    return text;
}

extern  bool transcription_data(GstElement *vosk, gchararray result, gpointer streamId) {
    printf("Recognized Speech: %s\n", result);
    JsonObject *caption = json_object_new();
    json_object_set_string_member(caption, "eventType", "caption");
    json_object_set_string_member(caption, "tech", "vosk");
    json_object_set_string_member(caption, "streamId", (gchar*) streamId);
    json_object_set_string_member(caption, "text", result);
    gchar * caption_data = get_string_from_json_object(caption);
   // StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);  
    callback((gchar*)streamId , "ctx->roomid" , caption_data);       //may cause issue

   //json_node_free(caption);
}

void registerCallback(Callback cb) {
    callback = cb;
}

