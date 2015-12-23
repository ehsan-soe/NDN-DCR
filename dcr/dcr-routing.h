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


#ifndef DCR_ROUTING_H_
#define DCR_ROUTING_H_

#include "ns3/ndn-app.h"
#include "ns3/name.h"

#include <map>
#include <list>
#include <boost/tuple/tuple.hpp>

#include "ns3/ndn-fib.h"
#include "ns3/ndn-pit.h"

#include "dcr-router.h"
#include "dcr-tables.h"


namespace ns3 {
namespace ndn {

using namespace dcr;

/**
 * @brief A Actual DCR application
 *
 * This applications demonstrates how to send DCR MSGs and respond to incoming interests
 *
 */
class DcrApp : public App
{
public:

  typedef uint32_t NodeId;
  typedef uint32_t NeighborNodeId;
  typedef std::map<std::string, uint32_t> PrefixList;


  // register NS-3 type "DcrApp"
  static TypeId
  GetTypeId ();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication ();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication ();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest (Ptr<const Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData (Ptr<const ndn::Data> contentObject);

  void
  OnUpdate (Ptr<const Packet> msg);

  void
  SendBroadcast ();

  void
  ScheduleNextUpdate ();

  bool
  SendUpdate();

  void
  ScheduleTriggerUpdate ();

  void
  SendTriggerUpdate ();

  void
  ScheduleDeleteNode(double);

  void
  DeleteNode();

  void
  AverageAnchors();

  void
  AverageVNHs();

  void
  AverageNeighbors();

//  void UpdateFIBAfterRemove ();

  void CheckNeighbours ();
  static void PrintStats();

  static void IncreaseOp(){
	  m_numOfOp++;
  }

//  	std::string NodeIdStr = interest->GetName().getSubName(2,1).toUri().substr(2);
 // 	return (uint32_t) std::atoi(NodeIdStr.c_str());
 // }

  static Time m_updateInterval; //= Seconds (10.0);
  static Time m_deadInterval ;// = 4 * m_helloInterval;



private:
  void
  SendInterest ();
  uint64_t m_updateMsgSeq;
  uint32_t m_nodeId;
  uint32_t m_deletedNodeId;

  double updateDelay;

  Time m_calcAvgVNH;
  Time m_calcAvgAnch;

  EventId m_sendUpdateEvent;
  EventId m_scheduleUpdateEvent;

  EventId m_SendUpdate;
  EventId m_sendDirectedUpdate;
  EventId m_printNT;
  EventId m_printRT;
  EventId m_updateRT;
  EventId m_trigerUpdate;

  void PrintNT (void);

  EventId m_checkNBRs;

  Ptr<DcrRouter> m_dcrRouter;

  Ptr<Fib> m_fib;
  Ptr<Pit> m_pit;
  // Statics:

  uint32_t hellomsgs;
  uint32_t events;
  uint32_t opers;
  bool changed;

  static uint32_t m_numOfSentHelloMsgs;
  static uint32_t m_numOfSentUpdateMsgs;
  static uint32_t m_numOfSentInfMsgs;
  static uint32_t m_numOfSentNewMsgs;

  static uint32_t m_numOfRecivedHelloMsgs;
  static uint32_t m_numOfRecivedUpdateMsgs;
  static uint32_t m_numOfRecivedInfMsgs;
  static uint32_t m_numOfRecivedNewMsgs;

  static double m_avgNumOfVNH;
  static double m_avgNumOfAnchors;
  static uint16_t m_numOfNeighbors;

  static uint32_t m_numOfEvents;
  static uint32_t m_numOfOp;
  static uint32_t m_numOfD;
  static std::set<uint32_t> changedNodes;
  static Time t;// = Simulator::Now();

};

} //namespace ndn
} // namespace ns3



#endif /* DCR_ROUTING_H_ */
