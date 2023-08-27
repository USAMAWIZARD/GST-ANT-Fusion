#define PIPE_GSTREAMER "Gstreamer"
#define PIPE_FFMPEG "FFmpeg"
#define PIPE_RTSP "RTSP"
#define RTP_OUT "RTP_OUT"
#define PIPE_RTMP "RTMP_OUT"

#define PACKET_TYPE_VIDEO 0
#define PACKET_TYPE_AUDIO 1
#define H264Parser "h264parse"
#define OPUSParser "opusparse"

#include <stdio.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/app/gstappsrc.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <gstreamer-1.0/gst/libav/gstav.h>
#include <gst/audio/audio.h>

#include <gstreamer-1.0/gst/video/video.h>

#define AV_ERROR_BUFFER_SIZE 512

static char *port = "8554";
GstRTSPMountPoints *mounts;
GHashTable *hash_table;
AVBSFContext *bsfContext = NULL;
pthread_mutex_t hashtable_mutex;

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
  AVPacket *video_avpkt;
  AVPacket *audio_avpkt;
  char *video_caps;
  char *audio_caps;
  char *audio_playloader;
  char *video_payloader;
  char *video_parser;
  char *audio_parser;
  char *rtsp_mountpoint;
  char *demuxer;
  AVRational *rational;
  GstElement *pipeline;
  volatile gboolean pipeline_initialized;
  AVBSFContext *bsfContext;
} volatile StreamMap;

typedef struct
{
  char *audioparser;
  char *videoparser;
} parsed_data;

enum PIPELINE_TYPE
{
  PIPELINE_TYPE_GSTREAMER,
  PIPELINE_TYPE_RTSP,
  PIPELINE_TYPE_FFMPEG,
  PIPELINE_TYPE_ERROR,
};
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
volatile int stream_count;

void onPacket(AVPacket *pkt, gchar *streamId, int pktType);
void init_rtsp_server();
char *register_pipeline(gchar *streamId, gchar *pipeline_type, gchar *pipeline);
char *add_gstreamer_pipeline(char *streamId, char *pipeline);
char *add_rtsp_pipeline(gchar *streamId, char *pipeline);
void setStreamInfo(char *streamId, AVCodecParameters *codecPar, AVRational *rational, int stream_type);
int init_Codec(StreamMap *stream);
AVBitStreamFilter *return_filter_and_setup_parser_and_also_setup_payloader(StreamMap *stream, int codecId);

