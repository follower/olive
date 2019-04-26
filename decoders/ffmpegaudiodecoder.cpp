#include "ffmpegaudiodecoder.h"

#include <QDebug>

#include "rendering/audio.h"

const AVSampleFormat kDestSampleFmt = AV_SAMPLE_FMT_FLTP;

FFmpegAudioDecoder::FFmpegAudioDecoder()
{

}

void FFmpegAudioDecoder::Open()
{
  FFmpegDecoder::Open();

  /*
  if (codec_ctx_->channel_layout == 0) codec_ctx_->channel_layout = av_get_default_channel_layout(stream_->codecpar->channels);

  // set up cache
  frame_cache_.append(av_frame_alloc());

  if (true) {
    AVFrame* reverse_frame = av_frame_alloc();

    reverse_frame->format = kDestSampleFmt;
    reverse_frame->nb_samples = current_audio_freq()*10;
    reverse_frame->channel_layout = frame_cache_->track()->sequence()->audio_layout;
    reverse_frame->channels = av_get_channel_layout_nb_channels(clip->track()->sequence()->audio_layout);
    av_frame_get_buffer(reverse_frame, 0);

    queue_.append(reverse_frame);
  }

  // Start creating filters
  char filter_args[200];

  snprintf(filter_args, sizeof(filter_args), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
           stream_->time_base.num,
           stream_->time_base.den,
           stream_->codecpar->sample_rate,
           av_get_sample_fmt_name(codec_ctx_->sample_fmt),
           codec_ctx_->channel_layout
           );

  avfilter_graph_create_filter(&buffersrc_ctx_, avfilter_get_by_name("abuffer"), "in", filter_args, nullptr, filter_graph);
  avfilter_graph_create_filter(&buffersink_ctx_, avfilter_get_by_name("abuffersink"), "out", nullptr, nullptr, filter_graph);

  enum AVSampleFormat sample_fmts[] = { kDestSampleFmt,  static_cast<AVSampleFormat>(-1) };
  if (av_opt_set_int_list(buffersink_ctx_, "sample_fmts", sample_fmts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
    qCritical() << "Could not set output sample format";
  }

  int64_t channel_layouts[] = { AV_CH_LAYOUT_STEREO, static_cast<AVSampleFormat>(-1) };
  if (av_opt_set_int_list(buffersink_ctx_, "channel_layouts", channel_layouts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
    qCritical() << "Could not set output sample format";
  }

  int target_sample_rate = current_audio_freq();

  //double playback_speed_ = clip->speed().value * m->speed;
  double playback_speed_ = 1.0;

  if (qFuzzyCompare(playback_speed_, 1.0)) {
    avfilter_link(buffersrc_ctx_, 0, buffersink_ctx_, 0);
  } else if (clip->speed().maintain_audio_pitch) {
    AVFilterContext* previous_filter = buffersrc_ctx_;
    AVFilterContext* last_filter = buffersrc_ctx_;

    char speed_param[10];

    double base = (playback_speed_ > 1.0) ? 2.0 : 0.5;

    double speedlog = log(playback_speed_) / log(base);
    int whole2 = qFloor(speedlog);
    speedlog -= whole2;

    if (whole2 > 0) {
      snprintf(speed_param, sizeof(speed_param), "%f", base);
      for (int i=0;i<whole2;i++) {
        AVFilterContext* tempo_filter = nullptr;
        avfilter_graph_create_filter(&tempo_filter, avfilter_get_by_name("atempo"), "atempo", speed_param, nullptr, filter_graph);
        avfilter_link(previous_filter, 0, tempo_filter, 0);
        previous_filter = tempo_filter;
      }
    }

    snprintf(speed_param, sizeof(speed_param), "%f", qPow(base, speedlog));
    last_filter = nullptr;
    avfilter_graph_create_filter(&last_filter, avfilter_get_by_name("atempo"), "atempo", speed_param, nullptr, filter_graph);
    avfilter_link(previous_filter, 0, last_filter, 0);

    avfilter_link(last_filter, 0, buffersink_ctx_, 0);
  } else {
    target_sample_rate = qRound64(target_sample_rate / playback_speed_);
    avfilter_link(buffersrc_ctx_, 0, buffersink_ctx_, 0);
  }

  int sample_rates[] = { target_sample_rate, 0 };
  if (av_opt_set_int_list(buffersink_ctx_, "sample_rates", sample_rates, 0, AV_OPT_SEARCH_CHILDREN) < 0) {
    qCritical() << "Could not set output sample rates";
  }

  avfilter_graph_config(filter_graph, nullptr);

  frame_ = av_frame_alloc();
  */
}
