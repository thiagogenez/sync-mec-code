/*
 * Farooque HassaN Kumbhar
 * This example is simple scenario where 1 eNB and 15 devices are there. 
 * Using constant position all UEs are situated.
 * this file generate an xml file using which we can simulate the scenario in NetAnim easily.
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/netanim-module.h"
using namespace ns3;

int main (int argc, char *argv[])
{
  // the rest of the simulation program follows



Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

//This will instantiate some common objects (e.g., the Channel object) and provide the methods to add eNBs and UEs and configure them.

//Create Node objects for the eNB(s) and the UEs:

NodeContainer enbNodes;
enbNodes.Create (1);
NodeContainer ueNodes;
ueNodes.Create (15);

//Note that the above Node instances at this point still don’t have an LTE protocol stack installed; they’re just empty nodes.

//Configure the Mobility model for all the nodes:

MobilityHelper mobility;
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (enbNodes);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (ueNodes);

// AnimationInterface::SetConstantPosition (enbNodes.Get (1), 10, 30);
 
  AnimationInterface::SetConstantPosition (ueNodes.Get (0), 7, 1);
  AnimationInterface::SetConstantPosition (ueNodes.Get (1), 5, 0);
  AnimationInterface::SetConstantPosition (ueNodes.Get (2), 5, 1);
  AnimationInterface::SetConstantPosition (ueNodes.Get (3), 5, 2);
  AnimationInterface::SetConstantPosition (ueNodes.Get (4), 5, 3);
  AnimationInterface::SetConstantPosition (ueNodes.Get (5), 5, 4);
  AnimationInterface::SetConstantPosition (ueNodes.Get (6), 5, 5);
  AnimationInterface::SetConstantPosition (ueNodes.Get (7), 5, 6);
  AnimationInterface::SetConstantPosition (ueNodes.Get (8), 5, 7);
  AnimationInterface::SetConstantPosition (ueNodes.Get (9), 5, 8);
  AnimationInterface::SetConstantPosition (ueNodes.Get (10), 5, 9);
  AnimationInterface::SetConstantPosition (ueNodes.Get (12), 5, 10);
  AnimationInterface::SetConstantPosition (ueNodes.Get (13), 3, 2);
  AnimationInterface::SetConstantPosition (ueNodes.Get (14), 2, 3);

//The above will place all nodes at the coordinates (0,0,0). Please refer to the documentation of the ns-3 mobility model for how to set your own position or configure node movement.

//Install an LTE protocol stack on the eNB(s):

NetDeviceContainer enbDevs;
enbDevs = lteHelper->InstallEnbDevice (enbNodes);

//Install an LTE protocol stack on the UEs:

NetDeviceContainer ueDevs;
ueDevs = lteHelper->InstallUeDevice (ueNodes);

//Attach the UEs to an eNB. This will configure each UE according to the eNB configuration, and create an RRC connection between them:

lteHelper->Attach (ueDevs, enbDevs.Get (0));

//Activate a data radio bearer between each UE and the eNB it is attached to:

enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
EpsBearer bearer (q);
lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

//this method will also activate two saturation traffic generators for that bearer, one in uplink and one in downlink.

//Set the stop time:

Simulator::Stop (Seconds (0.5));

///This is needed otherwise the simulation will last forever, because (among others) the start-of-subframe event is scheduled repeatedly, and the ns-3 simulator scheduler will hence never run out of events.

//Run the simulation:
  AnimationInterface anim ("lte_animation_test.xml");
  anim.SetMobilityPollInterval(Seconds(0.25));
  anim.EnablePacketMetadata(true);
  Simulator::Run ();

//Cleanup and exit:

Simulator::Destroy ();
return 0;
}

