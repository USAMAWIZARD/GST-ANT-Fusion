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
GstClockTime timestamp, duration;
int key=0;
int add_new_stream(gchar *streamId);
void sendPacket(AVPacket *pktPointer, gchar *streamId);
void init_rtsp_server();

typedef struct
{
  gboolean isRead;
  GstAppSrc *appsrc;
} StreamMap;
int i = 1;

void check_err(int exp, char *msg, int is_exit)
{
  if (!exp)
  {
    perror(msg);
    if (is_exit)
    {
      exit(EXIT_FAILURE);
    }
  }
}

static inline guint64
gst_ffmpeg_time_ff_to_gst(gint64 pts, AVRational base)
{
  guint64 out;

  if (pts == AV_NOPTS_VALUE)
  {
    out = GST_CLOCK_TIME_NONE;
  }
  else
  {
    AVRational bq = {1, GST_SECOND};
    out = av_rescale_q(pts, base, bq);
  }

  return out;
}
void sendPacket(AVPacket *pktPointer, gchar *streamId)
{
  AVPacket *pkt = (AVPacket *)pktPointer;
  gchar *id = (gchar *)streamId;
//  printf("Packet PTS %s: %ld %ld\n ", id, pkt->pts, pkt->dts);

  uint8_t *data = (uint8_t *)pkt->data;

  if (g_hash_table_contains(hash_table, id))
  {
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, id);

    if (ctx->isRead)
    {
      GstBuffer *buffer = gst_buffer_new_and_alloc(pkt->size);

      gst_buffer_fill(buffer, 0, data, pkt->size);

      AVRational time;
      time.num = 1;
      time.den = 30;
      


      // timestamp = gst_ffmpeg_time_ff_to_gst(pkt->pts, time);
      // if (GST_CLOCK_TIME_IS_VALID(timestamp))
      // {
      //   GST_BUFFER_TIMESTAMP(buffer) = timestamp;
      // }

      // GstClockTime duration = gst_ffmpeg_time_ff_to_gst(pkt->duration, time);

      // if (G_UNLIKELY(!duration))
      // {
      //   g_print("Warning: duration is GST_CLOCK_TIME_NONE.\n");
      //   duration = GST_CLOCK_TIME_NONE;
      // }
      // else
      //   GST_BUFFER_DURATION(buffer) = duration;
      // if (!(pkt->flags & AV_PKT_FLAG_KEY))
      // {
      //   GST_BUFFER_FLAG_SET(buffer, GST_BUFFER_FLAG_DELTA_UNIT);
      // }
      // //   GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(i++, GST_SECOND, 30);

   //   printf("Buffer %s: %ld %ld \n ", id, buffer->pts, buffer->dts);

    //  g_print("data pushed");
      g_assert(ctx->appsrc);

      if ((pkt->flags & AV_PKT_FLAG_KEY))
      {
        GST_BUFFER_FLAG_SET(buffer, GST_BUFFER_FLAG_DELTA_UNIT);
        key=1;
        printf("uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu");
      }
      if(key==1)
      gst_app_src_push_buffer((GstAppSrc *)ctx->appsrc, buffer);
    }
  }
  else
  {
    printf("new streamid recived adding new stream : %s \n", id);
    StreamMap *ctx = g_new0(StreamMap, 1);
    ctx->isRead = 0;
    g_hash_table_insert(hash_table, streamId, ctx);
    add_new_stream(streamId);
  }
}

static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer streamid)
{
  g_print("------------------------");
  GstElement *element, *appsrc;
  element = gst_rtsp_media_get_element(media);

  appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)streamid);

  // g_object_set(G_OBJECT(appsrc),"caps",
  // gst_caps_new_simple("video/x-h264",
  // "alignment", G_TYPE_STRING, "(string)au",
  // "codec_data", G_TYPE_STRING, "(buffer)014d0028ffe1001a274d002895a01e0089f970110000030001000003003c8da1c32a01000428ee3c80",
  // "stream-format", G_TYPE_STRING, "(string)avc",
  // "framerate", GST_TYPE_FRACTION, 0, 1,
  // NULL),
  // NULL);

  StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, (gchar *)streamid);
  ctx->appsrc = (GstAppSrc *)appsrc;
  ctx->isRead = 1;

  // g_signal_connect(appsrc, "need-data", (GCallback)need_data, streamid);
  // gst_object_unref(appsrc);
  gst_object_unref(element);
}
int add_new_stream(gchar *streamId)
{
  GstRTSPMediaFactory *factory;
  gchar pipe[700];
  gchar mountpoint[30];
  factory = gst_rtsp_media_factory_new();
  sprintf(pipe, "appsrc name=%s is-live=true ! capsfilter caps=\"video/x-h264, width=(int)1280,fps=30, height=(int)720, framerate=(fraction)30/1, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \"  !  h264parse ! rtph264pay name=pay0 pt=96", streamId);

  g_signal_connect(factory, "media-configure", (GCallback)media_configure, (gpointer)streamId);

  gst_rtsp_media_factory_set_launch(factory, pipe);

  sprintf(mountpoint, "/%s", streamId);
  gst_rtsp_mount_points_add_factory(mounts, mountpoint, factory);
  g_print("New stream ready at rtsp://127.0.0.1:%s/%s\n", port, streamId);
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
  check_err(server != NULL, "nullllllllllllllllllllllllllllllllllllllllllllllllllllll", 1);

  g_object_set(server, "service", port, NULL);
  printf("initialized RTSP Server Listening on Port %s \n", port);
  mounts = gst_rtsp_server_get_mount_points(server);
  // add_new_stream("Streamid");

  g_object_unref(mounts);

  gst_rtsp_server_attach(server, NULL);

  g_main_loop_run(loop);
}
