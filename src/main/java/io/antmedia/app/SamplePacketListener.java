
package io.antmedia.app;

import com.sun.jna.Library;
import com.sun.jna.Native;

import org.bytedeco.ffmpeg.avcodec.AVPacket;

import io.antmedia.plugin.api.IPacketListener;
import io.antmedia.plugin.api.StreamParametersInfo;

interface JNA_RTSP_SERVER extends Library {
	JNA_RTSP_SERVER INSTANCE = Native.load("./lib/native/libGstRTSP.so", JNA_RTSP_SERVER.class);

	void init_rtsp_server();

	void sendPacket(long pktPointer, String streamId);
}

public class SamplePacketListener implements IPacketListener{
	static int is_init=0;
	public SamplePacketListener(){
		if(is_init==0)
		call_init_rtsp_server();
		is_init=1;
	}

    static void call_init_rtsp_server() {
        Thread initThread = new Thread(() -> JNA_RTSP_SERVER.INSTANCE.init_rtsp_server());
        initThread.start();

    }

	private int packetCount = 0;

	@Override
	public void writeTrailer(String streamId) {
		System.out.println("SamplePacketListener.writeTrailer()");
		
	}

	@Override
	public AVPacket onVideoPacket(String streamId, AVPacket packet) {
		//System.out.println("Samp------------------------------Info()"+packet.pts()+" dts"+packet.dts());		
		JNA_RTSP_SERVER.INSTANCE.sendPacket(packet.address(),streamId);

		packetCount++;
		return packet;
	}
	
	@Override
	public AVPacket onAudioPacket(String streamId, AVPacket packet) {
		packetCount++;
		return packet;
	}

	@Override
	public void setVideoStreamInfo(String streamId, StreamParametersInfo videoStreamInfo) {
		System.out.println("SamplePacketListener.setVideoStreamInfo()");		
	}

	@Override
	public void setAudioStreamInfo(String streamId, StreamParametersInfo audioStreamInfo) {
		System.out.println("SamplePacketListener.setAudioStreamInfo()");		
	}

	public String getStats() {
		return "packets:"+packetCount;
	}

}
