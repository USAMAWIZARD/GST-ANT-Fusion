#define PIPE_GSTREAMER "Gstreamer"
#define PIPE_FFMPEG "FFmpeg"
#define PIPE_RTSP "RTSP"
#define RTP_OUT "RTP_OUT"

#define PACKET_TYPE_VIDEO 0
#define PACKET_TYPE_AUDIO 1

#include <stdio.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/app/gstappsrc.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>

static char *port = "8554";
GstRTSPMountPoints *mounts;
GHashTable *hash_table;

typedef struct
{
  GstAppSrc *videoappsrc;
  GstAppSrc *audioappsrc;

  GstElement *pipeline;
  volatile gboolean pipeline_initialized;
} StreamMap;

// TODO: do something with this
// typedef struct{
// char *streamid;
// char *pipeline_type; //tcp udp
// char *pipeline;
// struct network_sink_details {
//   char *host;
//   char *port;
// };
// }pipeline_deatils;

void onPacket(AVPacket *pktPointer, gchar *streamId, int pktType);
void init_rtsp_server();
void register_pipeline(gchar *streamId, gchar *pipeline_type, gchar *pipeline);
int add_gstreamer_pipeline(char *pipeline, char *streamId);
int add_rtsp_pipeline(gchar *streamId);

void onPacket(AVPacket *pktPointer, gchar *streamId, int pktType)
{
  AVPacket *pkt = (AVPacket *)pktPointer;
  // printf("Packet PTS %s: %ld %ld\n ", id, pkt->pts, pkt->dts);

  if (g_hash_table_contains(hash_table, streamId))
  {
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);

    if (ctx->pipeline_initialized)
    {
      GstBuffer *buffer = gst_buffer_new_and_alloc(pkt->size);
      uint8_t *data = (uint8_t *)pkt->data;
      gst_buffer_fill(buffer, 0, data, pkt->size);

      g_assert(ctx->videoappsrc);
      if (pktType == PACKET_TYPE_VIDEO)
      {
        // if ((pkt->flags & AV_PKT_FLAG_KEY) &&) {
        //   printf("-----------------key--frame--------------------- \n");
        // }
        // else {
        //   GST_BUFFER_FLAG_SET(buffer, GST_BUFFER_FLAG_DELTA_UNIT);
        // }
        gst_app_src_push_buffer((GstAppSrc *)ctx->videoappsrc, buffer);
      }
      else if (pktType == PACKET_TYPE_AUDIO)
      {
        // TODO:
        printf("writing audio packet to app src\n");
        gst_app_src_push_buffer((GstAppSrc *)ctx->audioappsrc, buffer);
      }
      else
      {
      }
    }
  }
  else
  {
  }
}

static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer streamId)
{

  if (g_hash_table_contains(hash_table, streamId))
  {
    g_print("------------------------");
    GstElement *element, *videoappsrc,*audioappsrc;
    element = gst_rtsp_media_get_element(media);

    char app_src[128];
    sprintf(app_src, "video_%s", (char *)streamId);
    videoappsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)app_src);
        printf("initializing video app src %s\n",app_src);

    sprintf(app_src, "audio_%s", (char *)streamId);
    audioappsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)app_src);

    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, (gchar *)streamId);
    g_assert(videoappsrc);
  
    ctx->videoappsrc = (GstAppSrc *)videoappsrc;
    printf("initializing video app src\n");
    ctx->audioappsrc = (GstAppSrc *)audioappsrc;

    ctx->pipeline_initialized = 1;
    ctx->pipeline = element;

    gst_object_unref(element);
  }
}

void register_pipeline(gchar *streamId, gchar *pipeline_type, gchar *pipeline)
{
  // register appropriate pipline
  if (g_hash_table_contains(hash_table, streamId))
  {
    if (g_strcmp0(pipeline_type, PIPE_GSTREAMER) == 0)
    {
      add_gstreamer_pipeline(pipeline, streamId);
    }
    else if (g_strcmp0(pipeline_type, PIPE_FFMPEG) == 0)
    {
    }
    else if (g_strcmp0(pipeline_type, PIPE_RTSP) == 0)
    {
      add_rtsp_pipeline(streamId);
    }

    else if (g_strcmp0(pipeline_type, RTP_OUT) == 0)
    {
      // RTP
      // SRT
      // rtmpsink

      char rtp_pipe[128];
      sprintf(rtp_pipe, "rtph264pay ! %s host=%s port=%s", "udpsink", "127.0.0.1", "8000"); // TODO: make generic for major protocols
      add_gstreamer_pipeline(rtp_pipe, streamId);
    }
    else
    {
      // invalid option
      printf("handler on streamId (%s) for pipeline (%s) is not defined\n", streamId, pipeline_type);
    }
  }
  else
  {
    printf("can't find streamId (%s) in table for pipeline type : %s\n", streamId, pipeline_type);
    // sreamid not found
  }
}

