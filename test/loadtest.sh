#Publish
for i in {0..100};
do
	ffmpeg -stream_loop 12 -re -i ./test.mp4 -c:v copy -c:a aac -f flv "rtmp://localhost/LiveApp/myStream-$i" &
done

read -p "Press enter to Play"

#Play
for i in {0..100};
do
	ffplay  rtsp://127.0.0.1:8554/myStream-$i &
done