void onPacket(AVPacket *pkt, gchar *streamId, int pktType)
{
  if (g_hash_table_contains(hash_table, streamId))
  {
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);
    if (ctx->pipeline_initialized)
    {
      switch (pktType)
      {
      case PACKET_TYPE_VIDEO:
      {
        if (ctx->video_caps == NULL && ctx->videoappsrc == NULL)
        {
          // printf("video caps are null\n");
          return;
        }
        if (ctx->bsfContext == NULL)
        {
          fprintf(stdout, "bsf context is null\n");
          return;
        }
        int ret = av_bsf_send_packet(ctx->bsfContext, pkt);
        if (ret != 0 && ret != AVERROR(EAGAIN))
        {
          av_strerror(ret, av_err_buffer, AV_ERROR_BUFFER_SIZE);
          g_error("can't push av_packet to filter\n");
          fprintf(stdout, "can't push av_packet to filter: %s: %s:%d\n", av_err_buffer, __FILE__, __LINE__);
          return;
        }
        ret = av_bsf_receive_packet(ctx->bsfContext, ctx->video_avpkt);
        if (ret != 0 && ret != AVERROR(EAGAIN))
        {
          av_strerror(ret, av_err_buffer, AV_ERROR_BUFFER_SIZE);
          fprintf(stdout, "can't recieve av_packet from filter: %s: %s:%d\n", av_err_buffer, __FILE__, __LINE__);
          return;
        }
        pkt = ctx->video_avpkt; // annexedb
        GstBuffer *buffer = gst_buffer_new_and_alloc(pkt->size);
        uint8_t *data = (uint8_t *)pkt->data;
        gst_buffer_fill(buffer, 0, data, pkt->size);
        g_assert(ctx->videoappsrc);
        gst_app_src_push_buffer((GstAppSrc *)ctx->videoappsrc, buffer);
        //     printf("pushing vdio packets to vid sink\n");
      }
      break;
      case PACKET_TYPE_AUDIO:
      {
        //  printf("recieved audio packets\n");

        if (ctx->audio_caps != NULL && ctx->audioappsrc !=NULL)
        {
          // TODO: add audio shit
          GstBuffer *buffer = gst_buffer_new_and_alloc(pkt->size);
          uint8_t *data = (uint8_t *)pkt->data;
          gst_buffer_fill(buffer, 0, data, pkt->size);
          g_assert(ctx->audioappsrc);
          gst_app_src_push_buffer((GstAppSrc *)ctx->audioappsrc, buffer);
          //    printf("pushing audio packets to audio sink\n");
        }
        break;
      }
      default:
      {
        printf("UNSUPPORTED MEDIA TYPE :%d\n", pktType);
        g_assert(0 && "UNSUPPORTED MEDIA TYPE");
        return;
      }
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
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, (gchar *)streamId);

    stream_count += 1;
    g_print("-----------play-request-------------( %s )    %d\n", (char *)streamId, stream_count);
    GstElement *element, *videoappsrc, *audioappsrc;
    element = gst_rtsp_media_get_element(media);
    ctx->pipeline = element;

    char app_src[128];
    if (ctx->video_caps != NULL)
    {
      sprintf(app_src, "video_%s", (char *)streamId);
      videoappsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)app_src);
      ctx->videoappsrc = (GstAppSrc *)videoappsrc;

      g_assert(videoappsrc);
    }
    if (ctx->audio_caps != NULL)
    {
      sprintf(app_src, "audio_%s", (char *)streamId);
      audioappsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), (gchar *)app_src);
      ctx->audioappsrc = (GstAppSrc *)audioappsrc;
      g_assert(audioappsrc);
    }
    
    // printf("initializing video app src\n");
    ctx->pipeline_initialized = 1;
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
char *strcat_dyn(char *str1, char *str2)
{
  size_t len1 = snprintf(NULL, 0, str1);
  size_t len2 = snprintf(NULL, 0, str2);
  char *str = malloc(sizeof(char) * (len1 + len2) + 1);
  strcpy(str, str1);
  strcpy(str + len1, str2);
  return str;
}
parse_codec(StreamMap *ctx)
{
  char *str_caps;

  AVCodec *decoder;
  AVCodecContext *codec_context;

  if (ctx->videopar != NULL)
  {
    decoder = (AVCodec *)avcodec_find_decoder(ctx->bsfContext->par_out->codec_id);

    codec_context = avcodec_alloc_context3(decoder);
    if (avcodec_parameters_to_context(codec_context, ctx->bsfContext->par_out) < 0)
      g_assert(1);
    printf("extradata %d , %d \n", codec_context->extradata_size, ctx->videopar->codec_id);
    printf("extradata , %d \n", ctx->bsfContext->par_out->extradata);
    int extradata_size = codec_context->extradata_size;
    codec_context->extradata_size = 0;
    gst_ffmpeg_codecid_to_caps(ctx->videopar->codec_id, codec_context, 1, &str_caps);
    codec_context->extradata_size = extradata_size;
    ctx->video_caps = str_caps;
  }
  if (ctx->audiopar != NULL)
  {
    decoder = (AVCodec *)avcodec_find_decoder(ctx->audiopar->codec_id);
    codec_context = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(codec_context, ctx->audiopar);
    gst_ffmpeg_codecid_to_caps(ctx->audiopar->codec_id, codec_context, 1, &str_caps);
    ctx->audio_caps = str_caps;
    printf("got the audio caps %s\n", str_caps);
  }
}
enum PIPELINE_TYPE generate_gst_pipeline(char **pipeline_out, StreamMap *stream_ctx, char *streamId, char *pipeline_type, char *pipeline)
{
  // TODO: generate pipeline based on the codec
  parse_codec(stream_ctx);
  char *common_pipeline = " ", *common_video, *common_audio;
  if (stream_ctx->video_caps == NULL && stream_ctx->audio_caps == NULL)
  {
    *pipeline_out = strdup("video and audio caps are null");
    return PIPELINE_TYPE_ERROR;
  }

