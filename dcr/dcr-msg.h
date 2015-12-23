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

#ifndef DCR_MSG_H_
#define DCR_MSG_H_

#include "ns3/simple-ref-count.h"
#include "ns3/ptr.h"

#include <ns3/ndnSIM/ndn.cxx/name.h>

namespace ns3 {

class Packet;

namespace ndn {

namespace dcr{
/**
 * @ingroup ndn
 * @brief DCR Msg Info (wire formats are defined in wire)
 **/
class RouteInfo : public SimpleRefCount<RouteInfo>
{
public:
  RouteInfo ();
  RouteInfo(Ptr<const Name> prefix, uint16_t dist, uint32_t anchor, uint16_t flag);

  Ptr<const Name>
  GetName () const {return m_name;};
  void
  SetName (Ptr<Name> prefix);

  uint32_t
  GetAnchor () const {return m_anchor;};
  void
  SetAnchor (uint32_t anchor) {m_anchor = anchor;};

  uint8_t
  GetDist () const {return m_dist;};
  void
  SetDist (uint8_t dist) {m_dist = dist;};

  uint32_t
  GetFlag () const {return m_flag;};
  void
  SetFlag (uint32_t flag)  {m_flag = flag;};

  void
  Print (std::ostream &os);

private:
  Ptr<const Name> m_name;
  uint8_t m_dist;
  uint32_t m_anchor;
  uint16_t m_flag;
};

class UpdateMsg : public SimpleRefCount<UpdateMsg>
{
public:
  UpdateMsg ();
  UpdateMsg(uint32_t &ver, uint32_t &nodeId, uint32_t &num, uint32_t reserved = 0);

  uint32_t
  GetVer () const {return m_ver;};
  void
  SetVer (uint32_t ver) {m_ver = ver;};

  uint32_t
  GetNodeId () const {return m_nodeId;};
  void
  SetNodeId (uint32_t nodeId) {m_nodeId = nodeId;};

  uint32_t
  GetNum () const {return m_num;};
  void
  SetNum (uint32_t num)  {m_num = num;};

  uint32_t
  GetReserved () const {return m_reserved;};
  void
  SetReserved (uint32_t reserved) {m_reserved = reserved;};

private:
  uint32_t m_ver;
  uint32_t m_nodeId;
  uint32_t m_num; //number of prefixs
  uint32_t m_reserved;
};

} // namespace dcr
} // namespace ndn
} // namespace ns3

#endif /* DCR_MSG_H_ */
