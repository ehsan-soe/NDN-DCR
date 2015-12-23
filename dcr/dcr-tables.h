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

#ifndef NDN_DRC_TABLES_H_
#define NDN_DRC_TABLES_H_

#include "ns3/ndn-app.h"
//#include "ns3/name.h" TODO check this!

#include "ns3/ndn-name.h"

#include "../../utils/trie/trie-with-policy.h"
#include "../../utils/trie/counting-policy.h"

//TODO check this
#include <map>
#include <list>
#include <boost/tuple/tuple.hpp>


namespace ns3 {
namespace ndn {

namespace dcr {

class RoutingTable;

namespace nt{

//namespace entry{

//}

class EntryInfo: public SimpleRefCount<EntryInfo>{
public:
  EntryInfo (uint32_t neighborNodeId){
    m_neighborNodeId = neighborNodeId;
    m_dist = -1;
    m_anchor = -1;
    m_linkCost = 0; // a pointer to
  }
  EntryInfo (const uint32_t &neighborNodeId, const uint8_t &dist, const uint32_t &anchor, const uint8_t * const linkCost):
    m_neighborNodeId (neighborNodeId),
    m_dist (dist),
    m_anchor (anchor),
    m_linkCost (linkCost)
    {};
  bool UpdateInfo (const uint8_t &dist, const uint32_t &anchor);
  uint32_t GetNeighborNodeId () {return m_neighborNodeId;};
  uint8_t GetDist () {return m_dist;};
  uint32_t GetAnchor () {return m_anchor;};
  uint8_t GetLinkCost () {return (m_linkCost)? *m_linkCost : -1;};
  void Print ();
private:
  uint32_t m_neighborNodeId; //NodeId of Neighbor
  uint8_t m_dist; //Distance
  uint32_t m_anchor; //Anchor ID
  const uint8_t * m_linkCost;
};

/* TODO change Entry class to something like this:
class Entry : public Object,
			  protected ndnSIM::trie< Name,
              ndnSIM::smart_pointer_payload_traits< EntryImpl >,
              ndnSIM::counting_policy_traits >
{

};*/

class Entry: public SimpleRefCount<Entry>{
private:

public:
  typedef std::map<Name, Ptr<EntryInfo> > NTEntry;
  typedef NTEntry::iterator Iterator;
  typedef NTEntry::const_iterator Const_Iterator;

  Entry(uint32_t neighborNodeId, uint32_t neigborIndex, Ptr<RoutingTable> routingTable, uint8_t linkCost);

  uint16_t
  GetNeighborIndex () {return m_neighborIndex;};

  Time
  LastRefreshTime () {return m_lastUpdate;};

  uint8_t
  GetLinkCost () {return m_linkCost;};

  Ptr<EntryInfo>
  UpdateInfo(Ptr<const Name> prefix, uint8_t dist, uint32_t anchor);

  Ptr<EntryInfo>
  AddInfo(Ptr<const Name> prefix, uint8_t dist, uint32_t anchor);

  Ptr<EntryInfo>
  GetInfo (Ptr<const Name> &prefix);

  void
  SetLinkCost (uint8_t linkCost) {m_linkCost = linkCost;};

  bool
  RemovePrefix(Ptr<const Name> &prefix);

  void
  RefreshEntry ();

  bool
  IsDead () {return m_dead;};
  void
  SetDead () {m_dead = true;};
  void
  ResetDead ();// {m_dead = false;};

  bool
  SendAll () {return m_sendAll;};
  void
  SetSendAll (bool sendAll) {m_sendAll = sendAll;};

  Ptr<Packet>
  GetDeadUpdateMsg ();

  void
  Clean();

  void
  Print(std::ostream &os);

private:
  NTEntry m_entry;
  uint32_t m_neighborNodeId;
  uint8_t m_linkCost; //link cost from node to the neighbor -1 means no link!
  bool m_dead;
  bool m_sendAll;
  Time m_lastUpdate; // Time recived Last UpdateMsg
  uint16_t m_neighborIndex; //neighborId assigend by Node!
  Ptr<RoutingTable> m_routingTable;
};

} //nt

class NeighborTable: public Object
{
public:
  typedef std::map<uint32_t, Ptr<nt::Entry> >NT;
  typedef NT::iterator Iterator;
  typedef NT::const_iterator Const_Iterator;
  typedef std::vector<uint32_t> IdMap;

