#ifndef MEDIANODE_H
#define MEDIANODE_H

#include "nodes/node.h"
#include "decoders/decoder.h"
#include "rendering/framebufferobject.h"

class NodeMedia : public Node
{
public:
  NodeMedia(Clip *c);

  virtual void Open() override;
  virtual void Close() override;
  virtual bool IsOpen() override;
  virtual void Process(double time) override;

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual EffectType type() override;
  virtual olive::TrackType subtype() override;
  virtual NodePtr Create(Clip *c) override;

private:
  Decoder* decoder_;

  EffectRow* matrix_input_;
  EffectRow* texture_output_;

  GLuint texture_;
  FramebufferObject buffer_;
};

#endif // MEDIANODE_H
