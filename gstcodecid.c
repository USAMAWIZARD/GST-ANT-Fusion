gst_ffmpeg_codecid_to_caps (enum AVCodecID codec_id,
    AVCodecContext * context, gboolean encode , gchar  **caps_str )
{
  GstCaps *caps = NULL;
  gboolean buildcaps = FALSE;
  g_printf ("codec_id:%d, context:%p, encode:%d", codec_id, context, encode);
  g_printf("\n");

  switch (codec_id) {
    case AV_CODEC_ID_MPEG1VIDEO:
      /* FIXME: bitrate */
      caps = gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/mpeg",
          "mpegversion", G_TYPE_INT, 1,
          "systemstream", G_TYPE_BOOLEAN, FALSE, NULL);
      break;

    case AV_CODEC_ID_MPEG2VIDEO:
      if (encode) {
        /* FIXME: bitrate */
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/mpeg",
            "mpegversion", G_TYPE_INT, 2, "systemstream", G_TYPE_BOOLEAN, FALSE,
            NULL);
      } else {
        /* decode both MPEG-1 and MPEG-2; width/height/fps are all in
         * the MPEG video stream headers, so may be omitted from caps. */
        caps = gst_caps_new_simple ("video/mpeg",
            "mpegversion", GST_TYPE_INT_RANGE, 1, 2,
            "systemstream", G_TYPE_BOOLEAN, FALSE, NULL);
      }
      break;

    case AV_CODEC_ID_H263:
      if (encode) {
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode,
            "video/x-h263", "variant", G_TYPE_STRING, "itu", "h263version",
            G_TYPE_STRING, "h263", NULL);
      } else {
        /* don't pass codec_id, we can decode other variants with the H263
         * decoder that don't have specific size requirements
         */
        caps =
            gst_ff_vid_caps_new (context, NULL, AV_CODEC_ID_NONE, encode,
            "video/x-h263", "variant", G_TYPE_STRING, "itu", NULL);
      }
      break;

    case AV_CODEC_ID_H263P:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-h263",
          "variant", G_TYPE_STRING, "itu", "h263version", G_TYPE_STRING,
          "h263p", NULL);
      if (encode && context) {

        gst_caps_set_simple (caps,
            "annex-f", G_TYPE_BOOLEAN, context->flags & AV_CODEC_FLAG_4MV,
            "annex-j", G_TYPE_BOOLEAN,
            context->flags & AV_CODEC_FLAG_LOOP_FILTER,
            "annex-i", G_TYPE_BOOLEAN, context->flags & AV_CODEC_FLAG_AC_PRED,
            "annex-t", G_TYPE_BOOLEAN, context->flags & AV_CODEC_FLAG_AC_PRED,
            NULL);
      }
      break;

    case AV_CODEC_ID_H263I:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-intel-h263", "variant", G_TYPE_STRING, "intel", NULL);
      break;

    case AV_CODEC_ID_H261:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-h261",
          NULL);
      break;

    case AV_CODEC_ID_RV10:
    case AV_CODEC_ID_RV20:
    case AV_CODEC_ID_RV30:
    case AV_CODEC_ID_RV40:
    {
      gint version;

      switch (codec_id) {
        case AV_CODEC_ID_RV40:
          version = 4;
          break;
        case AV_CODEC_ID_RV30:
          version = 3;
          break;
        case AV_CODEC_ID_RV20:
          version = 2;
          break;
        default:
          version = 1;
          break;
      }

      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-pn-realvideo", "rmversion", G_TYPE_INT, version, NULL);
      if (context) {
        if (context->extradata_size >= 8) {
          gst_caps_set_simple (caps,
              "subformat", G_TYPE_INT, GST_READ_UINT32_BE (context->extradata),
              NULL);
        }
      }
    }
      break;

    case AV_CODEC_ID_MP1:
      /* FIXME: bitrate */
      caps = gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/mpeg",
          "mpegversion", G_TYPE_INT, 1, "layer", G_TYPE_INT, 1, NULL);
      break;

    case AV_CODEC_ID_MP2:
      /* FIXME: bitrate */
      caps = gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/mpeg",
          "mpegversion", G_TYPE_INT, 1, "layer", G_TYPE_INT, 2, NULL);
      break;

    case AV_CODEC_ID_MP3:
      if (encode) {
        /* FIXME: bitrate */
        caps =
            gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/mpeg",
            "mpegversion", G_TYPE_INT, 1, "layer", G_TYPE_INT, 3, NULL);
      } else {
        /* Decodes MPEG-1 layer 1/2/3. Samplerate, channels et al are
         * in the MPEG audio header, so may be omitted from caps. */
        caps = gst_caps_new_simple ("audio/mpeg",
            "mpegversion", G_TYPE_INT, 1,
            "layer", GST_TYPE_INT_RANGE, 1, 3, NULL);
      }
      break;

    case AV_CODEC_ID_MUSEPACK7:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-ffmpeg-parsed-musepack", "streamversion", G_TYPE_INT, 7,
          NULL);
      break;

    case AV_CODEC_ID_MUSEPACK8:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-ffmpeg-parsed-musepack", "streamversion", G_TYPE_INT, 8,
          NULL);
      break;

    case AV_CODEC_ID_AC3:
      /* FIXME: bitrate */
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-ac3",
          NULL);
      break;

    case AV_CODEC_ID_EAC3:
      /* FIXME: bitrate */
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-eac3",
          NULL);
      break;

    case AV_CODEC_ID_TRUEHD:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-true-hd", NULL);
      break;

    case AV_CODEC_ID_ATRAC1:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-vnd.sony.atrac1", NULL);
      break;

    case AV_CODEC_ID_ATRAC3:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-vnd.sony.atrac3", NULL);
      break;

    case AV_CODEC_ID_DTS:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dts",
          NULL);
      break;

    case AV_CODEC_ID_APE:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-ffmpeg-parsed-ape", NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "depth", G_TYPE_INT, context->bits_per_coded_sample, NULL);
      }
      break;

    case AV_CODEC_ID_MLP:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-mlp",
          NULL);
      break;

    case AV_CODEC_ID_METASOUND:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-voxware", NULL);
      break;

    case AV_CODEC_ID_IMC:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-imc",
          NULL);
      break;

      /* MJPEG is normal JPEG, Motion-JPEG and Quicktime MJPEG-A. MJPEGB
       * is Quicktime's MJPEG-B. LJPEG is lossless JPEG. I don't know what
       * sp5x is, but it's apparently something JPEG... We don't separate
       * between those in GStreamer. Should we (at least between MJPEG,
       * MJPEG-B and sp5x decoding...)? */
    case AV_CODEC_ID_MJPEG:
    case AV_CODEC_ID_LJPEG:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/jpeg",
          "parsed", G_TYPE_BOOLEAN, TRUE, NULL);
      break;

    case AV_CODEC_ID_JPEG2000:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/x-j2c",
          NULL);
      if (!encode) {
        gst_caps_append (caps, gst_ff_vid_caps_new (context, NULL, codec_id,
                encode, "image/x-jpc", NULL));
        gst_caps_append (caps, gst_ff_vid_caps_new (context, NULL, codec_id,
                encode, "image/jp2", NULL));
      }
      break;

    case AV_CODEC_ID_SP5X:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/sp5x",
          NULL);
      break;

    case AV_CODEC_ID_MJPEGB:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-mjpeg-b", NULL);
      break;

    case AV_CODEC_ID_MPEG4:
      if (encode && context != NULL) {
        /* I'm not exactly sure what ffmpeg outputs... ffmpeg itself uses
         * the AVI fourcc 'DIVX', but 'mp4v' for Quicktime... */
        switch (context->codec_tag) {
          case GST_MAKE_FOURCC ('D', 'I', 'V', 'X'):
            caps =
                gst_ff_vid_caps_new (context, NULL, codec_id, encode,
                "video/x-divx", "divxversion", G_TYPE_INT, 5, NULL);
            break;
          case GST_MAKE_FOURCC ('m', 'p', '4', 'v'):
          default:
            /* FIXME: bitrate. libav doesn't expose the used profile and level */
            caps =
                gst_ff_vid_caps_new (context, NULL, codec_id, encode,
                "video/mpeg", "systemstream", G_TYPE_BOOLEAN, FALSE,
                "mpegversion", G_TYPE_INT, 4, NULL);
            break;
        }
      } else {
        /* The trick here is to separate xvid, divx, mpeg4, 3ivx et al */
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/mpeg",
            "mpegversion", G_TYPE_INT, 4, "systemstream", G_TYPE_BOOLEAN, FALSE,
            NULL);

        if (encode) {
          GValue arr = { 0, };
          GValue item = { 0, };

          g_value_init (&arr, GST_TYPE_LIST);
          g_value_init (&item, G_TYPE_STRING);
          g_value_set_string (&item, "simple");
          gst_value_list_append_value (&arr, &item);
          g_value_set_string (&item, "advanced-simple");
          gst_value_list_append_value (&arr, &item);
          g_value_unset (&item);

          gst_caps_set_value (caps, "profile", &arr);
          g_value_unset (&arr);

          gst_caps_append (caps, gst_ff_vid_caps_new (context, NULL, codec_id,
                  encode, "video/x-divx", "divxversion", G_TYPE_INT, 5, NULL));
        } else {
          gst_caps_append (caps, gst_ff_vid_caps_new (context, NULL, codec_id,
                  encode, "video/x-divx", "divxversion", GST_TYPE_INT_RANGE, 4,
                  5, NULL));
        }
      }
      break;

    case AV_CODEC_ID_RAWVIDEO:
      caps =
          gst_ffmpeg_codectype_to_video_caps (context, codec_id, encode, NULL);
      break;

    case AV_CODEC_ID_MSMPEG4V1:
    case AV_CODEC_ID_MSMPEG4V2:
    case AV_CODEC_ID_MSMPEG4V3:
    {
      gint version = 41 + codec_id - AV_CODEC_ID_MSMPEG4V1;

      /* encode-FIXME: bitrate */
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-msmpeg", "msmpegversion", G_TYPE_INT, version, NULL);
      if (!encode && codec_id == AV_CODEC_ID_MSMPEG4V3) {
        gst_caps_append (caps, gst_ff_vid_caps_new (context, NULL, codec_id,
                encode, "video/x-divx", "divxversion", G_TYPE_INT, 3, NULL));
      }
    }
      break;

    case AV_CODEC_ID_WMV1:
    case AV_CODEC_ID_WMV2:
    {
      gint version = (codec_id == AV_CODEC_ID_WMV1) ? 1 : 2;

      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-wmv",
          "wmvversion", G_TYPE_INT, version, NULL);
    }
      break;

    case AV_CODEC_ID_FLV1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-flash-video", "flvversion", G_TYPE_INT, 1, NULL);
      break;

    case AV_CODEC_ID_SVQ1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-svq",
          "svqversion", G_TYPE_INT, 1, NULL);
      break;

    case AV_CODEC_ID_SVQ3:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-svq",
          "svqversion", G_TYPE_INT, 3, NULL);
      break;

    case AV_CODEC_ID_DVAUDIO:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dv",
          NULL);
      break;

    case AV_CODEC_ID_DVVIDEO:
    {
      if (encode && context) {
        const gchar *format;

        switch (context->pix_fmt) {
          case AV_PIX_FMT_YUYV422:
            format = "YUY2";
            break;
          case AV_PIX_FMT_YUV420P:
            format = "I420";
            break;
          case AV_PIX_FMT_YUVA420P:
            format = "A420";
            break;
          case AV_PIX_FMT_YUV411P:
            format = "Y41B";
            break;
          case AV_PIX_FMT_YUV422P:
            format = "Y42B";
            break;
          case AV_PIX_FMT_YUV410P:
            format = "YUV9";
            break;
          default:
            GST_WARNING
                ("Couldnt' find format for pixfmt %d, defaulting to I420",
                context->pix_fmt);
            format = "I420";
            break;
        }
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-dv",
            "systemstream", G_TYPE_BOOLEAN, FALSE, "format", G_TYPE_STRING,
            format, NULL);
      } else {
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-dv",
            "systemstream", G_TYPE_BOOLEAN, FALSE, NULL);
      }
    }
      break;

    case AV_CODEC_ID_WMAV1:
    case AV_CODEC_ID_WMAV2:
    {
      gint version = (codec_id == AV_CODEC_ID_WMAV1) ? 1 : 2;

      if (context) {
        caps =
            gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-wma",
            "wmaversion", G_TYPE_INT, version, "block_align", G_TYPE_INT,
            context->block_align, "bitrate", G_TYPE_INT,
            (guint) context->bit_rate, NULL);
      } else {
        caps =
            gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-wma",
            "wmaversion", G_TYPE_INT, version, "block_align",
            GST_TYPE_INT_RANGE, 0, G_MAXINT, "bitrate", GST_TYPE_INT_RANGE, 0,
            G_MAXINT, NULL);
      }
    }
      break;
    case AV_CODEC_ID_WMAPRO:
    {
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-wma",
          "wmaversion", G_TYPE_INT, 3, NULL);
      break;
    }
    case AV_CODEC_ID_WMALOSSLESS:
    {
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-wma",
          "wmaversion", G_TYPE_INT, 4, NULL);
      break;
    }
    case AV_CODEC_ID_WMAVOICE:
    {
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-wms",
          NULL);
      break;
    }

    case AV_CODEC_ID_XMA1:
    {
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-xma",
          "xmaversion", G_TYPE_INT, 1, NULL);
      break;
    }
    case AV_CODEC_ID_XMA2:
    {
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-xma",
          "xmaversion", G_TYPE_INT, 2, NULL);
      break;
    }

    case AV_CODEC_ID_MACE3:
    case AV_CODEC_ID_MACE6:
    {
      gint version = (codec_id == AV_CODEC_ID_MACE3) ? 3 : 6;

      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-mace",
          "maceversion", G_TYPE_INT, version, NULL);
    }
      break;

    case AV_CODEC_ID_HUFFYUV:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-huffyuv", NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "bpp", G_TYPE_INT, context->bits_per_coded_sample, NULL);
      }
      break;

    case AV_CODEC_ID_CYUV:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-compressed-yuv", NULL);
      break;

    case AV_CODEC_ID_H264:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-h264",
          "alignment", G_TYPE_STRING, "au", NULL);
      if (!encode) {
        GValue arr = { 0, };
        GValue item = { 0, };
        g_value_init (&arr, GST_TYPE_LIST);
        g_value_init (&item, G_TYPE_STRING);
        g_value_set_string (&item, "avc");
        gst_value_list_append_value (&arr, &item);
        g_value_set_string (&item, "byte-stream");
        gst_value_list_append_value (&arr, &item);
        g_value_unset (&item);
        gst_caps_set_value (caps, "stream-format", &arr);
        g_value_unset (&arr);

        gst_caps_append (caps, gst_ff_vid_caps_new (context, NULL, codec_id,
                encode, "video/x-h264", "alignment", G_TYPE_STRING, "nal",
                "stream-format", G_TYPE_STRING, "byte-stream", NULL));

      } else if (context) {
        /* FIXME: ffmpeg currently assumes AVC if there is extradata and
         * byte-stream otherwise. See for example the MOV or MPEG-TS code.
         * ffmpeg does not distinguish the different types of AVC. */
        if (context->extradata_size > 0) {
          gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING, "avc",
              NULL);
        } else {
          gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING,
              "byte-stream", NULL);
        }
      }
      break;

    case AV_CODEC_ID_HEVC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-h265",
          "alignment", G_TYPE_STRING, "au", NULL);
      if (!encode) {
        GValue arr = { 0, };
        GValue item = { 0, };
        g_value_init (&arr, GST_TYPE_LIST);
        g_value_init (&item, G_TYPE_STRING);
        g_value_set_string (&item, "hvc1");
        gst_value_list_append_value (&arr, &item);
        g_value_set_string (&item, "hev1");
        gst_value_list_append_value (&arr, &item);
        g_value_set_string (&item, "byte-stream");
        gst_value_list_append_value (&arr, &item);
        g_value_unset (&item);
        gst_caps_set_value (caps, "stream-format", &arr);
        g_value_unset (&arr);
      } else if (context) {
        /* FIXME: ffmpeg currently assumes HVC1 if there is extradata and
         * byte-stream otherwise. See for example the MOV or MPEG-TS code.
         * ffmpeg does not distinguish the different types: HVC1/HEV1/etc. */
        if (context->extradata_size > 0) {
          gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING, "hvc1",
              NULL);
        } else {
          gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING,
              "byte-stream", NULL);
        }
      }
      break;

    case AV_CODEC_ID_INDEO5:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-indeo",
          "indeoversion", G_TYPE_INT, 5, NULL);
      break;

    case AV_CODEC_ID_INDEO4:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-indeo",
          "indeoversion", G_TYPE_INT, 4, NULL);
      break;

    case AV_CODEC_ID_INDEO3:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-indeo",
          "indeoversion", G_TYPE_INT, 3, NULL);
      break;

    case AV_CODEC_ID_INDEO2:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-indeo",
          "indeoversion", G_TYPE_INT, 2, NULL);
      break;

    case AV_CODEC_ID_FLASHSV:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-flash-screen", NULL);
      break;

    case AV_CODEC_ID_FLASHSV2:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-flash-screen2", NULL);
      break;

    case AV_CODEC_ID_VP3:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-vp3",
          NULL);
      break;

    case AV_CODEC_ID_VP5:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-vp5",
          NULL);
      break;

    case AV_CODEC_ID_VP6:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-vp6",
          NULL);
      break;

    case AV_CODEC_ID_VP6F:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-vp6-flash", NULL);
      break;

    case AV_CODEC_ID_VP6A:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-vp6-alpha", NULL);
      break;

    case AV_CODEC_ID_VP8:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-vp8",
          NULL);
      break;

    case AV_CODEC_ID_VP9:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-vp9",
          NULL);
      break;

    case AV_CODEC_ID_THEORA:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-theora", NULL);
      break;

    case AV_CODEC_ID_CFHD:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-cineform", NULL);
      break;

    case AV_CODEC_ID_SPEEDHQ:
      if (context && context->codec_tag) {
        gchar *variant = g_strdup_printf ("%" GST_FOURCC_FORMAT,
            GST_FOURCC_ARGS (context->codec_tag));
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode,
            "video/x-speedhq", "variant", G_TYPE_STRING, variant, NULL);
        g_free (variant);
      } else {
        caps =
            gst_ff_vid_caps_new (context, NULL, codec_id, encode,
            "video/x-speedhq", NULL);
      }
      break;

    case AV_CODEC_ID_AAC:
    {
      printf("aac found\n");
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/mpeg",
          NULL);
      printf("caps asjdklald : %s\n",gst_caps_to_string(caps ));

      if (!encode) {
        GValue arr = { 0, };
        GValue item = { 0, };

        g_value_init (&arr, GST_TYPE_LIST);
        g_value_init (&item, G_TYPE_INT);
        g_value_set_int (&item, 2);
        gst_value_list_append_value (&arr, &item);
        g_value_set_int (&item, 4);
        gst_value_list_append_value (&arr, &item);
        g_value_unset (&item);

        gst_caps_set_value (caps, "mpegversion", &arr);
        g_value_unset (&arr);

        g_value_init (&arr, GST_TYPE_LIST);
        g_value_init (&item, G_TYPE_STRING);
        g_value_set_string (&item, "raw");
        gst_value_list_append_value (&arr, &item);
        g_value_set_string (&item, "adts");
        gst_value_list_append_value (&arr, &item);
        g_value_set_string (&item, "adif");
        gst_value_list_append_value (&arr, &item);
        g_value_unset (&item);

        gst_caps_set_value (caps, "stream-format", &arr);
        g_value_unset (&arr);
        printf("not encoding shit\n");
        printf("%s\n", gst_caps_to_string(caps));
      } else {
        gst_caps_set_simple (caps, "mpegversion", G_TYPE_INT, 4,
            "base-profile", G_TYPE_STRING, "lc", NULL);

        /* FIXME: ffmpeg currently assumes raw if there is extradata and
         * ADTS otherwise. See for example the FDK AAC encoder. */
        if (context && context->extradata_size > 0) {
          gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING, "raw",
              NULL);
         // gst_codec_utils_aac_caps_set_level_and_profile (caps,
           //   context->extradata, context->extradata_size);
        } else if (context) {
          gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING, "adts",
              NULL);
        }
        printf("encoding shit\n");
        printf("%s\n", gst_caps_to_string(caps));
      }
      break;
    }
    case AV_CODEC_ID_AAC_LATM: /* LATM/LOAS AAC syntax */
      caps = gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/mpeg",
          "mpegversion", G_TYPE_INT, 4, "stream-format", G_TYPE_STRING, "loas",
          NULL);
      break;

    case AV_CODEC_ID_ASV1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-asus",
          "asusversion", G_TYPE_INT, 1, NULL);
      break;
    case AV_CODEC_ID_ASV2:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-asus",
          "asusversion", G_TYPE_INT, 2, NULL);
      break;

    case AV_CODEC_ID_FFV1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-ffv",
          "ffvversion", G_TYPE_INT, 1, NULL);
      break;

    case AV_CODEC_ID_4XM:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-4xm",
          NULL);
      break;

    case AV_CODEC_ID_XAN_WC3:
    case AV_CODEC_ID_XAN_WC4:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-xan",
          "wcversion", G_TYPE_INT, 3 - AV_CODEC_ID_XAN_WC3 + codec_id, NULL);
      break;

    case AV_CODEC_ID_CLJR:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-cirrus-logic-accupak", NULL);
      break;

    case AV_CODEC_ID_FRAPS:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-fraps",
          NULL);
      break;

    case AV_CODEC_ID_MDEC:
    case AV_CODEC_ID_ROQ:
    case AV_CODEC_ID_INTERPLAY_VIDEO:
      buildcaps = TRUE;
      break;

    case AV_CODEC_ID_VCR1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-ati-vcr", "vcrversion", G_TYPE_INT, 1, NULL);
      break;

    case AV_CODEC_ID_RPZA:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-apple-video", NULL);
      break;

    case AV_CODEC_ID_CINEPAK:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-cinepak", NULL);
      break;

      /* WS_VQA belogns here (order) */

    case AV_CODEC_ID_MSRLE:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-rle",
          "layout", G_TYPE_STRING, "microsoft", NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "depth", G_TYPE_INT, (gint) context->bits_per_coded_sample, NULL);
      } else {
        gst_caps_set_simple (caps, "depth", GST_TYPE_INT_RANGE, 1, 64, NULL);
      }
      break;

    case AV_CODEC_ID_QTRLE:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-rle",
          "layout", G_TYPE_STRING, "quicktime", NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "depth", G_TYPE_INT, (gint) context->bits_per_coded_sample, NULL);
      } else {
        gst_caps_set_simple (caps, "depth", GST_TYPE_INT_RANGE, 1, 64, NULL);
      }
      break;

    case AV_CODEC_ID_MSVIDEO1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-msvideocodec", "msvideoversion", G_TYPE_INT, 1, NULL);
      break;

    case AV_CODEC_ID_MSS1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-wmv",
          "wmvversion", G_TYPE_INT, 1, "format", G_TYPE_STRING, "MSS1", NULL);
      break;

    case AV_CODEC_ID_MSS2:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-wmv",
          "wmvversion", G_TYPE_INT, 3, "format", G_TYPE_STRING, "MSS2", NULL);
      break;

    case AV_CODEC_ID_WMV3:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-wmv",
          "wmvversion", G_TYPE_INT, 3, "format", G_TYPE_STRING, "WMV3", NULL);
      break;
    case AV_CODEC_ID_VC1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-wmv",
          "wmvversion", G_TYPE_INT, 3, NULL);
      if (!context && !encode) {
        GValue arr = { 0, };
        GValue item = { 0, };

        g_value_init (&arr, GST_TYPE_LIST);
        g_value_init (&item, G_TYPE_STRING);
        g_value_set_string (&item, "WVC1");
        gst_value_list_append_value (&arr, &item);
        g_value_set_string (&item, "WMVA");
        gst_value_list_append_and_take_value (&arr, &item);
        gst_caps_set_value (caps, "format", &arr);
        g_value_unset (&arr);
      } else {
        gst_caps_set_simple (caps, "format", G_TYPE_STRING, "WVC1", NULL);
      }
      break;
    case AV_CODEC_ID_QDM2:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-qdm2",
          NULL);
      break;

    case AV_CODEC_ID_MSZH:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-mszh",
          NULL);
      break;

    case AV_CODEC_ID_ZLIB:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-zlib",
          NULL);
      break;

    case AV_CODEC_ID_TRUEMOTION1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-truemotion", "trueversion", G_TYPE_INT, 1, NULL);
      break;
    case AV_CODEC_ID_TRUEMOTION2:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-truemotion", "trueversion", G_TYPE_INT, 2, NULL);
      break;

    case AV_CODEC_ID_ULTI:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-ultimotion", NULL);
      break;

    case AV_CODEC_ID_TSCC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-camtasia", NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "depth", G_TYPE_INT, (gint) context->bits_per_coded_sample, NULL);
      } else {
        gst_caps_set_simple (caps, "depth", GST_TYPE_INT_RANGE, 8, 32, NULL);
      }
      break;

    case AV_CODEC_ID_TSCC2:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-tscc", "tsccversion", G_TYPE_INT, 2, NULL);
      break;

    case AV_CODEC_ID_KMVC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-kmvc",
          NULL);
      break;

    case AV_CODEC_ID_NUV:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-nuv",
          NULL);
      break;

    case AV_CODEC_ID_GIF:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "image/gst-libav-gif", "parsed", G_TYPE_BOOLEAN, TRUE, NULL);
      break;

    case AV_CODEC_ID_PNG:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/png",
          NULL);
      break;

    case AV_CODEC_ID_PPM:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/ppm",
          NULL);
      break;

    case AV_CODEC_ID_PBM:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/pbm",
          NULL);
      break;

    case AV_CODEC_ID_PAM:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "image/x-portable-anymap", NULL);
      break;

    case AV_CODEC_ID_PGM:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "image/x-portable-graymap", NULL);
      break;

    case AV_CODEC_ID_PCX:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/x-pcx",
          NULL);
      break;

    case AV_CODEC_ID_SGI:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/x-sgi",
          NULL);
      break;

    case AV_CODEC_ID_TARGA:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/x-tga",
          NULL);
      break;

    case AV_CODEC_ID_TIFF:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "image/tiff",
          NULL);
      break;

    case AV_CODEC_ID_SUNRAST:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "image/x-sun-raster", NULL);
      break;

    case AV_CODEC_ID_SMC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-smc",
          NULL);
      break;

    case AV_CODEC_ID_QDRAW:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-qdrw",
          NULL);
      break;

    case AV_CODEC_ID_DNXHD:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-dnxhd",
          NULL);
      break;

    case AV_CODEC_ID_PRORES:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-prores", NULL);
      if (context) {
        switch (context->codec_tag) {
          case GST_MAKE_FOURCC ('a', 'p', 'c', 'o'):
            gst_caps_set_simple (caps, "variant", G_TYPE_STRING, "proxy", NULL);
            break;
          case GST_MAKE_FOURCC ('a', 'p', 'c', 's'):
            gst_caps_set_simple (caps, "variant", G_TYPE_STRING, "lt", NULL);
            break;
          default:
          case GST_MAKE_FOURCC ('a', 'p', 'c', 'n'):
            gst_caps_set_simple (caps, "variant", G_TYPE_STRING, "standard",
                NULL);
            break;
          case GST_MAKE_FOURCC ('a', 'p', 'c', 'h'):
            gst_caps_set_simple (caps, "variant", G_TYPE_STRING, "hq", NULL);
            break;
          case GST_MAKE_FOURCC ('a', 'p', '4', 'h'):
            gst_caps_set_simple (caps, "variant", G_TYPE_STRING, "4444", NULL);
            break;
          case GST_MAKE_FOURCC ('a', 'p', '4', 'x'):
            gst_caps_set_simple (caps, "variant", G_TYPE_STRING, "4444xq",
                NULL);
            break;
        }
      }
      break;

    case AV_CODEC_ID_MIMIC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-mimic",
          NULL);
      break;

    case AV_CODEC_ID_VMNC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-vmnc",
          NULL);
      break;

    case AV_CODEC_ID_TRUESPEECH:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-truespeech", NULL);
      break;

    case AV_CODEC_ID_QCELP:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/qcelp",
          NULL);
      break;

    case AV_CODEC_ID_AMV:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-amv",
          NULL);
      break;

    case AV_CODEC_ID_AASC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-aasc",
          NULL);
      break;

    case AV_CODEC_ID_LOCO:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-loco",
          NULL);
      break;

    case AV_CODEC_ID_ZMBV:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-zmbv",
          NULL);
      break;

    case AV_CODEC_ID_LAGARITH:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-lagarith", NULL);
      break;

    case AV_CODEC_ID_CSCD:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-camstudio", NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "depth", G_TYPE_INT, (gint) context->bits_per_coded_sample, NULL);
      } else {
        gst_caps_set_simple (caps, "depth", GST_TYPE_INT_RANGE, 8, 32, NULL);
      }
      break;

    case AV_CODEC_ID_AIC:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-apple-intermediate-codec", NULL);
      break;

    case AV_CODEC_ID_CAVS:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode,
          "video/x-cavs", NULL);
      break;

    case AV_CODEC_ID_WS_VQA:
    case AV_CODEC_ID_IDCIN:
    case AV_CODEC_ID_8BPS:
    case AV_CODEC_ID_FLIC:
    case AV_CODEC_ID_VMDVIDEO:
    case AV_CODEC_ID_VMDAUDIO:
    case AV_CODEC_ID_VIXL:
    case AV_CODEC_ID_QPEG:
    case AV_CODEC_ID_PGMYUV:
    case AV_CODEC_ID_FFVHUFF:
    case AV_CODEC_ID_WNV1:
    case AV_CODEC_ID_MP3ADU:
    case AV_CODEC_ID_MP3ON4:
    case AV_CODEC_ID_WESTWOOD_SND1:
    case AV_CODEC_ID_MMVIDEO:
    case AV_CODEC_ID_AVS:
      buildcaps = TRUE;
      break;

      /* weird quasi-codecs for the demuxers only */
    case AV_CODEC_ID_PCM_S16LE:
    case AV_CODEC_ID_PCM_S16BE:
    case AV_CODEC_ID_PCM_U16LE:
    case AV_CODEC_ID_PCM_U16BE:
    case AV_CODEC_ID_PCM_S8:
    case AV_CODEC_ID_PCM_U8:
    {
      GstAudioFormat format;

      switch (codec_id) {
        case AV_CODEC_ID_PCM_S16LE:
          format = GST_AUDIO_FORMAT_S16LE;
          break;
        case AV_CODEC_ID_PCM_S16BE:
          format = GST_AUDIO_FORMAT_S16BE;
          break;
        case AV_CODEC_ID_PCM_U16LE:
          format = GST_AUDIO_FORMAT_U16LE;
          break;
        case AV_CODEC_ID_PCM_U16BE:
          format = GST_AUDIO_FORMAT_U16BE;
          break;
        case AV_CODEC_ID_PCM_S8:
          format = GST_AUDIO_FORMAT_S8;
          break;
        case AV_CODEC_ID_PCM_U8:
          format = GST_AUDIO_FORMAT_U8;
          break;
        default:
          format = 0;
          g_assert (0);         /* don't worry, we never get here */
          break;
      }

      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-raw",
          "format", G_TYPE_STRING, gst_audio_format_to_string (format),
          "layout", G_TYPE_STRING, "interleaved", NULL);
    }
      break;

    case AV_CODEC_ID_PCM_MULAW:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-mulaw",
          NULL);
      break;

    case AV_CODEC_ID_PCM_ALAW:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-alaw",
          NULL);
      break;

    case AV_CODEC_ID_ADPCM_G722:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/G722",
          NULL);
      if (context)
        gst_caps_set_simple (caps,
            "block_align", G_TYPE_INT, context->block_align,
            "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);
      break;

    case AV_CODEC_ID_ADPCM_G726:
    {
      /* the G726 decoder can also handle G721 */
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-adpcm",
          "layout", G_TYPE_STRING, "g726", NULL);
      if (context)
        gst_caps_set_simple (caps,
            "block_align", G_TYPE_INT, context->block_align,
            "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);

      if (!encode) {
        gst_caps_append (caps, gst_caps_new_simple ("audio/x-adpcm",
                "layout", G_TYPE_STRING, "g721",
                "channels", G_TYPE_INT, 1, "rate", G_TYPE_INT, 8000, NULL));
      }
      break;
    }
    case AV_CODEC_ID_ADPCM_IMA_QT:
    case AV_CODEC_ID_ADPCM_IMA_WAV:
    case AV_CODEC_ID_ADPCM_IMA_DK3:
    case AV_CODEC_ID_ADPCM_IMA_DK4:
    case AV_CODEC_ID_ADPCM_IMA_OKI:
    case AV_CODEC_ID_ADPCM_IMA_WS:
    case AV_CODEC_ID_ADPCM_IMA_SMJPEG:
    case AV_CODEC_ID_ADPCM_IMA_AMV:
    case AV_CODEC_ID_ADPCM_IMA_ISS:
    case AV_CODEC_ID_ADPCM_IMA_EA_EACS:
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
    case AV_CODEC_ID_ADPCM_MS:
    case AV_CODEC_ID_ADPCM_4XM:
    case AV_CODEC_ID_ADPCM_XA:
    case AV_CODEC_ID_ADPCM_ADX:
    case AV_CODEC_ID_ADPCM_EA:
    case AV_CODEC_ID_ADPCM_CT:
    case AV_CODEC_ID_ADPCM_SWF:
    case AV_CODEC_ID_ADPCM_YAMAHA:
    case AV_CODEC_ID_ADPCM_SBPRO_2:
    case AV_CODEC_ID_ADPCM_SBPRO_3:
    case AV_CODEC_ID_ADPCM_SBPRO_4:
    case AV_CODEC_ID_ADPCM_EA_R1:
    case AV_CODEC_ID_ADPCM_EA_R2:
    case AV_CODEC_ID_ADPCM_EA_R3:
    case AV_CODEC_ID_ADPCM_EA_MAXIS_XA:
    case AV_CODEC_ID_ADPCM_EA_XAS:
    case AV_CODEC_ID_ADPCM_THP:
    {
      const gchar *layout = NULL;

      switch (codec_id) {
        case AV_CODEC_ID_ADPCM_IMA_QT:
          layout = "quicktime";
          break;
        case AV_CODEC_ID_ADPCM_IMA_WAV:
          layout = "dvi";
          break;
        case AV_CODEC_ID_ADPCM_IMA_DK3:
          layout = "dk3";
          break;
        case AV_CODEC_ID_ADPCM_IMA_DK4:
          layout = "dk4";
          break;
        case AV_CODEC_ID_ADPCM_IMA_OKI:
          layout = "oki";
          break;
        case AV_CODEC_ID_ADPCM_IMA_WS:
          layout = "westwood";
          break;
        case AV_CODEC_ID_ADPCM_IMA_SMJPEG:
          layout = "smjpeg";
          break;
        case AV_CODEC_ID_ADPCM_IMA_AMV:
          layout = "amv";
          break;
        case AV_CODEC_ID_ADPCM_IMA_ISS:
          layout = "iss";
          break;
        case AV_CODEC_ID_ADPCM_IMA_EA_EACS:
          layout = "ea-eacs";
          break;
        case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
          layout = "ea-sead";
          break;
        case AV_CODEC_ID_ADPCM_MS:
          layout = "microsoft";
          break;
        case AV_CODEC_ID_ADPCM_4XM:
          layout = "4xm";
          break;
        case AV_CODEC_ID_ADPCM_XA:
          layout = "xa";
          break;
        case AV_CODEC_ID_ADPCM_ADX:
          layout = "adx";
          break;
        case AV_CODEC_ID_ADPCM_EA:
          layout = "ea";
          break;
        case AV_CODEC_ID_ADPCM_CT:
          layout = "ct";
          break;
        case AV_CODEC_ID_ADPCM_SWF:
          layout = "swf";
          break;
        case AV_CODEC_ID_ADPCM_YAMAHA:
          layout = "yamaha";
          break;
        case AV_CODEC_ID_ADPCM_SBPRO_2:
          layout = "sbpro2";
          break;
        case AV_CODEC_ID_ADPCM_SBPRO_3:
          layout = "sbpro3";
          break;
        case AV_CODEC_ID_ADPCM_SBPRO_4:
          layout = "sbpro4";
          break;
        case AV_CODEC_ID_ADPCM_EA_R1:
          layout = "ea-r1";
          break;
        case AV_CODEC_ID_ADPCM_EA_R2:
          layout = "ea-r3";
          break;
        case AV_CODEC_ID_ADPCM_EA_R3:
          layout = "ea-r3";
          break;
        case AV_CODEC_ID_ADPCM_EA_MAXIS_XA:
          layout = "ea-maxis-xa";
          break;
        case AV_CODEC_ID_ADPCM_EA_XAS:
          layout = "ea-xas";
          break;
        case AV_CODEC_ID_ADPCM_THP:
          layout = "thp";
          break;
        default:
          g_assert (0);         /* don't worry, we never get here */
          break;
      }

      /* FIXME: someone please check whether we need additional properties
       * in this caps definition. */
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-adpcm",
          "layout", G_TYPE_STRING, layout, NULL);
      if (context)
        gst_caps_set_simple (caps,
            "block_align", G_TYPE_INT, context->block_align,
            "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);
    }
      break;

    case AV_CODEC_ID_AMR_NB:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/AMR",
          NULL);
      break;

    case AV_CODEC_ID_AMR_WB:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/AMR-WB",
          NULL);
      break;

    case AV_CODEC_ID_GSM:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-gsm",
          NULL);
      break;

    case AV_CODEC_ID_GSM_MS:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/ms-gsm",
          NULL);
      break;

    case AV_CODEC_ID_NELLYMOSER:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-nellymoser", NULL);
      break;

    case AV_CODEC_ID_SIPR:
    {
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-sipro",
          NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "leaf_size", G_TYPE_INT, context->block_align,
            "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);
      }
    }
      break;

    case AV_CODEC_ID_RA_144:
    case AV_CODEC_ID_RA_288:
    case AV_CODEC_ID_COOK:
    {
      gint version = 0;

      switch (codec_id) {
        case AV_CODEC_ID_RA_144:
          version = 1;
          break;
        case AV_CODEC_ID_RA_288:
          version = 2;
          break;
        case AV_CODEC_ID_COOK:
          version = 8;
          break;
        default:
          break;
      }

      /* FIXME: properties? */
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-pn-realaudio", "raversion", G_TYPE_INT, version, NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "leaf_size", G_TYPE_INT, context->block_align,
            "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);
      }
    }
      break;

    case AV_CODEC_ID_ROQ_DPCM:
    case AV_CODEC_ID_INTERPLAY_DPCM:
    case AV_CODEC_ID_XAN_DPCM:
    case AV_CODEC_ID_SOL_DPCM:
    {
      const gchar *layout = NULL;

      switch (codec_id) {
        case AV_CODEC_ID_ROQ_DPCM:
          layout = "roq";
          break;
        case AV_CODEC_ID_INTERPLAY_DPCM:
          layout = "interplay";
          break;
        case AV_CODEC_ID_XAN_DPCM:
          layout = "xan";
          break;
        case AV_CODEC_ID_SOL_DPCM:
          layout = "sol";
          break;
        default:
          g_assert (0);         /* don't worry, we never get here */
          break;
      }

      /* FIXME: someone please check whether we need additional properties
       * in this caps definition. */
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dpcm",
          "layout", G_TYPE_STRING, layout, NULL);
      if (context)
        gst_caps_set_simple (caps,
            "block_align", G_TYPE_INT, context->block_align,
            "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);
    }
      break;

    case AV_CODEC_ID_SHORTEN:
      caps = gst_caps_new_empty_simple ("audio/x-shorten");
      break;

    case AV_CODEC_ID_ALAC:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-alac",
          NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "samplesize", G_TYPE_INT, context->bits_per_coded_sample, NULL);
      }
      break;

    case AV_CODEC_ID_FLAC:
      /* Note that ffmpeg has no encoder yet, but just for safety. In the
       * encoder case, we want to add things like samplerate, channels... */
      if (!encode) {
        caps = gst_caps_new_empty_simple ("audio/x-flac");
      }
      break;

    case AV_CODEC_ID_OPUS:
      /* Note that ffmpeg has no encoder yet, but just for safety. In the
       * encoder case, we want to add things like samplerate, channels... */
      if (!encode) {
        /* FIXME: can ffmpeg handle multichannel Opus? */
        caps = gst_caps_new_simple ("audio/x-opus",
            "channel-mapping-family", G_TYPE_INT, 0, NULL);
      }
      break;

    case AV_CODEC_ID_S302M:
      caps = gst_caps_new_empty_simple ("audio/x-smpte-302m");
      break;

    case AV_CODEC_ID_DVD_SUBTITLE:
    case AV_CODEC_ID_DVB_SUBTITLE:
      caps = NULL;
      break;
    case AV_CODEC_ID_BMP:
      caps = gst_caps_new_empty_simple ("image/bmp");
      break;
    case AV_CODEC_ID_TTA:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-tta",
          NULL);
      if (context) {
        gst_caps_set_simple (caps,
            "samplesize", G_TYPE_INT, context->bits_per_coded_sample, NULL);
      }
      break;
    case AV_CODEC_ID_TWINVQ:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode,
          "audio/x-twin-vq", NULL);
      break;
    case AV_CODEC_ID_G729:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/G729",
          NULL);
      break;
    case AV_CODEC_ID_DSD_LSBF:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dsd",
          NULL);
      gst_caps_set_simple (caps, "lsbf", G_TYPE_BOOLEAN,
          TRUE, "planar", G_TYPE_BOOLEAN, FALSE, NULL);
      break;
    case AV_CODEC_ID_DSD_MSBF:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dsd",
          NULL);
      gst_caps_set_simple (caps, "lsbf", G_TYPE_BOOLEAN,
          FALSE, "planar", G_TYPE_BOOLEAN, FALSE, NULL);
      break;
    case AV_CODEC_ID_DSD_LSBF_PLANAR:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dsd",
          NULL);
      gst_caps_set_simple (caps, "lsbf", G_TYPE_BOOLEAN,
          TRUE, "planar", G_TYPE_BOOLEAN, TRUE, NULL);
      break;
    case AV_CODEC_ID_DSD_MSBF_PLANAR:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/x-dsd",
          NULL);
      gst_caps_set_simple (caps, "lsbf", G_TYPE_BOOLEAN,
          FALSE, "planar", G_TYPE_BOOLEAN, TRUE, NULL);
      break;
    case AV_CODEC_ID_APTX:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/aptx",
          NULL);
      break;
    case AV_CODEC_ID_APTX_HD:
      caps =
          gst_ff_aud_caps_new (context, NULL, codec_id, encode, "audio/aptx-hd",
          NULL);
      break;
    case AV_CODEC_ID_AV1:
      caps =
          gst_ff_vid_caps_new (context, NULL, codec_id, encode, "video/x-av1",
          NULL);
      break;
    default:
      GST_DEBUG ("Unknown codec ID %d, please add mapping here", codec_id);
      break;
  }

  if (buildcaps) {
    printf("entering build caps\n");
    const AVCodec *codec;
    if ((codec = avcodec_find_decoder (codec_id)) ||
        (codec = avcodec_find_encoder (codec_id))) {
      gchar *mime = NULL;

      g_printf ("Could not create stream format caps for %s", codec->name);
      g_printf("\n");

      switch (codec->type) {
        case AVMEDIA_TYPE_VIDEO:
          mime = g_strdup_printf ("video/x-gst-av-%s", codec->name);
          caps =
              gst_ff_vid_caps_new (context, NULL, codec_id, encode, mime, NULL);
          g_free (mime);
          break;
        case AVMEDIA_TYPE_AUDIO:
          mime = g_strdup_printf ("audio/x-gst-av-%s", codec->name);
          caps =
              gst_ff_aud_caps_new (context, NULL, codec_id, encode, mime, NULL);
          if (context)
            gst_caps_set_simple (caps,
                "block_align", G_TYPE_INT, context->block_align,
                "bitrate", G_TYPE_INT, (guint) context->bit_rate, NULL);
          g_free (mime);
          break;
        default:
          break;
      }
    }
  }

  if (caps != NULL) {

    /* set private data */
    if (context && context->extradata_size > 0) {
      GstBuffer *data = gst_buffer_new_and_alloc (context->extradata_size);

      gst_buffer_fill (data, 0, context->extradata, context->extradata_size);
      gst_caps_set_simple (caps, "codec_data", GST_TYPE_BUFFER, data, NULL);
      gst_buffer_unref (data);
    }

    g_printf ("caps for codec_id=%d: %" GST_PTR_FORMAT, codec_id, caps);
    g_printf("\n");


  } else {
    g_printf ("No caps found for codec_id=%d", codec_id);
    g_printf("\n");
  }
    printf("before returning\n");

if (caps_str != NULL){ 
    *caps_str =  gst_caps_to_string(caps);
    printf("%s\n", *caps_str);
 }
    return caps;
}