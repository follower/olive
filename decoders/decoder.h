#ifndef DECODER_H
#define DECODER_H

#include <QObject>

#include "project/media.h"
#include "rendering/pixelformats.h"

/**
 * @brief The Decoder class
 *
 * A decoder prepares a video frame or audio samples from an external media file for use within Olive.
 *
 * This class is abstract and does nothing. It must be derived to create a functional decoder. A decoder has the
 * following responsibilities:
 *
 * 1. Import packet from external file.
 * 2. Decode/decompress packet into frame/samples if necessary.
 * 3. Upload frame data to the GPU as an OpenGL texture.
 * 3. Convert raw uncompressed frame to Olive's internal format (defined in Config::playback_bit_depth, defaults to
 *   RGBA16F for playback) if necessary.
 * 4. Convert input's color space to scene linear.
 */
class Decoder : public QObject
{
public:
  Decoder();
  virtual ~Decoder();

  virtual void Open() = 0;
  virtual void Close() = 0;
  virtual uint8_t** Retrieve(double time) = 0;
  virtual bool IsOpen() = 0;

  void SetStream(FootageStream* fs);

protected:
  FootageStream* footage_stream_;

private:
};

#endif // DECODER_H
