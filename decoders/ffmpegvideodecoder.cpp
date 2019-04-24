#include "ffmpegvideodecoder.h"

void FFmpegVideoDecoder::Open()
{
  FFmpegDecoder::Open();

  // Allocate filtergraph
  filter_graph_ = avfilter_graph_alloc();

  // Handle failure to create filtergraph
  if (filter_graph_ == nullptr) {
    Error(tr("Failed to create filter graph"));
    return;
  }

  // Start creating filters
  char filter_args[200];

  // Set up parameters for buffer source (tell it what kind of video to expect)
  snprintf(filter_args, sizeof(filter_args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           stream_->codecpar->width,
           stream_->codecpar->height,
           stream_->codecpar->format,
           stream_->time_base.num,
           stream_->time_base.den,
           stream_->codecpar->sample_aspect_ratio.num,
           stream_->codecpar->sample_aspect_ratio.den
           );

  // Create buffer source
  int err = avfilter_graph_create_filter(&buffersrc_ctx_,
                                         avfilter_get_by_name("buffer"),
                                         "in",
                                         filter_args,
                                         nullptr,
                                         filter_graph_);

  // Handle failure to create buffer source
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Create buffer sink
  err = avfilter_graph_create_filter(&buffersink_ctx_,
                                     avfilter_get_by_name("buffersink"),
                                     "out",
                                     nullptr,
                                     nullptr,
                                     filter_graph_);

  // Handle failure to create buffer sink
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Create other filters and link them together
  AVFilterContext* last_filter = buffersrc_ctx_;

  // Check if the video is set as interlaced
  if (footage_stream_->video_interlacing != VIDEO_PROGRESSIVE) {

    // Set parameters for YADIF filter
    AVFilterContext* yadif_filter;
    snprintf(filter_args,
             sizeof(filter_args),
             "mode=3:parity=%d",
             ((footage_stream_->video_interlacing == VIDEO_TOP_FIELD_FIRST) ? 0 : 1));

    // Create YADIF filter to deinterlace footage
    // TODO there's a CUDA version we could possibly use for performance?
    err = avfilter_graph_create_filter(&yadif_filter,
                                       avfilter_get_by_name("yadif"),
                                       "yadif",
                                       filter_args,
                                       nullptr,
                                       filter_graph_);

    // Handle failure to create YADIF filter
    if (err < 0) {
      HandleFFError(err);
      return;
    }

    // Link YADIF filter with previous filter
    err = avfilter_link(last_filter, 0, yadif_filter, 0);

    // Handle failure to link filters
    if (err != 0) {
      HandleFFError(err);
      return;
    }

    last_filter = yadif_filter;
  }

  // Determine the best pixel format to use for this media. While all frames will eventually get converted to the
  // user-defined internal format on the GPU, for performance we keep 8-bit media at RGBA8888 for now.
  AVPixelFormat possible_pix_fmts[] = {
    AV_PIX_FMT_RGBA,
    AV_PIX_FMT_RGBA64,
    AV_PIX_FMT_NONE
  };

  // Use FFmpeg function to determine whether media is 8-bit or more
  AVPixelFormat pix_fmt = avcodec_find_best_pix_fmt_of_list(possible_pix_fmts,
                                                            static_cast<AVPixelFormat>(stream_->codecpar->format),
                                                            1,
                                                            nullptr);

  // Store pixel format for later conversion
  if (pix_fmt == AV_PIX_FMT_RGBA) {
    pix_fmt_ = olive::PIX_FMT_RGBA8;
  } else {
    pix_fmt_ = olive::PIX_FMT_RGBA16;
  }

  // Use pixel format as parameters for the format conversion
  const char* chosen_format = av_get_pix_fmt_name(pix_fmt);
  snprintf(filter_args, sizeof(filter_args), "pix_fmts=%s", chosen_format);

  AVFilterContext* format_conv;

  // Create filter for format conversion
  err = avfilter_graph_create_filter(&format_conv,
                                     avfilter_get_by_name("format"),
                                     "fmt",
                                     filter_args,
                                     nullptr,
                                     filter_graph_);

  // Handle failure to create format conversion filter
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Link last filter to the format conversion filter
  err = avfilter_link(last_filter, 0, format_conv, 0);

  // Handle failure to link
  if (err != 0) {
    HandleFFError(err);
    return;
  }

  last_filter = format_conv;

  // Finally link the last filter to the buffersink
  err = avfilter_link(last_filter, 0, buffersink_ctx_, 0);

  // Handle failure to link
  if (err != 0) {
    HandleFFError(err);
    return;
  }

  // Configure the graph
  err = avfilter_graph_config(filter_graph_, nullptr);

  // Handle failure to configure the graph
  if (err < 0) {
    HandleFFError(err);
    return;
  }
}

int FFmpegVideoDecoder::width()
{
  if (!IsOpen()) {
    return 0;
  }

  return stream_->codecpar->width;
}

int FFmpegVideoDecoder::height()
{
  if (!IsOpen()) {
    return 0;
  }

  return stream_->codecpar->height;
}