  if (stream_ctx->video_caps != NULL)
  {
    common_video = g_strdup_printf("appsrc name=video_%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"%s\" ! %s name=video  ", streamId, stream_ctx->video_caps, stream_ctx->video_parser);
    common_pipeline = strcat_dyn(common_pipeline, common_video);
    free(common_video);
  }
  if (stream_ctx->audio_caps != NULL)
  {
    common_audio = g_strdup_printf("appsrc name=audio_%s is-live=true do-timestamp=true  !  queue ! capsfilter caps=\"%s\" ! %s  name=audio  ", streamId, stream_ctx->audio_caps, stream_ctx->audio_parser);
    common_pipeline = strcat_dyn(common_pipeline, common_audio);
    printf("%s\n", common_pipeline);
    free(common_audio);
  }
  if (g_strcmp0(pipeline_type, PIPE_GSTREAMER) == 0)
  {
    *pipeline_out = strcat_dyn(common_pipeline, pipeline);
    free(common_pipeline);
    printf("pipeline generated :\n%s\n", *pipeline_out);
    return PIPELINE_TYPE_GSTREAMER;
  }
  else if (g_strcmp0(pipeline_type, PIPE_RTSP) == 0)
  {
    // memory leak 2 different allocation for audio and video
    char pay = 0;
    if (stream_ctx->video_caps != NULL)
    {
      common_pipeline = g_strdup_printf("%s video.  ! %s pt=96 name=pay%d ", common_pipeline, stream_ctx->video_payloader, pay);
      pay++;
    }
    if (stream_ctx->audio_caps != NULL)
    {
      common_pipeline = g_strdup_printf("%s audio.  ! %s pt=97 name=pay%d ", common_pipeline, stream_ctx->audio_playloader, pay);
      pay++;
    }
    // have to look into this too\n
    *pipeline_out = strdup(common_pipeline);
    printf("pipeline generated :\n%s\n", *pipeline_out);
    free(common_pipeline);
    return PIPELINE_TYPE_RTSP;
  }
  else if (g_strcmp0(pipeline_type, PIPE_RTMP) == 0)
  {
    char *rtmp_pipline = g_strdup_printf(" flvmux ! rtmpsink location=\"rtmp://127.0.0.1/rtmpout?streamid=rtmpout/%s_rtmp_out\"", streamId);
    *pipeline_out = strcat_dyn(common_pipeline, rtmp_pipline);
    free(rtmp_pipline);
    return PIPELINE_TYPE_GSTREAMER;
  }
  else if (g_strcmp0(pipeline_type, RTP_OUT) == 0)
  {
    char *rtp_pipeline = g_strdup_printf(" rtph264pay ! %s host=%s port=%s", "udpsink", "127.0.0.1", "8000");
    *pipeline_out = strcat_dyn(common_pipeline, rtp_pipeline);
    free(rtp_pipeline);
    printf("pipeline generated : %s\n", *pipeline_out);
    return PIPELINE_TYPE_GSTREAMER;
  }
  else if (g_strcmp0(pipeline_type, PIPE_FFMPEG) == 0)
  {

    // invalid option
    printf("handler on streamId (%s) for pipeline (%s) is not defined\n", streamId, pipeline_type);
    return PIPELINE_TYPE_ERROR;
  }
  else
  { // invalid option
    printf("handler on streamId (%s) for pipeline (%s) is not defined\n", streamId, pipeline_type);
    return PIPELINE_TYPE_ERROR;
  }

  // TODO: generate pipeline based on the codec
  // const char *STATIC_PIPELINE = "appsrc name=%s is-live=true  do-timestamp=true ! queue ! capsfilter caps=\"video/x-h264\" !  h264parse ! %s";
  // char *pipe = malloc(sizeof(char) * strlen(STATIC_PIPELINE) + strlen(streamId) + strlen(pipeline));
  // sprintf(pipe, STATIC_PIPELINE, streamId, pipeline);
  // printf("printing dynamically generated pipeline\n");
  // printf("%s\n", pipe);
}

char *register_pipeline(gchar *streamId, gchar *pipeline_type, gchar *pipeline) // Pipeline par is only for user's Custom pipeline otherwise we genrate our
{
  // register appropriate pipline
  streamId = strdup(streamId);
  pipeline_type = strdup(pipeline_type);
  pipeline = strdup(pipeline);

  if (g_hash_table_contains(hash_table, streamId))
  {

    char *pipeline_out;

    StreamMap *stream_ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);
    enum PIPELINE_TYPE type = generate_gst_pipeline(&pipeline_out, stream_ctx, (char *)streamId, (char *)pipeline_type, (char *)pipeline);
    switch (type)
    {
    case PIPELINE_TYPE_GSTREAMER:
    {
      char *err = add_gstreamer_pipeline(streamId, pipeline_out);
      return err;
    }
    case PIPELINE_TYPE_RTSP:
    {
      char *err = add_rtsp_pipeline(streamId, pipeline_out);
      return err;
    }
    case PIPELINE_TYPE_ERROR:
    {
      char *err = pipeline_out;
      if (err == NULL)
      {
        return strdup("error can't generate pipeline\n");
      }
      else
      {
        return pipeline_out;
      }
      break;
    }
    case PIPELINE_TYPE_FFMPEG:
    // fallthrough
    default:
    {

      char err[256];
      sprintf(err, "handler for pipeline type (%s) not defined", pipeline_type);
      char *err_str = strdup(err);
      return err_str;
    }
    }
  }
  else
  {
    char err[256];
    sprintf(err, "stream with streamId (%s) doesn't exist", streamId); // memory leak
    char *err_str = strdup(err);
    return err_str;
    // sreamid not found
  }
  return NULL;
}

