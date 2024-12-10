#include <gst/gst.h>
#include "./data_struct.h"

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
    return 0;
  }
  config.is_rtsp_enabled = g_key_file_get_boolean(keyfile, "DefaultProtocols", "RTSP", NULL);
  config.is_rtmp_enabled = g_key_file_get_boolean(keyfile, "DefaultProtocols", "RTMP", NULL);
  char *rtsp_port = g_key_file_get_string(keyfile, "RTSP", "Port", NULL);
  if (rtsp_port == NULL, strcasecmp(rtsp_port, "NULL") == 0 || rtsp_port == "" || !is_digits(rtsp_port))
  {
    config.rtsp_port = "8554";
  }
  else
  {
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
  g_key_file_free(keyfile);

  printf("----------\n");
  printf("rtsp port: %s\n", config.rtsp_port);
  printf("rtmp port: %s\n", config.rtmp_port);
  printf("protocol: %s\n", config.rtsp_protocol);
  printf("default rtmp:  %d\n", config.is_rtmp_enabled);
  printf("default rtsp:  %d\n", config.is_rtsp_enabled);
  printf("----------\n");
}
