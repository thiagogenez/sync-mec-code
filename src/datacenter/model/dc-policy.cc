/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"

#include "ns3/log.h"

#include "ns3/node-list.h"

#include "dc-policy.h"
#include "ns3/random-variable-stream.h"





NS_LOG_COMPONENT_DEFINE ("DcPolicy");

namespace ns3 {

/* ... */
NS_OBJECT_ENSURE_REGISTERED (DcMiddleBox);

/*TypeId
DcPolicy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcPolicy")
    .SetParent<Object> ()
    .AddConstructor<DcPolicy> ()
  ;
  return tid;
}
*/

DcPolicy::DcPolicy ()
{
	m_maxID = 0;
}

DcPolicy::~DcPolicy ()
{
}



PolicyItemIterator
DcPolicy::Begin (void)// const
{
  	return m_policyList.begin ();
}

PolicyItemIterator 
DcPolicy::End (void)// const
{
  	return m_policyList.end ();
}

uint32_t 
DcPolicy::GetN (void) const
{
  	return m_policyList.size ();
}

PolicyItemIterator 
DcPolicy::Get (uint32_t i)// const
{
	PolicyItemIterator it = (PolicyItemIterator)&m_policyList[i];
	NS_ASSERT_MSG(it->id == i, "ERROR: id error for policy, id = "<<it->id<<", index="<<i);
	return it;
}

//void 
//DcPolicy::Add (PolicyItem p)
//{
// 	m_policyList.push_back (p);
//}

PolicyItemIterator 
DcPolicy::createPolicyEntry(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport)
{
		PolicyItem p;
		p.id = m_maxID++;
		p.len = 0;
		p.saddr = saddr;
		p.daddr = daddr;
		p.sport = sport;
		p.dport = dport;
		p.proto = proto;
		//flowid fid = flowid(saddr, daddr, proto, sport, dport);
		//p.fid = fid;

		p.isOK = true;
		
		m_policyList.push_back(p);

		return Get(p.id);
}


//generate random policy seq and mb list
//when generating mb list, use all matched flows to determin the remaining capacity of each mb
uint32_t 
DcPolicy::generateRandomPolicy(PolicyItemIterator it, MiddleBoxContainer mbc, Ptr<DcFatTreeHelper> fattree, DcFlowContainer fc)
{
	Ptr<UniformRandomVariable> m_uniformRand = CreateObject<UniformRandomVariable> ();

	Ptr<DcFlowApplication> flow = fc.Get(it->flowID);
	
	//Genterate Length
	uint32_t len = m_uniformRand->GetInteger(1,3);	
	if(len != 1)//increase the possiblity of len=1
	{
		len = m_uniformRand->GetInteger(1,3);
	}
	it->len = len;

	//Genterate sequence
	int list[5];
	for(uint8_t i=0;i<len;i++)
	{
		bool ok = false;
		do {
		//std::cout<<"Generate MB type "<<(uint16_t)i<<" MB_MAX="<<MB_MAX<<std::endl;
			ok = true;
			list[i] = m_uniformRand->GetInteger(0,MB_MAX-1);
			for(int j=0;j<i;j++)
			{
				if (list[j]==list[i])
				{
					ok = false;
					break;
				}
			}			
		}while(!ok);
		it->Seq[i] = MiddleboxType(list[i]);
	}

	//std::cout<<"Generate policy len="<<len<<", seq={"<<it->Seq[0]<<","<<it->Seq[1]<<","<<it->Seq[2]<<","<<it->Seq[3]<<"}"<<std::endl;

	//std::cout<<"Generate Seq complete, now MB list"<<std::endl;

	it->mbList.ClearAll();

	//Generate MB list
	for(uint8_t i=0;i<len;i++)
	{
		MiddleboxType type = it->Seq[i];
		MiddleBoxContainer candidateMBs = mbc.getMiddleBoxByType(type);
		
		//std::cout<<"Get candidate MBs for type="<<type<<":"<<std::endl;
		//candidateMBs.print();

		uint32_t nodePrevious;
		uint32_t nodeNext;
		if(i==0)
		{			
			nodePrevious = flow->m_srcNodeID;
		}
		else
		{
			Ptr<DcMiddleBox> mbPrevious = it->mbList.Get(i-1);
			nodePrevious = mbPrevious->m_nodeID;
		}
		if(i==len-1)
		{			
			nodeNext = flow->m_dstNodeID;
		}
		
		Ptr<DcMiddleBox> mbCandidate;
		bool ok = false;
		do{
			ok = false;
			uint32_t index = m_uniformRand->GetInteger(0,candidateMBs.GetN()-1);
			mbCandidate = candidateMBs.Get(index);

			//std::cout<<"Try MB id="<<mbCandidate->m_id<<" on Node "<<mbCandidate->m_nodeID<<std::endl;
			
			//check capacity
			if(mbCandidate->isFlowAcceptable(flow))
			{
				//continue;				
				ok = true;
			}

			//Check connectivity
			//std::cout<<"Check "<<nodePrevious<<" and "<<mbCandidate->m_nodeID;
			if(ok && (fattree->checkConnectivity(nodePrevious,mbCandidate->m_nodeID)))
			{
				//if unreachable, continue to next
				//std::cout<<"............fail\n";
				//continue;
				ok = true;
			}
			else
				ok = false;
			//std::cout<<"............OK\n";

			if(i==len-1)
			{
				if(ok && (fattree->checkConnectivity(mbCandidate->m_nodeID, nodeNext)))
				{
					//if unreachable, continue to next
					//continue;
					ok = true;
				}
				else
					ok = false;
			}

			if(ok)
			{					
				//std::cout<<"Check index="<<index<<" MB id="<<mbCandidate->m_id<<std::endl;
				for(MiddleBoxContainer::Iterator itM=it->mbList.Begin();itM<it->mbList.End();itM++)
				{
					Ptr<DcMiddleBox> mb = *itM;
					if(mbCandidate->m_id==mb->m_id)
					{
						ok = false;
						break;
					}
				}
			}
		}while(!ok);
		
		//std::cout<<"Choose MB="<<mbCandidate->m_id<<std::endl;		
		
		it->mbList.Add(mbCandidate);
		mbCandidate->flowAssign(flow);
	}

	std::cout<<"Generate MB list complete"<<std::endl;
	//printPolicy(it);

	return len;
}

bool
DcPolicy::checkPolicy(PolicyItemIterator itP)
{
	for(uint32_t i=0;i<itP->len;i++)
	{
		Ptr<DcMiddleBox> mb = itP->mbList.Get(i);
		NS_ASSERT_MSG(mb->m_type==itP->Seq[i],
						"Policy ERROR: at index="<<i<<", Seq="<<itP->Seq[i]<<" while mblist[]="<<mb->m_type);
		//if(mb->m_type==itP->Seq[i])
		//{
		//	return false;
		//}
	}
	return true;
}

void 
DcPolicy::SetMiddleBoxList (PolicyItemIterator item, MiddleBoxContainer mbc)
{
	NS_ASSERT_MSG(item->len==mbc.GetN(),
		"ERROR: size of sequence and list of MBs is unequal for policy id="<<item->id);
	item->mbList = mbc;
}	

bool 
DcPolicy::isValid(PolicyItemIterator it)
{
	if(it>=m_policyList.begin() && it<m_policyList.end())
		return true;
	else
		return false;
}
bool 
DcPolicy::isOK(PolicyItemIterator it)
{
	if(!isValid(it))
	{
		return false;
	}
	if(!it->isOK)
	{
		//std::cout<<"it is an abondoned policy"<<std::endl;
		return false;
	}
	return true;
}


PolicyItemIterator 
DcPolicy::search(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport)
{
	PolicyItemIterator it;
	for(it=m_policyList.begin(); it!=m_policyList.end(); it++)
	{
		//std::cout<<"look for flow {src="<<(Ipv4Address(saddr))<<", daddr="<<Ipv4Address(daddr)<<", proto="<<(uint32_t)proto<<",sport="<<sport<<",dport="<<dport
		//		<<", get check flowflow {src="<<Ipv4Address(it->saddr)<<", daddr="<<Ipv4Address(it->daddr)<<", proto="<<(uint32_t)it->proto<<",sport="<<it->sport<<",dport="<<it->dport<<std::endl;
		//if(saddr==it->saddr && daddr==it->daddr && proto==it->proto
		//	&& sport==it->sport && dport==it->dport)
		//{
		//	return it;
		//}

		if(saddr==it->saddr && daddr==it->daddr && proto==it->proto
			&& (sport==it->sport||it->sport==0)
			&& (dport==it->dport||it->sport==0))
		{
			return it;
		}
	}
	return it;
}

PolicyItemIterator 
DcPolicy::getPolicyByFlow(Ptr<DcFlowApplication> flow)
{
	Ptr<Node> srcNode = NodeList::GetNode(flow->m_srcNodeID);
	Ptr<Node> dstNode = NodeList::GetNode(flow->m_dstNodeID);
	Ipv4Address saddr = srcNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	Ipv4Address daddr = dstNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

	uint8_t proto = flow->useTCP ? POLICY_TCP_PROTO : POLICY_UDP_PROTO;
	
	uint16_t sport = flow->m_srcPort;
	uint16_t dport = flow->m_dstPort;

	PolicyItemIterator itP = search(saddr.Get(),daddr.Get(),proto,sport,dport);

	return itP;
	
	//if(isOK(itP))
	//	return itP;
	//else 
	//	return End(); //if no valid policy, return the end
			
}

void 
DcPolicy::migratePolicy(PolicyItemIterator itPolicy, uint32_t seqIndex, Ptr<DcMiddleBox> newMB, DcFlowContainer fc)
{
	NS_ASSERT_MSG(seqIndex<itPolicy->len, "Error: index error in policy migration");
	NS_ASSERT_MSG(itPolicy->Seq[seqIndex]==newMB->m_type, "Error: type error in policy migration");

	Ptr<DcMiddleBox> oldMB = itPolicy->mbList.Get(seqIndex);

	//if the same MB, no migration
	NS_ASSERT_MSG(oldMB->m_id!=newMB->m_id,"ERROR: migrating the same MB to itself");
		
	itPolicy->mbList.setMB(seqIndex, newMB);

	//set MB capacity
	DcFlowContainer matchedFlows = getMatchedFlow(itPolicy,fc);
	for(uint32_t i=0;i<matchedFlows.GetN();i++)
	{
		Ptr<DcFlowApplication> flow = matchedFlows.Get(i);
		std::cout<<"CHECK: Policy Migration, flowrate="<<flow->m_dataRate.GetBitRate()/1000<<std::endl;		
		oldMB->flowRelease(flow);
		newMB->flowAssign(flow);
	}
	return;
}

DcFlowContainer DcPolicy::getMatchedFlow(PolicyItemIterator itPolicy, DcFlowContainer fc)
{
	DcFlowContainer matchedFlows;

	//printPolicy(itPolicy);
	
	for(uint32_t i=0;i<fc.GetN();i++)
	{
		Ptr<DcFlowApplication> flow = fc.Get(i);
		
		Ptr<Node> srcNode = NodeList::GetNode(flow->m_srcNodeID);
		Ptr<Node> dstNode = NodeList::GetNode(flow->m_dstNodeID);
		Ipv4Address saddr = srcNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		Ipv4Address daddr = dstNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	
		uint8_t proto = flow->useTCP ? POLICY_TCP_PROTO : POLICY_UDP_PROTO;
		
		uint16_t sport = flow->m_srcPort;
		uint16_t dport = flow->m_dstPort;

		//check if match
		if(saddr.Get()==itPolicy->saddr && daddr.Get()==itPolicy->daddr && proto==itPolicy->proto
			&& (sport==itPolicy->sport||itPolicy->sport==0)
			&& (dport==itPolicy->dport||itPolicy->sport==0))
		{
			matchedFlows.Add(flow);
			//std::cout<<"Match a flow: ";
			//flow->printFlow( std::cout);
		}
	}
	
	return matchedFlows;
}

//after creating flow and policies, call this function to initial setting remaining capacity of MBs
void DcPolicy::initialMiddleboxCapacity(DcFlowContainer fc)
{
	for(uint32_t i=0;i<fc.GetN();i++)
	{
		Ptr<DcFlowApplication> flow = fc.Get(i);

		Ptr<Node> srcNode = NodeList::GetNode(flow->m_srcNodeID);
		Ptr<Node> dstNode = NodeList::GetNode(flow->m_dstNodeID);
		Ipv4Address saddr = srcNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		Ipv4Address daddr = dstNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	
		uint8_t proto = flow->useTCP ? POLICY_TCP_PROTO : POLICY_UDP_PROTO;
		
		uint16_t sport = flow->m_srcPort;
		uint16_t dport = flow->m_dstPort;

		PolicyItemIterator itP = search(saddr.Get(),daddr.Get(),proto,sport,dport);

		if(isOK(itP))
		{
			for(uint32_t j=0;j<(uint32_t)itP->len;j++)
			{
				Ptr<DcMiddleBox> mb = itP->mbList.Get(j);
				mb->flowAssign(flow);
				
				NS_ASSERT_MSG(mb->m_capacityRemain>=0,"Error: middlbebox overflowed...");
			}
		}
		
	}
}

//check whether the capacity is OK for a MB
//bool DcPolicy::checkMiddleboxCapacity(Ptr<DcMiddleBox> mb, DcFlowContainer fc)
//{
//	
//	return true;
//}


void DcPolicy::printPolicy(PolicyItemIterator it)
{
	std::cout<<"Policy "<<it->id<<":["<<Ipv4Address(it->saddr)<<":"<<it->sport<<"]-->"
		<<"["<<Ipv4Address(it->daddr)<<":"<<it->dport<<"], "
		<<(it->proto==POLICY_TCP_PROTO?"TCP":"UDP")
		<<", isOK:"<<(it->isOK? "true":"false")
		<<", Seq: {";
	for(uint8_t i=0;i<it->len;i++)
	{
		std::cout<<"["<<(int)it->Seq[i]<<"]";
	}
		
	std::cout<<", MB list:{ ";
	for(uint8_t i=0;i<it->len;i++)
	{
		std::cout<<"["<<it->mbList.Get(i)->m_id<<"]";
	}
		
	std::cout<<" }"<<std::endl;
}

//set the last percentate% policies  to the status
void DcPolicy::setPolicyStatus(uint32_t percentate, bool status)
{
	uint32_t startIndex = GetN()*(100-percentate)/100+1;
	for(uint32_t i=startIndex;i<GetN();i++)
	{
		PolicyItemIterator it = Get(i);
		it->isOK = status;
	}
}


}

