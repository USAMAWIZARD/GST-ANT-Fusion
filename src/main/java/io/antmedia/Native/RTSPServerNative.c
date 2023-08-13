#define PIPE_GSTREAMER "Gstreamer"
#define PIPE_FFMPEG "FFmpeg"
#define PIPE_RTSP "RTSP"
#define RTP_OUT "RTP_OUT"
#define PIPE_RTMP "RTMP_OUT"

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

#define AV_ERROR_BUFFER_SIZE 512
static char *port = "8554";
GstRTSPMountPoints *mounts;
GHashTable *hash_table;
const AVBitStreamFilter *annexbfilter;
AVBSFContext *bsfContext = NULL;

AVCodecContext *codec_ctx;
AVCodec *codec;
AVPacket *outpacket;

char av_err_buffer[AV_ERROR_BUFFER_SIZE];

typedef struct
{
  GstAppSrc *videoappsrc;
  GstAppSrc *audioappsrc;
  AVCodecParameters *audiopar;
  AVCodecParameters *videopar;
  AVRational *rational;
  GstElement *pipeline;
  volatile gboolean pipeline_initialized;
  AVPacket *video_avpkt;
  AVPacket *audio_avpkt;
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

void onPacket(AVPacket *pkt, gchar *streamId, int pktType);
void init_rtsp_server();
char *register_pipeline(gchar *streamId, gchar *pipeline_type, gchar *pipeline);
char *add_gstreamer_pipeline(char *pipeline, char *streamId);
char *add_rtsp_pipeline(gchar *streamId);
void setStreamInfo(char *streamId, AVCodecParameters *codecPar, AVRational *rational, int stream_type);
void init_Codec(StreamMap *stream);

void onPacket(AVPacket *pkt, gchar *streamId, int pktType)
{

  if (g_hash_table_contains(hash_table, streamId))
  {
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);
    if (ctx->pipeline_initialized && ctx->videopar != NULL)
    {
      int ret = av_bsf_send_packet(bsfContext, pkt);
      if (ret != 0 && ret != AVERROR(EAGAIN))
      {
        av_strerror(ret, av_err_buffer, AV_ERROR_BUFFER_SIZE);
        g_error("can't push av_packet to filter\n");
        fprintf(stdout, "can't push av_packet to filter: %s: %s:%d\n", av_err_buffer, __FILE__, __LINE__);
        return;
      }
      ret = av_bsf_receive_packet(bsfContext, ctx->video_avpkt);
      if (ret != 0 && ret != AVERROR(EAGAIN))
      {
        av_strerror(ret, av_err_buffer, AV_ERROR_BUFFER_SIZE);
        fprintf(stdout, "can't recieve av_packet from filter: %s: %s:%d\n", av_err_buffer, __FILE__, __LINE__);
        return;
      }
      pkt = ctx->video_avpkt;
      GstBuffer *buffer = gst_buffer_new_and_alloc(pkt->size);
      uint8_t *data = (uint8_t *)pkt->data;
      gst_buffer_fill(buffer, 0, data, pkt->size);

      g_assert(ctx->videoappsrc);
      if (pktType == PACKET_TYPE_VIDEO)
      {
        gst_app_src_push_buffer((GstAppSrc *)ctx->videoappsrc, buffer);
      }
      else if (pktType == PACKET_TYPE_AUDIO)
      {
        // TODO:
        gst_app_src_push_buffer((GstAppSrc *)ctx->audioappsrc, buffer);
      }
      else
      {
        g_assert(0 && "UNSUPPORTED MEDIA TYPE");
        return;
      }
    }
  }
  else
  {
    printf("packet recieved for unregisted stream\n");
  }
}

