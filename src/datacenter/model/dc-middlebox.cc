/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"

#include "ns3/log.h"

#include "dc-middlebox.h"

NS_LOG_COMPONENT_DEFINE ("DcMiddleBox");

namespace ns3 {

/* ... */
NS_OBJECT_ENSURE_REGISTERED (DcMiddleBox);

TypeId
DcMiddleBox::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcMiddleBox")
    .SetParent<Object> ()
    .AddConstructor<DcMiddleBox> ()
  ;
  return tid;
}


DcMiddleBox::DcMiddleBox ()//uint16_t id, MiddleboxType type
{
  //NS_LOG_FUNCTION (this);
  	//m_id = id;
  	//m_type = type;
  	//m_capacityUsed = 0;
}

DcMiddleBox::~DcMiddleBox ()
{
  //std::cout<<"dead of MB\n";
  //NS_LOG_FUNCTION (this<<"dead of MB\n");
}

void 
DcMiddleBox::attachMiddleBox(Ptr<Node> node)
{
	//MiddleBoxItem mb;
	//mb.id = idSeed++;
	m_nodeID= node->GetId();

	int num = node->GetNDevices();
	
	NS_ASSERT_MSG(num>0,"Attaching MB ERROR, MB id:"<<m_id<<", NO device on node:"<<m_nodeID);
	
	Ipv4Address Addr = node->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	m_address = Addr;

	//std::cout<<"Address of MB "<<(int)m_id<<":"<<Addr<<", on node:"<<m_nodeID<<std::endl;

	/*
	for (uint32_t j = 0; j < m_ipv4->GetNInterfaces(); j++) {
	for (uint32_t i = 0; i < m_ipv4->GetNAddresses (j); i++) {
	std::cout<<"Local Address("<<j<<","<<i<<") "<<m_ipv4->GetAddress (j, i).GetLocal()<<std::endl;
	}
	}
	*/


}

/*
bool 
DcMiddleBox::getLocalNodeAddress()
{
	Ptr<Node> node = GetNode();
	int num = node->GetNDevices();
	if(num>0){
		//m_address = node->GetDevice(0)->GetAddress();
		//Ipv4Address::ConvertFrom(m_address)
		//uint32_t nextIf = node->GetDevice(0)->GetIfIndex();
		Ipv4Address Addr = node->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		//Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		m_address = Addr;
		std::cout<<"Address of MB "<<m_id<<":"<<Addr<<std::endl;
		return true;
	}
	else
	{
		m_address = Address(); //this will create an invalid address
		NS_LOG_INFO (this<<"invalid Address");
		return false;
	}
}
*/

//assign MB to flow, reduce the capacity
void DcMiddleBox::flowAssign(Ptr<DcFlowApplication> flow)
{
	m_capacityRemain -= flow->m_dataRate.GetBitRate()/1000; //kbps
}

//release MB from a flow, resume the capacity
void DcMiddleBox::flowRelease(Ptr<DcFlowApplication> flow)
{
	m_capacityRemain += flow->m_dataRate.GetBitRate()/1000; //kbps
}

//check whether the MB has enough capacity to acceptable a flow
bool DcMiddleBox::isFlowAcceptable(Ptr<DcFlowApplication> flow)
{
	if (m_capacityRemain - flow->m_dataRate.GetBitRate()/1000 >= 0)
		return true;
	else
		return false;
}


MiddleBoxContainer::MiddleBoxContainer ()
{
}

MiddleBoxContainer::Iterator 
MiddleBoxContainer::Begin (void) const
{
  return m_mbList.begin ();
}

MiddleBoxContainer::Iterator 
MiddleBoxContainer::End (void) const
{
  return m_mbList.end ();
}

uint32_t 
MiddleBoxContainer::GetN (void) const
{
  return m_mbList.size ();
}

Ptr<DcMiddleBox> 
MiddleBoxContainer::Get (uint32_t i) const
{
  return m_mbList[i];
}

void 
MiddleBoxContainer::Add (Ptr<DcMiddleBox> mb)
{
	m_mbList.push_back (mb);
}

void MiddleBoxContainer::setMB (uint32_t index, Ptr<DcMiddleBox> newMB)
{
	m_mbList[index] = newMB;
}

MiddleBoxContainer 
MiddleBoxContainer::getMiddleBoxByType(MiddleboxType type)
{
	MiddleBoxContainer mbc;
	for(MiddleBoxContainer::Iterator it = m_mbList.begin(); it!=m_mbList.end(); it++)
	{
		Ptr<DcMiddleBox> mb = *it;
		//std::cout<<"Type:"<<it->m_type<<std::endl;
		if(type==mb->m_type)
		{
			mbc.Add(mb);
		}
	}
	return mbc;
}




/*
* Create n MiddleBoxes, id incrased from 1
* 
*/
void 
MiddleBoxContainer::Create (uint32_t n)
{
	//uint16_t id = 1;
	for (uint32_t i = 0; i < n; i++)
	{
		Ptr<DcMiddleBox> mb = CreateObject<DcMiddleBox> ();
		mb->m_id = i;//id++;

		//MiddleboxType type;
		//switch()
		mb->m_type = (MiddleboxType)(i%MB_MAX);//() i%MB_MAX;  //generate each type evenly

		DataRate rate = DataRate ("100Mbps");// default 100Mbps, related to the link capacity?
		mb->m_capacity = rate.GetBitRate()/1000;//kbps
		mb->m_capacityRemain = mb->m_capacity;
		
		m_mbList.push_back (mb);
	}
}


void MiddleBoxContainer::Remove(Ptr<DcMiddleBox> mb)
{
	//DcVmContainer::Iterator it = GetVmByID(vm->m_id);
	//NS_ASSERT_MSG(it!=End(),"Can not find Vm in current server");

	std::vector< Ptr<DcMiddleBox> >::iterator it;
	for(it = m_mbList.begin(); it!=m_mbList.end(); it++)
	{
		Ptr<DcMiddleBox> mbTemp = *it;
		if(mbTemp->m_id==mb->m_id)
		{
			m_mbList.erase(it);
			return;
		}			
	}
	std::cout<<"Can not find MB in current container"<<std::endl;
}

void MiddleBoxContainer::ClearAll()
{
	m_mbList.clear();
}

void
MiddleBoxContainer::print()
{
	std::cout<<"Total: "<<GetN()<<" MBs"<<std::endl;
	for(MiddleBoxContainer::Iterator it = m_mbList.begin(); it!=m_mbList.end(); it++)
	{
		Ptr<DcMiddleBox> mb = *it;
		//Ptr<DcMiddleBox> mb = Get(0);
		//std::cout<<"MB "<<mb->m_id<<" : type=";
		std::cout<<"MB "<<mb->m_id<<" : type="<<mb->m_type<<", Address="<<mb->m_address<<" (Node: "<<mb->m_nodeID<<" )"<<std::endl;
	}
}



}

