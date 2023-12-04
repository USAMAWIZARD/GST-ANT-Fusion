package io.antmedia.rest;

import jakarta.servlet.ServletContext;
import jakarta.ws.rs.Consumes;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.POST;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.PathParam;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.Context;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import jakarta.ws.rs.core.Response.Status;


import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.context.WebApplicationContext;

import com.google.gson.Gson;
import com.mongodb.client.model.geojson.Point;

import io.antmedia.plugin.SamplePlugin;
import io.swagger.annotations.ApiModelProperty;
import io.swagger.annotations.ApiOperation;
import io.swagger.annotations.ApiParam;
import io.swagger.annotations.ApiModelProperty;
import io.antmedia.rest.model.Result;
import com.sun.jna.Structure;
import com.sun.jna.Pointer;
import io.antmedia.app.NativeInterface;
import io.antmedia.app.DataMaper;

class RequestPipeline extends NativeInterface {
	@ApiModelProperty(value = "Stream Id to register")
	public String streamId;

	@ApiModelProperty(value = "pipeline type Gstreamer/RTP/FFmpeg")
	public String pipeline_type;

	@ApiModelProperty(value = "actual gstremaer of ffmpeg pipelien Pass your own pipeline", required = false)
	public String pipeline;

	@ApiModelProperty(value = "protocol", required = false)
	public String protocol;

	@ApiModelProperty(value = "port number", required = false)
	public String port;

	@ApiModelProperty(value = "host name", required = false)
	public String hostname;
}

@Component
@Path("/pipeline")
public class RestService {

	@Context
	protected ServletContext servletContext;
	Gson gson = new Gson();

	@POST
	@Path("/register/{streamId}")
	@Produces(MediaType.APPLICATION_JSON)
	@Consumes(MediaType.APPLICATION_JSON)
	public Response register(@PathParam("streamId") String streamId) {
		SamplePlugin app = getPluginApp();
		app.register(streamId);

		return Response.status(Status.OK).entity("").build();
	}

	@POST
	@Path("/register-pipeline")
	@Produces({ MediaType.APPLICATION_JSON })
	@Consumes(MediaType.APPLICATION_JSON)
	public Response register_pipeline(@ApiParam(value = "register pipeline", required = true) RequestPipeline Request) {
		DataMaper pipeline_info = new DataMaper();

		//NativeInterface.JNA_RTSP_SERVER.INSTANCE.print_java_struct_val(pipeline_info);

		System.out.println("Registering Pipeline");
		SamplePlugin app = getPluginApp();

		pipeline_info.streamId = Request.streamId;
		pipeline_info.pipeline_type = Request.pipeline_type;

		switch (Request.pipeline_type) {
			case "Gstreamer": {
				if (Request.pipeline == null) {
					return Response.status(Status.OK)
							.entity(new Result(false, "Please Specify your Gstreamer Pipeline"))
							.build();
				}
				pipeline_info.pipeline = Request.pipeline;
			}
				break;
			case "RTSP_OUT": {
				pipeline_info.protocol = Request.protocol;
			}
				break;
			case "RTMP_OUT": {
			}
				break;
			case "SRT_OUT":
			case "RTP_OUT": {
				if (Request.port == null || Request.hostname == null) {
					return Response.status(Status.OK)
							.entity(new Result(false, "Please Specify port number and hostname "))
							.build();
				}
				pipeline_info.hostname = Request.hostname;
				pipeline_info.port_number = Request.port;
				pipeline_info.protocol = Request.protocol;
				break;
			}
			
			default:
				return Response.status(Status.OK)
						.entity(new Result(false, "Please Specify a valid pipeline type"))
						.build();

		}

		String err = NativeInterface.JNA_RTSP_SERVER.INSTANCE.register_pipeline(pipeline_info);
		if (err != null) {
			return Response.status(Status.EXPECTATION_FAILED).entity(new Result(false, err)).build();
		}

		return Response.status(Status.OK).entity(new Result(true, "Pipeline Registered Sucsessfully")).build();
	}

	private SamplePlugin getPluginApp() {
		ApplicationContext appCtx = (ApplicationContext) servletContext
				.getAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE);
		return (SamplePlugin) appCtx.getBean("plugin.myplugin");
	}
}
