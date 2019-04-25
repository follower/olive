#ifndef NODEIMAGEOUTPUT_H
#define NODEIMAGEOUTPUT_H

#include "nodes/node.h"

class NodeImageOutput : public Node
{
public:
  NodeImageOutput(Clip* c);

  GLuint texture();

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual EffectType type() override;
  virtual olive::TrackType subtype() override;
  virtual NodePtr Create(Clip *c) override;

  EffectRow* texture_input();

private:
  EffectRow* input_texture_;
};

#endif // NODEIMAGEOUTPUT_H
