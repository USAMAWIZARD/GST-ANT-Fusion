package io.antmedia.plugin;

import java.util.List;

import org.apache.commons.lang3.ObjectUtils.Null;
import org.bytedeco.ffmpeg.avcodec.AVCodecParameters;
import org.bytedeco.ffmpeg.avcodec.AVPacket;
import org.bytedeco.ffmpeg.global.avcodec;
import org.bytedeco.javacpp.BytePointer;
import org.bytedeco.javacpp.FunctionPointer;
import org.bytedeco.javacpp.Loader;
import org.bytedeco.javacpp.annotation.Name;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Component;

import com.sun.jna.Pointer;

import io.antmedia.AntMediaApplicationAdapter;
import io.antmedia.app.SamplePacketListener;
import io.antmedia.muxer.MuxAdaptor;
import io.antmedia.plugin.api.IFrameListener;
import io.antmedia.plugin.api.IStreamListener;
import io.vertx.core.Vertx;
import io.antmedia.app.NativeInterface;
import io.antmedia.app.NativeInterface;
import io.antmedia.settings.ServerSettings;
import java.util.Timer;
import org.bytedeco.ffmpeg.avcodec.AVPacket;
import org.bytedeco.ffmpeg.avcodec.AVCodec;
import java.util.Collection;


@Component(value = "plugin.myplugin")
public class SamplePlugin extends NativeInterface implements ApplicationContextAware, IStreamListener  {

	public static final String BEAN_NAME = "web.handler";
	protected static Logger logger = LoggerFactory.getLogger(SamplePlugin.class);

	private Vertx vertx;
	private SamplePacketListener packetListener = new SamplePacketListener();

	private ApplicationContext applicationContext;
	
	NativeInterface.JNA_RTSP_SERVER.receiveDataCallback receiveDataCallback;
	static int is_init = 0;
	public SamplePlugin() {
		if (is_init == 0)
			call_init_server();
		is_init = 1;
	}

	void call_init_server() {

		Thread initThread = new Thread(() -> JNA_RTSP_SERVER.INSTANCE.init_plugin());
		
		initThread.start();


		receiveDataCallback = new NativeInterface.JNA_RTSP_SERVER.receiveDataCallback() {
            @Override
            public void C_Callback(String streamId, String roomId , String data ) {
				AntMediaApplicationAdapter app = getApplication();

				//System.out.println(streamId  +"Received data from C: \n" + data + roomId);
				app.sendDataChannelMessage(streamId, data);
            }
        };
    
        NativeInterface.JNA_RTSP_SERVER.INSTANCE.registerCallback(receiveDataCallback);
    
	}

	@Override
	public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
		this.applicationContext = applicationContext;
		vertx = (Vertx) applicationContext.getBean("vertxCore");
		
		AntMediaApplicationAdapter app = getApplication();
		app.addStreamListener(this);
	}

	public MuxAdaptor getMuxAdaptor(String streamId) {
		AntMediaApplicationAdapter application = getApplication();
		MuxAdaptor selectedMuxAdaptor = null;
		if (application != null) {
			Collection<MuxAdaptor> muxAdaptors = application.getMuxAdaptors();
			for (MuxAdaptor muxAdaptor : muxAdaptors) {
				if (streamId.equals(muxAdaptor.getStreamId())) {
					selectedMuxAdaptor = muxAdaptor;
					break;
				}
			}
		}
		return selectedMuxAdaptor;
	}

	public AntMediaApplicationAdapter getApplication() {
		return (AntMediaApplicationAdapter) applicationContext.getBean(AntMediaApplicationAdapter.BEAN_NAME);
	}

	public IFrameListener createCustomBroadcast(String streamId) {	
		AntMediaApplicationAdapter app = getApplication();
		return app.createCustomBroadcast(streamId);
	}

	@Override
	public void streamStarted(String streamId) {
		AntMediaApplicationAdapter app = getApplication();
		long audioPar  = 0;
		long videoPar  = 0;
		long atimebase = 0;
		long vtimebase = 0;
		if (!app.getName().equals("rtmpout")) {
			logger.info("*************** Stream Started: {} {} ***************", app.getName(), streamId);

			MuxAdaptor Muxer = getMuxAdaptor(streamId);
			boolean videoEnabled = Muxer.isEnableVideo();
			boolean audioEnabled = Muxer.isEnableAudio();

			if (Muxer.isEnableAudio()) {
				audioPar = Muxer.getAudioCodecParameters().address();
				atimebase = Muxer.getAudioTimeBase().address();
			}
			if (Muxer.isEnableVideo()) {
				videoPar = Muxer.getVideoCodecParameters().address();
				vtimebase = Muxer.getVideoTimeBase().address();
			}
			System.out.println("Video Enabled : "+ Muxer.isEnableVideo() +" Audio Enabled : "+ Muxer.isEnableAudio());
			int is_video = videoEnabled ? 0 : 0;
			int is_audio = audioEnabled ? 1 : 0;
			JNA_RTSP_SERVER.INSTANCE.register_stream(streamId);
			JNA_RTSP_SERVER.INSTANCE.setStreamInfo(streamId, videoPar, vtimebase, is_video, 0);
			JNA_RTSP_SERVER.INSTANCE.setStreamInfo(streamId, audioPar, atimebase, is_audio, 1);
			JNA_RTSP_SERVER.INSTANCE.call_default_pipeline(streamId);
			app.addPacketListener(streamId, packetListener);
		}  
	}


	@Override
	public void streamFinished(String streamId) {
		logger.info("*************** Stream Finished: {} ***************", streamId);
		JNA_RTSP_SERVER.INSTANCE.unregister_stream(streamId);
		
	}

	@Override
	public void joinedTheRoom(String roomId, String streamId) {
		JNA_RTSP_SERVER.INSTANCE.joinedTheRoom(roomId, streamId);
		logger.info("*************** Stream Id:{} joined the room:{} ***************", streamId, roomId);
	}

	@Override
	public void leftTheRoom(String roomId, String streamId) {
		JNA_RTSP_SERVER.INSTANCE.leftTheRoom(roomId, streamId);
		logger.info("*************** Stream Id:{} left the room:{} ***************", streamId, roomId);
	}

}
