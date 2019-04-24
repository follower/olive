#include "fileinput.h"

FileInput::FileInput(Node* parent, const QString& id, const QString& name, bool savable, bool keyframable) :
  EffectRow(parent, id, name, savable, keyframable)
{
  AddField(new FileField(this));

  AddAcceptedNodeInput(olive::nodes::kFile);
}

QString FileInput::GetFileAt(double timecode)
{
  return GetValueAt(timecode).toString();
}
