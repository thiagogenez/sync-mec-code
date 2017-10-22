
#include <stdint.h>
#include <string>
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"

//#include "ns3/dc-flow.h"
#include "ns3/dc-vm-hypervisor.h"

#include "ns3/dc-vm.h"

//#include "ns3/scene_generator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DcVm);

TypeId
DcVm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcVm")
    .SetParent<Object> ()
    .AddConstructor<DcVm> ()
  ;
  return tid;
}

DcVm::DcVm ()
{
	m_uniformRand = CreateObject<UniformRandomVariable> ();
	
	//vmSeedID = 1;
	//flowSeedID = 1;

	tcpPortSeed = 1000;
	udpPortSeed = 1000;
}

DcVm::~DcVm ()
{
}

bool DcVm::startVM(bool allFlow)
{
	for (DcFlowContainer::Iterator it=m_flowList.Begin(); it!=m_flowList.End(); ++it)
	{
		Ptr<DcFlowApplication> flow = *it;

		//if(!flow->isOK)
		//	continue;

		if(!allFlow && flow->m_srcVmID!=m_id) //Only start flows from local VM
			continue;

		//NS_ASSERT_MSG(flow->m_srcNodeID==m_hostServer,"flow srcNode is not VM hostsrver");
		
		//flow->printFlow(std::cout);
		
		flow->StartApplication();
		
		//std::cout << "start flow ";

	}
	isStart = true;

	//Time nexTime = Time(MilliSeconds(m_uniformRand->GetInteger(1000, 4000)));
	//itVM->nextTime = Simulator::Now() + nexTime;
		
    std::cout << "VM " << m_id << " is started\n";
    return true;
}

bool DcVm::stopVM()
{
	for (DcFlowContainer::Iterator it=m_flowList.Begin(); it!=m_flowList.End(); ++it)
	{
		Ptr<DcFlowApplication> flow = *it;
		//Will stop flows both from and to local VM
		flow->StopApplication();
	}
	isStart = false;
    std::cout << "VM " << m_id << " is stopped\n";

    return true;
}

//update src/dst address of all flow related to vmID
//also need to update policies
//update server resources