int add_rtsp_pipeline(gchar *streamId)
{
  GstRTSPMediaFactory *factory;
  gchar pipe[700];
  gchar mountpoint[30];
  factory = gst_rtsp_media_factory_new();
  // video only pipeline
   sprintf(pipe, "appsrc name=video_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \"  !  h264parse ! rtph264pay name=pay0 pt=96", streamId);

  // audio video pipeline
  //sprintf(pipe, "appsrc name=video_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \"  !  h264parse ! rtph264pay name=pay0 pt=96 "
  //"appsrc name=audio_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"audio/x-opus \" ! opusparse ! rtpopuspay name=pay1 pt=97 ", streamId,streamId);

  g_signal_connect(factory, "media-configure", (GCallback)media_configure, (gpointer)streamId);

  gst_rtsp_media_factory_set_launch(factory, pipe);

  sprintf(mountpoint, "/%s", streamId);
  gst_rtsp_mount_points_add_factory(mounts, mountpoint, factory);
  g_print("New stream ready at rtsp://127.0.0.1:%s/%s\n", port, streamId);
}

int add_gstreamer_pipeline(char *pipeline, char *streamId)
{
  gchar pipe[700];
  gchar mountpoint[30];
  StreamMap *ctx;
  GError *err = NULL;

  if (g_hash_table_contains(hash_table, streamId))
  {
    ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);

    // video only
    // sprintf(pipe, "appsrc name=%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \" !  h264parse ! %s",
    //         streamId, pipeline);

    // audio video
    sprintf(pipe, "appsrc name=%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \" !  h264parse ! %s",
            streamId, pipeline);

    printf("---------starting streamer pipeline-----------\n");
    printf("%s\n", pipe);
    ctx->pipeline = gst_parse_launch(pipe, &err);
    if (err != NULL)
    {
      printf("%s\n", err->message);
      g_error_free(err);
      return -1;
    }
    gst_element_set_state(ctx->pipeline, GST_STATE_PLAYING);
    ctx->pipeline_initialized = 1;
    ctx->videoappsrc = (GstAppSrc *)gst_bin_get_by_name((GstBin *)ctx->pipeline, streamId);
    return 0;
  }
  else
  {
    printf("hash table doesn't contains pipeline in %s:%d\n", __FILE__, __LINE__);
    return -1;
  }
}
int add_ffmpeg_pipeline(char *pipeline, char *streamId)
{
  return 0;
}
void register_stream(char *streamId)
{
  printf("new streamid recived adding new stream : %s \n", streamId);
  StreamMap *ctx = g_new0(StreamMap, 1);
  ctx->pipeline_initialized = 0;
  g_hash_table_insert(hash_table, streamId, ctx);

  printf("added stream id\n");
}
void unregister_stream(char *streamId)
{
  if (g_hash_table_contains(hash_table, streamId))
  {
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);
    gst_element_set_state((GstElement *)ctx->videoappsrc, GST_STATE_NULL);
    gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
    g_hash_table_remove(hash_table, streamId);
    gst_object_unref(ctx->pipeline);
    gst_object_unref(ctx->videoappsrc);
    printf("stream unregistered\n");
  }
}
void init_rtsp_server()
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaFactory *factory;
  GError *error = NULL;
  hash_table = g_hash_table_new(g_str_hash, g_str_equal);

  setenv("GST_DEBUG", "3", 1);
  setenv("GST_DEBUG_FILE", "./loggggggggggggggggggggggg.log", 1);

  gst_init(NULL, NULL);
  g_print("-----------------------------------------------%s /n", getenv("GST_DEBUG"));

  loop = g_main_loop_new(NULL, FALSE);
  server = gst_rtsp_server_new();
  g_assert(server);

  g_object_set(server, "service", port, NULL);
  printf("initialized RTSP Server Listening on Port %s \n", port);
  mounts = gst_rtsp_server_get_mount_points(server);
  // add_rtsp_pipeline("Streamid");

  g_object_unref(mounts);

  gst_rtsp_server_attach(server, NULL);

  g_main_loop_run(loop);
}