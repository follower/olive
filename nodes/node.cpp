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

#include "node.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QDir>
#include <QPainter>
#include <QtMath>
#include <QMenu>
#include <QApplication>
#include <QFileDialog>

#include "panels/panels.h"
#include "panels/viewer.h"
#include "ui/viewerwidget.h"
#include "ui/collapsiblewidget.h"
#include "panels/project.h"
#include "undo/undo.h"
#include "timeline/sequence.h"
#include "timeline/clip.h"
#include "panels/timeline.h"
#include "panels/effectcontrols.h"
#include "panels/grapheditor.h"
#include "global/debug.h"
#include "global/path.h"
#include "ui/mainwindow.h"
#include "ui/menu.h"
#include "global/math.h"
#include "global/clipboard.h"
#include "global/config.h"
#include "effects/transition.h"
#include "undo/undostack.h"
#include "rendering/shadergenerators.h"
#include "global/timing.h"
#include "nodes/nodes.h"
#include "effects/effectloaders.h"

QVector<NodePtr> olive::node_library;

Node::Node(Node *parent) :
  QObject(parent),
  enabled_(true),
  expanded_(true),
  pipeline_(this)
{
}

Node::~Node() {
  // Clear graph editor if it's using one of these rows
  if (panel_graph_editor != nullptr) {
    for (int i=0;i<RowCount();i++) {
      if (row(i) == panel_graph_editor->get_row()) {
        panel_graph_editor->set_row(nullptr);
        break;
      }
    }
  }
}

olive::TrackType Node::type()
{
  return olive::kTypeInvalid;
}

void Node::AddRow(NodeIO *row)
{
  row->setParent(this);
  rows.append(row);
}

int Node::IndexOfRow(NodeIO *row)
{
  return rows.indexOf(row);
}

void Node::copy_field_keyframes(NodePtr e) {
  for (int i=0;i<rows.size();i++) {
    NodeIO* row = rows.at(i);
    NodeIO* copy_row = e->rows.at(i);
    copy_row->SetKeyframingInternal(row->IsKeyframing());
    for (int j=0;j<row->FieldCount();j++) {
      // Get field from this (the source) effect
      EffectField* field = row->Field(j);

      // Get field from the destination effect
      EffectField* copy_field = copy_row->Field(j);

      // Copy keyframes between effects
      copy_field->keyframes = field->keyframes;

      // Copy persistet data between effects
      copy_field->persistent_data_ = field->persistent_data_;
    }
  }
}

NodeIO* Node::row(int i) {
  return rows.at(i);
}

int Node::RowCount() {
  return rows.size();
}

EffectGizmo *Node::add_gizmo(int type) {
  EffectGizmo* gizmo = new EffectGizmo(this, type);
  gizmos.append(gizmo);
  return gizmo;
}

EffectGizmo *Node::gizmo(int i) {
  return gizmos.at(i);
}

int Node::gizmo_count() {
  return gizmos.size();
}

QVector<NodeEdgePtr> Node::GetAllEdges()
{
  QVector<NodeEdgePtr> edges;

  for (int i=0;i<RowCount();i++) {
    edges.append(row(i)->edges());
  }

  return edges;
}

void Node::refresh() {}

void Node::FieldChanged() {
  // Update the UI if a field has been modified, but don't bother if this effect is inactive
  if (parent() != nullptr) {
    update_ui(false);
  }
}

void Node::delete_self() {
  olive::undo_stack.push(new EffectDeleteCommand(this));
  update_ui(true);
}

void Node::save_to_file() {
  // save effect settings to file
  QString file = QFileDialog::getSaveFileName(olive::MainWindow,
                                              tr("Save Effect Settings"),
                                              QString(),
                                              tr("Effect XML Settings %1").arg("(*.xml)"));

  // if the user picked a file
  if (!file.isEmpty()) {

    // ensure file ends with .xml extension
    if (!file.endsWith(".xml", Qt::CaseInsensitive)) {
      file.append(".xml");
    }

    QFile file_handle(file);
    if (file_handle.open(QFile::WriteOnly)) {

      file_handle.write(save_to_string());

      file_handle.close();
    } else {
      QMessageBox::critical(olive::MainWindow,
                            tr("Save Settings Failed"),
                            tr("Failed to open \"%1\" for writing.").arg(file),
                            QMessageBox::Ok);
    }
  }
}

