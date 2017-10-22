/*
 * scenario-generator.h
 *
 *  Created on: 8 Jun 2017
 *      Author: thiagogenez
 */

#ifndef SCENARIO_GENERATOR_H_
#define SCENARIO_GENERATOR_H_

#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/mec-backhaul-core-helper.h"
#include "ns3/mec-vm-container.h"
#include "ns3/application-container.h"

namespace ns3 {

class MecScenarioGeneratorHelper {
public:
	MecScenarioGeneratorHelper();
	virtual ~MecScenarioGeneratorHelper();

	void GenerateMiddleboxes(NodeContainer *aggrNodes, NodeContainer *edgeNodes,
			MecMiddleBoxContainer *middleboxContainer);

	void GenerateVMsAndUEs(NodeContainer *allHosts,
			MecVmContainer *mecVmContainer, MecVmContainer *ueContainer);

	void GenerateFlows(NodeContainer *allHosts, MecVmContainer *mecVmContainer,
			MecVmContainer *ueContainer, MecFlowContainer *vmToUeFlowContainer,
			MecFlowContainer *ueToVmFlowContainer);

	void GeneratePolicies(MecFlowContainer *vmToUeFlowContainer,
			MecFlowContainer *ueToVmFlowContainer,
			MecPolicyContainer *vmToUePolicyContainer,
			MecPolicyContainer *ueToVmPolicyContainer,
			MecMiddleBoxContainer *middleboxContainer,
			Ptr<MecBackhaulCoreHelper> backhaulCoreHelper);

	void GeneratingHypervisors(NodeContainer *allHosts, ApplicationContainer *applicationContainer);

private:

	// used to generate VMs and UEs
	void DeployOnBaseStation(MecVmContainer *container, NodeContainer *allHosts,
			std::vector<uint32_t> &numHostedVmAtServer,
			std::vector<uint32_t> &suffledServerHostIds);
	void SetupVm(Ptr<MecVm> vm);
	Ptr<Node> GetRandomHostServerForVm(NodeContainer *allHosts,
			std::vector<uint32_t>& numHostedVmAtServer,
			std::vector<uint32_t>& suffledServerHostIds);

	// used to generate flows
	void InstallSocketsOnServerHosts(NodeContainer *allHosts);
	void AssignFlows(MecFlowContainer *flowContainer, MecVmContainer *from,
			MecVmContainer* to, std::vector<uint32_t> &numberOfFlows,
			uint32_t numberOfPairs, bool isDownloadFlow);
	void SetupFlow(Ptr<MecFlow> flow, bool useTcp, bool isDownloadFlow);

	// used to generate plocies
	void SetupPolicies(MecFlowContainer *flowContainer,
			MecPolicyContainer *mecPolicyContainer,
			MecMiddleBoxContainer *middleboxContainer,
			Ptr<MecBackhaulCoreHelper> backhaulCoreHelper);
	bool ConstructPolicies(Ptr<MecFlow> flow, Ptr<MecPolicy> policy,
			MecMiddleBoxContainer *middleboxContainer,
			Ptr<MecBackhaulCoreHelper> backhaulCoreHelper);

	Ptr<MecMiddleBox> GetMiddlebox(Ptr<MecFlow> flow, Ptr<MecPolicy> policy,
			MecMiddleBoxContainer *candidates,
			Ptr<MecBackhaulCoreHelper> backhaulCoreHelper,
			uint32_t previousNodeId, bool isEgress);

};

} /* namespace ns3 */

#endif /* SCENARIO_GENERATOR_H_ */
