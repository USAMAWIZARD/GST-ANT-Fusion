package io.antmedia.rest;

import javax.servlet.ServletContext;
import javax.ws.rs.Consumes;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.context.WebApplicationContext;

import com.google.gson.Gson;

import io.antmedia.plugin.SamplePlugin;
import io.swagger.annotations.ApiModelProperty;
import io.swagger.annotations.ApiOperation;
import io.swagger.annotations.ApiParam;
import io.swagger.annotations.ApiModelProperty;
import io.antmedia.rest.model.Result;

class RequestPipeline {
	@ApiModelProperty(value = "Stream Id to register")
	public String streamId;

	@ApiModelProperty(value = "pipeline type Gstreamer/RTP/FFmpeg")
	public String pipeline_type;

	@ApiModelProperty(value = "actual gstremaer of ffmpeg pipelien Pass your own pipeline", required = false)
	public String pipeline;
}

@Component
@Path("/sample-plugin")
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
	public Response register_pipeline(@ApiParam(value = "casdfasdfasdf", required = true) RequestPipeline Request) {
		System.out.println("Registering Pipeline");
		SamplePlugin app = getPluginApp();
		String response = app.register_pipeline(Request.streamId, Request.pipeline_type, Request.pipeline);
		if (response == null) {
			return Response.status(Status.OK).entity("").build();
		}
		return Response.status(Status.EXPECTATION_FAILED).entity(new Result(false, response)).build();
	}

	private SamplePlugin getPluginApp() {
		ApplicationContext appCtx = (ApplicationContext) servletContext
				.getAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE);
		return (SamplePlugin) appCtx.getBean("plugin.myplugin");
	}
}
