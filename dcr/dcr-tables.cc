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

#include "dcr-tables.h"
#include "ns3/log.h"
#include "dcr-msg.h"
#include "dcr-msg-wire.h"
#include "ns3/packet.h"
#include "dcr-router.h"
#include <boost/foreach.hpp>


NS_LOG_COMPONENT_DEFINE ("ndn.DcrTables");

namespace ns3 {
namespace ndn {

namespace dcr{
NS_OBJECT_ENSURE_REGISTERED (NeighborTable);
NS_OBJECT_ENSURE_REGISTERED (RoutingTable);

/*
 * NT Entry Info Class Implementation
 */
bool
nt::EntryInfo::UpdateInfo(const uint8_t &dist, const uint32_t &anchor)
{
  bool result = false;
  if (m_dist != dist){
	  m_dist = dist;
	  result = true;
  }
  if (m_anchor != anchor){
	  m_anchor = anchor;
	  result = true;
  }
  return result;
}

void
nt::EntryInfo::Print()
{
  std::cout << "nt::EntryInfo::Print" << std::endl;
  //TODO Delete this func
}

/*
 * NT Entry Class implimentation
 */

nt::Entry::Entry(uint32_t neighborNodeId, uint32_t neighborIndex, Ptr<RoutingTable> routingTable, uint8_t linkCost)
{
  m_neighborNodeId = neighborNodeId;
  m_neighborIndex = neighborIndex;
  m_routingTable = routingTable;
  m_lastUpdate = Simulator::Now();
  m_linkCost = linkCost;
}

/*
nt::Entry::Entry(uint32_t neighborId, Ptr<const Name> &prefix, const uint8_t &dist, const uint32_t &anchor)
{
	m_neighborId = neighborId;
	EntryInfo info = Create<EntryInfo> (dist, anchor);
	//TODO Update RT too!
	m_entry [*prefix] = info;
	m_rtEntry->
}

nt::Entry::Entry(uint32_t neighborId, Ptr<const Name> &prefix, Ptr<EntryInfo> info)
{
	m_neighborId = neighborId;
	m_entry[*prefix] = info;
	//TODO Update RT too
}
*/

Ptr<nt::EntryInfo>
nt::Entry::UpdateInfo(Ptr<const Name> prefix, uint8_t dist, uint32_t anchor){
  Iterator eit = m_entry.find(*prefix);
  if (eit == m_entry.end()){
	  Ptr<nt::EntryInfo> info = Create<nt::EntryInfo> (m_neighborNodeId, dist, anchor, &m_linkCost);
	  //m_entry [*prefix] = info;
	  std::pair<Iterator, bool> ret = m_entry.insert(std::pair<Name, Ptr<EntryInfo> > (*prefix, info));
	  NS_ASSERT_MSG (ret.second , "Never should be the case!");
	  m_routingTable->GetEntry(prefix)->SetNeighborInfo(m_neighborIndex, info);
	  return info;
  }
  eit->second->UpdateInfo(dist, anchor);
  return eit->second;
}

Ptr<nt::EntryInfo>
nt::Entry::AddInfo(Ptr<const Name> prefix, uint8_t dist, uint32_t anchor){
	Ptr<nt::EntryInfo> info = Create<nt::EntryInfo> (m_neighborNodeId, dist, anchor, &m_linkCost);
	std::pair<Iterator, bool> ret = m_entry.insert(std::pair<Name, Ptr<EntryInfo> > (*prefix, info));
	NS_ASSERT_MSG (ret.second , "Never should be the case!");
	m_routingTable->GetEntry(prefix)->SetNeighborInfo(m_neighborIndex, info);
	return info;
}

Ptr<nt::EntryInfo>
nt::Entry::GetInfo(Ptr<const Name> &prefix){
  Iterator eit = m_entry.find(*prefix);
  if (eit == m_entry.end())
    {
      return 0;
	  Ptr<nt::EntryInfo> info = Create<nt::EntryInfo> (m_neighborNodeId);
	  //m_entry [*prefix] = info;
	  std::pair<Iterator, bool> ret = m_entry.insert(std::pair<Name, Ptr<EntryInfo> > (*prefix, info));
	  NS_ASSERT_MSG (ret.second , "Never should be the case!");
	  //TODO Update RT too
	  m_routingTable->GetEntry(prefix)->SetNeighborInfo(m_neighborIndex, info);
	  return info;
  }
  return eit->second;
}

void
nt::Entry::RefreshEntry()
{
  m_lastUpdate = Simulator::Now();
  if (IsDead ())
    {
      ResetDead (); //m_dead = false;
      SetSendAll (true);
    }
}

void
nt::Entry::ResetDead()
{
  m_dead = false;
  m_linkCost = 1;
}

Ptr<Packet>
nt::Entry::GetDeadUpdateMsg()
{
  Ptr<Packet> updateMsg = Create<Packet> ();
  uint16_t infoCount = 0;
  for (Iterator it = m_entry.begin(); it != m_entry.end(); it++)
    {
      uint16_t flag = dcr::DcrRouter::UPDATE;
      Ptr<Name> prefix = Create <Name> (it->first);
      Ptr<dcr::RouteInfo> info = Create<dcr::RouteInfo> (
	  ConstCast<const Name> (prefix), -1, it->second->GetAnchor(), flag); // if we set it as it is it will not trigger any updates!
      wire::ndnSIM::dcr::RouteInfo rInfo (info);
      updateMsg->AddHeader(rInfo);
      infoCount++;
    }
  if (!infoCount) return 0;
  wire::ndnSIM::dcr::UpdateMsg uMsg;
  uMsg.AddHeader(updateMsg, 0, m_neighborNodeId, infoCount, 0);
  return updateMsg;
}

void
nt::Entry::Clean(){

	m_entry.clear();
}

void
nt::Entry::Print(std::ostream &os)
{
  for (Iterator it = m_entry.begin(); it != m_entry.end(); it++){
    os << "\t \t Name: " << it->first << " Anchor: " << it->second->GetAnchor () <<
	" Dist: " << static_cast<uint16_t> (it->second->GetDist ()) << " Link Cost: " << static_cast<uint16_t> (it->second->GetLinkCost ()) << " Neighbor: " << it->second->GetNeighborNodeId () << std::endl;
  }
}

/*
 * Neighbor Table Class
 */

NeighborTable::NeighborTable(uint32_t nodeId, Ptr<RoutingTable> routingTable)
  : m_nodeId(nodeId)
  , m_routingTable (routingTable)
  , m_neighborCount (0)
{
  AddEntry(m_nodeId);
};

TypeId
NeighborTable::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::dcr::NeighborTable") // cheating ns3 object system
    .SetParent<Object> ()
    .SetGroupName ("Ndn")
    .AddConstructor <NeighborTable> ()
  ;
  return tid;
}

void
NeighborTable::CleanTable()
{
  for (Iterator it = m_nt.begin(); it != m_nt.end(); it++)
  {
	  it->second->Clean();
  }
  m_nt.clear();
}

bool
NeighborTable::IsDead(uint32_t neighborIndex)
{
  uint32_t neighborId = m_neighborIdMap.at (neighborIndex);
  return m_nt.find(neighborId)->second->IsDead();
}

Ptr<nt::Entry>
NeighborTable::AddEntry(uint32_t neighborNodeId)
{
  uint8_t linkCost = (neighborNodeId == m_nodeId)? 0 : 1;
  Ptr<nt::Entry> entry = Create<nt::Entry> (neighborNodeId, m_neighborCount, m_routingTable, linkCost);
  std::pair<Iterator, bool> ret = m_nt.insert(std::pair<uint32_t, Ptr<nt::Entry> > (neighborNodeId, entry));
  NS_ASSERT_MSG (ret.second, "Never should be the case! Out of memory!");
  m_neighborIdMap.push_back(neighborNodeId);
  m_neighborCount++;
  return entry;
}

Ptr<nt::Entry>
NeighborTable::GetEntry(uint32_t neighborNodeId)
{
  Iterator it = m_nt.find(neighborNodeId);
  if (it == m_nt.end()) return AddEntry(neighborNodeId);
  return it->second;
}

Ptr<nt::Entry>
NeighborTable::UpdateNT( uint32_t neighborNodeId, Ptr<const Name> &prefix,
		const uint8_t &dist, const uint32_t &anchor, const uint32_t &neighborIndex)
{
  //TODO remove the function?
  return 0;
}

void
NeighborTable::RemoveNeighbor(uint32_t nodeId)
{
  Ptr<nt::Entry> entry = GetEntry(nodeId);
  if (entry){
	  entry->Clean();
  }
  m_nt.erase(nodeId);
}

std::list<uint32_t>
NeighborTable::GetDeadNeighbors (Time interval)
{
  std::list<uint32_t> deadNeighbors;
  for (Iterator it = m_nt.begin(); it != m_nt.end(); it++)
    {
      if (it->first == m_nodeId)  continue;
      if (it->second->IsDead()) continue;
      if ((Simulator::Now() - it->second->LastRefreshTime()) > 4 * interval){
	  it->second->SetDead();
	  deadNeighbors.push_back(it->first);
	  it->second->SetLinkCost(-1);
      }
    }
  return deadNeighbors;
}


void
NeighborTable::Print(std::ostream &os)
{
  os << "Neighbor Table of node" << m_nodeId << ": " << std::endl;
  for (Iterator it = m_nt.begin(); it != m_nt.end(); it++){
	  os << "\t Neighbor ID is: " << it->first << " NeighborIndex: " << it->second->GetNeighborIndex() << " Last Update: " << it->second->LastRefreshTime ().GetMinutes() << std::endl;
	  it->second->Print(os);
  }
}

/*
 * RUI class definition
 * Has been deleted!
 * TODO delete it!

bool
rt::RUI::SetRUI(uint8_t dist, uint32_t anchor, uint8_t fDist)
{
	m_dist = dist;
	m_anchor = anchor;
	m_fDist = fDist;
	return true;
}
 */

/*
 * Entry class Definition
 */

rt::Entry::Entry()
{
  m_dist = -1;
  m_anchor = -1;
  m_fDist = -1; //TODO check to set 0!
  m_vnh.clear();
  m_bestHop = -1;
  m_sendUpdate = 0;
  m_sendQuery = 0;
  m_sendReply = 0;
  m_querySent = 0;
  m_mode = PASSIVE;
  m_state = O1;
  m_neighborInfo.clear();
}

void
rt::Entry::SetNeighborInfo(uint32_t neighborIndex, Ptr<nt::EntryInfo> &info)
{
	if (m_neighborInfo.size() <= neighborIndex)
		m_neighborInfo.resize(neighborIndex + 1, 0); //neighborIndex starts from 0
	m_neighborInfo.at(neighborIndex) = info;
}

bool
rt::Entry::SendUpdate(uint32_t neighborIndex)
{
  return ( m_sendUpdate & (1 << (neighborIndex-1)));  //index 0 is node Itself!
}

void
rt::Entry::SetSendUpdate ()
{
  m_sendUpdate = -1; //We send Update to all neighbors. EasyPeasy!
}

void
rt::Entry::ResetSendUpdate(uint32_t neighborIndex)
{
  uint32_t neighbor = ~(1 << (neighborIndex-1));
  m_sendUpdate &= neighbor;
}

bool
rt::Entry::SendQuery(uint32_t neighborIndex)
{
  return ( m_sendQuery & (1 << (neighborIndex-1)));  //index 0 is node Itself!
}

void
rt::Entry::SetSendQuery()
{
  m_sendQuery = -1;
}

void
rt::Entry::ResetSendQuery(uint32_t neighborIndex)
{
  uint32_t neighbor = ~(1 << (neighborIndex-1));
  m_sendQuery &= neighbor;
}

void
rt::Entry::SatisfyQueries()
{
  m_sendReply = m_queryRcvd;
  m_queryRcvd = 0;
}

bool
rt::Entry::SendReply(uint32_t neighborIndex)
{
  return (m_sendReply & (1 <<(neighborIndex - 1)));
}

void
rt::Entry::SetSendReply(uint32_t neighborIndex)
{
  m_sendReply |= (1 << (neighborIndex -1));
}

void
rt::Entry::ResetSendReply(uint32_t neighborIndex)
{
  uint32_t neighbor = ~(1 << (neighborIndex-1));
  m_sendReply &= neighbor;
}

void
rt::Entry::SetQuerySent(uint32_t neighborIndex)
{
  m_querySent |= (1 << (neighborIndex -1));
}

void
rt::Entry::ResetQuerySent(uint32_t neighborIndex)
{
  uint32_t neighbor = ~(1 << (neighborIndex-1));
  m_querySent &= neighbor;
}

void
rt::Entry::SetQueryRcvd(uint32_t neighborIndex)
{
  m_queryRcvd |= (1 << (neighborIndex -1));
}

bool
rt::Entry::UpdateEntry()
{
  bool found = false; //found a best hop
  for (uint32_t i = 0; i < m_neighborInfo.size(); i++)
  {
    uint8_t neighborDist = (m_neighborInfo.at(i) ? m_neighborInfo.at(i)->GetDist() : -1);
    uint8_t linkCost = (m_neighborInfo.at(i) ? m_neighborInfo.at(i)->GetLinkCost() : -1);
    uint8_t Dist = ((neighborDist == 255) or (linkCost == 255)) ? -1 : neighborDist + linkCost;
    uint32_t neighborId = (m_neighborInfo.at(i) ? m_neighborInfo.at(i)->GetNeighborNodeId(): -1);
    if (neighborDist < m_fDist
	and ((Dist < m_dist)
	     or ((Dist == m_dist) and (neighborId < m_bestHop))))
      {
	m_fDist = Dist;
	m_dist = Dist;
	m_bestHop = neighborId;
	m_anchor = m_neighborInfo.at(i)->GetAnchor();
	found = true;
      }
  }
  if (found)
    {
      //m_mode = PASSIVE;
      SetSendUpdate ();
    }
  /*
  else
    {
      m_mode = ACTIVE;
      SetSendQuery();
    }*/
  return found;
}

bool
rt::Entry::UpdateEntry(uint32_t neighborNodeId, uint8_t neighborDist,  uint8_t linkCost, uint32_t anchor)
{
  uint8_t dist = ((neighborDist == 255) or (linkCost == 255)) ? -1 : (neighborDist + linkCost);
  if (neighborDist < m_fDist)
    {
      if (dist < m_dist) //m_dist and m_fDist are equal when we are in passive mode!
	{
	  m_dist = dist;
	  m_fDist = m_dist;
	  m_anchor = anchor;
	  m_bestHop = neighborNodeId;
	  m_sendUpdate = 255; //TODO Do we need to send update to this neighbor too? SetSendUpdate ();
	  return true;
	}
      else if ((dist == m_dist) and (neighborNodeId < m_bestHop))
	{
	  m_bestHop = neighborNodeId;
	  if (m_anchor != anchor) //TODO Do we really need to announce new anchor?
	    {
	      m_anchor = anchor;
	      m_sendUpdate = 255;
	      return true;
	    }
	}
    }
  else if ((neighborDist >= m_fDist) and (neighborNodeId == m_bestHop))
    {
      m_dist = dist;
      return UpdateEntry();
    }
  return true;
}

void
rt::Entry::SetMode(Mode mode)
{
  m_mode = mode;
}

void
rt::Entry::Print()
{
  std::cout << " Dist:  " << static_cast<uint16_t> (m_dist) << " Next Hop: " << m_bestHop << " Anchor: " << m_anchor <<
      " FeasibleDist: " << static_cast<uint16_t> (m_fDist) << " Mode: " << m_mode << std::endl;
}

/*
 * RoutingTable Class
 */

TypeId
RoutingTable::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::dcr::RoutingTable") // cheating ns3 object system
    .SetParent<Object> ()
    .SetGroupName ("Ndn")
	.AddConstructor <RoutingTable> ()
  ;
  return tid;
}

void
RoutingTable::CleanTable()
{
  for (Iterator it = m_rt.begin(); it != m_rt.end(); it++)
    {
      it->second->ClearVNH();
    }
  m_rt.clear();
}

Ptr<rt::Entry>
RoutingTable::GetEntry(Ptr<const Name> prefix, bool create /* = true */)
{
  Iterator it = m_rt.find(*prefix);
  if (it == m_rt.end())
    {
      if (!create) return 0;
      std::pair<Iterator, bool > ret = m_rt.insert(std::pair<Name, Ptr<rt::Entry> >(*prefix, Create<rt::Entry> ()));
      NS_ASSERT_MSG (ret.second, "Coudl not Add entry!");
      it = ret.first;
    }
  return it->second;
}

Ptr<rt::Entry>
RoutingTable::AddEntry(Ptr<const Name> prefix)
{
  std::pair<Iterator, bool > ret = m_rt.insert(std::pair<Name, Ptr<rt::Entry> >(*prefix, Create<rt::Entry> ()));
  NS_ASSERT_MSG (ret.second, "Could not Add entry!");
  return ret.first->second;
}

Ptr<Packet>
RoutingTable::GetUpdateMsg(uint32_t destId, bool isDead /* = false */, bool sendAll /* = false */) //destId is neighborIndex not NodeId;
{
  Const_Iterator it = m_lastEntrySent;
  Ptr<Packet> updateMsg = Create<Packet> ();
  uint16_t infoCount = 0;
  while (it != m_rt.end ())
    {
      uint16_t flag = 0;
      Ptr<rt::Entry> rtEntry = it->second;
      if (rtEntry->SendUpdate(destId) or sendAll)
	{
	  flag = flag | dcr::DcrRouter::UPDATE;
	  rtEntry->ResetSendUpdate(destId);
	}
      if (rtEntry->SendQuery(destId))
	{
	  flag = flag | dcr::DcrRouter::QUERY;
	  rtEntry->ResetSendQuery(destId);
	  if (!isDead) rtEntry->SetQuerySent(destId);
	}
      if (rtEntry->SendReply(destId))
	{
	  flag = flag | dcr::DcrRouter::REPLY;
	  rtEntry->ResetSendReply(destId);
	}

      if (flag)
	{
	  //std::cout << "Node: " << m_nodeId << " send Flag: " << flag << std::endl;
	  Ptr<Name> prefix = Create <Name> (it->first);
	  Ptr<dcr::RouteInfo> info = Create<dcr::RouteInfo> (
	      ConstCast<const Name> (prefix), rtEntry->GetDist(), rtEntry->GetAnchor(), flag);
	  wire::ndnSIM::dcr::RouteInfo rInfo (info);
	  updateMsg->AddHeader(rInfo);
	  infoCount++;
	}
      it ++;
    }
  m_lastEntrySent =  it;
  if (!infoCount) return 0;
  wire::ndnSIM::dcr::UpdateMsg uMsg;
  uMsg.AddHeader(updateMsg, 0, m_nodeId, infoCount, 0);
  return updateMsg;
}

void
RoutingTable::Print(std::ostream &os)
{
  os << "Print node " << m_nodeId << " routing table:" << std::endl;
  for (Const_Iterator it = m_rt.begin(); it != m_rt.end(); it ++)
    {
      os << "\t Prefix: " << it ->first;
      it->second->Print ();
    }
}


} // dcr
} // ndn
} // ns3




