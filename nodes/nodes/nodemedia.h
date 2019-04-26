#ifndef MEDIANODE_H
#define MEDIANODE_H

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "nodes/node.h"
#include "decoders/decoder.h"
#include "rendering/memorycache.h"

class NodeMedia : public EffectNode
{
public:
  NodeMedia(Clip *c);

  void SetFootageStream(FootageStream* fs);

  virtual void Open() override;
  virtual void Close() override;
  virtual bool IsOpen() override;
  virtual void Process(double time) override;

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual NodeSubType subclip_type() override;
  virtual olive::TrackType type() override;
  virtual NodePtr Create(Node *c) override;

  NodeParameter* matrix_input();
  NodeParameter* texture_output();

private:
  Decoder* decoder_;

  NodeParameter* matrix_input_;
  NodeParameter* texture_output_;

  GLuint texture_;
  bool texture_is_allocated_;
  MemoryCache::MemoryBuffer buffer_;

  QOpenGLVertexArrayObject vao_;
  QOpenGLBuffer vertex_buffer_;
  QOpenGLBuffer texcoord_buffer_;

  FootageStream* footage_stream_;

  QOpenGLShaderProgramPtr shader_;

  GLuint ocio_lut_;
};

#endif // MEDIANODE_H
