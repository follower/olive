#include "decoder.h"

Decoder::Decoder() :
  footage_stream_(nullptr)
{

}

Decoder::~Decoder()
{

}

void Decoder::SetStream(FootageStream *fs)
{
  footage_stream_ = fs;
}
