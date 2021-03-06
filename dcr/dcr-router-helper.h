/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of California, Santa Cruz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Ehsan Hemmati <ehsan@ce.ucsc.edu>
 */


#ifndef DCR_ROUTING_HELPER_H_
#define DCR_ROUTING_HELPER_H_

#include "ns3/ptr.h"
#include "ns3/nstime.h"

namespace ns3 {

class Node;
class NodeContainer;
class Channel;

namespace ndn {

/**
 * @ingroup ccn-helpers
 * @brief Helper for DcrRouter interface
 */
class DcrRouterHelper
{
public:
  /**
   * @brief Install DcrRouter interface on a node
   *
   * Note that DcrRouter will also be installed on all connected nodes and channels
   *
   * @param node Node to install DcrRouter interface
   */
  void
  Install (Ptr<Node> node);


  /**
   * @brief Install DcrRouter interface on nodes
   *
   * Note that DcrRouter will also be installed on all connected nodes and channels
   *
   * @param nodes NodeContainer to install DcrRouter interface
   */
  void
  Install (const NodeContainer &nodes);

  /**
   * @brief Install DcrRouter interface on all nodes
   */
  void
  InstallAll ();

  /**
   * @brief Add `prefix' as origin on `node'
   * @param prefix Prefix that is originated by node, e.g., node is a producer for this prefix
   * @param node   Pointer to a node
   */
  void
  AddOrigin (const std::string &prefix, Ptr<Node> node);

  /**
   * @brief Add `prefix' as origin on all `nodes'
   * @param prefix Prefix that is originated by nodes
   * @param nodes NodeContainer
   */
  void
  AddOrigins (const std::string &prefix, const NodeContainer &nodes);

  /**
   * @brief Add `prefix' as origin on node `nodeName'
   * @param prefix     Prefix that is originated by node, e.g., node is a producer for this prefix
   * @param nodeName   Name of the node that is associated with Ptr<Node> using ns3::Names
   */
  void
  AddOrigin (const std::string &prefix, const std::string &nodeName);

  /**
   * @brief Add origin to each node based on the node's name (using Names class)
   */
  void
  AddOriginsForAll ();

  void
  AddAddTimePrefix (Time time, const std::string &prefix, Ptr<Node> node);

  void
  AddRemoveTimePrefix (Time time, const std::string &prefix, Ptr<Node> node);

  void
  PrintRoutingTable (Ptr<Node> node);

  void
  PrintRoutingTableAll (Time t);

  void
  PrintNeighborTable (Ptr<Node> node);

  void
  PrintNeighborTableAll (Time t);

private:
  void
  Install (Ptr<Channel> channel);
};

} // namespace ccn
} // namespace ns3




#endif /* DCR_ROUTING_HELPER_H_ */
