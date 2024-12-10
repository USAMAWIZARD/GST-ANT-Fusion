#include <gst/gst.h>
#include "./data_struct.h"

void set_appsrc(StreamMap *ctx, gchar *streamId);

static gboolean
timeout(GstRTSPServer *server)
{
  GstRTSPSessionPool *pool;
  pool = gst_rtsp_server_get_session_pool(server);
  gst_rtsp_session_pool_cleanup(pool);
  g_object_unref(pool);

  return TRUE;
}

void init_rtsp(){
    GstRTSPServer *server;
    GstRTSPMediaFactory *factory;
    server = gst_rtsp_server_new();
    g_assert(server);

    g_object_set(server, "service", config.rtsp_port, NULL);
    printf("initialized RTSP Server Listening on Port %s \n", config.rtsp_port);

    mounts = gst_rtsp_server_get_mount_points(server);

    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);
    g_timeout_add_seconds(2, (GSourceFunc)timeout, server);
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

static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer streamId)
{

  if (g_hash_table_contains(hash_table, streamId))
  {
    StreamMap *ctx = (StreamMap *)g_hash_table_lookup(hash_table, (gchar *)streamId);
    g_print("-----------play-request-------------( %s ) \n", (char *)streamId);
    ctx->pipeline = gst_rtsp_media_get_element(media);
    // GST_DEBUG_BIN_TO_DOT_FILE(element, GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
    set_appsrc(ctx, (char *)streamId);
    // printf("initializing video app src\n");
  }
  else
  {
    fprintf(stdout, "media_configure(): streamId (%s) it doesn't exist in hashmap\n", (gchar *)streamId);
    return;
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
