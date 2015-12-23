/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2015 UCSC
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
 * Author:  Ehsan Hemmati <ehsan@ce.ucsc.edu>
 */

#include "dcr-router.h"
#include "ns3/ndn-l3-protocol.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-name.h"
#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "dcr-routing.h"

using namespace boost;

namespace ns3 {
namespace ndn {
namespace dcr {

uint32_t DcrRouter::m_idCounter = 0;

NS_OBJECT_ENSURE_REGISTERED (DcrRouter);

TypeId
DcrRouter::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::dcr::DcrRouter")
    .SetGroupName ("Ccn")
    .SetParent<Object> ()
    .AddConstructor<DcrRouter> ()
    .AddAttribute ("Msg Length", "less than min MTU of the network",
		   ns3::UintegerValue (1400),
                   MakeUintegerAccessor(&DcrRouter::m_msgSize),
                   MakeUintegerChecker<uint16_t>())

  ;
  return tid;
}

DcrRouter::DcrRouter ()
{
  m_id = m_idCounter;
  m_idCounter ++;
  m_application = 0;
  m_dcrApp = 0;
}

DcrRouter::DcrRouter (uint32_t nodeId)
{
  m_id = m_idCounter;
  m_idCounter ++;
  m_nodeId = nodeId;
  m_rt = Create<RoutingTable> (m_nodeId);
  m_nt = Create<NeighborTable> (m_nodeId, m_rt);
  //m_nt->AddEntry(m_nodeId); //Create an entry for the node Itself! It conna be neighbor #0 !
  m_application = 0;
  m_dcrApp = 0;
}

void
DcrRouter::NotifyNewAggregate ()
{
  if (m_ndn == 0)
    {
      m_ndn = GetObject<L3Protocol> ();
    }
  Object::NotifyNewAggregate ();
}

uint32_t
DcrRouter::GetId () const
{
  return m_id;
}

Ptr<L3Protocol>
DcrRouter::GetL3Protocol () const
{
  return m_ndn;
}

void
DcrRouter::AddLocalPrefix (Ptr< Name > prefix)
{
  Ptr<nt::Entry> entry = m_nt->GetEntry (m_nodeId);
  entry->AddInfo(prefix, 0, m_nodeId);
  Ptr<Packet> updateMsg = Create<Packet> ();
  uint16_t flag = dcr::DcrRouter::UPDATE;
  Ptr<dcr::RouteInfo> info = Create<dcr::RouteInfo> (ConstCast<const Name> (prefix), 0, m_nodeId, flag);
  wire::ndnSIM::dcr::RouteInfo rInfo (info);
  updateMsg->AddHeader(rInfo);
  wire::ndnSIM::dcr::UpdateMsg uMsg;
  uMsg.AddHeader(updateMsg, 0, m_nodeId, 1, 0);
  ProcessUpdateMsg(updateMsg, true);
}


void
DcrRouter::RemoveLocalPrefix (Ptr< Name > prefix)
{
  Ptr<Packet> updateMsg = Create<Packet> ();
  uint16_t flag = dcr::DcrRouter::UPDATE;
  Ptr<dcr::RouteInfo> info = Create<dcr::RouteInfo> (ConstCast<const Name> (prefix), -1, m_nodeId, flag);
  wire::ndnSIM::dcr::RouteInfo rInfo (info);
  updateMsg->AddHeader(rInfo);
  wire::ndnSIM::dcr::UpdateMsg uMsg;
  uMsg.AddHeader(updateMsg, 0, m_nodeId, 1, 0);
  ProcessUpdateMsg(updateMsg, true);

}

void
DcrRouter::AddAddPrefix (Time time, Ptr< Name > prefix)
{
  Simulator::Schedule(time, &DcrRouter::AddLocalPrefix, this, prefix);

}

void
DcrRouter::AddRemovePrefix (Time time, Ptr< Name > prefix)
{
  Simulator::Schedule(time, &DcrRouter::RemoveLocalPrefix, this, prefix);
}

void
DcrRouter::ProcessUpdateMsg(Ptr<const Packet> msg, bool internal /* = false*/)
{
  PrintUpdateMsg (msg);
  Ptr<Packet> p = msg->Copy();
  wire::ndnSIM::dcr::UpdateMsg u;
  p->RemoveHeader (u);

  uint32_t neighborNodeId = u.GetMsg()->GetNodeId();
  Ptr<nt::Entry> ntEntry = m_nt->GetEntry (neighborNodeId);

  if (!internal) ntEntry->RefreshEntry();
  for (uint32_t i = 0; i < u.GetMsg()->GetNum(); i++)
    {
      Ptr<dcr::RouteInfo> ri = wire::ndnSIM::dcr::RouteInfo::GetRouteInfo(p);
      Ptr<const Name> prefix = ri->GetName();
      Ptr<nt::EntryInfo> ntEInfo = ntEntry->GetInfo(prefix);
      if (!ntEInfo)
	ntEInfo = ntEntry->AddInfo(prefix, ri->GetDist(), ri->GetAnchor());
      else
	ntEInfo->UpdateInfo(ri->GetDist(), ri->GetAnchor());
      Ptr<rt::Entry> rtEntry = m_rt->GetEntry(prefix);
      uint32_t neighborIndex = ntEntry->GetNeighborIndex();
      //std::cout << "Node: " << m_nodeId << " Recieved Flag: " << ri->GetFlag() << std::endl;
      //if (ri->GetFlag() & QUERY) rtEntry->SetSendReply(neighborIndex);
      if ((ri->GetFlag() & REPLY) or (ri->GetFlag() & UPDATE)) rtEntry->ResetQuerySent(neighborIndex); //TODO check the update one

      switch (rtEntry->GetMode())
      {
	case rt::ACTIVE:
	  {
	    if (ri->GetFlag() & REPLY)
		if (rtEntry->LastReply())
	      {
		rtEntry->SetMode(rt::PASSIVE);
		rtEntry->setFDist(-1);
		rtEntry->UpdateEntry();
		rtEntry->SatisfyQueries();
	      }
	    if (ri->GetFlag() & QUERY) rtEntry->SetSendReply(neighborIndex);
	    break;
	  }
	case rt::PASSIVE:
	  {
	    if (rtEntry->UpdateEntry(neighborNodeId, ri->GetDist(), ntEntry->GetLinkCost(), ri->GetAnchor()))
	      {
		Ptr<DcrApp> app = DynamicCast <DcrApp> (m_application);
		if (m_dcrApp) m_dcrApp->ScheduleTriggerUpdate();
		if (ri->GetFlag() & QUERY) rtEntry->SetSendReply(neighborIndex);
	      }
	    else
	      {
		rtEntry->SetMode(rt::ACTIVE);
		rtEntry->SetSendQuery();
		Ptr<DcrApp> app = DynamicCast <DcrApp> (m_application);
		if (app) app->ScheduleTriggerUpdate();
		if (ri->GetFlag() & QUERY) rtEntry->SetQueryRcvd(neighborIndex);

	      }
	  }
      }

    }
}

void
DcrRouter::CheckDeadNeighbors(Time interval)
{
  std::list<uint32_t> deadNeighbors = m_nt->GetDeadNeighbors(interval);
  while (!deadNeighbors.empty())
    {
      uint32_t deadNeighborId = deadNeighbors.back ();
      deadNeighbors.pop_back ();
      Simulator::ScheduleNow (&DcrRouter::UpdateDeadNeighbor, this, deadNeighborId);
    }
  Simulator::Schedule(interval, &DcrRouter::CheckDeadNeighbors, this, interval);
}

void
DcrRouter::UpdateDeadNeighbor(uint32_t deadNeighbor)
{
  Ptr<Packet> updateMsg = m_nt->GetEntry(deadNeighbor)->GetDeadUpdateMsg();
  ProcessUpdateMsg(updateMsg, true);
}

void
DcrRouter::SetApplication(Ptr<Application> application)
{
  m_application = application;
  m_dcrApp = DynamicCast<DcrApp> (application);
}

void
DcrRouter::PrintUpdateMsg(Ptr<const Packet> msg) const
{
  Ptr<Packet> p = msg->Copy();
  wire::ndnSIM::dcr::UpdateMsg u;
  p->RemoveHeader (u);
  std::cout << "At Node: " << m_nodeId << "  " ;
  u.Print(std::cout);
  for (uint32_t i = 0; i < u.GetMsg()->GetNum(); i++)
      {
	Ptr<dcr::RouteInfo> ri = wire::ndnSIM::dcr::RouteInfo::GetRouteInfo(p);
	std::cout << " \t ";
	ri->Print(std::cout);
      }
}


} //dcr
} //ndn namespace
} //ns3 namespace
