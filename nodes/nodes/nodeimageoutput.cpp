#include "nodeimageoutput.h"

NodeImageOutput::NodeImageOutput(Clip *c) :
  Node(c)
{
  input_texture_ = new EffectRow(this, "texture", tr("Texture"), true, false);
  input_texture_->AddAcceptedNodeInput(olive::nodes::kTexture);
  input_texture_->SetValueAt(0, 0);
}

GLuint NodeImageOutput::texture()
{
  return input_texture_->GetValueAt(0).toUInt();
}

QString NodeImageOutput::name()
{
  return tr("Image Output");
}

QString NodeImageOutput::id()
{
  return "org.olivevideoeditor.Olive.imageoutput";
}

QString NodeImageOutput::category()
{
  return tr("Outputs");
}

QString NodeImageOutput::description()
{
  return tr("Used for outputting images outside of the node graph.");
}

EffectType NodeImageOutput::type()
{
  return EFFECT_TYPE_EFFECT;
}

olive::TrackType NodeImageOutput::subtype()
{
  return olive::kTypeVideo;
}

NodePtr NodeImageOutput::Create(Clip *c)
{
  return std::make_shared<NodeImageOutput>(c);
}

EffectRow *NodeImageOutput::texture_input()
{
  return input_texture_;
}
