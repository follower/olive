#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <QString>

#include "rendering/clipqueue.h"
#include "decoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

/**
 * @brief The FFmpegVideoDecoder class
 *
 * A derivative of Decoder for decoding common video formats through FFmpeg.
 *
 * For optimization, this class uses a frame cache.
 */
class FFmpegDecoder : public Decoder
{
public:
  FFmpegDecoder();

  virtual void Open() override;
  virtual DecoderFrame Retrieve(double time) override;
  virtual void Close() override;
  virtual bool IsOpen() override;

protected:
  void HandleFFError(int error_code);
  void Error(const QString& s);

  ClipQueue frame_cache_;

  AVFormatContext* fmt_ctx_;
  AVCodecContext* codec_ctx_;
  AVStream* stream_;
  AVCodec* codec_;
  AVDictionary* opts_;
  AVFilterGraph* filter_graph_;
  AVFilterContext* buffersrc_ctx_;
  AVFilterContext* buffersink_ctx_;
  AVPacket* pkt_;
  AVFrame* frame_;

  olive::PixelFormat pix_fmt_;
};

#endif // FFMPEGDECODER_H
