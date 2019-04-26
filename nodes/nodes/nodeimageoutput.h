#ifndef NODEIMAGEOUTPUT_H
#define NODEIMAGEOUTPUT_H

#include "nodes/node.h"

class NodeImageOutput : public SubClipNode
{
public:
  NodeImageOutput(Clip* c);

  GLuint texture();

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual EffectType subclip_type() override;
  virtual olive::TrackType type() override;
  virtual NodePtr Create(Node *c) override;

  NodeIO* texture_input();

private:
  NodeIO* input_texture_;
};

#endif // NODEIMAGEOUTPUT_H
