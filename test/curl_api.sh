#!/bin/bash
set -xe
#ffmpeg -stream_loop -1 -re -i ./test.mp4 -c:v libx264 -an -f mpegts "srt://localhost:4200?streamid=LiveApp/stream111" &
ffmpeg -stream_loop 1 -re -i ./test.mp4 -c:v libx264 -c:a aac -f flv "rtmp://localhost/LiveApp/stream111" &
sleep 5
if [[  $1 == "RTSP" ]]; then
curl -X POST -H "Content-Type: application/json"  -d '{"streamId":"stream111","pipeline_type":"RTSP_OUT","protocol":"TCP"}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/
elif [[  $1 == "RTMP" ]]; then
curl -X POST -H "Content-Type: application/json"  -d '{"streamId":"stream111","pipeline_type":"RTMP_OUT","protocol":"TCP"}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/
echo "\n"
fi

gst-launch-1.0 -v videotestsrc ! video/x-raw, height=360, width=640 ! videoconvert ! x264enc tune=zerolatency ! video/x-h264, profile=high ! mpegtsmux ! srtsink uri=srt://:8881 

gst-launch-1.0 -v srtsrc uri="srt://127.0.0.1:8881" ! decodebin ! autovideosink\

gst-launch-1.0 videotestsrc   name=video_stream111 is-live=true  ! x264enc ! capsfilter caps="video/x-h264, width=(int)1920, height=(int)1080, alignment=(string)au, stream-format=(string)byte-stream"  ! decodebin ! x264enc  name=video video.  !  muxer.    mpegtsmux name=muxer ! srtsink uri=srt://:9006 wait-for-connection=false


GST_DEBUG=3 gst-launch-1.0 -v srtsrc uri="srt://127.0.0.1:9006" ! tsparse ! tsdemux ! decodebin ! videoconvert ! xvimagesink



rtpfunnel   