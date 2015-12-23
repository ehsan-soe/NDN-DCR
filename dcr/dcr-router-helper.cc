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

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

#include "dcr-router-helper.h"

#include "ns3/ndn-l3-protocol.h"
#include "../ndn-net-device-face.h"
#include "dcr-router.h"
#include "ns3/ndn-name.h"
#include "ns3/ndn-fib.h"

#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/channel-list.h"
#include "ns3/object-factory.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

// #include "boost-graph-ndn-dcr-routing-helper.h"

#include <math.h>

NS_LOG_COMPONENT_DEFINE ("ndn.DcrRouterHelper");

using namespace std;
using namespace boost;

namespace ns3 {
namespace ndn {

using namespace dcr;

void
DcrRouterHelper::Install (Ptr<Node> node)
{
  NS_LOG_LOGIC ("Node: " << node->GetId ());

  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol> ();
  NS_ASSERT_MSG (ndn != 0, "Cannot install DcrRouterHelper before Ndn is installed on a node");

  Ptr<DcrRouter> dr = node->GetObject<DcrRouter> ();
  if (dr != 0)
    {
      NS_LOG_DEBUG ("DcrRouter is already installed: " << dr);
      return; // already installed
    }

  dr = CreateObject<DcrRouter> (node->GetId());
  node->AggregateObject (dr);

  for (uint32_t faceId = 0; faceId < ndn->GetNFaces (); faceId++)
    {
      Ptr<NetDeviceFace> face = DynamicCast<NetDeviceFace> (ndn->GetFace (faceId));
      if (face == 0)
	{
	  NS_LOG_DEBUG ("Skipping non-netdevice face");
	  continue;
	}
      // Install DCR for each face on fib!
        std::string prefix = "/DCR";
        Ptr<Name> name = Create<Name> (boost::lexical_cast<Name> (prefix));


        NS_LOG_LOGIC ("[" << node->GetId () << "]$ route add " << prefix << " via " << *face << " metric " << 0);

        Ptr<Fib>  fib  = node->GetObject<Fib> ();

        NameValue prefixValue;
        prefixValue.DeserializeFromString (prefix, MakeNameChecker ());
        fib->Add (prefixValue.Get (), face, 1);
      /*
      Ptr<NetDevice> nd = face->GetNetDevice ();
      if (nd == 0)
	{
	  NS_LOG_DEBUG ("Not a NetDevice associated with NetDeviceFace");
	  continue;
	}

      Ptr<Channel> ch = nd->GetChannel ();

      if (ch == 0)
	{
	  NS_LOG_DEBUG ("Channel is not associated with NetDevice");
	  continue;
	}
      */
     /* if (ch->GetNDevices () == 2) // e.g., point-to-point channel
	{
	  for (uint32_t deviceId = 0; deviceId < ch->GetNDevices (); deviceId ++)
	    {
	      Ptr<NetDevice> otherSide = ch->GetDevice (deviceId);
	      if (nd == otherSide) continue;

	      Ptr<Node> otherNode = otherSide->GetNode ();
	      NS_ASSERT (otherNode != 0);

	      Ptr<DcrRouter> otherGr = otherNode->GetObject<DcrRouter> ();
	      if (otherGr == 0)
		{
		  Install (otherNode);
		}
	      otherGr = otherNode->GetObject<DcrRouter> ();
	      NS_ASSERT (otherGr != 0);
	      //dr->AddIncidency (face, otherGr);
	    }
	}
      else
	{
	  Ptr<DcrRouter> drChannel = ch->GetObject<DcrRouter> ();
	  //if (drChannel == 0)
	   // {
	    //  Install (ch);
	   // }
	  //drChannel = ch->GetObject<DcrRouter> ();

	  //dr->AddIncidency (face, drChannel);
	}*/
    }
}

void
DcrRouterHelper::Install (Ptr<Channel> channel)
{
  NS_LOG_LOGIC ("Channel: " << channel->GetId ());

  Ptr<DcrRouter> dr = channel->GetObject<DcrRouter> ();
  if (dr != 0)
    return;

  dr = CreateObject<DcrRouter> ();
  channel->AggregateObject (dr);

  for (uint32_t deviceId = 0; deviceId < channel->GetNDevices (); deviceId ++)
    {
      Ptr<NetDevice> dev = channel->GetDevice (deviceId);

      Ptr<Node> node = dev->GetNode ();
      NS_ASSERT (node != 0);

      Ptr<DcrRouter> drOther = node->GetObject<DcrRouter> ();
      if (drOther == 0)
	{
	  Install (node);
	}
      drOther = node->GetObject<DcrRouter> ();
      NS_ASSERT (drOther != 0);

      //dr->AddIncidency (0, drOther);
    }
}

void
DcrRouterHelper::Install (const NodeContainer &nodes)
{
  for (NodeContainer::Iterator node = nodes.Begin ();
       node != nodes.End ();
       node ++)
    {
      NS_LOG_DEBUG ("DcrRouter is installing on: " << (*node)->GetId());
      Install (*node);
     // Ptr<Node> cNode = *node;
    }
}

void
DcrRouterHelper::InstallAll ()
{
  Install (NodeContainer::GetGlobal ());
}


void
DcrRouterHelper::AddOrigin (const std::string &prefix, Ptr<Node> node)
{
  Ptr<DcrRouter> dr = node->GetObject<DcrRouter> ();
  NS_ASSERT_MSG (dr != 0,
		 "DcrRouter is not installed on the node");
  std::cout << "DR: "<< dr << " DR->NT: " << dr->GetNeighbourTable() << std::endl;
  Ptr<Name> name = Create<Name> (boost::lexical_cast<Name> (prefix));
  dr->AddLocalPrefix (name);
}

void
DcrRouterHelper::AddOrigins (const std::string &prefix, const NodeContainer &nodes)
{
  for (NodeContainer::Iterator node = nodes.Begin ();
       node != nodes.End ();
       node++)
    {
      AddOrigin (prefix, *node);
    }
}

void
DcrRouterHelper::AddOrigin (const std::string &prefix, const std::string &nodeName)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  NS_ASSERT_MSG (node != 0, nodeName << "is not a Node");

