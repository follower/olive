#include "labelwidget.h"

LabelWidget::LabelWidget(Node *parent, const QString &name, const QString &text) :
  NodeParameter(parent, nullptr, name, false, false)
{
  AddField(new LabelField(this, text));
}
