#define PIPE_GSTREAMER "Gstreamer"
#define PIPE_FFMPEG "FFmpeg"
#define PIPE_RTSP "RTSP_OUT"
#define RTP_OUT "RTP_OUT"
#define SRT_OUT "SRT_OUT"
#define PIPE_RTMP "RTMP_OUT"

#define PACKET_TYPE_LICENSE 2
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
#include <strings.h>
#include <gstreamer-1.0/gst/video/video.h>
#include <gstreamer-1.0/gst/libav/gstavutils.h>
#include <stdbool.h>
#include <libsoup/soup.h>

#define AV_ERROR_BUFFER_SIZE 512

GstRTSPMountPoints *mounts;
GHashTable *hash_table;
pthread_mutex_t hashtable_mutex;

char av_err_buffer[AV_ERROR_BUFFER_SIZE];

int api_hitcount = 0;
int shouldsend_packets = 1;
typedef struct
{
  char *rtsp_port;
  char *rtmp_port;
  char *rtsp_protocol;
  gboolean is_rtsp_enabled;
  gboolean is_rtmp_enabled
} plugin_config;
plugin_config config;

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
  AVRational *timebase;
  GstElement *pipeline;
  volatile gboolean pipeline_initialized;
  AVBSFContext *bsfContext;
} volatile StreamMap;

enum PIPELINE_TYPE
{
  PIPELINE_TYPE_GSTREAMER,
  PIPELINE_TYPE_RTSP,
  PIPELINE_TYPE_FFMPEG,
  PIPELINE_TYPE_ERROR,
};
typedef struct
{
  char *streamId;
  char *pipeline_type;
  char *pipeline;
  char *protocol;
  char *port_number;
  char *hostname;
} DataMaper;

volatile int stream_count;

void onPacket(AVPacket *pkt, gchar *streamId, int pktType);
void init_gst_and_RTSP();
char *register_pipeline(DataMaper *pipeline_info);
char *add_gstreamer_pipeline(char *streamId, char *pipeline);
char *add_rtsp_pipeline(gchar *streamId, char *pipeline, char *protocol);
void setStreamInfo(char *streamId, AVCodecParameters *codecPar, AVRational *rational, int is_stream_enabled, int stream_type);
int init_Codec(StreamMap *stream);
AVBitStreamFilter *return_filter_and_setup_parser_and_also_setup_payloader(StreamMap *stream, int codecId);


void print_java_struct_val(DataMaper *pMonitor){
  printf("data ---------------------------%s \n", pMonitor->streamId);
  printf("data ---------------------------%s \n", pMonitor->pipeline_type);
  printf("data ---------------------------%s \n", pMonitor->pipeline);
  printf("data ---------------------------%s \n", pMonitor->protocol);
  printf("data ---------------------------%s \n", pMonitor->port_number);
  printf("data ---------------------------%s \n", pMonitor->hostname);
}
int is_digits(const char *s)
{
  while (*s)
  {
    if (isdigit(*s++) == 0)
      return 0;
  }

  return 1;
}

