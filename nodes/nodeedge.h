#ifndef NODEEDGE_H
#define NODEEDGE_H

#include <memory>

class NodeParameter;

class NodeEdge;
using NodeEdgePtr = std::shared_ptr<NodeEdge>;

class NodeEdge
{
public:
  /**
   * @brief NodeEdge Constructor
   *
   * Creates a node edge and stores its two connections.
   *
   * This should not be used directly. Use the static function EffectRow::ConnectEdge instead.
   *
   * @param output
   *
   * Output/from EffectRow that this edge "starts" at
   *
   * @param input
   *
   * Input/to EffectRow that this edge "ends" at
   */
  NodeEdge(NodeParameter* output, NodeParameter* input);

  NodeParameter* output();
  NodeParameter* input();

private:
  NodeParameter* output_;
  NodeParameter* input_;
};

#endif // NODEEDGE_H
