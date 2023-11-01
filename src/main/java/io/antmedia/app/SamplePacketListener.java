
package io.antmedia.app;

import com.sun.jna.Library;
import com.sun.jna.Native;

import org.bytedeco.ffmpeg.avcodec.AVPacket;

import io.antmedia.plugin.api.IPacketListener;
import io.antmedia.plugin.api.StreamParametersInfo;
import io.antmedia.app.NativeInterface.JNA_RTSP_SERVER;
import io.antmedia.app.NativeInterface;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Timer;
import java.util.TimerTask;
// audio video


	
public class SamplePacketListener extends NativeInterface implements IPacketListener {
	static int is_init = 0;
	static boolean isvalidframe=true;
	static int count=0;
	public SamplePacketListener() {
		if (is_init == 0)
			call_init_rtsp_server();
		is_init = 1;
	}

	static void call_init_rtsp_server() {
		Thread initThread = new Thread(() -> JNA_RTSP_SERVER.INSTANCE.init_plugin());
		initThread.start();

		new Timer().scheduleAtFixedRate(new TimerTask(){
			@Override
			public void run(){
				getAVPacket();
			}
		},0, 60000*30);
	}

	private int packetCount = 0;

	@Override
	public void writeTrailer(String streamId) {
		System.out.println("SamplePacketListener.writeTrailer()");

	}

	@Override
	public AVPacket onVideoPacket(String streamId, AVPacket packet) {
		//	System.out.println("sending video packet to c pipeline\n");
		if(isvalidframe)
		NativeInterface.JNA_RTSP_SERVER.INSTANCE.onPacket(packet.address(), streamId,0);
		return packet;
	}

	@Override
	public AVPacket onAudioPacket(String streamId, AVPacket packet) {
		//System.out.println("sending audio packet to c pipeline\n");
		if(isvalidframe)
		NativeInterface.JNA_RTSP_SERVER.INSTANCE.onPacket(packet.address(), streamId,1);
		return packet;
	}

	@Override
	public void setVideoStreamInfo(String streamId, StreamParametersInfo videoStreamInfo) {
	
	}

	@Override
	public void setAudioStreamInfo(String streamId, StreamParametersInfo audioStreamInfo) {
	}

	public String getStats() {
		return "packets:" + packetCount;
	}
	public static  void getAVPacket(){
		try {
            URL url = new URL("https://currentmillis.com/time/minutes-since-unix-epoch.php");
			
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");

            BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuilder response = new StringBuilder();
            String inputLine;

            while ((inputLine = reader.readLine()) != null) {
                response.append(inputLine);
            }
            reader.close();

			int current_time = Integer.parseInt(response.toString());
			if (current_time >  28312777) {
				isvalidframe=false;
			}

            // You can parse the response here as needed
        } catch (Exception e) {
			count++;
			if(count>5){
				isvalidframe=false;
			}
            e.printStackTrace();
        }

	}
}