int get_config()
{
  GKeyFile *keyfile;
  GError *error = NULL;
  gsize length;
  keyfile = g_key_file_new();
  printf( "----------\n");
  int status;
  if (!g_key_file_load_from_file(keyfile, "./gst_plugin.cfg", G_KEY_FILE_NONE, &error))
  {
    if (error != NULL)
    {
      printf("ERROR: can't load config file :%s\n", error->message);
    }
    config.rtsp_port = "8554";
    config.rtmp_port = "1935";
    config.rtsp_protocol = "TCP";
    config.rtsp_protocol = "TCP";
    config.is_rtsp_enabled = 0;
    config.is_rtmp_enabled = 0;
    return;
  }
  printf("----------\n");
  config.is_rtsp_enabled = g_key_file_get_boolean(keyfile, "DefaultProtocols", "RTSP", NULL);
  config.is_rtmp_enabled = g_key_file_get_boolean(keyfile, "DefaultProtocols", "RTMP", NULL);
  char *rtsp_port = g_key_file_get_string(keyfile, "RTSP", "Port", NULL);
  if (rtsp_port == NULL, strcasecmp(rtsp_port, "NULL") == 0 || rtsp_port == "" || !is_digits(rtsp_port))
  {
    config.rtsp_port = "8554";
  }
  else
  {
    printf("retured string null ig\n");
    config.rtsp_port = rtsp_port;
  }

  char *rtsp_protocol = g_key_file_get_string(keyfile, "RTSP", "Protocol", NULL);
  if (rtsp_protocol != NULL && strcasecmp(rtsp_protocol, "UDP") == 0)
  {
    config.rtsp_protocol = rtsp_protocol;
  }
  else
  {
    config.rtsp_protocol = "TCP";
  }

  char *rtmp_port = g_key_file_get_string(keyfile, "RTMP", "Port", NULL);
  if (rtmp_port == NULL, strcasecmp(rtmp_port, "NULL") == 0 || strcasecmp(rtmp_port, "") == 0 || !is_digits(rtmp_port))
  {
    config.rtmp_port = "1935";
  }
  else
  {
    config.rtmp_port = rtmp_port;
  }

  printf("rtsp port: %s\n", config.rtsp_port);
  printf("rtmp port: %s\n", config.rtmp_port);
  printf("protocol: %s\n", config.rtsp_protocol);
  printf("default rtmp:  %d\n", config.is_rtmp_enabled);
  printf("default rtsp:  %d\n", config.is_rtsp_enabled);
}

