#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "nodes/nodeparameter.h"

class LabelWidget : public NodeParameter
{
public:
  LabelWidget(Node* parent, const QString& name, const QString& text);
};

#endif // LABELWIDGET_H
