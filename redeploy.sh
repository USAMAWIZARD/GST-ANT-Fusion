#!/bin/sh
AMS_DIR=/home/usama/Music/antmedia/
mvn clean install -Dmaven.javadoc.skip=true -Dmaven.test.skip=true -Dgpg.skip=true
gcc -shared -fPIC -o /home/usama/Music/antmedia/lib/native/libGstRTSP.so src/main/java/io/antmedia/Native/RTSPServerNative.c -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0` -lavcodec

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
cd  /home/usama/Music/antmedia/
./start-debug.sh



