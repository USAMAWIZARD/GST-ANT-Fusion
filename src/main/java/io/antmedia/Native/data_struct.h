#include <gst/gst.h>
#ifndef SOME_HEADER_GUARD_WITH_UNIQUE_NAME
#define SOME_HEADER_GUARD_WITH_UNIQUE_NAME

GHashTable *hash_table;
GstRTSPMountPoints *mounts;

typedef  struct  
{
  GstAppSrc *videoappsrc;
  GstAppSrc *audioappsrc;
  AVCodecParameters *audiopar;
  AVCodecParameters *videopar;
  AVPacket *video_avpkt;
  AVPacket *audio_avpkt;
  char *video_caps;
  char *audio_caps;
  char *audio_playloader;
  char *video_payloader;
  char *video_parser;
  char *audio_parser;
  char *rtsp_mountpoint;
  AVRational timebase;
  GstElement *pipeline;
  char *roomid;
  volatile gboolean pipeline_initialized;
  AVBSFContext *bsfContext;
} volatile StreamMap;

typedef struct 
{
  char *rtsp_port;
  char *rtmp_port;
  char *rtsp_protocol;
  gboolean is_rtsp_enabled;
  gboolean is_rtmp_enabled;
} plugin_config;
plugin_config config;

#endif
