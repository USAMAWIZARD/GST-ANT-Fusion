// #include <gstreamer-1.0/gst/libav/gstavutils.h>
#include <string.h>
#include <gst/gst.h>
#include <gst/gst.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>

#include <gst/pbutils/codec-utils.h>
#include <gstreamer-1.0/gst/libav/gstavutils.h>
#include <gstreamer-1.0/gst/libav/gstavcodecmap.h>

#include <gstreamer-1.0/gst/libav/gstavcfg.h>
#include <gstreamer-1.0/gst/libav/gstavviddec.h>
#include <gstreamer-1.0/gst/libav/gstavauddec.h>

#include <gst/pbutils/pbutils.h>
#include <gst/pbutils/codec-utils.h>

#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <gstreamer-1.0/gst/libav/gstav.h>
#include <gstreamer-1.0/gst/gstplugin.h>
#include <gst/pbutils/pbutils.h>

#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>


void main()
{
    printf("%s ", gst_ffmpeg_codecid_to_caps(AV_CODEC_ID_VC1IMAGE, NULL, 0));
    return;
}