static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer streamId)
{

  if (g_hash_table_contains(hash_table, streamId))
  {
    g_print("-----------play-request-------------\n");
    GstElement *element, *videoappsrc, *audioappsrc;
    element = gst_rtsp_media_get_element(media);

    char app_src[128];
    sprintf(app_src, "video_%s", (char *)streamId);
    videoappsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)app_src);
    // printf("initializing video app src %s\n",app_src);

    sprintf(app_src, "audio_%s", (char *)streamId);
    audioappsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)app_src);

    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, (gchar *)streamId);
    g_assert(videoappsrc);

    ctx->videoappsrc = (GstAppSrc *)videoappsrc;
    // printf("initializing video app src\n");
    ctx->audioappsrc = (GstAppSrc *)audioappsrc;
    ctx->pipeline_initialized = 1;
    ctx->pipeline = element;

    gst_object_unref(element);
  }
  else
  {
    fprintf(stdout, "media_configure(): streamId (%s) it doesn't exist in hashmap\n", (gchar *)streamId);
    return;
  }
}
static gboolean
timeout(GstRTSPServer *server)
{
  GstRTSPSessionPool *pool;
  pool = gst_rtsp_server_get_session_pool(server);
  gst_rtsp_session_pool_cleanup(pool);
  g_object_unref(pool);

  return TRUE;
}
char *register_pipeline(gchar *streamId, gchar *pipeline_type, gchar *pipeline)
{

  // register appropriate pipline
  streamId = strdup(streamId);
  pipeline_type = strdup(pipeline_type);
  pipeline = strdup(pipeline);
  if (g_hash_table_contains(hash_table, streamId))
  {

    if (g_strcmp0(pipeline_type, PIPE_GSTREAMER) == 0)
    {
      char *err = add_gstreamer_pipeline(pipeline, streamId);
      return err;
    }
    else if (g_strcmp0(pipeline_type, PIPE_FFMPEG) == 0)
    {
    }
    else if (g_strcmp0(pipeline_type, PIPE_RTSP) == 0)
    {
      add_rtsp_pipeline(streamId);
    }
    else if (g_strcmp0(pipeline_type, PIPE_RTMP) == 0)
    {
      char rtp_pipe[128];
      sprintf(rtp_pipe, "flvmux ! rtmpsink location=rtmp://127.0.0.1/rtmpout?streamid=rtmpout/%s_rtmp_out", streamId);
      char *err = add_gstreamer_pipeline(rtp_pipe, streamId);

    }

    else if (g_strcmp0(pipeline_type, RTP_OUT) == 0)
    {
      // RTP
      // SRT
      // rtmpsink

      char rtp_pipe[128];
      sprintf(rtp_pipe, "rtph264pay ! %s host=%s port=%s", "udpsink", "127.0.0.1", "8000"); // TODO: make generic for major protocols
      char *err =add_gstreamer_pipeline(rtp_pipe, streamId);
    }
    else
    {
      // invalid option
      printf("handler on streamId (%s) for pipeline (%s) is not defined\n", streamId, pipeline_type);
      return 0;
    }
  }
  else
  {
    char err[256];
    sprintf(err, "stream with streamId (%s) doesn't exist", streamId);
    char * err_str = strdup(err);
    return err_str; 

    // sreamid not found
  }
  return NULL;
}

void init_Codec(StreamMap *stream)
{
  // AVBitStreamFilter * h264bsfc = av_bsf_get_by_name("h264_mp4toannexb");
  // bsfContext = new AVBSFContext(null);

  // av_bsf_alloc(h264bsfc, bsfContext);

  // AVCodecParameters codecpar = inputContext.streams(videoIndex).codecpar();
  // avcodec_parameters_copy(bsfContext.par_in(), codecpar );
  // videoTimebase = inputContext.streams(videoIndex).time_base();

  // bsfContext.time_base_in(videoTimebase);
  // av_bsf_init(bsfContext);
  // videoTimebase = bsfContext.time_base_out();
  if (bsfContext == NULL)
  {
    printf("Initializing Filters \n");
    annexbfilter = av_bsf_get_by_name("h264_mp4toannexb");

    if (annexbfilter == NULL)
    {
      printf("can't find filter\n");
    }
    printf("it didnt crash 1\n");
    if (av_bsf_alloc(annexbfilter, &bsfContext) != 0)
    {
      printf("bsf allocation failed\n");
    }

    printf("it didnt crash 2\n");
    AVCodecParameters *codecpar = stream->videopar;
    avcodec_parameters_copy(bsfContext->par_in, codecpar);
    printf("it didnt crash 3\n");
    av_bsf_init(bsfContext);
  }
  stream->video_avpkt = av_packet_alloc(); // remove from here and alloc only once

  printf("Filters Initialized \n");
}

