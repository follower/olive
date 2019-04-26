#ifndef VIDEOTRACK_H
#define VIDEOTRACK_H

#include "track.h"

class VideoTrack : public Track
{
public:
  VideoTrack(Sequence* p);

  virtual void Process(double timecode) override;

  NodeParameter* texture_input();
  NodeParameter* texture_output();

private:
  NodeParameter* texture_input_;
  NodeParameter* texture_output_;
};

#endif // VIDEOTRACK_H