AVBitStreamFilter *return_filter_and_setup_parser_and_also_setup_payloader(StreamMap *stream, int codecId)
{
  printf("setting payloder and parser for codec ID %d\n",codecId);
  const AVBitStreamFilter *video_stream_filter = NULL;
  switch (codecId)
  {
  case AV_CODEC_ID_H264:
    // stream->video_payloader = " rtph264pay  ";
    video_stream_filter = av_bsf_get_by_name("h264_mp4toannexb");
    printf("h264 initilaized \n");
    stream->video_payloader = " rtph264pay  ";
    stream->video_parser = " h264parse  ";
    break;

  case AV_CODEC_ID_VP8: // vp8
    stream->video_payloader = " rtpvp8pay  ";
    stream->video_parser = " queue  ";
    video_stream_filter = av_bsf_get_by_name("null");
    break;

  case AV_CODEC_ID_H265:
    stream->video_payloader = " rtph265pay  ";
    stream->video_parser = " h265parse ";
    video_stream_filter = av_bsf_get_by_name("hevc_mp4toannexb");
    break;
  case AV_CODEC_ID_MP2:
  case AV_CODEC_ID_AAC:
    printf("setting audio thing\n");
    stream->audio_parser = " aacparse  ";
    stream->audio_playloader = " rtpmp4apay ";
    printf("setting audio thing\n");
    break;

  case AV_CODEC_ID_MPEG1VIDEO :
  case AV_CODEC_ID_MPEG2VIDEO:
    printf("setting video thing\n");
    stream->audio_parser = " mpegvideoparse  ";
    stream->audio_playloader = " rtpmp4vpay ";
    printf("setting video thing\n");
    break;


  default:
    video_stream_filter = av_bsf_get_by_name("null");
    printf("null filter \n");
    break;
  }

  return video_stream_filter;
}

int init_Codec(StreamMap *stream)
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

  printf("Initializing Filters %d \n", stream->videopar->codec_id);
  AVBitStreamFilter *video_stream_filter = return_filter_and_setup_parser_and_also_setup_payloader(stream, stream->videopar->codec_id);
  if (video_stream_filter == NULL)
  {
    printf("can't find filter\n");
    return -1;
  }
  printf("filter initialized \n");
  if (av_bsf_alloc(video_stream_filter, &stream->bsfContext) < 0)
  {
    printf("bsf allocation failed\n");
    g_assert(1);
    return -1;
  }

  printf("filter allocated \n");
  AVCodecParameters *codecpar = stream->videopar;
  // bsfContext->time_base_in = *(stream->rational);
  if (avcodec_parameters_copy(stream->bsfContext->par_in, codecpar) < 0)
    g_assert(1);
  printf("parameters copied \n");
  av_bsf_init(stream->bsfContext);

  stream->video_avpkt = av_packet_alloc(); // remove from here and alloc only once

  printf("Filters Initialized \n");
  return 1;
}

// rtsp section

