## Contributors 
[@notshahzad](https://github.com/notshahzad) 
[@USAMA](https://github.com/USAMAWIZARD/)

<h2>About the Gstreamer plugin for Ant Media Server</h2>


<p>The GST-ANT Fusion plugin seamlessly integrates Ant Media Server with <a href="https://antmedia.io/gstreamer-tutorial-how-to-publish-play-webrtc-streams/">Gstreamer</a>, which is a well-known open-source multimedia framework.</p>

<h3>Who can use this plugin?</h3>

<p>The plugin is a game-changer for streamers and businesses providing the ability to customize streaming pipelines to suit specific use cases and preferences. Whether it’s RTSP, RTP, RTMP, or even a combination of these protocols, the plugin empowers users to define and create their own Gstreamer pipelines that precisely align with their streaming requirements.</p>

<h3>Getting Started</h3>

<h4>Prerequisites:</h4>

<p>You need to have an active Ant Media Server license. You can subscribe through <a href="https://aws.amazon.com/marketplace/pp/prodview-464ritgzkzod6?sr=0-1&amp;ref_=beagle&amp;applicationId=AWSMPContessa&amp;_ga=2.27383807.275147218.1658729962-339983394.1658135231#pdp-pricing">AWS Marketplace</a> and deploy Ant Media Server with just one click. Or you can directly subscribe to a self-hosted license <a href="https://antmedia.io/">on our website</a>. Check out our <a href="https://antmedia.io/docs/">documentation</a> to learn how to deploy Ant Media Server.</p>

  <li><a href="https://antmedia.io/marketplace-demo-request/?wpf78324_4=GST%%20Ant%20Fusion%20Demo%20Request">Request a demo</a> Request a Demo from us</li>

<h4>Installing:</h4>

<ul>
  
  <li><a href="https://github.com/USAMAWIZARD/GST-ANT-Fusion/releases/"> Download Plugin dist.zip file according to your Ant Media Version</li>
  <li>Unzip dist.zip file and run the install script to install the plugin</li>
</ul>
<pre><code>sudo sh gst_plugin_install.sh</code></pre>
<pre><code>sudo service antmedia restart</code></pre>

<p>The plugin comes with <a href="https://antmedia.io/how-to-live-stream-rtsp-output-with-ant-media-server/">RTSP, RTMP, and RTP output</a> capabilities by default. Also, you can register any Gstreamer pipeline you want by simply calling some REST endpoints.</p>

<h3>RTSP Output</h3>

<p>There are two ways to register a stream for RTMP output:</p>

<ol>
  <li>If you want to have RTSP output for any stream, you can simply send a REST API request to enable RTSP for that particular stream.</li>
  <li>If you want to have RTSP output for every stream without having to call a REST API, you can simply set RTSP by default in the plugin configuration file. In this way, you don’t need to call the REST API, and the RTSP stream will be available for every stream that you publish to Ant Media Server.</li>
</ol>

<h4>REST API Method:</h4>

<ol>
  <li>You can send a video stream either with WebRTC, RTMP, RTSP, or SRT.</li>
  <li>Send a REST API request. RTSP supports both UDP and TCP, so you can set the protocol as TCP or UDP.</li>
</ol>

<pre><code>curl -X POST -H "Content-Type: application/json" -d '{"streamId":"stream1","pipeline_type":"RTSP_OUT","protocol":"TCP"}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/</code></pre>

<ol start="3">
  <li>Play the stream with ffplay.</li>
  <li>Change the IP Address in the command.</li>
  <li><code>ffplay rtsp://127.0.0.1:8554/stream1</code></li>
</ol>

<h4>Enabling RTSP by Default In the Configuration File:</h4>

<ol>
  <li>Edit to <code>/usr/local/antmedia/gst-plugin.cfg</code></li>
  <li>Set RTSP in Default Protocols to 1 to set the desired protocol and port for RTSP output.</li>
</ol>

<pre><code># set true or 1 for enabling by default 
[DefaultProtocols]RTSP=1
RTMP=0
[RTSP]Port=8554
Protocol=TCP
[RTMP]appname=rtmpout
Port=1935
[RTP]</code></pre>

<ol start="3">
  <li>Restart the server.</li>
  <li>Now for every stream that is published to the Ant Media server, The RTSP output stream will be available by default.</li>
</ol>


<h2>RTMP Output Configuration</h2>

<p>1.Create a new <strong>Appname</strong> on Ant Media with the name of <strong>rtmpout</strong>.</p>

<p>2. Set the following in application settings of rtmpout application :</p>
<pre>
  <code>
settings.webRTCEnabled=false
settings.hlsMuxingEnabled=false
dashHttpStreaming=false
rtmpPlaybackEnabled=true
  </code>
</pre>

<h3>REST API Method:</h3>

<p>To enable RTMP output for a specific stream using the REST API, you can send a request as follows:</p>

<pre>
  <code>
curl -X POST -H "Content-Type: application/json" -d '{"streamId":"stream1","pipeline_type":"rtmp_OUT","protocol":"TCP"}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/
  </code>
</pre>

<p>To play the stream using <code>ffplay</code>, use the following command:</p>

<pre>
  <code>
ffplay rtmp://localhost/rtmpout/streamid_rtmp_out
  </code>
</pre>

<p>Make sure to change the IP address in the command accordingly.</p>

<h3>Enabling RTMP by Default in the Configuration File:</h3>

<ol>
  <li>Edit the configuration file:</li>
  <pre>
    <code>
sudo nano /usr/local/antmedia/gst-plugin.cfg
    </code>
  </pre>

  <li>Set RTMP in Default Protocols to 1 to enable RTMP by default:</li>
  <pre>
    <code>
[DefaultProtocols]
RTMP=1
RTSP=0
    </code>
  </pre>

  <li>Restart the Ant Media server:</li>
  <pre>
    <code>
sudo systemctl restart antmedia
    </code>
  </pre>
</ol>

<p>Now, RTMP output will be enabled by default for every stream published to the Ant Media server. Adjust the configuration settings as needed.</p>

<p>To play the stream using <code>ffplay</code>, use the following command:</p>

<pre>
  <code>
ffplay rtmp://localhost/rtmpout/streamid_rtmp_out
  </code>
</pre>

<h3>RTP Output (Supports only Video for now)</h3>

<ol>
  <li>You can send a video stream either with WebRTC, RTMP, RTSP, or SRT.</li>
  <li>Send a REST API request.</li>
</ol>

<pre><code>curl -X POST -H "Content-Type: application/json" -d '{"streamId":"stream1","pipeline_type":"RTP_OUT" , "port": "5000" , "host","127.0.0.1"}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/</code></pre>

<ol start="3">
  <li>Change the IP Address in the command.</li>
</ol>

<pre><code>gst-launch-1.0 -v udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! videoconvert ! autovideosink</code></pre>

<h3>Custom Gstreamer Pipeline</h3>

<ol>
  <li>You can send a video stream either with WebRTC, RTMP, RTSP, or SRT.</li>
  <li>Send a REST API request and pass your own custom Gstreamer pipeline with it.</li>
</ol>

<p>Encoded video element will be available with the name element name as video and encoded audio element will be available with the name as audio. You can access the elements by name and to further complete the pipeline as desired. For example, you can get the video and audio and link it something like this:</p>

<pre><code>mp4mux name=muxer video. ! muxer. ! audio. ! muxer. ! filesink location=./abc.mp4</code></pre>

<p>If you don’t need to use any one of them you can simply link it to fakesink. For example, if you don’t need audio then you can connect <code>audio. ! fakesink</code></p>

<ol start="3">
  <li>Recording with Gstreamer plugin using a custom Muxer (MKV, FLV, etc). The file will be saved in /usr/local/antmedia directory</li>
</ol>

<pre><code>curl -X POST -H "Content-Type: application/json" -d '{"streamId":"streamId_6FGwsZSNW","pipeline_type":"Gstreamer","protocol":"TCP", "pipeline":"matroskamux name=muxer video. ! muxer. ! audio. ! muxer. ! filesink location./abc.mp4 "}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/</code></pre>

<ol start="3">
  <li>Re-Stream to an SRT-Server</li>
</ol>

<pre><code>curl -X POST -H "Content-Type: application/json" -d '{"streamId":"streamId_6FGwsZSNW","pipeline_type":"Gstreamer","protocol":"TCP", "pipeline":"mpegtsmux name=muxer video. ! muxer. ! audio. ! muxer. ! srtsink uri=<srt://SRT_SERVER_ADDRESS>:PORT?streamid=streamid "}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline/</code></pre>

<ol start="3">
  <li>Save only Audio of the stream as mp3 file.</li>
</ol>

<pre><code>curl -X POST -H "Content-Type: application/json" -d '{"streamId":"streamId_6FGwsZSNW","pipeline_type":"Gstreamer","protocol":"TCP", "pipeline":"lamemp3enc name=muxer video. ! fakesink. ! audio. ! muxer. ! filesink location=abc.mp3 "}' http://localhost:5080/LiveApp/rest/pipeline/register-pipeline</code></pre>

<h3>Support</h3>

<p>This plugin was developed by Ant Media Community. Create a new issue or search for related keywords to find an answer right away on our <a href="https://github.com/orgs/ant-media/discussions">Github discussions</a>.</p>


