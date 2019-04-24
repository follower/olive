#include "decoder.h"

Decoder::Decoder() :
  footage_stream_(nullptr)
{

}

Decoder::~Decoder()
{

}

int Decoder::width()
{
  return 0;
}

int Decoder::height()
{
  return 0;
}

void Decoder::SetStream(FootageStream *fs)
{
  footage_stream_ = fs;
}

DecoderFrame::DecoderFrame() :
  data_(nullptr),
  linesize_(nullptr)
{
}

DecoderFrame::DecoderFrame(uint8_t **d, int *ls, olive::PixelFormat pix, int width, int height) :
  data_(d),
  linesize_(ls),
  pix_fmt_(pix),
  width_(width),
  height_(height)
{
}

bool DecoderFrame::IsValid()
{
  return (data_ != nullptr);
}

uint8_t **DecoderFrame::data()
{
  return data_;
}

int *DecoderFrame::linesize()
{
  return linesize_;
}

olive::PixelFormat DecoderFrame::pix_fmt()
{
  return pix_fmt_;
}

int DecoderFrame::width()
{
  return width_;
}

int DecoderFrame::height()
{
  return height_;
}
