#ifndef NODEIMAGEOUTPUT_H
#define NODEIMAGEOUTPUT_H

#include "nodes/node.h"

class NodeImageOutput : public Node
{
public:
  NodeImageOutput(Node* c);

  GLuint texture();

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual NodeSubType subclip_type() override;
  virtual olive::TrackType type() override;
  virtual NodePtr Create(Node *c) override;

  NodeParameter* texture_input();

private:
  NodeParameter* input_texture_;
};

#endif // NODEIMAGEOUTPUT_H