  static TypeId GetTypeId ();
  NeighborTable () {};
  NeighborTable(uint32_t nodeId, Ptr<RoutingTable> routingTable);
  void CleanTable();
  //NeighborTable (uint32_t);
  void SetNodeId (uint32_t nodeid) {m_nodeId = nodeid; };
  uint32_t GetNodeId () {return m_nodeId;};
  IdMap GetNeighborIdMap () {return m_neighborIdMap;};
  bool IsDead (uint32_t neighborIndex);
  Ptr<nt::Entry> AddEntry (uint32_t neighborNodeId);
  Ptr<nt::Entry> GetEntry (uint32_t neighborNodeId);
  Ptr<nt::EntryInfo> GetInfo (const uint32_t&, Ptr<const Name>&);
  Ptr<nt::Entry> UpdateNT (
		  uint32_t neighborNodeId, Ptr<const Name>&prefix,
		  		  const uint8_t &dist, const uint32_t &anchor, const uint32_t &neighborId);
  NT& GetNaighborTable(){return m_nt;};
  void RemoveNeighbor(uint32_t);
  uint16_t GetNeighborTableSize(){return m_nt.size();};
  //RUIbool RemovePrefixFromNT(uint32_t, Ptr<const Name> &);
  //uint32_t GetNeighborNodeId (uint32_t neighborId);
  std::list<uint32_t> GetDeadNeighbors (Time interval);
  void Print (std::ostream &os);

private:
  uint32_t m_nodeId;
  //std::vector<uint32_t> m_neighborIdMap;
  NT m_nt;
  Ptr<RoutingTable> m_routingTable;
  uint32_t m_neighborCount;
  IdMap m_neighborIdMap;
};

namespace rt{

typedef std::set<uint32_t> VNH;

enum Mode
{
	PASSIVE,
	ACTIVE
};

enum State
{
	O0,
	O1,
	O2,
	O3,
};

class Entry : public SimpleRefCount<Entry>
{
public:
  typedef std::vector<Ptr<nt::EntryInfo> > NeighborInfo;

  Entry ();

  void
  SetDist (uint8_t dist) {m_dist = dist;};

  uint8_t
  GetDist () {return m_dist;};

  void
  SetAnchor (uint32_t anchor) {m_anchor = anchor;};

  uint32_t
  GetAnchor () {return m_anchor;};

  void
  setFDist (uint8_t fDist) {m_fDist = fDist;};

  uint8_t
  GetFDist () {return m_fDist;};

  void
  InsertVNH (uint32_t vnh) {m_vnh.insert(vnh);};

  //TODO GetVNH! Define VNH as a class and return a pointer!

  void
  ClearVNH () {m_vnh.clear ();};

  void
  SetBestHop (uint32_t bh) {m_bestHop = bh;};

  void
  SetNeighborInfo (uint32_t neighborIndex, Ptr<nt::EntryInfo> &info);

  uint32_t
  GetSendUpdate () {return m_sendUpdate;};

  bool
  SendUpdate (uint32_t neighborIndex);

  void
  SetSendUpdate ();

  void
  ResetSendUpdate (uint32_t neighborIindex);

  bool
  SendQuery (uint32_t neighborIndex);

  void
  SetSendQuery ();

  void
  ResetSendQuery (uint32_t neighborIndex);

  void
  SatisfyQueries ();

  bool
  SendReply (uint32_t neighborIndex);

  void
  SetSendReply (uint32_t neighborIndex);

  void
  ResetSendReply (uint32_t neighborIndex);

  void
  SetQuerySent (uint32_t neighborIndex);

  void
  ResetQuerySent (uint32_t neighborIndex);

  bool
  LastReply () {return !m_querySent;};

  void
  SetQueryRcvd (uint32_t neighborIndex);

  bool
  UpdateEntry ();

  //return true if we have valid nex hop, false if there is no valid next hop -> should go to active mode
  bool
  UpdateEntry (uint32_t neighborNodeId, uint8_t dist, uint8_t linkCost, uint32_t anchor);

  Mode
  GetMode () {return m_mode;};

  void
  SetMode (Mode mode);

  void
  Print ();

private:
  uint8_t m_dist;
  uint32_t m_anchor;
  uint8_t m_fDist;
  uint32_t m_bestHop;
  VNH m_vnh;
  uint32_t m_sendUpdate; // Need to send Update
  uint32_t m_sendQuery; // Need to send Query
  uint32_t m_sendReply; // Need to send Reply (Reply is ready)
  uint32_t m_querySent; // Query sent, wait for Reply
  uint32_t m_queryRcvd; //Query received
  Mode m_mode;
  State m_state;
  NeighborInfo m_neighborInfo;

};
} //rt

class RoutingTable : public Object
{
public:
  static TypeId GetTypeId ();
  typedef std::map<Name, Ptr<rt::Entry> > RT;
  typedef RT::iterator Iterator;
  typedef RT::const_iterator Const_Iterator;

private:
  uint32_t m_nodeId;
  RT m_rt;
  RT::const_iterator m_lastEntrySent;

public:
  RoutingTable(){};

  RoutingTable(uint32_t nodeId): m_nodeId(nodeId){};

  void
  CleanTable();

  void
  SetNodeId (uint32_t nodeid) { m_nodeId = nodeid; };

  uint32_t
  GetNodeId (void) {return m_nodeId;};

  Ptr<rt::Entry>
  GetEntry (Ptr<const Name> prefix, bool create = true);

  Ptr<rt::Entry>
  AddEntry (Ptr<const Name> prefix);

  Ptr<Packet>
  GetUpdateMsg (uint32_t destId, bool isDead = false, bool sendAll = false);

  void
  ResetEntrySent () {m_lastEntrySent = m_rt.begin ();};

  bool
  HasMoreToSend () {return m_lastEntrySent != m_rt.end (); };

  void
  Print(std::ostream &os);

};

} //dcr
} //ndn
} //ns3



#endif /* NDN_DRC_TABLES_H_ */
