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

#include "dcr-msg-wire.h"

using namespace std;

#include <ns3/header.h>
#include <ns3/packet.h>
#include <ns3/log.h>

#include "../wire/ndnsim/wire-ndnsim.h"
NS_LOG_COMPONENT_DEFINE ("ndn.wire.dcrMsg");

NDN_NAMESPACE_BEGIN

namespace wire {
namespace ndnSIM {
namespace dcr{

RouteInfo::RouteInfo()
{
  m_info = Create<ndn::dcr::RouteInfo> ();
}
RouteInfo::RouteInfo(Ptr<ndn::dcr::RouteInfo> info):
  m_info (info)
{
}

Ptr<ndn::dcr::RouteInfo>
RouteInfo::GetRouteInfo()
{
  return m_info;
}

Ptr<ndn::dcr::RouteInfo>
RouteInfo::GetRouteInfo(Ptr<Packet> packet)
{
  RouteInfo routeInfo;
  packet->RemoveHeader(routeInfo);
  return routeInfo.GetRouteInfo();
}

TypeId
RouteInfo::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::ndn::dcr::RouteInfo")
	  .SetGroupName ("Ndn")
	  .SetParent<Header> ()
	  .AddConstructor<RouteInfo> ()
	  ;
  return tid;
}

TypeId
RouteInfo::GetInstanceTypeId() const
{
  return GetTypeId ();
}

uint32_t
RouteInfo::GetSerializedSize() const
{
  size_t size =
		  4 /* anchor */ + 2 /* dist */ + 2 /* flag */ +
		  NdnSim::SerializedSizeName(*m_info->GetName ());
  return size;
}

void
RouteInfo::Serialize(Buffer::Iterator start) const
{
  NdnSim::SerializeName (start, *m_info->GetName ());
  start.WriteU16(m_info->GetDist ());
  start.WriteU32(m_info->GetAnchor ());
  start.WriteU16(m_info->GetFlag ());
}

uint32_t
RouteInfo::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_info->SetName(NdnSim::DeserializeName (i));
  m_info->SetDist (i.ReadU16 ());
  m_info->SetAnchor(i.ReadU32());
  m_info->SetFlag(i.ReadU16());

  return i.GetDistanceFrom(start);
}

void
RouteInfo::Print(std::ostream &os) const
{
  os << "DCR Message Info";
}

UpdateMsg::UpdateMsg()
{
  m_msg = Create<ndn::dcr::UpdateMsg> ();
}
UpdateMsg::UpdateMsg(Ptr<ndn::dcr::UpdateMsg> info):
		m_msg (info)
{
}

Ptr<ndn::dcr::UpdateMsg>
UpdateMsg::GetMsg()
{
  return m_msg;
}

TypeId
UpdateMsg::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::ndn::dcr::UpdateMsg")
	  .SetGroupName ("Ndn")
	  .SetParent<Header> ()
	  .AddConstructor<UpdateMsg> ()
	  ;
  return tid;
}

TypeId
UpdateMsg::GetInstanceTypeId() const
{
  return GetTypeId ();
}

uint32_t
UpdateMsg::GetSerializedSize() const
{
  size_t size =
		  4 /* ver */ + 4 /* nodeId */ + 4 /* num */ + 4 /* reserved */ ;
  return size;
}

void
UpdateMsg::Serialize(Buffer::Iterator start) const
{
  start.WriteU32(m_msg->GetVer ());
  start.WriteU32(m_msg->GetNodeId ());
  start.WriteU32(m_msg->GetNum ());
  start.WriteU32(m_msg->GetReserved ());
}

uint32_t
UpdateMsg::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_msg->SetVer (i.ReadU32 ());
  m_msg->SetNodeId (i.ReadU32());
  m_msg->SetNum (i.ReadU32());
  m_msg->SetReserved (i.ReadU32 ());

  return i.GetDistanceFrom(start);
}

void
UpdateMsg::AddHeader(Ptr<Packet> &payload, uint32_t ver, uint32_t nodeId, uint32_t num, uint32_t reserved)
{
  Ptr<ns3::ndn::dcr::UpdateMsg> um = Create<ns3::ndn::dcr::UpdateMsg> (ver, nodeId, num, reserved);
  UpdateMsg u (um);
  payload->AddHeader(u);

}
void
UpdateMsg::Print(std::ostream &os) const
{
  os << "Time: " << Simulator::Now().GetSeconds() << " ver: " <<
      m_msg->GetVer() << " NodeId: " << m_msg->GetNodeId() <<
      " Num: " << m_msg->GetNum() << std::endl;
}


} // dcr
} // ndnSIM
} // wire

NDN_NAMESPACE_END