bool DcVm::MigrateToServer(uint32_t newNodeID, DcPolicy *policies, bool allowTempOverload)
{
	NS_ASSERT_MSG(!isStart, "ERROR: Trying to migrate a running VM, vmID="<<m_id);

	//update all related flows and policies
	Ipv4Address ipv4addr = NodeList::GetNode(newNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	uint32_t new_addr = ipv4addr.Get();
	for (DcFlowContainer::Iterator it=m_flowList.Begin(); it!=m_flowList.End(); ++it)
	{
		Ptr<DcFlowApplication> flow = *it;

		//if(!flow->isOK)
		//	continue;
		
		uint8_t proto = flow->useTCP?6:17;
		ipv4addr = NodeList::GetNode(flow->m_srcNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		uint32_t saddr = ipv4addr.Get();
		ipv4addr = NodeList::GetNode(flow->m_dstNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		uint32_t daddr = ipv4addr.Get();
		uint16_t sport = flow->m_srcPort;
		uint16_t dport = flow->m_dstPort;

		PolicyItemIterator itPolicy = policies->search(saddr,daddr,proto,sport,dport);

		NS_ASSERT_MSG(policies->isValid(itPolicy),"policy failed to flow");
		
		//update flow and policy address
		if(flow->m_srcVmID==m_id)//update the src of a policy and flow
		{
			itPolicy->saddr = new_addr;
			flow->m_srcNodeID = newNodeID;
		}
		else if(flow->m_dstVmID==m_id)//update the dst of a policy and flow
		{
			itPolicy->daddr = new_addr;
			flow->m_dstNodeID = newNodeID;
		}
	}

	//updating both src and dst server resources
	Ptr<DcVmHypervisorApplication> srcHvapp = getHyperVisor(m_hostServer);
	Ptr<DcVmHypervisorApplication> dstHvapp = getHyperVisor(newNodeID);
	dstHvapp->m_useCPU += m_cpu;
	dstHvapp->m_useMemory += m_memory;
	srcHvapp->m_useCPU -= m_cpu;
	srcHvapp->m_useMemory -= m_memory;

	if(dstHvapp->m_useCPU<=dstHvapp->m_totalCPU && dstHvapp->m_useMemory<=dstHvapp->m_totalMemory)
	{
		dstHvapp->m_overloaded = false;
	}
	else if(allowTempOverload)
	{
		dstHvapp->m_overloaded = true;
	}
	else
	{
		NS_ASSERT_MSG(dstHvapp->m_useCPU<=dstHvapp->m_totalCPU && dstHvapp->m_useMemory<=dstHvapp->m_totalMemory,
			"Trying to migrate a VM to a server without enough resource, vm id="<<m_id
				<<", from server "<<m_hostServer<<" to server "<<newNodeID);
	}
	
	//update VM hostserver
	m_hostServer = newNodeID;
	
	return true;
}

uint64_t 
DcVm::CalcMigrationTraffic()
{
	//std::cout<<", itVM->liveMig_VmSize:"<<itVM->liveMig_VmSize<<std::endl;

	uint32_t numCycle = 0;
	uint64_t totalTraffic = 0;

	uint32_t numCondition1 = ceil(log((1.0*liveMig_T*liveMig_L)/liveMig_VmSize)
		/log(1.0*liveMig_DirtyRate/liveMig_L));
	uint32_t numCondition2 = ceil(log((liveMig_X*liveMig_DirtyRate)/(liveMig_VmSize*(liveMig_L-liveMig_DirtyRate)))
		/log(1.0*liveMig_DirtyRate/liveMig_L));
	if(numCondition1>numCondition2)
		numCycle = numCondition2;
	else
		numCycle = numCondition1;

	if(numCycle<0)
		numCycle = 0;

	totalTraffic = liveMig_VmSize*
		(1-pow(1.0*liveMig_DirtyRate/liveMig_L,numCycle+1)) //1-(R/L)^(n+1)
		/(1-1.0*liveMig_DirtyRate/liveMig_L);

	/*
	double time = 1.0*totalTraffic/itVM->liveMig_L;

	std::cout<<"VM Migration Cost: numCycle:"<<numCycle
			<<", totalTraffic:"<<totalTraffic
			<<", time:"<<time
			<<", numCondition1:"<<numCondition1
			<<", numCondition2:"<<numCondition2
			<<", itVM->liveMig_VmSize:"<<itVM->liveMig_VmSize
			<<", pow(itVM->liveMig_DirtyRate/itVM->liveMig_L,numCycle+1):"<<pow(1.0*itVM->liveMig_DirtyRate/itVM->liveMig_L,numCycle+1)
			<<", itVM->liveMig_DirtyRate/itVM->liveMig_L:"<<1.0*itVM->liveMig_DirtyRate/itVM->liveMig_L	
			<<std::endl;
			*/

	migrationTraffic = totalTraffic;
	return totalTraffic;	
}

uint64_t 
DcVm::CalcMigrationCost(uint32_t toServerID)
{
	if(m_hostServer==toServerID)
		return 0;
	else
		return migrationTraffic;
}


Ptr<DcFlowApplication> 
DcVm::createFlows(uint32_t srcVmID, uint32_t dstVmID, bool isTCP, DataRate rate, uint32_t numPackets)
{
	Ptr<DcFlowApplication>  flow;
	return flow;

/*
	if(srcVmID>=vmSeedID || dstVmID>=vmSeedID )
	{
		std::cout<<"The input VM ID for creating flow is overflow"<<std::endl;
	}
	
	Ptr<DcFlowApplication> dcflow = CreateObject<DcFlowApplication> ();;	
	//dcflow->m_flowID = flowSeedID++;

	dcflow->m_srcVmID = srcVmID;
	dcflow->m_dstVmID = dstVmID;

	DcVmItemI it = getVM(srcVmID);
	NS_ASSERT_MSG(isValid(it),"VM id is not valid");
	dcflow->m_srcNodeID = it->hostServer;
	it->flows.insert(std::pair<uint32_t,Ptr<DcFlowApplication> >(dcflow->m_flowID,dcflow));
	
	it = getVM(dstVmID);
	NS_ASSERT_MSG(isValid(it),"VM id is not valid");
	dcflow->m_dstNodeID = it->hostServer;
	it->flows.insert(std::pair<uint32_t,Ptr<DcFlowApplication> >(dcflow->m_flowID,dcflow));
	
	dcflow->useTCP = isTCP;
	if(isTCP)
	{
		dcflow->m_srcPort = tcpPortSeed++;
		dcflow->m_dstPort = DC_TCP_DST_PORT;//tcpPortSeed++;
	}
	else		
	{
		dcflow->m_srcPort = udpPortSeed++;
		dcflow->m_dstPort = DC_UDP_DST_PORT;//udpPortSeed++;
	}

	dcflow->m_dataRate = rate;

	dcflow->m_nPackets = numPackets;
	
	m_allFlows.push_back(dcflow);

	//create packet sink application on the dst node
	
	return dcflow;	
*/
}

bool 
DcVm::createVM(Ptr<Node> node, uint32_t cpu, uint32_t memory,uint32_t liveMig_VmSize,uint32_t liveMig_DirtyRate,uint32_t liveMig_L,uint32_t liveMig_T,uint32_t liveMig_X)
{
return false;
/*
	Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node);

	if((hvapp->m_totalCPU - hvapp->m_useCPU) > cpu && 
		(hvapp->m_totalMemory - hvapp->m_useMemory) > memory)
	{
		DcVmItem vm;
		vm.id = vmSeedID++;
		vm.cpu = cpu;
		vm.memory = memory;
		vm.hostServer = node->GetId();
		vm.isStart = false;


		//for migraiton cost
		//std::cout<<"VMs liveMig_VmSize:"<<liveMig_VmSize<<std::endl;		
		vm.liveMig_VmSize = liveMig_VmSize;
		vm.liveMig_DirtyRate = liveMig_DirtyRate;
		vm.liveMig_L = liveMig_L;
		vm.liveMig_T = liveMig_T;
		vm.liveMig_X = liveMig_X;
		
		m_allVMs.push_back(vm);

		hvapp->m_useCPU += cpu;
		hvapp->m_useMemory += memory;			


		//for migraiton cost
		//DcVmItemI itVM = getVM(vm.id);
		//std::cout<<", vm.liveMig_VmSize:"<<vm.liveMig_VmSize<<std::endl;
		//std::cout<<", itVM->liveMig_VmSize:"<<itVM->liveMig_VmSize<<std::endl;
	
		//uint64_t cost = hvapp->CalcMigrationCost(vm.id,26,5);
		//std::cout<<"VMs liveMig_VmSize:"<<liveMig_VmSize<<std::endl;

		//std::cout<<"VMs Migration Cost"<<cost<<std::endl;

		//Calculate migration cost for future use
		DcVmItemI itVM = getVM(vm.id);
		itVM->migrationTraffic = hvapp->CalcMigrationTraffic(itVM->id);		

		return true;
	}
	else
	{
		std::cout<<"Resouce is out on Node:"<<node->GetId()<<std::endl;
		return false;
	}
*/
}


// the last 10% flows are not matched to any policy, so this method will 
// affect the first 9% and last 1% flows
void DcVm::setFirstNFlow(uint32_t num, bool status)
{
/*
	DcFlowItemI it;
	DataRate rate = DataRate ("50kb/s");
	char rateStr [100];
	uint32_t rateValue = 0;

	// first 9%
	uint32_t numX = m_allFlows.size()*0.09;
	
	for(it=m_allFlows.begin(); it<m_allFlows.begin()+numX; it++)
	{
		Ptr<DcFlowApplication> flow = (Ptr<DcFlowApplication>)*it;

		if(num==0)//set false
		{
			flow->isOK = status;
		}
		else if(num==1)//reset rate
		{
			rateValue = m_uniformRand->GetInteger(50, 1000);
			sprintf(rateStr,"%dkb/s", rateValue);
			rate = DataRate (rateStr);
			flow->m_dataRate = rate;
		}
	}

	// last 1%
	numX = m_allFlows.size()*0.01;
	for(it=m_allFlows.end()-numX; it<m_allFlows.end(); it++)
	{
		Ptr<DcFlowApplication> flow = (Ptr<DcFlowApplication>)*it;

		if(num==0)//set false
		{
			flow->isOK = status;
		}
		else if(num==1)//reset rate
		{
			rateValue = m_uniformRand->GetInteger(50, 1000);
			sprintf(rateStr,"%dkb/s", rateValue);
			rate = DataRate (rateStr);
			flow->m_dataRate = rate;
		}
	}
*/
}

Ptr<DcVmHypervisorApplication> 
DcVm::getHyperVisor(uint32_t NodeID)
{
    //return hyperVisor;

    Ptr<DcVmHypervisorApplication> hvapp;
    Ptr<Application> app;

    uint32_t  num = NodeList::GetNode(NodeID)->GetNApplications();

    for(uint32_t i=0;i<num;i++) 
    {
        app = NodeList::GetNode(NodeID)->GetApplication (i);
        NS_ASSERT (app != 0);
        hvapp = app->GetObject<DcVmHypervisorApplication> ();
		if(hvapp!=0)
			return hvapp;
        //NS_ASSERT (hvapp != 0);
    }

	NS_ASSERT_MSG(hvapp != 0,"Can't find the DcVmHypervisorApplication");

    return hvapp;
}


DcVmContainer::DcVmContainer ()
{
}

DcVmContainer::Iterator 
DcVmContainer::Begin (void) const
{
  return m_VmList.begin ();
}

DcVmContainer::Iterator 
DcVmContainer::End (void) const
{
  return m_VmList.end ();
}

uint32_t 
DcVmContainer::GetN (void) const
{
  return m_VmList.size ();
}

Ptr<DcVm> 
DcVmContainer::Get (uint32_t i) const
{
  return m_VmList[i];
}

DcVmContainer::Iterator 
DcVmContainer::GetVmByID (uint32_t vmID) const
{
	for(DcVmContainer::Iterator it = m_VmList.begin(); it!=m_VmList.end(); it++)
	{
		if((*it)->m_id==vmID)
			return it;
	}
	return m_VmList.end();
}

bool
DcVmContainer::IsVmExist (uint32_t vmID) const
{
	Iterator it = GetVmByID (vmID);
	if(it==m_VmList.end())
		return false;
	else
		return true;
}


void 
DcVmContainer::Add (Ptr<DcVm> vm)
{
	m_VmList.push_back (vm);
}

void 
DcVmContainer::Add (DcVmContainer other)
{
	for (Iterator i = other.Begin (); i != other.End (); i++)
	{
		Ptr<DcVm> vm = *i;
		if(IsVmExist(vm->m_id))//if VM already exist in current container, passed
			continue;
		m_VmList.push_back (*i);
	}
}


void DcVmContainer::Remove(Ptr<DcVm> vm)
{
	//DcVmContainer::Iterator it = GetVmByID(vm->m_id);
	//NS_ASSERT_MSG(it!=End(),"Can not find Vm in current server");

	std::vector< Ptr<DcVm> >::iterator it;
	for(it = m_VmList.begin(); it!=m_VmList.end(); it++)
	{
		if((*it)->m_id==vm->m_id)
		{
			m_VmList.erase(it);
			return;
		}			
	}
	std::cout<<"Can not find Vm in current server, vm id="<<vm->m_id<<std::endl;
	//print( std::cout);
}

void 
DcVmContainer::Create (uint32_t n)
{
	//uint16_t id = 1;
	for (uint32_t i = 0; i < n; i++)
	{
		Ptr<DcVm> vm= CreateObject<DcVm> ();
		vm->m_id = i;//id++;
		m_VmList.push_back (vm);
	}
}

void
DcVmContainer::print(std::ostream& os)
{
	std::cout<<"Total: "<<GetN()<<" VMs"<<std::endl;
	for(DcVmContainer::Iterator it = m_VmList.begin(); it!=m_VmList.end(); it++)
	{
		Ptr<DcVm> vm = *it;
		os<<"VM "<<vm->m_id<<" on server: "<<vm->m_hostServer<<"\n";
	}
}


} // namespace ns3

