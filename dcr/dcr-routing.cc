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

#include "dcr-routing.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/node-list.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/random-variable.h"
#include "ns3/dcr-router-helper.h"
#include "ns3/ndn-l3-protocol.h"
#include "../ndn-net-device-face.h"

#include "dcr-msg-wire.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>




NS_LOG_COMPONENT_DEFINE ("ndn.DcrApp");

namespace ns3 {

namespace ndn {
NS_OBJECT_ENSURE_REGISTERED (DcrApp);

// register NS-3 type
TypeId
DcrApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::DcrApp")
    .SetGroupName ("Ndn")
    .SetParent<App> ()
    .AddConstructor<DcrApp> ()
    .AddAttribute ("calcAvgAnch",
			      "Calculate Average",
			      TimeValue (Seconds (289)),
			      MakeTimeAccessor (&DcrApp::m_calcAvgAnch),
			      MakeTimeChecker ())
    .AddAttribute ("calcAvgknow",
			      "Calculate Average",
			      TimeValue (Seconds (289)),
			      MakeTimeAccessor (&DcrApp::m_calcAvgVNH),
			      MakeTimeChecker ())
    .AddAttribute ("deletedNodeId", "DeletedNodeId",
			      UintegerValue (0),
			      MakeUintegerAccessor(&DcrApp::m_deletedNodeId),
			      MakeUintegerChecker<uint32_t>())
    ;
  return tid;
}

// Processing upon start of the application
void
DcrApp::StartApplication ()
{
  // initialize ndn::App
  App::StartApplication ();
  m_nodeId = GetNode()->GetId();

  m_dcrRouter = GetNode ()->GetObject<DcrRouter> ();//CreateObject<RoutingTable>(m_nodeId);
  m_dcrRouter->SetApplication(this);
  updateDelay = 4.0;
  hellomsgs = 0;;
  events = 0;
  opers = 0;
  changed = false;

  // Create a name components object for name ``/DCR``
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/DCR"); // now prefix contains ``/DCR``

  /////////////////////////////////////////////////////////////////////////////
  // Creating FIB entry that ensures that we will receive incoming Interests //
  /////////////////////////////////////////////////////////////////////////////

  // Get FIB object
  m_fib = GetNode ()->GetObject<Fib> ();
  m_pit = GetNode ()-> GetObject<Pit>();

  // Add entry to FIB
  // Note that ``m_face`` is cretaed by ndn::App

  m_fib->Add (*prefix, m_face, 0);

  Time offset = Seconds((rand()%100)/100.0);
  Simulator::Schedule (offset, &DcrApp::SendBroadcast, this);

  NS_LOG_DEBUG( "Consumer app has been started: " << Simulator::Now () << std::endl);
  m_dcrRouter->CheckDeadNeighbors(m_updateInterval);

}

// Processing when application is stopped
void
DcrApp::StopApplication ()
{
  // cleanup ndn::App
  App::StopApplication ();
}


void
DcrApp::ScheduleTriggerUpdate()
{
  if (m_trigerUpdate.IsRunning()) return;
  std::cout << "At Time: " << Simulator::Now().GetSeconds() << "  ********** Schedule Trigger Update ******* "  << std::endl ;
  m_trigerUpdate = Simulator::Schedule(Seconds(1), &DcrApp::SendTriggerUpdate, this);
}

void
DcrApp::SendTriggerUpdate()
{
  std::cout << "At time: " << Simulator::Now().GetSeconds() << " Send TriggerUpdate " << std::endl;
}


void
DcrApp::ScheduleDeleteNode(double dtime){
	Simulator::Schedule (
			Seconds(dtime),
		    &DcrApp::DeleteNode, this);
}

/*
 * TODO: Not Tested! Need revision!
 */
void
DcrApp::DeleteNode(){
	//m_nt->CleanTable();
	//m_rt->CleanTable();
	bool havechanged = false;
	/*
	BOOST_FOREACH (const Ptr<const Name> &prefix, m_dcrRouter->GetLocalPrefixes ()){
		NS_LOG_DEBUG ("Prefix Pushed Back: " << prefix->toUri());

		//m_nt->AddNT(m_nodeId, prefix->toUri(), 0, m_nodeId, 1);
		havechanged = true;
		//m_rt->AddRT(prefix->toUri(),true,0, m_nodeId, 1, m_nodeId, m_nodeId);
		m_numOfOp++;
	}*/
	if (havechanged){
		//std::cout << "GetPrefixesFromRouter" << std::endl;
		//m_ntc->PrintNT();
		//m_rtc->PrintRT();
		//ScheduleSendUpdate(updateDelay);
	}
}

void
DcrApp::SendBroadcast ()
{
  Ptr<Packet> updateMsg = Create<Packet> ();
  wire::ndnSIM::dcr::UpdateMsg uMsg;
  uMsg.AddHeader(updateMsg, 0, m_nodeId, 0, 0);

  Ptr<ndn::Interest> update = Create<ndn::Interest> (updateMsg);
  Ptr<ndn::Name> updateName = Create<ndn::Name> ("/DCR");
  updateName->appendSeqNum(500);
  updateName->appendSeqNum(m_nodeId);
  uint64_t nounce = rand() % std::numeric_limits<uint64_t>::max ();
  updateName->appendNumber(nounce);
  update->SetName(updateName);
  //Check if router has more to send!
  NS_LOG_DEBUG( "Update: " << update << " Msg: " << *update);

  // Call trace (for logging purposes)
  m_transmittedInterests (update, this, m_face);
  bool didsent = m_face->ReceiveInterest (update);
  if (didsent) m_numOfSentUpdateMsgs++;

	m_scheduleUpdateEvent = Simulator::Schedule (
			m_updateInterval,
                                 &DcrApp::ScheduleNextUpdate, this);
}

void
DcrApp::ScheduleNextUpdate(){
  if(!m_scheduleUpdateEvent.IsRunning ())
  {//Just a check, there should be no send Update event by now!
    uint8_t sign = rand()%2;
    Time helloInterval;
    if (sign){
	helloInterval = m_updateInterval + Seconds((rand()%100)/100.0);
    }else{
	helloInterval = m_updateInterval - Seconds((rand()%100)/100.0);
    }
    //std::cout << "Hello Interval for node: " << m_nodeId << " is: " << helloInterval.ToDouble(Time::S) << std::endl;
    m_scheduleUpdateEvent = Simulator::Schedule (
		    m_updateInterval,
				   &DcrApp::ScheduleNextUpdate, this);
    if (!m_sendUpdateEvent.IsRunning ())
	    m_sendUpdateEvent = Simulator::ScheduleNow(&DcrApp::SendUpdate, this);
  }

}

bool
DcrApp::SendUpdate()
{
  NS_LOG_DEBUG( "Send Update");
  if (m_trigerUpdate.IsRunning())
    {
      std::cout << "cancel triger update" << std::endl;
      m_trigerUpdate.Cancel();
    }
  NeighborTable::IdMap neighborMap = m_dcrRouter->GetNeighbourTable()->GetNeighborIdMap();
  bool didsent = false;
  for (uint32_t neighbor = 1; neighbor < neighborMap.size (); neighbor++) //neighbor 0 is the node itself!
    {
      //if (m_dcrRouter->GetNeighbourTable()->IsDead(neighbor)) continue; we want to detect link restore!!
      m_dcrRouter->GetRoutingTable()->ResetEntrySent();
      bool sendAll =  m_dcrRouter->GetNeighbourTable()->GetEntry(neighborMap.at (neighbor))->SendAll();
      while (m_dcrRouter->GetRoutingTable()->HasMoreToSend())
	{
	  bool isDead = m_dcrRouter->GetNeighbourTable()->GetEntry(neighborMap.at (neighbor))->IsDead();
	  Ptr<Packet> updateMsg = m_dcrRouter->GetRoutingTable()->GetUpdateMsg(neighbor, isDead, sendAll);
	  if (!updateMsg) //If has nothing to send just send an empty message to refresh neighbor table
	  {
	    updateMsg = Create<Packet> ();
	    wire::ndnSIM::dcr::UpdateMsg uMsg;
	    uMsg.AddHeader(updateMsg, 0, m_nodeId, 0, 0);
	  }
	  Ptr<ndn::Interest> update = Create<ndn::Interest> (updateMsg);
	  Ptr<ndn::Name> updateName = Create<ndn::Name> ("/DCR");
	  updateName->appendSeqNum(neighborMap.at (neighbor));
	  updateName->appendSeqNum(m_nodeId);
	  uint64_t nounce = rand() % std::numeric_limits<uint64_t>::max ();
	  updateName->appendNumber(nounce);
	  update->SetName(updateName);
	  NS_LOG_DEBUG( "Update: " << update << " Msg: " << *update);

	  // Call trace (for logging purposes)
	  m_transmittedInterests (update, this, m_face);
	  didsent = m_face->ReceiveInterest (update);
	  if (didsent) m_numOfSentUpdateMsgs++;
	}
      m_dcrRouter->GetNeighbourTable()->GetEntry(neighborMap.at (neighbor))->SetSendAll(false);
    }
  return didsent;
}

void
DcrApp::SendInterest ()
{
  NS_LOG_ERROR ("Sending Interest packet! ERROR");
}

// Callback that will be called when Interest arrives
void
DcrApp::OnInterest (Ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest (interest);

  NS_LOG_DEBUG ("Received Interest packet for " << interest->GetName ());

  // Create and configure ndn::Data and ndn::DataTail
  // (header is added in front of the packet, tail is added at the end of the packet)

  // Note that Interests send out by the app will not be sent back to the app !
  Name interestName = interest->GetName();
  if (interestName.getSubName(0,1).toUri() != "/DCR")
  {
	  NS_LOG_ERROR ("DCR App got an unrelated Interest!");
	  return;
  }
  OnUpdate (interest->GetPayload());
}

void
DcrApp::OnUpdate(Ptr<const Packet> msg){
  Ptr<Packet> p = msg->Copy();
  wire::ndnSIM::dcr::UpdateMsg u;
  p->RemoveHeader (u);
  if (u.GetMsg()->GetVer() != 0)
    {
      NS_LOG_ERROR ("Only Supports Version 0");
      return;
    }
  m_dcrRouter->ProcessUpdateMsg(msg);

}

// Callback that will be called when Data arrives
void
DcrApp::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << contentObject->GetName ());
}