// rtsp section

void init_rtsp_server()
{
  // file = fopen("nal_packets.h264", "w+");
  // if (file  == NULL){

  //}
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaFactory *factory;
  GError *error = NULL;
  hash_table = g_hash_table_new(g_str_hash, g_str_equal);

  setenv("GST_DEBUG", "4", 1);
  setenv("GST_DEBUG_FILE", "/home/usama/loggggggggggggggggggggggg.log", 1);

  gst_init(NULL, NULL);
  g_print("-----------------------------------------------%s \n", getenv("GST_DEBUG"));

  loop = g_main_loop_new(NULL, FALSE);
  server = gst_rtsp_server_new();
  g_assert(server);

  // g_object_set(server, "service", port, NULL);
  printf("initialized RTSP Server Listening on Port %s \n", port);

  mounts = gst_rtsp_server_get_mount_points(server);
  // add_rtsp_pipeline("Streamid");

  g_object_unref(mounts);

  gst_rtsp_server_attach(server, NULL);
  g_timeout_add_seconds(2, (GSourceFunc)timeout, server);

  g_main_loop_run(loop);
}
static void
media_constructed(GstRTSPMediaFactory *factory, GstRTSPMedia *media)
{
  guint i, n_streams;

  n_streams = gst_rtsp_media_n_streams(media);

  for (i = 0; i < n_streams; i++)
  {
    GstRTSPAddressPool *pool;
    GstRTSPStream *stream;
    gchar *min, *max;

    stream = gst_rtsp_media_get_stream(media, i);

    /* make a new address pool */
    pool = gst_rtsp_address_pool_new();

    min = g_strdup_printf("224.3.0.%d", (2 * i) + 1);
    max = g_strdup_printf("224.3.0.%d", (2 * i) + 2);
    gst_rtsp_address_pool_add_range(pool, min, max,
                                    5000 + (10 * i), 5010 + (10 * i), 1);
    g_free(min);
    g_free(max);

    gst_rtsp_stream_set_address_pool(stream, pool);
    g_object_unref(pool);
  }
}

char *add_rtsp_pipeline(gchar *streamId)
{
  GstRTSPMediaFactory *factory;
  gchar pipe[700];
  gchar mountpoint[30];
  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_shared(factory, TRUE);
  g_signal_connect(factory, "media-constructed", (GCallback)media_constructed, NULL);
  // video only pipeline
  // sprintf(pipe, "appsrc name=video_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=high \"  !  h264parse ! rtph264pay name=pay0 pt=96", streamId);
  sprintf(pipe, "appsrc name=video_%s is-live=true  do-timestamp=true  ! queue ! capsfilter caps=\"video/x-h264 \" ! h264parse   ! rtph264pay name=pay0 pt=96", streamId);

  // audio video pipeline
  // sprintf(pipe, "appsrc name=video_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \"  !  h264parse ! rtph264pay name=pay0 pt=96 "
  //"appsrc name=audio_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"audio/x-opus \" ! opusparse ! rtpopuspay name=pay1 pt=97 ", streamId,streamId);

  g_signal_connect(factory, "media-configure", (GCallback)media_configure, (gpointer)streamId);

  gst_rtsp_media_factory_set_launch(factory, pipe);

  sprintf(mountpoint, "/%s", streamId);
  gst_rtsp_mount_points_add_factory(mounts, mountpoint, factory);
  g_print("New stream ready at rtsp://127.0.0.1:%s/%s\n", port, streamId);

  return 0;
}

