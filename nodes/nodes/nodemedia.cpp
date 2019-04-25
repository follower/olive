#include "nodemedia.h"

#include "rendering/renderfunctions.h"
#include "rendering/shadergenerators.h"

NodeMedia::NodeMedia(Clip* c) :
  Node(c),
  footage_stream_(nullptr),
  texture_is_allocated_(false)
{
  matrix_input_ = new EffectRow(this, "matrix", tr("Matrix"), true, false);
  matrix_input_->AddAcceptedNodeInput(olive::nodes::kMatrix);
  matrix_input_->SetValueAt(0, QMatrix4x4());

  texture_output_ = new EffectRow(this, "texture", tr("Texture"), true, false);
  texture_output_->SetOutputDataType(olive::nodes::kTexture);
  texture_output_->SetValueAt(0, 0);
}

void NodeMedia::SetFootageStream(FootageStream *fs)
{
  footage_stream_ = fs;
}

void NodeMedia::Open()
{
  if (footage_stream_ == nullptr) {
    return;
  }

  // Open decoder
  decoder_->Open();

  // If the decoder opened successfully, we'll set up the destination texture and conversion buffer
  if (decoder_->IsOpen()) {
    QOpenGLFunctions* f = ctx()->functions();

    // create texture object
    f->glGenTextures(1, &texture_);

    f->glBindTexture(GL_TEXTURE_2D, texture_);

    // set texture filtering to bilinear
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping to clamp
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    f->glBindTexture(GL_TEXTURE_2D, 0);

    // Create buffers
    vao_.create();
    vertex_buffer_.create();
    texcoord_buffer_.create();

    // Set default input colorspace
    QString input_cs = footage_stream_->footage()->Colorspace();

    // Try to get a shader based on the input color space to scene linear
    try {
      OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
      OCIO::ConstProcessorRcPtr processor = config->getProcessor(input_cs.toUtf8(),
                                                                 OCIO::ROLE_SCENE_LINEAR);

      shader_ = olive::shader::SetupOCIO(ctx(),
                                         ocio_lut_,
                                         processor,
                                         footage_stream_->footage()->alpha_is_associated);
    } catch (OCIO::Exception& e) {
      qWarning() << e.what();
    }
  }
}

void NodeMedia::Close()
{
  vao_.destroy();
  vertex_buffer_.destroy();
  texcoord_buffer_.destroy();

  decoder_->Close();
  buffer_.Close();

  ctx()->functions()->glDeleteTextures(1, &texture_);
  ctx()->functions()->glDeleteTextures(1, &ocio_lut_);
  texture_is_allocated_ = false;

  shader_ = nullptr;
}

bool NodeMedia::IsOpen()
{
  return decoder_->IsOpen();
}

