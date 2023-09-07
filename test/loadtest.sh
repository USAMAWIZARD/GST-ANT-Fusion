for i in {0..50};
do
	ffplay  rtsp://127.0.0.1:8554/myStream-$i &
done
