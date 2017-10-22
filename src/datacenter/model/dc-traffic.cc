/*
 * dc-traffic.cc
 *
 *  Created on: Mar 16, 2012
 *      Author: poscotso
 */
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/bulk-send-application.h"
#include "dc-traffic.h"

//#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include  <math.h>

#include "ns3/dc-vm-hypervisor.h"


NS_LOG_COMPONENT_DEFINE ("DCTrafficApplication");

namespace ns3 {

unsigned long DCTrafficApplication::m_flowsGenerated=0;
FILE * DCTrafficApplication::m_flowCompletion;
//unsigned long DCTrafficApplication::m_bytesGenerated=0;

NS_OBJECT_ENSURE_REGISTERED (DCTrafficApplication);

TypeId
DCTrafficApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DCTrafficApplication")
    .SetParent<Application> ()
    .AddConstructor<DCTrafficApplication> ()
    .AddAttribute ("SendSize", "The amount of data to send each time.",
                   UintegerValue (512),
                   MakeUintegerAccessor (&DCTrafficApplication::m_sendSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&DCTrafficApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. "
                   "Once these bytes are sent, "
                   "no data  is sent again. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DCTrafficApplication::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&DCTrafficApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&DCTrafficApplication::m_txTrace))
  ;
  return tid;
}


DCTrafficApplication::DCTrafficApplication ()
{
  //srand (time (0));
 // srand((unsigned int)Simulator::Now().GetSeconds());
  m_isStart = false;
  m_isNodeListReady = false;
  m_isLogFlowCreated = false;

  m_activeFlows = 0;
  m_totalFlows = 0;

  m_timeScale=110;
  m_uniFlowSize=100000;

  m_bigFlowCount = 0;

  GenFlowTime();
  NS_LOG_FUNCTION (this);
}

DCTrafficApplication::~DCTrafficApplication ()
{
	std::cout<<"Total Number of Flows: \t"<<m_flowsGenerated<<"\t"<<m_totalFlows<<"\n";
	//std::cout<<"Total Number of Bytes: "<<m_bytesGenerated<<"\n";
  NS_LOG_FUNCTION (this);
}

void
DCTrafficApplication::GenFlowTime ()
{
	double lamda=1;

	for (uint32_t i=0; i<10000; i++)
	{
		double x = (double)RandomVariable(UniformVariable(0, 6)).GetValue();
		//*double nextFlowTime = lamda*exp((0-lamda)*x)*1000;
		//std::cout << "nextTime: "<<nextFlowTime<<"\n";
		m_nextFlowTime.push(lamda*exp((0-lamda)*x));
	}
}

void
DCTrafficApplication::ScheduleAFlow()
{
	if (!m_isLogFlowCreated)
	{
		char fileName[100];
		sprintf(fileName,"baat_flow_time_%d.csv",GetNode()->GetId());
		m_logFlowTime = fopen(fileName,"w+");
		m_isLogFlowCreated = true;
	}

	double nextFlowTime = m_nextFlowTime.front();
	fprintf(m_logFlowTime,"%d\t%f\n", GetNode()->GetId(), nextFlowTime);

	m_nextFlowTime.pop();
	Time nextTime = Time (MilliSeconds (nextFlowTime * m_timeScale));
	Simulator::Schedule (nextTime,&DCTrafficApplication::StartApplication, this);
}

void
DCTrafficApplication::SetMaxBytes (uint32_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
DCTrafficApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return 0;
}

void
DCTrafficApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  //m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void DCTrafficApplication::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);


//comment by cuilin
  //if(!m_isNodeListReady)
 //	  ConstructNodeList();

  //std::cout<<Simulator::Now().GetMilliSeconds() <<" @StartApplication\n";


  Ptr<Socket>   socket;
  SocketList	socketList;
  //uint32_t		maxBytes=0;
  uint32_t		totalBytes=0;
  uint32_t		concurrentFlow=0;
  concurrentFlow = 0;

  // Create the socket if not already
  //if (!socket)
  //1. get number of flows need to be generated

  //if(m_activeFlows <=0 )
  //{
	  if(!m_isStart)
	  {
		  concurrentFlow = 3;
		  m_isStart = true;
		  //std::cout<<"Server: "<<GetNode()->GetId()<<" has started\n";
	  }
	  //else{
		  //empty m_socketList first
		  /*for( socketListI it=m_socketList.end(); it!=m_socketList.begin(); it--)
		  {
			  //remove sockets have waited > 1 second for connections.
			  if(Simulator::Now().GetMilliSeconds() - m_socketList.end()->createTime >= 500 )
			  {
				  m_socketList.end()->socket->Close();
				  m_socketList.end()->connected = false;
				  m_socketList.pop_back();
			  }
		  }*/

	  //}
  //}


  //for(uint32_t i=0; i<concurrentFlow; i++)
  //{
	  socket = Socket::CreateSocket (GetNode(), m_tid);

      // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
      if (socket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
    		  socket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
        {
          NS_FATAL_ERROR ("Using BulkSend with an incompatible socket type. "
                          "BulkSend requires SOCK_STREAM or SOCK_SEQPACKET. "
                          "In other words, use TCP instead of UDP.");
        }


/*
InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9999);
 mysocket->Bind (local);
*/
      socket->Bind ();
      //std::cout << "peer's ip "<<NodeList::GetNode(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal()<<"\n";
      socket->Connect (InetSocketAddress ( NodeList::GetNode(GetDestination())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 9));
      //socket->Connect (InetSocketAddress ( NodeList::GetNode(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 9));
      socket->ShutdownRecv ();
      socket->SetConnectCallback (
        MakeCallback (&DCTrafficApplication::ConnectionSucceeded, this),
        MakeCallback (&DCTrafficApplication::ConnectionFailed, this));
      socket->SetSendCallback (
        MakeCallback (&DCTrafficApplication::DataSend, this));

      socketList.socket = socket;
      //uint32_t flowSize = GetFlowSize();
      //std::cout<<"Flow Size "<<flowSize<<"\n";
      socketList.maxBytes = GetFlowSize();
      totalBytes += socketList.maxBytes;
      socketList.totalBytes = 0;
      socketList.connected = false;
      socketList.createTime = Simulator::Now().GetMilliSeconds();
      m_socketList.push_back(socketList);

  //}

  //std::cout<<"\n** No of concurrent flow: "<<concurrentFlow
		  //<<"\n  total byte generated: "<<totalBytes
		  //<<"\n  No of active flows "<< m_activeFlows
		  //<<"\n  No of sockets: "<<m_socketList.size()<<"\n";
  totalBytes = 0;

  for( socketListI it=m_socketList.begin(); it!=m_socketList.end(); it++)
  {
	  if(it->connected)
		  SendData (socket);
  }

  ScheduleAFlow();
}

void DCTrafficApplication::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  if (m_socketList.size() != 0)
    {
	  for( socketListI it=m_socketList.begin(); it!=m_socketList.end(); it++)
	  {
		  it->socket->Close ();
		  it->connected = false;
		  m_socketList.erase(it);
		  m_activeFlows = 0;
	  }
    }
  else
    {
      NS_LOG_WARN ("DCTrafficApplication found null socket to close in StopApplication");
    }
}


// Private helpers

void DCTrafficApplication::SendData (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);

