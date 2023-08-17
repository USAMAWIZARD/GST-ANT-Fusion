
package io.antmedia.app;

import com.sun.jna.Library;
import com.sun.jna.Native;

import org.bytedeco.ffmpeg.avcodec.AVPacket;

import io.antmedia.plugin.api.IPacketListener;
import io.antmedia.plugin.api.StreamParametersInfo;
import io.antmedia.app.NativeInterface;

	
public class SamplePacketListener extends NativeInterface implements IPacketListener {
	static int is_init = 0;

	public SamplePacketListener() {
		if (is_init == 0)
			call_init_rtsp_server();
		is_init = 1;
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
		NativeInterface.JNA_RTSP_SERVER.INSTANCE.onPacket(packet.address(), streamId,0);
		return packet;
	}

	@Override
	public AVPacket onAudioPacket(String streamId, AVPacket packet) {
		//System.out.println("sending audio packet to c pipeline\n");
		NativeInterface.JNA_RTSP_SERVER.INSTANCE.onPacket(packet.address(), streamId,1);
		return packet;
	}

	@Override
	public void setVideoStreamInfo(String streamId, StreamParametersInfo videoStreamInfo) {
		long codecPar = videoStreamInfo.getCodecParameters().address();
		long getTimeBase = videoStreamInfo.getTimeBase().address();
		int streamType =0;
		System.err.println("salskjdlaksdjaladlketVideoStreamInfo()");
		System.out.println("SamplePacketListener.setVideoStreamInfo()");
		System.err.flush();
		NativeInterface.JNA_RTSP_SERVER.INSTANCE.setStreamInfo(streamId , codecPar, getTimeBase,streamType);
		
	}

	@Override
	public void setAudioStreamInfo(String streamId, StreamParametersInfo audioStreamInfo) {
		System.out.println("SamplePacketListener.setAudioStreamInfo()");
	}

	public String getStats() {
		return "packets:" + packetCount;
	}

}