  AddOrigin (prefix, node);
}

void
DcrRouterHelper::AddOriginsForAll ()
{
  for (NodeList::Iterator node = NodeList::Begin (); node != NodeList::End (); node ++)
    {
      Ptr<DcrRouter> dr = (*node)->GetObject<DcrRouter> ();
      string name = Names::FindName (*node);

      if (dr != 0 && !name.empty ())
        {
          AddOrigin ("/"+name, *node);
        }
    }
}

void
DcrRouterHelper::AddAddTimePrefix (Time time, const std::string &prefix, Ptr<Node> node)
{
  Ptr<DcrRouter> dcrR = node->GetObject<DcrRouter> ();
  NS_ASSERT_MSG (dcrR != 0,
		 "NlsrRouter is not installed on the node");

  Ptr<Name> name = Create<Name> (boost::lexical_cast<Name> (prefix));
  dcrR->AddAddPrefix(time, name);
}

void
DcrRouterHelper::AddRemoveTimePrefix (Time time, const std::string &prefix, Ptr<Node> node)
{
  Ptr<DcrRouter> dcrR = node->GetObject<DcrRouter> ();
  NS_ASSERT_MSG (dcrR != 0,
		 "NlsrRouter is not installed on the node");

  Ptr<Name> name = Create<Name> (boost::lexical_cast<Name> (prefix));
  dcrR->AddRemovePrefix(time, name);
}

void
DcrRouterHelper::PrintRoutingTable(Ptr<Node> node)
{
  Ptr<DcrRouter> dr = node->GetObject<DcrRouter> ();
    NS_ASSERT_MSG (dr != 0,
  		 "DcrRouter is not installed on the node");
    dr->PrintRoutingTable ();
}

void
DcrRouterHelper::PrintRoutingTableAll (Time t)
{
  for (NodeList::Iterator node = NodeList::Begin (); node != NodeList::End (); node ++)
    {
      Ptr<DcrRouter> dr = (*node)->GetObject<DcrRouter> ();
      Simulator::Schedule(t, &ndn::dcr::DcrRouter::PrintRoutingTable, dr);
    }
}

void
DcrRouterHelper::PrintNeighborTable(Ptr<Node> node)
{
  Ptr<DcrRouter> dr = node->GetObject<DcrRouter> ();
    NS_ASSERT_MSG (dr != 0,
  		 "DcrRouter is not installed on the node");
    dr->PrintNeighborTable ();
}

void
DcrRouterHelper::PrintNeighborTableAll (Time t)
{
  for (NodeList::Iterator node = NodeList::Begin (); node != NodeList::End (); node ++)
    {
      Ptr<DcrRouter> dr = (*node)->GetObject<DcrRouter> ();
      Simulator::Schedule(t, &ndn::dcr::DcrRouter::PrintNeighborTable, dr);
    }
}

} // namespace ndn
} // namespace ns3