void NodeMedia::Process(double time)
{
  if (!IsOpen()) {
    return;
  }

  // Set texture to null
  texture_output_->SetValueAt(0, 0);

  DecoderFrame frame = decoder_->Retrieve(time);

  // Check if we retrieved valid data
  if (!frame.IsValid()) {
    return;
  }

  // We did, so now we'll upload the data to OpenGL

  int video_width = frame.width();
  int video_height = frame.height();

  const olive::PixelFormatInfo& pix_fmt_info = olive::pixel_formats.at(frame.pix_fmt());

  QOpenGLFunctions* f = ctx()->functions();

  f->glBindTexture(GL_TEXTURE_2D, texture_);

  f->glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.linesize()[0]/pix_fmt_info.bytes_per_pixel);

  if (texture_is_allocated_) {

    f->glTexSubImage2D(GL_TEXTURE_2D,
                       0,
                       0,
                       0,
                       video_width,
                       video_height,
                       pix_fmt_info.pixel_format,
                       pix_fmt_info.pixel_type,
                       frame.data()[0]
        );

  } else {

    // the raw frame size may differ from the one we're using (e.g. a lower resolution proxy), so we make sure
    // the texture is using the correct dimensions, but then treat it as if it's the original resolution in the
    // composition
    f->glTexImage2D(
          GL_TEXTURE_2D,
          0,
          pix_fmt_info.internal_format,
          video_width,
          video_height,
          0,
          pix_fmt_info.pixel_format,
          pix_fmt_info.pixel_type,
          frame.data()[0]
        );

  }

  f->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  // Using the MemoryCache, we'll convert this frame to Olive's current internal format according to the matrix
  // provided. We'll also convert to scene linear here.

  buffer_.Buffer()->BindBuffer();

  // set texture filter to bilinear
  olive::rendering::PrepareToDraw(f);

  // draw clip on screen according to gl coordinates
  shader_->bind();

  // set default coordinates based on the sequence, with 0 in the direct center
  Sequence* s = parent_clip->track()->sequence();
  int half_width = s->width/2;
  int half_height = s->height/2;
  QMatrix4x4 projection;
  projection.ortho(-half_width, half_width, -half_height, half_height, -1, 1);

  shader_->setUniformValue("mvp_matrix", projection * matrix_input_->GetValueAt(0).value<QMatrix4x4>());
  shader_->setUniformValue("texture", 0);
  shader_->setUniformValue("opacity", 1.0f);

  GLTextureCoords coords;
  coords.vertex_top_left = QVector3D(-video_width/2, -video_height/2, 0.0f);
  coords.vertex_top_right = QVector3D(video_width/2, -video_height/2, 0.0f);
  coords.vertex_bottom_left = QVector3D(-video_width/2, video_height/2, 0.0f);
  coords.vertex_bottom_right = QVector3D(video_width/2, video_height/2, 0.0f);
  coords.texture_top_left = QVector2D(0.0f, 0.0f);
  coords.texture_top_right = QVector2D(1.0f, 0.0f);
  coords.texture_bottom_left = QVector2D(0.0f, 1.0f);
  coords.texture_bottom_right = QVector2D(1.0f, 1.0f);

  GLfloat vertices[] = {
    coords.vertex_top_left.x(), coords.vertex_top_left.y(), 0.0f,
    coords.vertex_top_right.x(), coords.vertex_top_right.y(), 0.0f,
    coords.vertex_bottom_right.x(), coords.vertex_bottom_right.y(), 0.0f,

    coords.vertex_top_left.x(), coords.vertex_top_left.y(), 0.0f,
    coords.vertex_bottom_left.x(), coords.vertex_bottom_left.y(), 0.0f,
    coords.vertex_bottom_right.x(), coords.vertex_bottom_right.y(), 0.0f,
  };

  GLfloat texcoords[] = {
    coords.texture_top_left.x(), coords.texture_top_left.y(),
    coords.texture_top_right.x(), coords.texture_top_right.y(),
    coords.texture_bottom_right.x(), coords.texture_bottom_right.y(),

    coords.texture_top_left.x(), coords.texture_top_left.y(),
    coords.texture_bottom_left.x(), coords.texture_bottom_left.y(),
    coords.texture_bottom_right.x(), coords.texture_bottom_right.y(),
  };

  vao_.bind();

  vertex_buffer_.bind();
  vertex_buffer_.allocate(vertices, 18 * sizeof(GLfloat));
  vertex_buffer_.release();

  texcoord_buffer_.bind();
  texcoord_buffer_.allocate(texcoords, 12 * sizeof(GLfloat));
  texcoord_buffer_.release();

  GLuint vertex_location = shader_->attributeLocation("a_position");
  vertex_buffer_.bind();
  f->glEnableVertexAttribArray(vertex_location);
  f->glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
  vertex_buffer_.release();

  GLuint tex_location = shader_->attributeLocation("a_texcoord");
  texcoord_buffer_.bind();
  f->glEnableVertexAttribArray(tex_location);
  f->glVertexAttribPointer(tex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
  texcoord_buffer_.release();

  f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  f->glDrawArrays(GL_TRIANGLES, 0, 6);

  shader_->release();

  // release final clip texture
  f->glBindTexture(GL_TEXTURE_2D, 0);

  buffer_.Buffer()->ReleaseBuffer();

  texture_output_->SetValueAt(0, buffer_.Buffer()->texture());
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

EffectRow *NodeMedia::matrix_input()
{
  return matrix_input_;
}

EffectRow *NodeMedia::texture_output()
{
  return texture_output_;
}
