/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019  Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef EFFECT_H
#define EFFECT_H

#include <memory>
#include <QObject>
#include <QString>
#include <QVector>
#include <QColor>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMutex>
#include <QThread>
#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <random>

#include "timeline/tracktypes.h"
#include "rendering/qopenglshaderprogramptr.h"
#include "inputs.h"
#include "effects/effectgizmo.h"
#include "nodeparameter.h"

class EffectGizmo;
class KeyframeDataChange;

enum NodeType {
  kTransformEffect,
  kTextInput,
  kSolidInput,
  kNoiseInput,
  kVolumeEffect,
  kPanEffect,
  kToneInput,
  kShakeEffect,
  kTimecodeEffect,
  kMaskEffect,
  kFillLeftRightEffect,
  kVstEffect,
  kCornerPinEffect,
  kRichTextInput,
  kMediaInput,
  kShaderEffect,
  kImageOutput,
  kCrossDissolveTransition,
  kLinearFadeTransition,
  kExponentialFadeTransition,
  kLogarithmicFadeTransition,
  kVideoTrackNode,
  kAudioTrackNode,
  kVideoClipNode,
  kAudioClipNode,
  kSequenceNode,
  kInvalidNode
};

double log_volume(double linear);

enum NodeSubType {
  EFFECT_TYPE_INVALID,
  EFFECT_TYPE_EFFECT,
  EFFECT_TYPE_TRANSITION
};

enum EffectKeyframeType {
  EFFECT_KEYFRAME_LINEAR,
  EFFECT_KEYFRAME_BEZIER,
  EFFECT_KEYFRAME_HOLD
};

struct GLTextureCoords {
  QMatrix4x4 matrix;

  QVector3D vertex_top_left;
  QVector3D vertex_top_right;
  QVector3D vertex_bottom_left;
  QVector3D vertex_bottom_right;

  QVector2D texture_top_left;
  QVector2D texture_top_right;
  QVector2D texture_bottom_left;
  QVector2D texture_bottom_right;

  float opacity;
};

class Node;
using NodePtr = std::shared_ptr<Node>;

class Sequence;

class Node : public QObject {
  Q_OBJECT
public:
  Node(Node *parent);
  virtual ~Node();

  virtual QString name() = 0;
  virtual QString id() = 0;
  virtual NodePtr Create(Node *c) = 0;
  virtual NodeSubType subclip_type();
  virtual olive::TrackType type();
  virtual QString category();
  virtual QString description();

  virtual void Open();
  virtual void Close();
  virtual bool IsOpen();
  virtual void Process(double time);

  bool IsStatic();
  void SetStatic(bool s);

  virtual void AddChild(NodePtr child);
  virtual Node *AddChild(NodeType type);

  void AddRow(NodeParameter* row);
  int IndexOfRow(NodeParameter* row);
  NodeParameter* row(int i);
  int RowCount();

  QVector<NodeEdgePtr> GetAllEdges();

  bool IsEnabled();
  bool IsExpanded();

  virtual void refresh();

  virtual NodePtr copy(Node* c) = 0;
  void copy_field_keyframes(Node* e);

  virtual void load(QXmlStreamReader& stream);
  virtual void custom_load(QXmlStreamReader& stream);
  virtual void save(QXmlStreamWriter& stream);

  void load_from_string(const QByteArray &s);
  QByteArray save_to_string();

  const QPointF& pos();

  template <typename T>
  T randomNumber()
  {
    static std::random_device device;
    static std::mt19937 generator(device());
    static std::uniform_int_distribution<> distribution(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    return distribution(generator);
  }

  template <typename T>
  T randomFloat()
  {
    static std::random_device device;
    static std::mt19937 generator(device());
    static std::uniform_int_distribution<> distribution(-1.0, 1.0);
    return distribution(generator);
  }

public slots:
  void FieldChanged();
  void SetEnabled(bool b);
  void SetExpanded(bool e);
  void SetPos(const QPointF& pos);
signals:
  void EnabledChanged(bool);
  void ChildCountChanged();
private slots:
  void delete_self();
  void save_to_file();
  void load_from_file();
protected:
  QOpenGLContext* ctx();
  Node* parent_;
  QVector<NodePtr> children_;

private:
  virtual Sequence* GetSequence();

  QVector<NodeParameter*> rows;

  bool enabled_;
  bool expanded_;
  bool static_;

  QPointF pos_;
};

class EffectNode : public Node {
public:
  EffectNode(Node* c);

  EffectGizmo* add_gizmo(int subclip_type);
  EffectGizmo* gizmo(int i);
  int gizmo_count();
  virtual void gizmo_draw(double timecode, GLTextureCoords& coords);
  void gizmo_move(EffectGizmo* sender, int x_movement, int y_movement, double timecode, bool done);
  void gizmo_world_to_screen(const QMatrix4x4 &matrix, const QMatrix4x4 &projection);
  bool are_gizmos_enabled();

  /**
   * @brief Get the current clip/media time
   *
   * A convenience function that can be plugged into GetValueAt() to get the value wherever the appropriate Sequence's
   * playhead it.
   *
   * @return
   *
   * Current clip/media time in seconds.
   */
  double Now();

  /**
   * @brief Retrieve the current clip as a frame number
   *
   * Same as Now() but retrieves the value as a frame number (in the appropriate Sequence's frame rate) instead of
   * seconds.
   *
   * @return
   *
   * The current clip time in frames
   */
  long NowInFrames();

private:
  QVector<EffectGizmo*> gizmos;
  QVector<KeyframeDataChange*> gizmo_dragging_actions_;
};

namespace olive {
  extern QVector<NodePtr> node_library;
}

#endif // EFFECT_H
