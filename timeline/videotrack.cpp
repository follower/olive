#include "videotrack.h"


VideoTrack::VideoTrack(Sequence *p) :
  Track(p, olive::kTypeVideo)
{
  texture_input_ = new NodeParameter(this, "tex_in", tr("Input"), true, false);
  texture_output_ = new NodeParameter(this, "tex_in", tr("Output"), true, false);
}

void VideoTrack::Process(double timecode)
{
  GLuint blend_texture = 0;

  // Find out which clip is active
  for (int i=0;i<children_.size();i++) {
    Clip* c = static_cast<Clip*>(children_.at(i).get());

    if (c->IsActiveAt(qRound(timecode))) {
      c->Process(timecode);
      break;
    }
  }

  // TODO Merge textures
  //GLuint base_texture = texture_input_->GetValueAt(0).toUInt();

  // TODO Set merged texture to value
  texture_output_->SetValueAt(0, 0);
}
