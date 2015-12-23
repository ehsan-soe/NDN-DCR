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

#ifndef DCR_ROUTER_H_
#define DCR_ROUTER_H_

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "dcr-tables.h"
#include "dcr-msg-wire.h"

#include <list>
#include <map>
#include <boost/tuple/tuple.hpp>

namespace ns3 {

class Channel;

namespace ndn {

class L3Protocol;
class Face;
class Name;
class DcrApp;

namespace dcr{
/**
 * @ingroup ndn
 * @brief Class representing DCR router interface for ndnSIM
 */
class DcrRouter : public Object
{
public:
  /**
   * @brief List of locally exported prefixes
   */
  typedef std::list< Ptr<Name> > LocalPrefixList;
  typedef uint32_t NodeID;
  typedef std::list<NodeID> NodeList;
  typedef std::map<NodeID, Time> NeighborList;
  typedef boost::tuple<Time, Ptr <Name> > TimeNameEntry; //a template for add/remove nodes
  typedef std::list<TimeNameEntry> TimeNameList; //List of Add/Remove nodes

  typedef enum
    {
      NONE 	= 0,   //!< No flags
      UPDATE  	= 1,   //!< Update
      QUERY  	= 2,   //!< Query
      REPLY  	= 4,   //!< Reply
    } Flags_t;

  /**
   * \brief Interface ID
   *
   * \return interface ID
   */
  static TypeId
  GetTypeId ();

  /**
   * @brief Default constructor
   */
  DcrRouter ();

  DcrRouter (uint32_t nodeId);

  /**
   * @brief Get numeric ID of the node (internally assigned)
   */
  uint32_t
  GetId () const;

  /**
   * @brief Helper function to get smart pointer to ndn::L3Protocol object (basically, self)
   */
  Ptr<L3Protocol>
  GetL3Protocol () const;

  /**
   * @brief Add new locally exported prefix
   * @param prefix Prefix
   */
  void
  AddLocalPrefix (Ptr< Name > prefix);

  /**
   * @brief REmove a locally exported prefix
   * @param prefix Prefix
   */
  void
  RemoveLocalPrefix (Ptr< Name > prefix);
  /**
   * @brief Get list of locally exported prefixes
   */

void
  AddAddPrefix (Time time, Ptr< Name > prefix);

  void
  AddRemovePrefix (Time time, Ptr< Name > prefix);

  Ptr<NeighborTable>
  GetNeighbourTable(){ return m_nt;};

  Ptr<RoutingTable>
  GetRoutingTable () {return m_rt;};

  void
  ProcessUpdateMsg (Ptr<const Packet> msg, bool internal = false);

  void
  CheckDeadNeighbors (Time interval);

  void
  UpdateDeadNeighbor (uint32_t deadNeighbor);

  void
  SetApplication (Ptr<Application> appliacation);// {m_application =  appliacation;};

  void
  PrintNeighborTable () const {m_nt->Print(std::cout);};

  void
  PrintRoutingTable () const {m_rt->Print(std::cout);};

  void
  PrintUpdateMsg (Ptr<const Packet> msg) const;

protected:
  virtual void
  NotifyNewAggregate (); ///< @brief Notify when the object is aggregated to another object (e.g., Node)

  uint16_t m_msgSize;
  uint32_t m_id;
  uint32_t m_nodeId;


  Ptr<L3Protocol> m_ndn;
  static uint32_t m_idCounter;

  NeighborList m_nl;
  std::list<uint32_t> m_deadNeighborList;

  TimeNameList m_addprefixlist;
  TimeNameList m_removeprefixlist;

  Ptr<NeighborTable> m_nt;
  Ptr<RoutingTable> m_rt;

  Ptr<Application> m_application;
  Ptr<DcrApp> m_dcrApp;

};

} // dcr
} // namespace ndn
} // namespace ns3

#endif /* NDN_DCR_ROUTER_H_ */

