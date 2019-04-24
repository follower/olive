#include "nodemedia.h"

NodeMedia::NodeMedia(Clip* c) :
  Node(c)
{
  matrix_input_ = new EffectRow(this, "matrix", tr("Matrix"), true, false);
  matrix_input_->AddAcceptedNodeInput(olive::nodes::kMatrix);

  texture_output_ = new EffectRow(this, "texture", tr("Texture"), true, false);
  texture_output_->SetOutputDataType(olive::nodes::kTexture);
}

void NodeMedia::Open()
{
  // Open decoder
  decoder_->Open();

  // If the decoder opened successfully, we'll set up the destination texture and conversion buffer
  if (decoder_->IsOpen()) {

  }
}

void NodeMedia::Process(double time)
{
  if (!IsOpen()) {

  }

  // Set texture to null
  texture_output_->SetValueAt(0, 0);

  if (!decoder_->IsOpen()) {

  }

  uint8_t** data = decoder_->Retrieve(Now());

  if (data == nullptr) {
    return;
  }


}

QString NodeMedia::name()
{
  return tr("Media");
}

QString NodeMedia::id()
{
  return "org.olivevideoeditor.Olive.media";
}

QString NodeMedia::category()
{
  return tr("Inputs");
}

QString NodeMedia::description()
{
  return tr("Retrieve frames from a media source.");
}

EffectType NodeMedia::type()
{
  return EFFECT_TYPE_EFFECT;
}

olive::TrackType NodeMedia::subtype()
{
  return olive::kTypeVideo;
}

NodePtr NodeMedia::Create(Clip *c)
{
  return std::make_shared<NodeMedia>(c);
}