void
DcrApp::AverageAnchors(){
	//m_avgNumOfAnchors += m_rtc->GetAvgNumOfKnownAnchors();
}

void
DcrApp::AverageVNHs(){
	//m_avgNumOfVNH += m_rtc->GetAvgNumOfVNH();
}

void
DcrApp::AverageNeighbors(){
	//m_numOfNeighbors += m_ntc->GetNaighborTable().size();
}

void DcrApp::PrintStats(){
  std::cout << "Stats: ***** ***** DCR\n";
  std::cout << "num Of Sent Hello Msgs: " << DcrApp::m_numOfSentHelloMsgs << '\n';
  std::cout << "num Of Time: " << DcrApp::t.ToDouble(Time::S) << '\n';
  std::cout << "num Of Sent New Msgs: " << DcrApp::m_numOfSentNewMsgs << '\n';

  std::cout << "num Of Recived Hello Msgs: " << DcrApp::m_numOfRecivedHelloMsgs << '\n';
  std::cout << "num Of Recived Update Msgs: " << DcrApp::m_numOfRecivedUpdateMsgs << '\n';
  std::cout << "num Of Recived New Msgs: " << DcrApp::m_numOfRecivedNewMsgs << '\n';

  std::cout << "num Of Avg Anchors: " << DcrApp::m_avgNumOfAnchors / 154  << '\n';
  std::cout << "num Of ValidNextHops: " << DcrApp::m_avgNumOfVNH / 154 << '\n';

  std::cout << "num Of Events: " << DcrApp::m_numOfEvents << '\n';
  std::cout << "num Of NumOfOperations: " << DcrApp::m_numOfOp << '\n';

  std::cout << "num Of m_numOfNeighbors: " << DcrApp::m_numOfNeighbors << '\n';


  //ndn::fw::DcrFw::PrintStats();
  std::cout << "End Print Stat "<< std::endl;

}

Time DcrApp::m_updateInterval = Seconds (10.0);
Time DcrApp::m_deadInterval = 4 * m_updateInterval;

uint32_t DcrApp::m_numOfSentHelloMsgs = 0;
uint32_t DcrApp::m_numOfSentUpdateMsgs = 0;
uint32_t DcrApp::m_numOfSentInfMsgs = 0;
uint32_t DcrApp::m_numOfSentNewMsgs = 0;

uint32_t DcrApp::m_numOfRecivedHelloMsgs = 0;
uint32_t DcrApp::m_numOfRecivedUpdateMsgs = 0;
uint32_t DcrApp::m_numOfRecivedInfMsgs = 0;
uint32_t DcrApp::m_numOfRecivedNewMsgs = 0;

double DcrApp::m_avgNumOfVNH = 0;
double DcrApp::m_avgNumOfAnchors = 0;
uint16_t DcrApp::m_numOfNeighbors = 0;

uint32_t DcrApp::m_numOfOp = 0;

uint32_t DcrApp::m_numOfEvents = 0;

uint32_t DcrApp::m_numOfD = 0;

std::set<uint32_t> DcrApp::changedNodes;
Time DcrApp::t = Simulator::Now();


} // namespace ndn
} // namespace ns3






