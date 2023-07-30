package io.antmedia.app;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class NativeInterface {
    public static interface JNA_RTSP_SERVER extends Library {
        JNA_RTSP_SERVER INSTANCE = Native.load("./lib/native/libGstRTSP.so", JNA_RTSP_SERVER.class);

        void onPacket(long pktPointer, String streamId, int pktType);

        void init_rtsp_server();

        void register_stream(String streamId);

        void unregister_stream(String streamId);

        void register_pipeline(String streamId, String pipeline_type, String pipeline);
    }
}