void Node::load_from_file() {
  // load effect settings from file
  QString file = QFileDialog::getOpenFileName(olive::MainWindow,
                                              tr("Load Effect Settings"),
                                              QString(),
                                              tr("Effect XML Settings %1").arg("(*.xml)"));

  // if the user picked a file
  if (!file.isEmpty()) {
    QFile file_handle(file);
    if (file_handle.open(QFile::ReadOnly)) {

      olive::undo_stack.push(new SetEffectData(this, file_handle.readAll()));

      file_handle.close();

      update_ui(false);
    } else {
      QMessageBox::critical(olive::MainWindow,
                            tr("Load Settings Failed"),
                            tr("Failed to open \"%1\" for reading.").arg(file),
                            QMessageBox::Ok);
    }
  }
}

QOpenGLContext *Node::ctx()
{
  return QOpenGLContext::currentContext();
}

bool Node::IsEnabled() {
  return enabled_;
}

bool Node::IsExpanded()
{
  return expanded_;
}

void Node::SetExpanded(bool e)
{
  expanded_ = e;
}

void Node::SetPos(const QPointF &pos)
{
  pos_ = pos;
}

void Node::SetEnabled(bool b) {
  enabled_ = b;
  emit EnabledChanged(b);
}

void Node::load(QXmlStreamReader& stream) {
  /*
  int row_count = 0;

  QString tag = stream.name().toString();

  while (!stream.atEnd() && !(stream.name() == tag && stream.isEndElement())) {
    stream.readNext();
    if (stream.name() == "row" && stream.isStartElement()) {
      if (row_count < rows.size()) {
        EffectRow* row = rows.at(row_count);

        while (!stream.atEnd() && !(stream.name() == "row" && stream.isEndElement())) {
          stream.readNext();

          // read field
          if (stream.name() == "field" && stream.isStartElement()) {
            int field_number = -1;

            // match field using ID
            for (int k=0;k<stream.attributes().size();k++) {
              const QXmlStreamAttribute& attr = stream.attributes().at(k);
              if (attr.name() == "id") {
                for (int l=0;l<row->FieldCount();l++) {
                  if (row->Field(l)->id() == attr.value()) {
                    field_number = l;
                    break;
                  }
                }
                break;
              }
            }

            if (field_number > -1) {
              EffectField* field = row->Field(field_number);

              // get current field value
              for (int k=0;k<stream.attributes().size();k++) {
                const QXmlStreamAttribute& attr = stream.attributes().at(k);
                if (attr.name() == "value") {
                  field->persistent_data_ = field->ConvertStringToValue(attr.value().toString());
                  break;
                }
              }

              while (!stream.atEnd() && !(stream.name() == "field" && stream.isEndElement())) {
                stream.readNext();

                // read keyframes
                if (stream.name() == "key" && stream.isStartElement()) {
                  row->SetKeyframingInternal(true);

                  EffectKeyframe key;
                  for (int k=0;k<stream.attributes().size();k++) {
                    const QXmlStreamAttribute& attr = stream.attributes().at(k);
                    if (attr.name() == "value") {
                      key.data = field->ConvertStringToValue(attr.value().toString());
                    } else if (attr.name() == "frame") {
                      key.time = attr.value().toLong();
                    } else if (attr.name() == "type") {
                      key.type = attr.value().toInt();
                    } else if (attr.name() == "prehx") {
                      key.pre_handle_x = attr.value().toDouble();
                    } else if (attr.name() == "prehy") {
                      key.pre_handle_y = attr.value().toDouble();
                    } else if (attr.name() == "posthx") {
                      key.post_handle_x = attr.value().toDouble();
                    } else if (attr.name() == "posthy") {
                      key.post_handle_y = attr.value().toDouble();
                    }
                  }
                  field->keyframes.append(key);
                }
              }

              field->Changed();

            }
          }
        }

      } else {
        qCritical() << "Too many rows for effect" << id << ". Project might be corrupt. (Got" << row_count << ", expected <" << rows.size()-1 << ")";
      }
      row_count++;
    } else if (stream.isStartElement()) {
      custom_load(stream);
    }
  }
  */
}

