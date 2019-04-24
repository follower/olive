#ifndef FFMPEGVIDEODECODER_H
#define FFMPEGVIDEODECODER_H

#include "ffmpegdecoder.h"

class FFmpegVideoDecoder : public FFmpegDecoder
{
public:
  virtual void Open() override;

  virtual int width() override;
  virtual int height() override;
};

#endif // FFMPEGVIDEODECODER_H
