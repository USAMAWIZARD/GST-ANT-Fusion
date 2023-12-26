#!/bin/sh
AMS_DIR=/usr/local/antmedia
JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
set -xe

mvn clean install -Dmaven.javadoc.skip=true -Dmaven.test.skip=true -Dgpg.skip=true

#gcc ./gst-libav/build/ext/libav/libgstlibav.so.p/* -shared -fPIC -o $AMS_DIR/lib/native/libGstRTSP.so src/main/java/io/antmedia/Native/RTSPServerNative.c   -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 libsoup-2.4 ` -lavcodec -lpthread  
gcc ./gst-libav/build/ext/libav/libgstlibav.so.p/* -shared -fPIC -o $AMS_DIR/lib/native/libGstRTSP.so src/main/java/io/antmedia/Native/RTSPServerNative.c   -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 libsoup-2.4 ` -lavcodec -lpthread 

#scp -i /home/usama/Desktop/aws/ovh/amstest.pem   $AMS_DIRplugins/PluginApp.jar      ubuntu@ec2-3-110-204-50.ap-south-1.compute.amazonaws.com:$AMS_DIRplugins/
#scp -i /home/usama/Desktop/aws/ovh/amstest.pem   $AMS_DIRlib/native/libGstRTSP.so   ubuntu@ec2-3-110-204-50.ap-south-1.compute.amazonaws.com:$AMS_DIRlib/native/


OUT=$?

if [ $OUT -ne 0 ]; then
    exit $OUT
fi

if [ -e "$AMS_DIR/plugins/PluginApp*" ]; then
	rm -r $AMS_DIR/plugins/PluginApp*
fi
cp target/PluginApp.jar $AMS_DIR/plugins/

cp target/PluginApp.jar ./dist
cp $AMS_DIR/lib/native/libGstRTSP.so ./dist


OUT=$?

if [ $OUT -ne 0 ]; then
    exit $OUT
fi
cd  $AMS_DIR
./start-debug.sh