void Node::custom_load(QXmlStreamReader &) {}

void Node::save(QXmlStreamWriter& stream) {
  /*
  stream.writeAttribute("name", meta->category + "/" + meta->name);
  stream.writeAttribute("enabled", QString::number(IsEnabled()));

  for (int i=0;i<rows.size();i++) {
    EffectRow* row = rows.at(i);
    if (row->IsSavable()) {
      stream.writeStartElement("row"); // row
      for (int j=0;j<row->FieldCount();j++) {
        EffectField* field = row->Field(j);

        if (!field->id().isEmpty()) {
          stream.writeStartElement("field"); // field
          stream.writeAttribute("id", field->id());
          stream.writeAttribute("value", field->ConvertValueToString(field->persistent_data_));
          for (int k=0;k<field->keyframes.size();k++) {
            const EffectKeyframe& key = field->keyframes.at(k);
            stream.writeStartElement("key");
            stream.writeAttribute("value", field->ConvertValueToString(key.data));
            stream.writeAttribute("frame", QString::number(key.time));
            stream.writeAttribute("type", QString::number(key.type));
            stream.writeAttribute("prehx", QString::number(key.pre_handle_x));
            stream.writeAttribute("prehy", QString::number(key.pre_handle_y));
            stream.writeAttribute("posthx", QString::number(key.post_handle_x));
            stream.writeAttribute("posthy", QString::number(key.post_handle_y));
            stream.writeEndElement(); // key
          }
          stream.writeEndElement(); // field
        }
      }
      stream.writeEndElement(); // row
    }
  }
  */
}

void Node::load_from_string(const QByteArray &s) {
  // clear existing keyframe data
  for (int i=0;i<rows.size();i++) {
    NodeIO* row = rows.at(i);
    row->SetKeyframingInternal(false);
    for (int j=0;j<row->FieldCount();j++) {
      EffectField* field = row->Field(j);
      field->keyframes.clear();
    }
  }

  // write settings with xml writer
  QXmlStreamReader stream(s);

  bool found_id = false;

  while (!stream.atEnd()) {
    stream.readNext();

    // find the effect opening tag
    if (stream.name() == "effect" && stream.isStartElement()) {

      // check the name to see if it matches this effect
      const QXmlStreamAttributes& attributes = stream.attributes();
      for (int i=0;i<attributes.size();i++) {
        const QXmlStreamAttribute& attr = attributes.at(i);
        if (attr.name() == "id") {
          load(stream);
          found_id = true;
          break;
        }
      }

      // we've found what we're looking for
      break;
    }
  }

  if (!found_id) {
    QMessageBox::critical(olive::MainWindow,
                          tr("Load Settings Failed"),
                          tr("This settings file doesn't match this effect."),
                          QMessageBox::Ok);
  }
}

QByteArray Node::save_to_string() {
  QByteArray save_data;

  // write settings to string with xml writer
  QXmlStreamWriter stream(&save_data);

  stream.writeStartDocument();

  stream.writeStartElement("effect");

  // pass off to standard saving function
  save(stream);

  stream.writeEndElement(); // effect

  stream.writeEndDocument();

  return save_data;
}

const QPointF &Node::pos()
{
  return pos_;
}

NodePtr Node::copy(Node *c) {
  NodePtr copy = Create(c);
  copy->SetEnabled(IsEnabled());
  copy_field_keyframes(copy);
  return copy;
}

int GetNodeLibraryIndexFromId(const QString& id) {
  for (int i=0;i<olive::node_library.size();i++) {
    if (olive::node_library.at(i)->id() == id) {
      return i;
    }
  }

  return -1;
}

SubClipNode::SubClipNode(Clip *c) :
  Node(c)
{
}

Clip *SubClipNode::GetClipParent()
{
  return static_cast<Clip*>(parent());
}

double SubClipNode::Now()
{
  return playhead_to_clip_seconds(GetClipParent(), GetClipParent()->track()->sequence()->playhead);
}

long SubClipNode::NowInFrames()
{
  return playhead_to_clip_frame(GetClipParent(), GetClipParent()->track()->sequence()->playhead);
}