void init_rtsp_server()
{
  // file = fopen("nal_packets.h264", "w+");
  // if (file  == NULL){

  //}
  pthread_mutex_init(&hashtable_mutex, NULL);
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaFactory *factory;
  GError *error = NULL;
  hash_table = g_hash_table_new(g_str_hash, g_str_equal);

  setenv("GST_DEBUG", "4", 1);
  setenv("GST_DEBUG_FILE", "/home/usama/gstlogs/gstreamer.log", 0);
  setenv("GST_DEBUG_DUMP_DOT_DIR", "./", 1);

  gst_init(NULL, NULL);
  g_print("-----------------------------------------------%s \n", getenv("GST_DEBUG"));

  loop = g_main_loop_new(NULL, FALSE);
  server = gst_rtsp_server_new();
  g_assert(server);

  // g_object_set(server, "service", port, NULL);
  printf("initialized RTSP Server Listening on Port %s \n", port);

  mounts = gst_rtsp_server_get_mount_points(server);

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

char *add_rtsp_pipeline(gchar *streamId, char *pipeline)
{
  if (!g_hash_table_contains(hash_table, streamId))
  {
    size_t err_len = snprintf(NULL, 0, "streamId (%s) doesn't exists", streamId);
    char *err = malloc(sizeof(char) * err_len);
    sprintf(err, "streamId (%s) doesn't exists", streamId);
    return err;
  }
  GstRTSPMediaFactory *factory;
  gchar *mountpoint = malloc(128);
  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_shared(factory, TRUE);
  g_signal_connect(factory, "media-constructed", (GCallback)media_constructed, NULL);

  StreamMap *stream_ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);

  g_signal_connect(factory, "media-configure", (GCallback)media_configure, (gpointer)streamId);

  gst_rtsp_media_factory_set_launch(factory, pipeline);

  sprintf(mountpoint, "/%s", streamId);
  stream_ctx->rtsp_mountpoint = mountpoint;
  gst_rtsp_mount_points_add_factory(mounts, mountpoint, factory);
  g_print("New stream ready at rtsp://127.0.0.1:%s/%s\n", port, streamId);
  return 0;
}

// gstreamer s
char *add_gstreamer_pipeline(char *streamId, char *pipeline)
{
  gchar pipe[1200];
  gchar mountpoint[30];
  StreamMap *ctx;
  GError *err = NULL;
  if (g_hash_table_contains(hash_table, streamId))
  {
    ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);

    ctx->pipeline = gst_parse_launch(pipeline, &err);
    if (err != NULL)
    {
      printf("%s\n", err->message);
      char *error = strdup(err->message);
      g_error_free(err);
      return error;
    }
    gst_element_set_state(ctx->pipeline, GST_STATE_PLAYING);

    char appsrc[256];
    sprintf(appsrc, "video_%s", streamId);
    ctx->videoappsrc = (GstAppSrc *)gst_bin_get_by_name((GstBin *)ctx->pipeline, appsrc);

    sprintf(appsrc, "audio_%s", (char *)streamId);
    ctx->audioappsrc = gst_bin_get_by_name_recurse_up((GstBin *)ctx->pipeline, appsrc);
    ctx->pipeline_initialized = 1;
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
      printf("--------------------------------------------------initializing video filters\n");
      printf("setting Stream info VIDEO %s Codecid %d \n", streamId, codecPar->codec_id);
      ctx->videopar = codecPar;
      ctx->rational = rational;
      init_Codec(ctx);
    }
    else if (stream_type == PACKET_TYPE_AUDIO)
    {
      ctx->audiopar = codecPar;
      printf("--------------------------------------------------initializing audio filters\n");
      return_filter_and_setup_parser_and_also_setup_payloader(ctx, ctx->audiopar->codec_id);
    }
    else
    {
      printf("could not set the streaminfo ID does not exist\n");
    }
    free(streamId);
  }
}
void register_stream(char *streamId)
{
  printf("registering Stream %s\n", streamId);
  StreamMap *ctx = g_new0(StreamMap, 1);
  ctx->pipeline_initialized = 0;
  g_hash_table_insert(hash_table, strdup(streamId), ctx);
}
void unregister_stream(char *streamId) // TODO : Free allocated resources
{
  pthread_mutex_lock(&hashtable_mutex);
  char *streamid_d = strdup(streamId);

  if (g_hash_table_contains(hash_table, streamid_d))
  {
    printf("unregistering Stream (%s)\n", streamid_d);
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamid_d);
    if (ctx->pipeline != NULL)
    {
      // printf("%s\n",gst_element_set_state(ctx->pipeline));
      gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
      ctx->pipeline = NULL;
      printf("unrefing pipelien(%s)\n", streamid_d);
      gst_object_unref(ctx->pipeline);
      printf("seted pipelien to null(%s)\n", streamid_d);
      if(ctx->rtsp_mountpoint != NULL)
      {
        gst_rtsp_mount_points_remove_factory(mounts, ctx->rtsp_mountpoint);
      }


    }
    else
    {
      printf("pipelien already null\n ");
    }
    g_hash_table_remove(hash_table, streamid_d);
    if (g_hash_table_contains(hash_table, streamid_d))
    {
      printf("-------------------------not freed for some reason\n");
    }

    printf("stream unregistered %s\n", streamid_d);
  }
  else
  {
    printf("cannot unregistersream does not exist %s", streamid_d);
  }
  free(streamid_d);
  pthread_mutex_unlock(&hashtable_mutex);
}