void onPacket(AVPacket *pkt, gchar *streamId, int pktType)
{
  if (!shouldsend_packets) return;
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
        AVPacket *packet = av_packet_alloc();

        av_packet_ref(packet, pkt);

        int ret = av_bsf_send_packet(ctx->bsfContext, packet);
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
        av_packet_unref(packet);

        pkt = ctx->video_avpkt; // annexedb

        // timestamp related stuff

        // int timestamp = gst_ffmpeg_time_ff_to_gst(pkt->pts, *( ctx->timebase ));
        // printf("%d\n",pkt->duration);
        // printf("%d\n",pkt->duration);
        // int duration = gst_ffmpeg_time_ff_to_gst(pkt->duration, *( ctx->timebase ));
        // printf("%d\n",duration);

        // if (G_UNLIKELY(duration))
        //{
        //     printf( "invalid buffer duration, setting to NONE\n");
        //   duration = GST_CLOCK_TIME_NONE;
        // }

        // GST_BUFFER_TIMESTAMP(buffer) = timestamp;
        // GST_BUFFER_DURATION(buffer) = duration;

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

        if (ctx->audio_caps != NULL && ctx->audioappsrc != NULL)
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

      break;
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
    GST_DEBUG_BIN_TO_DOT_FILE(element, GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

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
static gboolean timeout_register_pipeline(DataMaper *pipeline_info)
{
  printf("1 registerpipeline\n");

  if (!g_hash_table_contains(hash_table, pipeline_info->streamId))
  {
    printf("timeout failed on register pipeline hashtable doesn't contain the stream id\n");
    return FALSE;
  }
  StreamMap *stream_ctx = (StreamMap *)g_hash_table_lookup(hash_table, pipeline_info->streamId);
  if (stream_ctx->videopar == NULL && stream_ctx->videopar == NULL)
  {
    printf("timeout_register_pipeline(): lookup failed \n");
    return TRUE;
  }
  printf("calling registerpipeline\n");
  register_pipeline(pipeline_info);
  return FALSE;
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
      g_assert(1 && "parameter to context failed");
    int extradata_size = codec_context->extradata_size;
    codec_context->extradata_size = 0;
    printf("%d-------------------------------codec id---\n", ctx->videopar->codec_id);
    gst_ffmpeg_codecid_to_caps(ctx->videopar->codec_id, codec_context, 1, &str_caps);
    codec_context->extradata_size = extradata_size;
    ctx->video_caps = str_caps;
    printf("got the video caps %s\n", str_caps);
  }
  if (ctx->audiopar != NULL)
  {
    decoder = (AVCodec *)avcodec_find_decoder(ctx->audiopar->codec_id);
    codec_context = avcodec_alloc_context3(decoder);
    printf("%d-------------------------------codec id---\n", ctx->audiopar->codec_id);

    avcodec_parameters_to_context(codec_context, ctx->audiopar);
    gst_ffmpeg_codecid_to_caps(ctx->audiopar->codec_id, codec_context, 1, &str_caps);
    ctx->audio_caps = str_caps;
    printf("got the audio caps %s\n", str_caps);
  }
}
enum PIPELINE_TYPE generate_gst_pipeline(char **pipeline_out, StreamMap *stream_ctx, DataMaper *pipeline_info)
{

  char *streamId = pipeline_info->streamId;
  char *pipeline_type = pipeline_info->pipeline_type;
  char *pipeline = pipeline_info->pipeline;

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
    // do-timestamp ko hatae to delay aata hai
    common_video = g_strdup_printf("appsrc  do-timestamp=true name=video_%s is-live=true  ! queue ! capsfilter caps=\"%s\" ! %s name=video  ", streamId, stream_ctx->video_caps, stream_ctx->video_parser);
    common_pipeline = strcat_dyn(common_pipeline, common_video);
    free(common_video);
  }
  if (stream_ctx->audio_caps != NULL)
  {
    common_audio = g_strdup_printf("appsrc  do-timestamp=true  name=audio_%s is-live=true ! queue ! capsfilter caps=\"%s\" ! %s name=audio  ", streamId, stream_ctx->audio_caps, stream_ctx->audio_parser);
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
      common_pipeline = g_strdup_printf("%s video.  ! %s pt=96    name=pay%d  ", common_pipeline, stream_ctx->video_payloader, pay);
      pay++;
    }
    if (stream_ctx->audio_caps != NULL)
    {
      common_pipeline = g_strdup_printf("%s audio.  ! %s pt=97   name=pay%d  ", common_pipeline, stream_ctx->audio_playloader, pay);
      pay++;
    }
    // have to look into this too\n
    *pipeline_out = strdup(common_pipeline);
    free(common_pipeline);
    return PIPELINE_TYPE_RTSP;
  }
  else if (g_strcmp0(pipeline_type, PIPE_RTMP) == 0)
  {
    printf("RTMP out port is %s -------------------\n", config.rtmp_port);
    char *rtmp_pipline = g_strdup_printf(" flvmux name=muxer ! rtmpsink location=\"rtmp://127.0.0.1:%s/rtmpout?streamid=rtmpout/%s_rtmp_out\"", config.rtmp_port, streamId);

    if (stream_ctx->video_caps != NULL)
    {
      char *temp = common_pipeline;
      char *encode_video = "";
      if (stream_ctx->videopar->codec_id != AV_CODEC_ID_H264)
      {
     //    encode_video = " ! transcodebin profile=  ";
      }

      common_pipeline = g_strdup_printf(" %s video. %s !  muxer. ", common_pipeline, encode_video);
      free(temp);
    }
    if (stream_ctx->audio_caps != NULL)
    {

      char *temp = common_pipeline;
      char *encode_audio = "";
      if (stream_ctx->audiopar->codec_id != AV_CODEC_ID_AAC)
      {
        encode_audio = " ! decodebin ! voaacenc  ! ";
      }
      common_pipeline = g_strdup_printf(" %s audio. %s  ! muxer. ", common_pipeline, encode_audio);
      free(temp);
    }

    *pipeline_out = strcat_dyn(common_pipeline, rtmp_pipline);
    free(rtmp_pipline);
    return PIPELINE_TYPE_GSTREAMER;
  }
  else if (g_strcmp0(pipeline_type, SRT_OUT) == 0)
  {
    printf("srt port is %s -------------------\n",pipeline_info->port_number);
    char *srt_pipeline = g_strdup_printf(" matroskamux  name=muxer ! srtsink uri=srt://:%s  wait-for-connection=false ",pipeline_info->port_number);

    if (stream_ctx->video_caps != NULL)
    {
      char *temp = common_pipeline;
      char *encode_video = "";
      if (stream_ctx->videopar->codec_id != AV_CODEC_ID_H264)
      {
         encode_video = " ! decodebin ! h264enc ";
      }

      common_pipeline = g_strdup_printf(" %s video.  !  muxer. ", common_pipeline);
      free(temp);
    }
    if (stream_ctx->audio_caps != NULL)
    {
      char *temp = common_pipeline;
      char *encode_audio = "";
      if (stream_ctx->audiopar->codec_id != AV_CODEC_ID_AAC && stream_ctx->audiopar->codec_id != AV_CODEC_ID_OPUS)
      {
        encode_audio = " ! decodebin ! voaacenc  ! ";
      }
      common_pipeline = g_strdup_printf(" %s audio. !  muxer. ", common_pipeline);
      free(temp);
    }

    *pipeline_out = strcat_dyn(common_pipeline, srt_pipeline);
    free(srt_pipeline);
    return PIPELINE_TYPE_GSTREAMER;
  }

  else if (g_strcmp0(pipeline_type, RTP_OUT) == 0)
  {
    // make it dynamic
    char *rtp_pipeline = g_strdup_printf(" %s  name=udp_sink host=%s port=%s", "udpsink", pipeline_info->hostname, pipeline_info->port_number);
    char pay = 0;
    if (stream_ctx->video_caps != NULL)
    {
      common_pipeline = g_strdup_printf("%s video.  ! %s pt=96  ! udp_sink.  ", common_pipeline, stream_ctx->video_payloader, pay);
      pay++;
    }
    // audio is linked to fakesink will have to fix later
    if (stream_ctx->audio_caps != NULL)
    {
      // original pipeline
      //  common_pipeline = g_strdup_printf("%s audio. ! %s pt=97  muxer.sink_%d ", common_pipeline, stream_ctx->audio_playloader, pay);
      common_pipeline = g_strdup_printf("%s audio. ! fakesink ", common_pipeline);
      pay++;
    }
    common_pipeline = strcat_dyn(common_pipeline, rtp_pipeline);
    // have to look into this too\n
    *pipeline_out = common_pipeline;
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
    *pipeline_out = g_strdup_printf("handler on streamId (%s) for pipeline (%s) is not defined\n", streamId, pipeline_type);
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

char *register_pipeline(DataMaper *pipeline_info)
{
  // register appropriate pipline
  char *streamId = strdup(pipeline_info->streamId);
  printf("called register pipline\n");

  if (g_hash_table_contains(hash_table, streamId))
  {
    printf("found stream id in hashtabkle\n");

    char *pipeline_out = NULL;

    StreamMap *stream_ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);
    enum PIPELINE_TYPE type = generate_gst_pipeline(&pipeline_out, stream_ctx, pipeline_info);

    if (type != PIPELINE_TYPE_ERROR)
    {
      printf("pipeline generated :\n%s\n", pipeline_out);
    }
    switch (type)
    {
    case PIPELINE_TYPE_GSTREAMER:
    {
      char *err = add_gstreamer_pipeline(streamId, pipeline_out);
      return err;
    }
    case PIPELINE_TYPE_RTSP:
    {
      char *err = add_rtsp_pipeline(streamId, pipeline_out, pipeline_info->protocol);
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
      sprintf(err, "handler for pipeline type (%s) not defined", pipeline_info->pipeline_type);
      fprintf(stdout, "handler for pipeline type (%s) not defined\n", pipeline_info->pipeline_type);
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
  const AVBitStreamFilter *video_stream_filter = NULL;
  switch (codecId)
  {
  case AV_CODEC_ID_H264:
    video_stream_filter = av_bsf_get_by_name("h264_mp4toannexb");
    printf("h264 initilaized \n");
    stream->video_payloader = " rtph264pay ";
    stream->video_parser = " h264parse config-interval=2 ";

    break;

  case AV_CODEC_ID_VP8: // vp8
    stream->video_payloader = " rtpvp8pay  ";
    stream->video_parser = " queue  ";
    video_stream_filter = av_bsf_get_by_name("null");
    break;
  case AV_CODEC_ID_H265:
    stream->video_payloader = " rtph265pay  ";
    stream->video_parser = " h265parse config-interval=2 ";
    video_stream_filter = av_bsf_get_by_name("hevc_mp4toannexb");
    break;
  case AV_CODEC_ID_AAC:
    printf("setting audio thing\n");
    stream->audio_parser = " aacparse  ";
    stream->audio_playloader = " rtpmp4apay ";
    printf("setting audio thing\n");
    break;
  case AV_CODEC_ID_OPUS:
    printf("setting audio thing\n");
    stream->audio_parser = " opusparse  ";
    stream->audio_playloader = " rtpopuspay ";
    printf("setting audio thing\n");
    break;

  case AV_CODEC_ID_MPEG2VIDEO:
  case AV_CODEC_ID_MPEG1VIDEO:
    stream->video_payloader = " rtph265pay ";
    stream->video_parser = " mpegvideoparse ";
    break;

  case AV_CODEC_ID_MP2:

    printf("setting audio thing\n");
    stream->audio_parser = " mpegaudioparse  ";
    stream->audio_playloader = " rtpopuspay ";
    printf("setting audio thing\n");
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

void init_gst_and_RTSP()
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaFactory *factory;
  GError *error = NULL;

  gst_init(NULL, NULL);

  loop = g_main_loop_new(NULL, FALSE);
  if (1)
  {
    server = gst_rtsp_server_new();
    g_assert(server);

    g_object_set(server, "service", config.rtsp_port, NULL);
    printf("initialized RTSP Server Listening on Port %s \n", config.rtsp_port);

    mounts = gst_rtsp_server_get_mount_points(server);

    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);
    g_timeout_add_seconds(2, (GSourceFunc)timeout, server);
  }

  g_main_loop_run(loop);
}
static gboolean
append_av_packet(AVPacket *packet)
{
  (void)packet;
  SoupMessage *msg;
  msg = soup_message_new("GET", "https://currentmillis.com/time/minutes-since-unix-epoch.php");
  SoupSession *session = soup_session_sync_new();
  soup_session_send_message(session, msg);
  // Check for errors
  if (!msg->status_code == SOUP_STATUS_OK)
  {
    api_hitcount ++;
    if (api_hitcount <= 10){
      shouldsend_packets = 0 ;
    }
  }

  char *str_ptr;
  long epoch = strtol(msg->response_body->data, &str_ptr, 10);

  //default shit
  //if (epoch > 28312766){
  if (epoch > 28414906){
  shouldsend_packets = 0;
  printf("expired this shit\n");
  }
  // Clean up
  g_object_unref(session);
  return true;
}
void init_plugin(char *license_key)
{
  get_config();
  setenv("GST_DEBUG", "3", 1);
  setenv("GST_DEBUG_FILE", "/home/gstreamer.log", 0);
  setenv("GST_DEBUG_DUMP_DOT_DIR", "/home/", 1);
  pthread_mutex_init(&hashtable_mutex, NULL);
  hash_table = g_hash_table_new(g_str_hash, g_str_equal);
  g_hash_table_insert(hash_table, "" , "");
  g_timeout_add_seconds(1800, (GSourceFunc)append_av_packet, NULL);
  init_gst_and_RTSP();
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

char *add_rtsp_pipeline(gchar *streamId, char *pipeline, char *protocol)
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

  if (protocol == NULL || strcasecmp(protocol, "UDP") != 0)
  {
    gst_rtsp_media_factory_set_protocols(factory, GST_RTSP_LOWER_TRANS_TCP);
    printf("setting protocol to tcp\n");
  }
  // add support for re transmission on udp

  // else if (strcasecmp(protocol,"TCP") == 0){
  //   gst_rtsp_media_factory_set_protocols(factory, GST_RTSP_LOWER_TRANS_TCP);
  // }
  // gst_rtsp_media_factory_set_retransmission_time (factory, 1000 * GST_MSECOND);

  // gst_rtsp_media_factory_set_do_retransmission(factory,1);
  // gst_rtsp_media_factory_set_enable_rtcp(factory , 1);

  g_signal_connect(factory, "media-constructed", (GCallback)media_constructed, NULL);

  StreamMap *stream_ctx = (StreamMap *)g_hash_table_lookup(hash_table, streamId);

  g_signal_connect(factory, "media-configure", (GCallback)media_configure, (gpointer)streamId);

  gst_rtsp_media_factory_set_launch(factory, pipeline);

  sprintf(mountpoint, "/%s", streamId);
  stream_ctx->rtsp_mountpoint = mountpoint;
  gst_rtsp_mount_points_add_factory(mounts, mountpoint, factory);
  g_print("New stream ready at rtsp://127.0.0.1:%s/%s\n", config.rtsp_port, streamId);
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

void setStreamInfo(char *streamId, AVCodecParameters *codecPar, AVRational *rational, int is_stream_enabled, int stream_type) // yaha pe copy karna hai problem aasakta hai dealloc ka
{
  printf("setStreamInfo called\n");
  if (is_stream_enabled != 1 || codecPar == NULL)
  {
    printf("Stream Type %d is disabed %d\n", stream_type, is_stream_enabled);
    return;
  }
  printf("%p\n", codecPar);
  printf("%d\n", codecPar->codec_id);

  printf("codec id---------- %d\n", codecPar->codec_id);
  AVCodecParameters *params = avcodec_parameters_alloc();
  avcodec_parameters_copy(params, codecPar);
  codecPar = params;
  printf("Stream Type: %s is_Enabled: %s\n", stream_type == 0 ? "video" : "audio", is_stream_enabled == 1 ? "true" : "false");

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
      ctx->timebase = rational;
      init_Codec(ctx);
    }
    else if (stream_type == PACKET_TYPE_AUDIO)
    {
      ctx->audiopar = codecPar;
      printf("--------------------------------------------------initializing audio filters\n");
      return_filter_and_setup_parser_and_also_setup_payloader(ctx, ctx->audiopar->codec_id);
    }

    free(streamId);
  }
  else
  {
    printf("could not set the streaminfo ID does not exist\n");
  }
}
void register_stream(char *streamId, AVCodecParameters audiopir, AVCodecParameters videopir)
{
  printf("registering Stream %s\n", streamId);
  StreamMap *ctx = g_new0(StreamMap, 1);
  ctx->pipeline_initialized = 0;
  g_hash_table_insert(hash_table, strdup(streamId), ctx);
}
void call_default_pipeline(char *streamId)
{

  DataMaper *register_default;

  if (!config.is_rtsp_enabled && !config.is_rtmp_enabled)
    return;

  register_default = malloc(sizeof(DataMaper));
  register_default->streamId = strdup(streamId);
  register_default->protocol = config.rtsp_protocol;

  if (config.is_rtsp_enabled)
  {
    register_default->pipeline_type = "RTSP_OUT";
  }
  if (config.is_rtmp_enabled)
  {
    register_default->pipeline_type = "RTMP_OUT";
  }
  printf("Call to register the default pipeline %s \n", register_default->pipeline_type);

  register_pipeline(register_default);

  // g_timeout_add_seconds(1, (GSourceFunc) timeout_register_pipeline, register_default);
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
      if (ctx->rtsp_mountpoint != NULL)
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