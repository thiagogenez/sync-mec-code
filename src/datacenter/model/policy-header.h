/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef POLICY_HEADER_H
#define POLICY_HEADER_H

//#include "ns3/random-variable-stream.h"


#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"

//#include "ns3/ipv6-address.h"

#include "ns3/core-module.h"

#include "ns3/network-module.h"

//#include "ns3/fivetuple.h"




namespace ns3 {
/**
 * \ingroup Policy
 * \brief Packet header for Policy packets
 */

#define POLICY_HEADER_TYPE 0x68//68
#define POLICY_PORT 1001
//#define POLICY_HEADER_MB_LIST_LEN 10

//#define POLICY_UDP_PROTO	17
//#define POLICY_TCP_PROTO	6


/*
typedef struct PolicyItem{
	// each flow identifier
	flowid	 fid;
	uint32_t saddr;
	uint32_t daddr;
	uint8_t  proto;
	uint16_t sport;
	uint16_t dport;

	//middlebox list
	uint8_t len;
	uint8_t m_MB_list[4]; // including 4 middleboxes in maximum

	bool isOK;
}PolicyItem;

typedef std::vector<PolicyItem>::iterator PolicyItemI;


typedef struct MiddleBoxItem{
	uint16_t id;
	MiddleboxType type;
	uint32_t nodeID; // the node ID that this MB is attached to
	Ipv4Address m_address;
}MiddleBoxItem;

typedef std::vector<MiddleBoxItem>::iterator MiddleBoxItemI;
*/

/*
class DcMiddleBoxNew
{
private:
	std::vector<MiddleBoxItem> m_MB_list;  //list of all MB
	uint16_t idSeed;  //seed for generating MB id

public:
	DcMiddleBoxNew ()
	{
		idSeed = 1;
	};
	~DcMiddleBoxNew ()
	{
	};

	uint8_t getSize()
	{
		//NS_ASSERT_MSG(m_MB_list.size()==idSeed-1, "MB Size Error");
		return m_MB_list.size();
	};

	void attachMiddleBox(Ptr<Node> node)
	{
		MiddleBoxItem mb;
		mb.id = idSeed++;
		mb.nodeID = node->GetId();

		int num = node->GetNDevices();
		if(num>0){
			Ipv4Address Addr = node->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
			mb.m_address = Addr;
			
			m_MB_list.push_back(mb);
			
			Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
			std::cout<<"Address of MB "<<(int)mb.id<<":"<<Addr<<", on node:"<<mb.nodeID<<std::endl;

			
			//for (uint32_t j = 0; j < m_ipv4->GetNInterfaces(); j++) {
			//	for (uint32_t i = 0; i < m_ipv4->GetNAddresses (j); i++) {
			//		 std::cout<<"Local Address("<<j<<","<<i<<") "<<m_ipv4->GetAddress (j, i).GetLocal()<<std::endl;
			//	}
			//}
			
		}
		else
		{
			std::cout<<"Attaching MB ERROR, MB id:"<<mb.id<<", NO device on node:"<<mb.nodeID<<std::endl;
		}
	
	};

	MiddleBoxItemI getMiddleBox(uint16_t id)
	{
		MiddleBoxItemI it;
		for(it=m_MB_list.begin(); it!=m_MB_list.end(); it++)
		{
			if(id==it->id)
			{
				return it;
			}
		}
		return it;
	};

	MiddleBoxItemI getMiddleBoxList(MiddleboxType type)
	{
		MiddleBoxItemI it;
		for(it=m_MB_list.begin(); it!=m_MB_list.end(); it++)
		{
			if(type==it->type)
			{
				return it;
			}
		}
		return it;
	};

	bool isValid(MiddleBoxItemI it)
	{
		if(it>=m_MB_list.begin() && it<m_MB_list.end())
			return true;
		else
			return false;
	};

	uint32_t generateRandomMiddleBoxList(uint16_t* list)
	{
		Ptr<UniformRandomVariable> m_uniformRand = CreateObject<UniformRandomVariable> ();
		uint32_t len = m_uniformRand->GetInteger(1,3);
		
		uint16_t sizeMB = getSize();

		
		//if(len==2)//reduce the probability of len=2
		//{
		//	len = RandomVariable(UniformVariable(1, 3)).GetInteger();
		//}
		
			
		list[0] = m_uniformRand->GetInteger(1,sizeMB);
		if(len==2)
		{
			do {
				list[1] = m_uniformRand->GetInteger(1,sizeMB);
			}while(list[1]==list[0]);
		}
		return len;
	}
	
};

class DcPolicyOLD
{
private:
	std::vector<PolicyItem> m_policies;

	//std::vector<PolicyItem> m_policies_backup;

	PolicyItemI _search(flowid fid)
	{
		PolicyItemI it;
		for(it=m_policies.begin(); it!=m_policies.end(); it++)
		{
			//std::cout<<"look for flow:"<<fid<<", get check flow"<<it->fid<<std::endl;
			//if(fid==it->fid)
			if(fid==it->fid)
			{
				return it;
			}
		}
		return it;
	};
	PolicyItemI _search(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport)
	{
		PolicyItemI it;
		for(it=m_policies.begin(); it!=m_policies.end(); it++)
		{
			//std::cout<<"look for flow:"<<fid<<", get check flow"<<it->fid<<std::endl;
			if(saddr==it->saddr && daddr==it->daddr && proto==it->proto
				&& sport==it->sport && dport==it->dport)
			{
				return it;
			}
		}
		return it;
	};

public:

	DcPolicyOLD ()
	{
	
	};
	DcPolicyOLD ()
	{
	};

	PolicyItemI createPolicyEntry(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport)
	{
		PolicyItem p;
		p.len = 0;
		p.saddr = saddr;
		p.daddr = daddr;
		p.sport = sport;
		p.dport = dport;
		p.proto = proto;
		flowid fid = flowid(saddr, daddr, proto, sport, dport);
		p.fid = fid;

		p.isOK = true;
		
		m_policies.push_back(p);

		return lookup(fid);
	};

	void SetMiddleBoxList (flowid& fid, uint8_t * list, uint8_t len)
	{
		PolicyItemI item = lookup(fid);

		SetMiddleBoxList(item,list,len);
		//item->len = len;
		//for(int i=0;i<len;i++)
	    //{
	    //    item->m_MB_list[i] = list[i];
	    //}
	};

	void SetMiddleBoxList (PolicyItemI item, uint8_t * list, uint8_t len)
	{
		//PolicyItemI item = lookup(fid);
		
		item->len = len;
		for(int i=0;i<len;i++)
	    {
	    	//int x=list[i];
	    	//std::cout<<"x:"<<x<<",list[i]:"<<(int)list[i]<<",len:"<<(int)len<<std::endl;
	        item->m_MB_list[i] = list[i];
	    }
	};
	
	PolicyItemI lookup(flowid fid)
	{
		PolicyItemI it;
		
		it = _search(fid);

		// check if this policy is ok
		//if(!it->isOK)
		//	it = m_policies.end()+1;
		
		if(isValid(it))
		{
			return it;
		}

		return it;
		
	};

	PolicyItemI lookup(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport)
	{
	
		//flowid fid = flowid(saddr, daddr, proto, sport, dport);
		//return lookup(fid);
		
		PolicyItemI it = _search(saddr, daddr, proto, sport, dport);

		// check if this policy is ok
		//if(!it->isOK)
		//	it = m_policies.end()+1;
		
		return it;
	};

	bool isValid(PolicyItemI it)
	{
		if(it>=m_policies.begin() && it<m_policies.end())
			return true;
		else
			return false;
	};

	bool isOK(PolicyItemI it)
	{
		//std::cout<<"Check if policy is OK"<<std::endl;
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
	};

	uint8_t getFirstMB(PolicyItemI it)
	{
		NS_ASSERT_MSG(it->len>0,"Policy error");
		return it->m_MB_list[0];
	};
	uint8_t getLastMB(PolicyItemI it)
	{
		NS_ASSERT_MSG(it->len>0,"Policy error");
		return it->m_MB_list[it->len-1];
	};

	void setFirstNPolicies(uint32_t num, bool status)
	{
		//move the first N policies
		//if(num<=0)
			num = m_policies.size()/10;

		std::cout<<"Set first "<<num<<" policies to be "<<(status?"true":"false")<<std::endl;
		
		PolicyItemI it;
		for(it=m_policies.begin(); it<m_policies.begin()+num; it++)
		{
			it->isOK = status;
			//std::cout<<"Set as not OK:";
		}
		//m_policies.erase(m_policies.begin(), m_policies.begin()+num);
	};

	void disableLastNPolicies()
	{
		uint32_t num = m_policies.size()/10;

		PolicyItemI it;
		for(it=m_policies.end()-num; it!=m_policies.end(); it++)
		{
			it->isOK = false;
		}
		//m_policies.erase(m_policies.begin(), m_policies.begin()+num);
	};

	void resetMBList(uint32_t num, DcMiddleBoxNew * middleboxes)
	{
		//move the first N policies
		//if(num<=0)
			num = m_policies.size()/10;
		
		PolicyItemI it;
		for(it=m_policies.begin(); it<m_policies.begin()+num; it++)
		{
			uint8_t mblist[10];// = {1,1};
			uint8_t len = middleboxes->generateRandomMiddleBoxList(mblist);
			SetMiddleBoxList(it,mblist,len);				
		}
		//m_policies.erase(m_policies.begin(), m_policies.begin()+num);
	};

	void printPolicies()
	{
		std::cout<<"All Policies:"<<std::endl;
		for(PolicyItemI it=m_policies.begin(); it!=m_policies.end(); it++)
		{
			printPolicy(it);
		}
	};
	void printPolicy(PolicyItemI it)
	{
		std::cout<<"["<<Ipv4Address(it->saddr)<<":"<<it->sport<<"]-->"
			<<"["<<Ipv4Address(it->daddr)<<":"<<it->dport<<"], "
			<<(it->proto==POLICY_TCP_PROTO?"TCP":"UDP")
			<<", isOK:"<<(it->isOK? "true":"false")
			<<", MB list:{ ";
		for(int i=0;i<it->len;i++)
		{
			std::cout<<"["<<(int)it->m_MB_list[i]<<"]";
		}
			
		std::cout<<" }"<<std::endl;
	};

	void testScene()
	{
		Ptr<Node> n1 = NodeList::GetNode(20);
		Ptr<Node> n2 = NodeList::GetNode(21);
		Ipv4Address addr1 = n1->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		Ipv4Address addr2 = n2->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		PolicyItemI it = createPolicyEntry(addr1.Get(),addr2.Get(),POLICY_UDP_PROTO,0,1001);
		if(isValid(it))
		{
			uint8_t mblist[] = {1,1};
			SetMiddleBoxList(it,mblist,2);	
			printPolicies();
		}
		else
			std::cout<<"creat policy failed\n";

	};

};

*/


class PolicyHeader : public Header 
{
public:

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  PolicyHeader ();
  ~PolicyHeader ();



  //void SetLength (uint8_t len);
  //uint8_t GetLength (void) const;

  //void SetMiddleBoxList (uint8_t * list, uint8_t len);
  //uint8_t * GetMiddleBoxList (void);
  //uint8_t ReadNextMiddleBox (void);
  //uint8_t ReadCurrentMiddleBox (void);

  bool IsPolicyHeader();
  void SetPolicyID (uint16_t id);
  uint16_t GetPolicyID (void) const;
  void SetIndex (uint8_t i);
  uint8_t GetIndex (void) const;

  
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);


private:

  uint8_t m_type; //const 0x68 0x68
  uint8_t m_index;
  uint16_t m_policyID;
  
};

} // namespace ns3

#endif /* Policy_HEADER */
