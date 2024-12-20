sudo apt-get update
sudo apt-get install ffmpeg
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libgstrtspserver-1.0-dev gstreamer1.0-rtsp
sudo chown -R antmedia:antmedia ./libGstAntFusion.so ./GstAntFusion.jar
sudo cp ./GstAntFusion.jar /usr/local/antmedia/plugins/
sudo cp ./libGstAntFusion.so /usr/local/antmedia/lib/native/
sudo cp ./gst_plugin.cfg /usr/local/antmedia/

sudo systemctl restart antmedia

#sudo apt-get -y install openjdk-11-jdk-headless
#sudo apt install gcc

# settings.webRTCEnabled=false
# settings.hlsMuxingEnabled=false
# dashHttpStreaming=false
# rtmpPlaybackEnabled=true
#rm -rf ./GstAntFusion.jar ./libGstAntFusion.so 