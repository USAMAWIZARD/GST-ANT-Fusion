package io.antmedia.app;

import org.bytedeco.ffmpeg.avcodec.AVCodecParameters;
import org.bytedeco.ffmpeg.avutil.AVRational;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class NativeInterface {
    public static interface JNA_RTSP_SERVER extends Library {
        JNA_RTSP_SERVER INSTANCE = Native.load("./lib/native/libGstRTSP.so", JNA_RTSP_SERVER.class);

        void onPacket(long pktPointer, String streamId, int pktType);  // pkt type 0= video 1=audio

        void init_rtsp_server();

        void register_stream(String streamId);

        void unregister_stream(String streamId);

        String register_pipeline(String streamId, String pipeline_type, String pipeline);

        void setStreamInfo(String streamId, long codecPar ,  long rational ,int streamType ); // pkt codecType 0=video codecType 1=audio
    }
}