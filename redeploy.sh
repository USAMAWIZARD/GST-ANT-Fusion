#!/bin/sh
#AMS_DIR=/usr/local/antmedia
AMS_DIR=/home/usama/Music/antmedia
JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
set -xe

mvn clean install -Dmaven.javadoc.skip=true -Dmaven.test.skip=true -Dgpg.skip=true

#gcc ./gst-libav/build/ext/libav/libgstlibav.so.p/* -shared -fPIC -o $AMS_DIR/lib/native/libGstAntFusion.so src/main/java/io/antmedia/Native/gst_antmedia.c   -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 libsoup-2.4 ` -lavcodec -lpthread  
gcc ./gst-libav/build/ext/libav/libgstlibav.so.p/* -shared -fPIC -o $AMS_DIR/lib/native/libGstAntFusion.so src/main/java/io/antmedia/Native/gst_antmedia.c   -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 libsoup-2.4 json-glib-1.0 ` -lavcodec -lpthread

#scp -i /home/usama/Desktop/aws/ovh/amstest.pem   $AMS_DIRplugins/GstAntFusion.jar      ubuntu@ec2-3-110-204-50.ap-south-1.compute.amazonaws.com:$AMS_DIRplugins/
#scp -i /home/usama/Desktop/aws/ovh/amstest.pem   $AMS_DIRlib/native/libGstAntFusion.so   ubuntu@ec2-3-110-204-50.ap-south-1.compute.amazonaws.com:$AMS_DIRlib/native/


OUT=$?

if [ $OUT -ne 0 ]; then
    exit $OUT
fi

if [ -e "$AMS_DIR/plugins/GstAntFusion*" ]; then
	rm -r $AMS_DIR/plugins/GstAntFusion*
fi
cp target/GstAntFusion.jar $AMS_DIR/plugins/

cp target/GstAntFusion.jar ./dist
cp $AMS_DIR/lib/native/libGstAntFusion.so ./dist


OUT=$?

if [ $OUT -ne 0 ]; then
    exit $OUT
fi
cd  $AMS_DIR
./start-debug.sh