  for( socketListI it=m_socketList.begin(); it!=m_socketList.end(); it++)
  {
	if(it->socket == socket)
	{
	  while (it->maxBytes == 0 || it->totalBytes < it->maxBytes)
		{ // Time to send more
		  uint32_t toSend = m_sendSize;
		  // Make sure we don't send too many
		  if (it->maxBytes > 0)
			{
			  toSend = std::min (m_sendSize, it->maxBytes - it->totalBytes);
			}
		  NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
		  Ptr<Packet> packet = Create<Packet> (toSend);
		  m_txTrace (packet);
		  int actual = it->socket->Send (packet);
		  if (actual > 0)
			{
			  it->totalBytes += actual;
			  //m_bytesGenerated += actual;
			}
		  // We exit this loop when actual < toSend as the send side
		  // buffer is full. The "DataSent" callback will pop when
		  // some buffer space has freed ip.
		  if ((unsigned)actual != toSend)
			{
			  break;
			}
		}//end while
	  // Check if time to close (all sent)
	  if (it->totalBytes >= it->maxBytes && it->connected)
		{
		  it->socket->Close ();
		  it->connected = true;
		  //std::cout<<"totalBytes sent: "<<it->totalBytes;
		  //std::cout<<"\t before delete a flow - m_activeFlows: "<<m_activeFlows
			//	  <<"socketlist: "<<m_socketList.size()<<"\n";
		  if(!m_flowCompletion) m_flowCompletion = fopen("flow_completion.csv","w+");
		  uint32_t elpasedTime = Simulator::Now().GetMilliSeconds() - it->createTime;
		  fprintf(m_flowCompletion,"%d\n",elpasedTime);
		  fflush(m_flowCompletion);
		  m_socketList.erase(it);
		  m_activeFlows--;
		 // std::cout<<"\t after delete a flow - m_activeFlows: "<<m_activeFlows
		  				 // <<"socketlist: "<<m_socketList.size()<<"\n";

		  if(m_socketList.empty()) m_activeFlows = 0;

		  //Simulator::ScheduleNow (&DCTrafficApplication::StartApplication, this);
		}

	  break;
	}//end if
  }//end for
}

void DCTrafficApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("DCTrafficApplication Connection succeeded");

  //m_connected = true;
  m_activeFlows ++;
  m_flowsGenerated++;
  m_totalFlows++;
  //std::cout<<Simulator::Now().GetMilliSeconds()<<", connection succeeded, Node: "<<GetNode()->GetId()<<" m_activeFlows: "<<m_activeFlows<<"\n";
  for( socketListI it=m_socketList.begin(); it!=m_socketList.end(); it++)
	  if(socket==it->socket)
		  it->connected = true;

  //SendData (socket);
}

void DCTrafficApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("DCTrafficApplication, Connection Failed");
}

