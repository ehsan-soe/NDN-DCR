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

#ifndef DCR_FW_H_
#define DCR_FW_H_

#include "ns3/ndn-forwarding-strategy.h"
#include "ns3/log.h"

#include "ns3/ndn-fib.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit-entry.h"
#include "../ndn-net-device-face.h"
#include "ns3/channel.h"

#include <boost/foreach.hpp>

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * @ingroup ndn-fw
 * @brief Implementation of experimental NACK messages (enables with EnableNACKs option)
 */
template<class Parent>
class DcrFw :
    public Parent
{
private:
  typedef Parent super;

public:
  static TypeId
  GetTypeId ();

  /**
   * @brief Helper function to retrieve logging name for the forwarding strategy
   */
  static std::string
  GetLogName ();

  DcrFw () {};

  static void PrintStats();

protected:
  TracedCallback<Ptr<const Data>,
                 Ptr<const Face> > m_outDcr; ///< @brief trace of outgoing NACKs

  TracedCallback<Ptr<const Data>,
                 Ptr<const Face> > m_inDcr; ///< @brief trace of incoming NACKs

  TracedCallback<Ptr<const Data>,
                 Ptr<const Face> > m_dropDcr; ///< @brief trace of dropped NACKs

  virtual bool
  DoPropagateInterest (Ptr<Face> inFace,
                       Ptr<const Interest> interest,
                       Ptr<pit::Entry> pitEntry);

  static LogComponent g_log;

  uint32_t GetDestId (Ptr<const Interest> interest);

  static uint32_t m_numOfHelloMsgs;
  static uint32_t m_numOfUpdateMsgs;
  static uint32_t m_numOfInfMsgs;

};

/*
 * Implimentation
 */



template <class Parent>
LogComponent DcrFw<Parent>::g_log = LogComponent (DcrFw<Parent>::GetLogName ().c_str ());;

template<class Parent>
TypeId
DcrFw<Parent>::GetTypeId (void)
{
static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::DcrFw").c_str ())
  .SetGroupName ("Ndn")
  .template SetParent <super> ()
  .template AddConstructor <DcrFw> ()

  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////

  .AddTraceSource ("OutDcr",  "OutDcr",  MakeTraceSourceAccessor (&DcrFw<Parent>::m_outDcr))
  .AddTraceSource ("InDcr",   "InDcr",   MakeTraceSourceAccessor (&DcrFw<Parent>::m_inDcr))
  .AddTraceSource ("DropDcr", "DropDcr", MakeTraceSourceAccessor (&DcrFw<Parent>::m_dropDcr))
  ;
return tid;
}

template<class Parent>
std::string
DcrFw<Parent>::GetLogName ()
{
return super::GetLogName () + ".DcrFw";
}

template<class Parent>
bool
DcrFw<Parent>::DoPropagateInterest (Ptr<Face> inFace,
                             Ptr<const Interest> interest,
                             Ptr<pit::Entry> pitEntry)
{
NS_LOG_FUNCTION (this);
if (interest->GetName().getSubName(0,1).toUri() == "/DCR"){
  if (inFace->GetMetric() == 0)
    {
      // send to the destination
      int propagatedCount = 0;
      uint32_t destId = GetDestId(interest);
      uint32_t otherNodeId = -1;
      BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ())
      {
	NS_LOG_DEBUG ("Trying " << boost::cref(metricFace));
	if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in the front of the list
	  break;
	Ptr<NetDeviceFace> face = DynamicCast<NetDeviceFace> (metricFace.GetFace());
	if (face == 0) continue;
	Ptr<NetDevice> nd = face->GetNetDevice ();
	if (nd == 0)
	  {
	    NS_LOG_DEBUG ("Not a NetDevice associated with NetDeviceFace");
	    continue;
	  }
	Ptr<Channel> ch = nd->GetChannel ();
	if (ch == 0)
	  {
	    NS_LOG_DEBUG ("Channel is not associated with NetDevice");
	    continue;
	  }
	otherNodeId = -1;
	if (ch->GetNDevices () == 2) // e.g., point-to-point channel
	  {

	    for (uint32_t deviceId = 0; deviceId < ch->GetNDevices (); deviceId ++)
	      {
		Ptr<NetDevice> otherSide = ch->GetDevice (deviceId);
		if (nd == otherSide) continue;

		Ptr<Node> otherNode = otherSide->GetNode ();
		NS_ASSERT (otherNode != 0);
		otherNodeId = otherNode->GetId();

	//std::cout << "Node: "  <<this->GetObject<Node> ()->GetId () << "\t face: " << boost::cref(metricFace) << " \t OtherNode: " << otherNode->GetId() << std::endl;
	      }
	  }
	if ((destId < 300)  and (otherNodeId != destId)) continue;
	if (!super::TrySendOutInterest (inFace, metricFace.GetFace (), interest, pitEntry))
	  {
	    NS_LOG_DEBUG (">>> Was UnSuccess!!! :(");
	    continue;
	  }
	NS_LOG_DEBUG (">>> Was Successful :)");
	m_numOfUpdateMsgs ++;
	propagatedCount++;
      }
      NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
      return propagatedCount > 0;
    }else{
      int propagatedCount = 0;
      BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ())
      {
	NS_LOG_DEBUG ("Sssssecond Trying " << boost::cref(metricFace));

	if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in front
	      break;

	if (metricFace.GetRoutingCost()){
		std::cout << "**&&**&&**&&  Ehsan NOTICE HERE &&**&&**&&**" << std::endl;
		continue;
	}
	if (!super::TrySendOutInterest (inFace, metricFace.GetFace (), interest, pitEntry))
	  {
	    continue;
	  }
	//std::cout<<" ("<<metricFace.GetRank()<<", "<<metricFace.GetRoutingCost()<<") ";
	propagatedCount++;
	break; // do only once
      }
      //std::cout<<"\t ndn-dcr-fw.cc 167 \n";
      NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
      return propagatedCount > 0;
    }
  }else{
    return super::DoPropagateInterest(inFace, interest, pitEntry);
  }
return false;


}

template<class Parent>
uint32_t
DcrFw<Parent>::GetDestId(Ptr<const Interest> interest){
  uint32_t temp = static_cast<uint32_t> (interest->GetName().getSubName(1,1).get(0).toSeqNum());
  return temp; //static_cast<uint32_t> (interest->GetName().getSubName(2,1).get(0).toNumber());
}

template<class Parent>
void
DcrFw<Parent>::PrintStats(){
  std::cout << "Router Stats: \n";
  std::cout << "num Of  Hello Msgs: " << DcrFw<Parent>::m_numOfHelloMsgs << '\n';
  std::cout << "num Of  Update Msgs: " << DcrFw<Parent>::m_numOfUpdateMsgs << '\n';
  std::cout << "num Of  Inf Msgs: " << DcrFw<Parent>::m_numOfInfMsgs << '\n';
}

template<class Parent> uint32_t DcrFw<Parent>::m_numOfHelloMsgs = 0;
template<class Parent> uint32_t DcrFw<Parent>::m_numOfUpdateMsgs = 0;
template<class Parent> uint32_t DcrFw<Parent>::m_numOfInfMsgs = 0;

} // namespace fw
} // namespace ndn
} // namespace ns3




#endif /* DCR_FW_H_ */
