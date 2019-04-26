#include "nodeedge.h"

#include <QDebug>

#include "effects/nodeparameter.h"

NodeEdge::NodeEdge(NodeParameter *output, NodeParameter *input) :
  output_(output),
  input_(input)
{
}

NodeParameter *NodeEdge::output()
{
  return output_;
}

NodeParameter *NodeEdge::input()
{
  return input_;
}
