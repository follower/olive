#ifndef NODESHADER_H
#define NODESHADER_H

#include "nodes/node.h"

class NodeShader : public SubClipNode {
  Q_OBJECT
public:
  NodeShader(Clip *c,
             const QString& name,
             const QString& id,
             const QString& category,
             const QString& filename);

  virtual QString name() override;
  virtual QString id() override;
  virtual QString category() override;
  virtual QString description() override;
  virtual EffectType subclip_type() override;
  virtual olive::TrackType type() override;
  virtual bool IsCreatable() override;
  virtual NodePtr Create(Node *c) override;

private:
  QString name_;
  QString id_;
  QString category_;
  QString description_;
  QString filename_;
};

#endif // NODESHADER_H
