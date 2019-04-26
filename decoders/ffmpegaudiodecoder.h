#ifndef FFMPEGAUDIODECODER_H
#define FFMPEGAUDIODECODER_H

#include "ffmpegdecoder.h"

class FFmpegAudioDecoder : public FFmpegDecoder
{
public:
  FFmpegAudioDecoder();

  virtual void Open() override;
};

#endif // FFMPEGAUDIODECODER_H
