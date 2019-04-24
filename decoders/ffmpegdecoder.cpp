#include "ffmpegdecoder.h"

#include <QStatusBar>
#include <QDebug>

#include "ui/mainwindow.h"
#include "rendering/pixelformats.h"

FFmpegDecoder::FFmpegDecoder() :
  fmt_ctx_(nullptr),
  codec_ctx_(nullptr),
  stream_(nullptr),
  codec_(nullptr),
  opts_(nullptr),
  filter_graph_(nullptr),
  buffersrc_ctx_(nullptr),
  buffersink_ctx_(nullptr),
  pkt_(nullptr),
  frame_(nullptr)
{
}

void FFmpegDecoder::Open()
{
  if (footage_stream_ == nullptr) {
    qWarning() << "Failed to start FFmpeg decoder - null footage stream";
    return;
  }

  const char* c_fn = footage_stream_->footage()->url.toUtf8().constData();

  // Open the file with FFmpeg
  int err = avformat_open_input(&fmt_ctx_,
                                c_fn,
                                nullptr,
                                nullptr);

  // Handle FFmpeg failing to open the file
  if (err != 0) {
    HandleFFError(err);
    return;
  }

  // Find the stream information of this file
  err = avformat_find_stream_info(fmt_ctx_, nullptr);

  // Handle FFmpeg failing to find stream info
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Dump the format information
  av_dump_format(fmt_ctx_, 0, c_fn, 0);

  // Get AVStream
  stream_ = fmt_ctx_->streams[footage_stream_->file_index];

  // Find decoder for this stream
  codec_ = avcodec_find_decoder(stream_->codecpar->codec_id);

  // Handle FFmpeg unable to find decoder
  if (codec_ == nullptr) {
    Error(tr("Failed to find decoder for this stream"));
    return;
  }

  // Create codec context
  codec_ctx_ = avcodec_alloc_context3(codec_);

  // Handle failure to create codec context
  if (codec_ctx_ == nullptr) {
    Error(tr("Failed to create codec context"));
  }

  // Copy codec parameters to the codec context
  err = avcodec_parameters_to_context(codec_ctx_, stream_->codecpar);

  // Handle failure to copy codec parameters
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Default to multithreaded decoding
  err = av_dict_set(&opts_, "threads", "auto", 0);

  // Handle failure to set multithreaded decoding
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Open codec
  err = avcodec_open2(codec_ctx_, codec_, &opts_);

  // Handle failure to open codec
  if (err < 0) {
    HandleFFError(err);
    return;
  }

  // Finally allocate a miscellaneous frame for decoding
  frame_ = av_frame_alloc();

  if (frame_ == nullptr) {
    Error(tr("Failed to create AVFrame"));
    return;
  }
}

DecoderFrame FFmpegDecoder::Retrieve(double time)
{
  if (!IsOpen()) {
    return DecoderFrame();
  }

  int64_t timestamp = qRound64(time * av_q2d(av_inv_q(stream_->time_base)));

  // If the timestamp exceeds the duration, return nothing

  // Retrieve a frame

  // If the retrieved frame PTS is greater than the timestamp, seek backwards

  // Keep retrieving frames until the timestamp is reached

  // Return that

  return DecoderFrame(frame_->data, frame_->linesize, pix_fmt_, frame_->width, frame_->height);
}

void FFmpegDecoder::Close()
{
  // Free anything that's allocated
  if (pkt_ != nullptr) {
    av_packet_free(&pkt_);
    pkt_ = nullptr;
  }

  if (frame_ != nullptr) {
    av_frame_free(&frame_);
    frame_ = nullptr;
  }

  if (filter_graph_ != nullptr) {
    avfilter_graph_free(&filter_graph_);
    filter_graph_ = nullptr;
  }

  if (codec_ctx_ != nullptr) {
    avcodec_free_context(&codec_ctx_);
    codec_ctx_ = nullptr;
  }

  if (opts_ != nullptr) {
    av_dict_free(&opts_);
    opts_ = nullptr;
  }

  if (fmt_ctx_ != nullptr) {
    avformat_free_context(fmt_ctx_);
    fmt_ctx_ = nullptr;
  }

  // Free frame cache
  frame_cache_.clear();
}

bool FFmpegDecoder::IsOpen()
{
  // It's assumed that if the fmt_ctx_ is valid, the decoder is open. Even if opening fails after the fmt_ctx_ is
  // allocated, the error handling will run Close() which will free it and set it back to nullptr.

  return (fmt_ctx_ != nullptr);
}

void FFmpegDecoder::HandleFFError(int error_code)
{
  char err[1024];
  av_strerror(error_code, err, 1024);
  Error(err);
}

void FFmpegDecoder::Error(const QString &s)
{
  QString url;

  if (footage_stream_ != nullptr) {
    url = footage_stream_->footage()->url;
  } else {
    url = tr("(unknown file)");
  }

  qCritical() << "Could not open" << url << "-" << s;
  olive::MainWindow->statusBar()->showMessage(tr("Could not open %1 - %2").arg(url, s));
  Close();
}
