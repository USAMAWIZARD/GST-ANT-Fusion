# h264 aac
ffmpeg -re -i ./test.mp4 -c:v libx264 -c:a aac -f mpegts "srt://localhost:4200?streamid=LiveApp/stream1"
# h265 aac 
ffmpeg -re -i ./test.mp4 -c:v libx265 -c:a aac -f mpegts "srt://localhost:4200?streamid=LiveApp/stream2"
# h264 mpegts no audio 
ffmpeg -re -i ./test.mp4 -c:v libx264 -an -f mpegts "srt://localhost:4200?streamid=LiveApp/stream3"
# h265 opus 
ffmpeg -re -i ./test.mp4 -c:v libx265 -c:a libopus -f mpegts "srt://localhost:4200?streamid=LiveApp/stream4"
