#ifndef NODESHADER_H
#define NODESHADER_H

#include "nodes/node.h"

class NodeShader : public EffectNode {
  Q_OBJECT
public:
  NodeShader(Node *c,
             const QString& name,
             const QString& id,
             const QString& category,
             const QString& filename);

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual NodeSubType subclip_type() override;
  virtual olive::TrackType type() override;
  virtual NodePtr Create(Node *c) override;

private:
  QString name_;
  QString id_;
  QString category_;
  QString description_;
  QString filename_;

  QString shader_vert_path_;
  QString shader_frag_path_;
  QString shader_function_name_;
  int iterations_;
};

#endif // NODESHADER_H