void DCTrafficApplication::DataSend (Ptr<Socket> socket, uint32_t size)
{
  NS_LOG_FUNCTION (this);

  for( socketListI it=m_socketList.begin(); it!=m_socketList.end(); it++)
  {
	  if( (it->socket == socket)&&(it->connected) )
		  Simulator::ScheduleNow (&DCTrafficApplication::SendData, this,socket);
  }

}

void DCTrafficApplication::ConstructNodeList()
{
	 Ipv4Address ipv4Local = GetNode()->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	 Ipv4Address ipv4Remote;
	 Ipv4Address subnetAddr = ipv4Local.GetSubnetDirectedBroadcast("255.255.0.0");

	for(uint32_t i=0; i<NodeList::GetNNodes(); i++)
	{
		//std::cout<<" "<<i<<" has "<<NodeList::GetNode(i)->GetObject<Ipv4>()->GetNInterfaces()<<" interfaces\n";
		//more than 2 interfaces per node, probably they are switches
		//if((NodeList::GetNode(i)->GetObject<Ipv4>()->GetNInterfaces() != 2) ||

		//changed by cuilin
		//if( !(getHyperVisor()->fattree->IsHost(i)) ||
		if( !(NodeList::GetNode(i)->IsHost()) ||
				(GetNode()->GetId() == i))
			continue;

		ipv4Remote = NodeList::GetNode(i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		//std::cout<<"Local subnet "<<subnetAddr<<" Remote subnet: "<<ipv4Remote.GetSubnetDirectedBroadcast("255.255.0.0")<<"\n";
		if(ipv4Remote.GetSubnetDirectedBroadcast("255.255.0.0") == subnetAddr) //IPs have the same subnet address
			m_inRackHost.push_back(i);
		else
			m_outRackHost.push_back(i);
	}

	m_isNodeListReady = true;
}

int DCTrafficApplication::ProbabilityGenerator(int probability)
{
 return ((rand () % 1000 + 1) < probability) ? 1 : 0;
}

uint32_t
DCTrafficApplication::GetDestination()
{
	uint32_t selectId = 0;

	//if(ProbabilityGenerator(20)) //in rack
		//selectId = m_inRackHost.at((rand () % m_inRackHost.size()));
	//else //out of rack
		selectId = m_outRackHost.at((rand () % m_outRackHost.size()));

	//std::cout<<"m_inRackHost.size() "<<m_inRackHost.size()<<" m_outRackHost.size()" << m_outRackHost.size()<<"\n"
			//<<"\t("<<GetNode()->GetId()<<") Destination "<<selectId<<"Active flows: "<<m_activeFlows<<"\n";

	return selectId;
}

uint32_t
DCTrafficApplication::GetFlowSize()
{
	uint32_t flowSize = 0;

	if(ProbabilityGenerator(999)) // flow size, 0-100 MB
	{
		//if(ProbabilityGenerator(60))
			flowSize = 4000;
		//else
			//flowSize = 8000;
	}
	else{
		if(m_bigFlowCount<1)
		{
			flowSize = 100000000;
			std::cout << "**** LARGE Flow size "<<flowSize<<"****\n";
			m_bigFlowCount++;
		}else
			flowSize = 4000;

	}

	//std::cout << "Flow size "<<flowSize<<"\n";

	//return (flowSize/1000);
	//return 4000; //4kB
	//return 8000; //8kB
	//return 100000; //100kB
	return m_uniFlowSize;
	//return flowSize;
}

//add by cuilin
// get the hypervisor of the current Traffic
Ptr<DcVmHypervisorApplication> 
DCTrafficApplication::getHyperVisor()
{
    //return hyperVisor;

    Ptr<DcVmHypervisorApplication> hvapp;
    Ptr<Application> app;

    uint32_t  num = GetNode()->GetNApplications();

    for(uint32_t i=0;i<num;i++) 
    {
        app =  GetNode()->GetApplication (i);
        NS_ASSERT (app != 0);
        hvapp = app->GetObject<DcVmHypervisorApplication> ();
		if(hvapp!=0)
			return hvapp;
        //NS_ASSERT (hvapp != 0);
    }

	NS_ASSERT_MSG(hvapp != 0,"Can't find the DcVmHypervisorApplication");

    return hvapp;
}
Ptr<DcVmHypervisorApplication> 
DCTrafficApplication::getHyperVisor(uint32_t NodeID)
{
    //return hyperVisor;

    Ptr<DcVmHypervisorApplication> hvapp;
    Ptr<Application> app;

    uint32_t  num = NodeList::GetNode(NodeID)->GetNApplications();

    for(uint32_t i=0;i<num;i++) 
    {
        app =  NodeList::GetNode(NodeID)->GetApplication (i);
        NS_ASSERT (app != 0);
        hvapp = app->GetObject<DcVmHypervisorApplication> ();
		if(hvapp!=0)
			return hvapp;
        //NS_ASSERT (hvapp != 0);
    }

	NS_ASSERT_MSG(hvapp != 0,"Can't find the DcVmHypervisorApplication");

    return hvapp;
}



} // Namespace ns3
