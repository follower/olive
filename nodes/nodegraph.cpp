#include "nodegraph.h"

#include <QDebug>

NodeGraph::NodeGraph() :
  output_node_(nullptr)
{

}

void NodeGraph::AddNode(NodePtr node)
{
  nodes_.append(node);
}

void NodeGraph::SetOutputNode(NodePtr node)
{
  AddNode(node);
  output_node_ = node.get();
}

Node *NodeGraph::OutputNode()
{
  return output_node_;
}