// gstreamer s
char *add_gstreamer_pipeline(char *pipeline, char *streamId)
{
  gchar pipe[700];
  gchar mountpoint[30];
  StreamMap *ctx;
  GError *err = NULL;

  if (g_hash_table_contains(hash_table, streamId))
  {
    ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);

    sprintf(pipe, "appsrc name=%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264\" !  h264parse ! %s", streamId, pipeline);

    // video only
    // sprintf(pipe, "appsrc name=%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264, alignment=(string)nal, stream-format=(string)byte-stream,profile=baseline \" !  h264parse ! %s",
    //         streamId, pipeline);

    // audio video

    // printf("---------starting streamer pipeline-----------\n");
    // printf("%s\n", pipe);
    ctx->pipeline = gst_parse_launch(pipe, &err);
    if (err != NULL)
    {
      printf("%s\n", err->message);
      char *error = strdup(err->message);
      g_error_free(err);
      return error;
    }
    gst_element_set_state(ctx->pipeline, GST_STATE_PLAYING);
    ctx->pipeline_initialized = 1;
    ctx->videoappsrc = (GstAppSrc *)gst_bin_get_by_name((GstBin *)ctx->pipeline, streamId);
    return 0;
  }
  else
  {
    printf("hash table doesn't contains pipeline in %s:%d\n", __FILE__, __LINE__);
    return NULL;
  }
  return NULL;
}
int add_ffmpeg_pipeline(char *pipeline, char *streamId)
{
  return 0;
}

void setStreamInfo(char *streamId, AVCodecParameters *codecPar, AVRational *rational, int stream_type) // yaha pe copy karna hai problem aasakta hai dealloc ka
{

  printf("setting Stream info\n");
  streamId = strdup(streamId);

  if (hash_table == NULL)
  {
    printf("hash table is null\n");
  }
  if (g_hash_table_contains(hash_table, streamId))
  {
    if (codecPar == NULL)
    {
      printf("codec parameter is null\n");
    }
    printf("found stream (%s) in hashtable\n", streamId);
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);
    if (stream_type == PACKET_TYPE_VIDEO)
    {

      ctx->videopar = codecPar;
      ctx->rational = rational;
      g_print("Video Width: %d  ----------- Video Height %d \n", ctx->videopar->width, ctx->videopar->height);
      init_Codec(ctx);
    }
    else if (stream_type == PACKET_TYPE_AUDIO)
      ctx->audiopar = codecPar;
    printf("streaminfo set %s \n", streamId);
  }
  else
  {
    printf("could not set the streaminfo ID does not exist\n");
  }
  free(streamId);
}

void register_stream(char *streamId)
{
  printf("registering Stream %s\n", streamId);
  StreamMap *ctx = g_new0(StreamMap, 1);
  ctx->pipeline_initialized = 0;
  g_hash_table_insert(hash_table, strdup(streamId), ctx);

  // add_rtsp_pipeline(strdup(streamId));

  printf("registered  Stream %s\n", streamId);
}
void unregister_stream(char *streamId) // setfault
{
  char *streamid_d = strdup(streamId);
  if (g_hash_table_contains(hash_table, streamid_d))
  {
    printf("unregistering Stream %s\n", streamid_d);
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamid_d);
    if (ctx->videoappsrc != NULL)
    {
      printf("unrefing video src to null\n ");
      gst_object_unref(ctx->videoappsrc);
      printf("video src set to null\n ");
    }
    else
    {
      printf("video src already null\n ");
    }
    if (ctx->pipeline != NULL) // crash ho sakta hai state check karna pade ga
    {
      // printf("%s\n",gst_element_set_state(ctx->pipeline));
      gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
      printf("unrefing pipelien\n ");
      gst_object_unref(ctx->pipeline);
      printf("seted pipelien to null\n ");
    }
    else
    {
      printf("pipelien already null\n ");
    }
    g_hash_table_remove(hash_table, streamid_d);
    printf("stream unregistered %s\n", streamid_d);
  }
  else
  {
    printf("cannot unregistersream does not exist %s", streamid_d);
  }
  free(streamid_d);
}
