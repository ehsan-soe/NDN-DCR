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

#ifndef DCR_MSG_WIRE_H_
#define DCR_MSG_WIRE_H_

#include "ns3/ndn-common.h"
#include "ns3/header.h"
#include "dcr-msg.h"

NDN_NAMESPACE_BEGIN
/**
 * @brief Namespace encapsulating wire operations
 */
namespace wire {

/**
 * @brief Namespace for ndnSIM wire format operations
 */
namespace ndnSIM {

/**
 * @brief Namespace for DCR wire format operations
 */
namespace dcr{

class RouteInfo : public Header
{
public:
  RouteInfo ();
  RouteInfo (Ptr<ndn::dcr::RouteInfo> info);

  Ptr<ndn::dcr::RouteInfo>
  GetRouteInfo ();

  static Ptr<ndn::dcr::RouteInfo>
  GetRouteInfo (Ptr<Packet> packet);

  // from Header
  static
  TypeId GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;

  virtual void
  Print (std::ostream &os) const;

  virtual uint32_t
  GetSerializedSize (void) const;

  virtual void
  Serialize (Buffer::Iterator start) const;

  virtual uint32_t
  Deserialize (Buffer::Iterator start);

private:
  Ptr<ndn::dcr::RouteInfo> m_info;
};

class UpdateMsg : public Header
{
public:
  UpdateMsg ();
  UpdateMsg (Ptr<ndn::dcr::UpdateMsg> info);

  Ptr<ndn::dcr::UpdateMsg>
  GetMsg ();

  // from Header
  static TypeId
  GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;

  virtual void
  Print (std::ostream &os) const;

  virtual uint32_t
  GetSerializedSize (void) const;

  virtual void
  Serialize (Buffer::Iterator start) const;

  virtual uint32_t
  Deserialize (Buffer::Iterator start);

  void
  AddHeader(Ptr<Packet>& payload, uint32_t ver, uint32_t nodeId, uint32_t num, uint32_t reserved = 0);

  Ptr<ndn::dcr::UpdateMsg>
  GetHeader (Ptr<Packet> &msg);

private:
  Ptr<ndn::dcr::UpdateMsg> m_msg;
};

} //namespace dcr
} //namespace ndnSIM
} //namespace wire

NDN_NAMESPACE_END
#endif /* DCR_MSG_H_ */