QString SubClipNode::category()
{
  return QString();
}

bool SubClipNode::IsCreatable()
{
  return true;
}

void SubClipNode::gizmo_draw(double, GLTextureCoords &) {}

void SubClipNode::gizmo_move(EffectGizmo* gizmo, int x_movement, int y_movement, double timecode, bool done) {
  // Loop through each gizmo to find `gizmo`
  for (int i=0;i<gizmos.size();i++) {

    if (gizmos.at(i) == gizmo) {

      // If (!done && gizmo_dragging_actions_.isEmpty()), that means the drag just started and we're going to save
      // the current state of the attach fields' keyframes in KeyframeDataChange objects to make the changes undoable
      // by the user later.
      if (!done && gizmo_dragging_actions_.isEmpty()) {
        if (gizmo->x_field1 != nullptr) {
          gizmo_dragging_actions_.append(new KeyframeDataChange(gizmo->x_field1));
        }
        if (gizmo->y_field1 != nullptr) {
          gizmo_dragging_actions_.append(new KeyframeDataChange(gizmo->y_field1));
        }
        if (gizmo->x_field2 != nullptr) {
          gizmo_dragging_actions_.append(new KeyframeDataChange(gizmo->x_field2));
        }
        if (gizmo->y_field2 != nullptr) {
          gizmo_dragging_actions_.append(new KeyframeDataChange(gizmo->y_field2));
        }
      }

      // Update the field values
      if (gizmo->x_field1 != nullptr) {
        gizmo->x_field1->SetValueAt(timecode,
                                    gizmo->x_field1->GetDoubleAt(timecode) + x_movement*gizmo->x_field_multi1);
      }
      if (gizmo->y_field1 != nullptr) {
        gizmo->y_field1->SetValueAt(timecode,
                                    gizmo->y_field1->GetDoubleAt(timecode) + y_movement*gizmo->y_field_multi1);
      }
      if (gizmo->x_field2 != nullptr) {
        gizmo->x_field2->SetValueAt(timecode,
                                    gizmo->x_field2->GetDoubleAt(timecode) + x_movement*gizmo->x_field_multi2);
      }
      if (gizmo->y_field2 != nullptr) {
        gizmo->y_field2->SetValueAt(timecode,
                                    gizmo->y_field2->GetDoubleAt(timecode) + y_movement*gizmo->y_field_multi2);
      }

      // If (done && !gizmo_dragging_actions_.isEmpty()), that means the drag just ended and we're going to save
      // the new state of the attach fields' keyframes in KeyframeDataChange objects to make the changes undoable
      // by the user later.
      if (done && !gizmo_dragging_actions_.isEmpty()) {

        // Store all the KeyframeDataChange objects into a ComboAction to send to the undo stack (makes them all
        // undoable together rather than having to be undone individually).
        ComboAction* ca = new ComboAction();

        for (int j=0;j<gizmo_dragging_actions_.size();j++) {
          // Set the current state of the keyframes as the "new" keyframes (the old values were set earlier when the
          // KeyframeDataChange object was constructed).
          gizmo_dragging_actions_.at(j)->SetNewKeyframes();

          // Add this KeyframeDataChange object to the ComboAction
          ca->append(gizmo_dragging_actions_.at(j));
        }

        olive::undo_stack.push(ca);

        gizmo_dragging_actions_.clear();
      }
      break;
    }
  }
}

void SubClipNode::gizmo_world_to_screen(const QMatrix4x4& matrix, const QMatrix4x4& projection) {
  for (int i=0;i<gizmos.size();i++) {
    EffectGizmo* g = gizmos.at(i);

    for (int j=0;j<g->get_point_count();j++) {

      // Convert the world point from the gizmo into a screen point relative to the sequence's dimensions

      QVector3D screen_pos = g->world_pos.at(j).project(matrix,
                                                        projection,
                                                        QRect(0,
                                                              0,
                                                              parent_clip->track()->sequence()->width,
                                                              parent_clip->track()->sequence()->height));

      g->screen_pos[j] = QPoint(screen_pos.x(), parent_clip->track()->sequence()->height-screen_pos.y());

    }
  }
}

bool SubClipNode::are_gizmos_enabled() {
  return (gizmos.size() > 0);
}
