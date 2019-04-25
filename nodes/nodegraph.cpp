#include "nodegraph.h"

#include <QDebug>

NodeGraph::NodeGraph(Clip *parent) :
  output_node_(nullptr),
  parent_(parent)
{

}

void NodeGraph::AddNode(NodePtr node)
{
  nodes_.append(node);
}

Node *NodeGraph::AddNode(NodeType type)
{
  NodePtr n = olive::node_library[type]->Create(parent_);
  AddNode(n);
  return n.get();
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
