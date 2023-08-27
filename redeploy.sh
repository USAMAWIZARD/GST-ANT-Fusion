#!/bin/sh
AMS_DIR=/usr/local/antmedia
mvn clean install -Dmaven.javadoc.skip=true -Dmaven.test.skip=true -Dgpg.skip=true

gcc ./gst-libav/build/ext/libav/libgstlibav.so.p/* -shared -fPIC -o $AMS_DIR/lib/native/libGstRTSP.so src/main/java/io/antmedia/Native/RTSPServerNative.c   -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0` -lavcodec -lpthread 



OUT=$?

if [ $OUT -ne 0 ]; then
    exit $OUT
fi

rm -r $AMS_DIR/plugins/PluginApp*
cp target/PluginApp.jar $AMS_DIR/plugins/

OUT=$?

if [ $OUT -ne 0 ]; then
    exit $OUT
fi
cd  $AMS_DIR
./start-debug.sh



#rtmp://127.0.0.1/LiveApp/?streamid=LiveApp/stream11