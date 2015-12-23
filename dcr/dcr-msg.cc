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

#include "dcr-msg.h"

namespace ns3 {
namespace ndn {
namespace dcr{

RouteInfo::RouteInfo()
  : m_name ()
  , m_dist (-1)
  , m_anchor (-1)
  , m_flag (0)
{
}

RouteInfo::RouteInfo(Ptr<const Name> prefix, uint16_t dist, uint32_t anchor, uint16_t flag)
  : m_name (prefix)
  , m_dist (dist)
  , m_anchor (anchor)
  , m_flag (flag)
{
}

void
RouteInfo::SetName(Ptr<Name> prefix)
{
  m_name = prefix;
}

void
RouteInfo::Print(std::ostream &os)
{
  os << " Name: " << *GetName () << " Dist: " << static_cast<uint16_t> (GetDist ()) << " Anchor: " << GetAnchor () << " Flag: " <<   GetFlag () << std::endl;
}


/*
 * Update Msg Class
 */

UpdateMsg::UpdateMsg()
  : m_ver (-1)
  , m_nodeId (-1)
  , m_num (-1)
  , m_reserved (0)
{
}

UpdateMsg::UpdateMsg(uint32_t &ver, uint32_t &nodeId, uint32_t &num, uint32_t reserved)
  : m_ver (ver)
  , m_nodeId (nodeId)
  , m_num (num)
  , m_reserved (reserved)
{
}



} // dcr
} // ndn
} // ns3
