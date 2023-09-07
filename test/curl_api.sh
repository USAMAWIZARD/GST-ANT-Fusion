#!/bin/bash
set -xe
#ffmpeg -stream_loop -1 -re -i ./high.mp4 -c:v libx264 -an -f mpegts "srt://localhost:4200?streamid=LiveApp/stream111" &
ffmpeg -stream_loop 1 -re -i ./high.mp4 -c:v libx264 -c:a aac -f flv "rtmp://localhost/LiveApp/stream111" &
sleep 5
if [[  $1 == "RTSP" ]]; then
curl -X POST -H "Content-Type: application/json"  -d '{"streamId":"stream111","pipeline_type":"RTSP_OUT","protocol":"TCP"}' http://localhost:5080/LiveApp/rest/sample-plugin/register-pipeline/
elif [[  $1 == "RTMP" ]]; then
curl -X POST -H "Content-Type: application/json"  -d '{"streamId":"stream111","pipeline_type":"RTMP_OUT","protocol":"TCP"}' http://localhost:5080/LiveApp/rest/sample-plugin/register-pipeline/
echo "\n"
fi