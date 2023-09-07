package io.antmedia.plugin;

import java.util.List;

import org.apache.commons.lang3.ObjectUtils.Null;
import org.bytedeco.ffmpeg.avcodec.AVCodecParameters;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Component;

import com.sun.jna.Pointer;

import io.antmedia.AntMediaApplicationAdapter;
import io.antmedia.app.SampleFrameListener;
import io.antmedia.app.SamplePacketListener;
import io.antmedia.muxer.MuxAdaptor;
import io.antmedia.plugin.api.IFrameListener;
import io.antmedia.plugin.api.IStreamListener;
import io.vertx.core.Vertx;
import io.antmedia.app.NativeInterface;


@Component(value = "plugin.myplugin")
public class SamplePlugin extends NativeInterface implements ApplicationContextAware, IStreamListener {

	public static final String BEAN_NAME = "web.handler";
	protected static Logger logger = LoggerFactory.getLogger(SamplePlugin.class);

	private Vertx vertx;
	private SampleFrameListener frameListener = new SampleFrameListener();
	private SamplePacketListener packetListener = new SamplePacketListener();
	private ApplicationContext applicationContext;

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
			List<MuxAdaptor> muxAdaptors = application.getMuxAdaptors();
			for (MuxAdaptor muxAdaptor : muxAdaptors) {
				if (streamId.equals(muxAdaptor.getStreamId())) {
					selectedMuxAdaptor = muxAdaptor;
					break;
				}
			}
		}

		return selectedMuxAdaptor;
	}

	public void register(String streamId) {
	AntMediaApplicationAdapter app = getApplication();

		app.addFrameListener(streamId, frameListener);
		app.addPacketListener(streamId, packetListener);
	}

	public AntMediaApplicationAdapter getApplication() {
		return (AntMediaApplicationAdapter) applicationContext.getBean(AntMediaApplicationAdapter.BEAN_NAME);
	}

	public IFrameListener createCustomBroadcast(String streamId) {
		AntMediaApplicationAdapter app = getApplication();
		return app.createCustomBroadcast(streamId);
	}

	public String getStats() {
		return frameListener.getStats() + "\t" + packetListener.getStats();
	}

//struct is_streaminfo_set
// queue datanikalega register pipeline
// 
//set_info [ streamid ] append
	@Override
	public void streamStarted(String streamId) {
		AntMediaApplicationAdapter app = getApplication();
		long audioPar=0;
		long videoPar=0;
		long atimebase=0;
		long vtimebase=0;
		if (!app.getName().equals("rtmpout")) {
			logger.info("*************** Stream Started: {} {} ***************",app.getName() , streamId);

			MuxAdaptor Muxer =  getMuxAdaptor(streamId);
			boolean videoEnabled = Muxer.isEnableVideo();
			boolean audioEnabled = Muxer.isEnableAudio();

			if(Muxer.isEnableAudio()){
			audioPar = Muxer.getAudioCodecParameters().address();
			atimebase = Muxer.getAudioTimeBase().address();
			}
			if(Muxer.isEnableVideo()){
			videoPar = Muxer.getVideoCodecParameters().address();
			vtimebase = Muxer.getVideoTimeBase().address();
			}
			System.out.println(Muxer.isEnableVideo()+ " yaha pe " + Muxer.isEnableAudio());
			int  is_video = videoEnabled ? 1 : 0;
			int  is_audio = audioEnabled ? 1 : 0;
			NativeInterface.JNA_RTSP_SERVER.INSTANCE.register_stream(streamId);
			NativeInterface.JNA_RTSP_SERVER.INSTANCE.setStreamInfo(streamId , videoPar, vtimebase, is_video ,0);
			NativeInterface.JNA_RTSP_SERVER.INSTANCE.setStreamInfo(streamId , audioPar, atimebase,  is_audio,1);
			NativeInterface.JNA_RTSP_SERVER.INSTANCE.call_default_pipeline(streamId);

			
			app.addPacketListener(streamId, packetListener);
		}
	}

	@Override
	public void streamFinished(String streamId) {
		logger.info("*************** Stream Finished: {} ***************", streamId);
		NativeInterface.JNA_RTSP_SERVER.INSTANCE.unregister_stream(streamId);
	}

	@Override
	public void joinedTheRoom(String roomId, String streamId) {
		logger.info("*************** Stream Id:{} joined the room:{} ***************", streamId, roomId);
	}

	@Override
	public void leftTheRoom(String roomId, String streamId) {
		logger.info("*************** Stream Id:{} left the room:{} ***************", streamId, roomId);
	}

}
