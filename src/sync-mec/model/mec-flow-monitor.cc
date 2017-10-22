/*
 * mec-flow-monitor.cc
 *
 *  Created on: 13 Jul 2017
 *      Author: thiagogenez
 */

#include "mec-flow-monitor.h"

#include "ns3/simulator.h"
#include "ns3/ipv4-flow-classifier.h"

#include "ns3/mec-constants-helper.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-logger.h"

#include <numeric>      // std::accumulate
#include <vector>      // std::accumulate
#include <map>      // std::accumulate

namespace ns3 {

MecFlowMonitor::MecFlowMonitor(NodeContainer allHost,
		MecPairCommunicationContainer& pairCommunicationContainer) {

	m_flowMonitor = m_flowMonitorHelper.Install(allHost);
	m_pairCommunicationContainer = pairCommunicationContainer;

}

MecFlowMonitor::~MecFlowMonitor() {
}

void MecFlowMonitor::StopApplication() {
}

void MecFlowMonitor::StartApplication() {

	// schedule the flow monitor
	ScheduleNextMonitorCheck(Simulator::Now());

}

FlowId MecFlowMonitor::GetStatsLastFlow(Ptr<MecFlow> flow,
		std::map<FlowId, FlowMonitor::FlowStats>& flowStats,
		Ptr<Ipv4FlowClassifier> classing) {

	FlowId id = 0;
	std::vector<FlowId> teste;
	// It is necessary to check from  ALL FLOWS
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator itStats =
			flowStats.begin(); itStats != flowStats.end(); ++itStats) {

		Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow(
				itStats->first);

		// getting the stats for the current pair's flor (pair UE<->VM)
		if (fiveTuple.sourcePort == flow->GetSrcPort()) {

			if (itStats->first > id) {
				id = itStats->first;
			}
			teste.push_back(itStats->first);

//			std::cout << "Time= " << Simulator::Now() << "PORT="
//					<< flow->GetSrcPort() << " flowInternalId="
//					<< itStats->first << "flowId=" << flow->GetId() << "\tFTx="
//					<< itStats->second.timeFirstTxPacket.GetSeconds() << " LTx="
//					<< itStats->second.timeLastTxPacket.GetSeconds() << " FRx="
//					<< itStats->second.timeFirstRxPacket.GetSeconds() << " LRx="
//					<< itStats->second.timeLastRxPacket.GetSeconds()
//
//					<< "\t" << fiveTuple.sourceAddress << ":"
//					<< fiveTuple.sourcePort << "->"
//					<< fiveTuple.destinationAddress << ":"
//					<< fiveTuple.destinationPort << " tx="
//					<< itStats->second.txPackets << " rx="
//					<< itStats->second.rxPackets << std::endl;
//			flow->Print(std::cout);
		}
	}

	return id;
}

MecFlowStats MecFlowMonitor::GetCurrentStats(MecFlowContainer& flowContainer,
		MecPolicyContainer& policyContainer) {

	std::map<FlowId, FlowMonitor::FlowStats> flowStats =
			m_flowMonitor->GetFlowStats();
	Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier>(
			m_flowMonitorHelper.GetClassifier());

	MecFlowStats mecStats;

	for (MecFlowContainer::const_iterator itFlow = flowContainer.CBegin();
			itFlow != flowContainer.CEnd(); itFlow++) {

		Ptr<MecFlow> flow = (*itFlow);
		FlowId id = GetStatsLastFlow(flow, flowStats, classing);

		std::map<FlowId, FlowMonitor::FlowStats>::iterator it;
		it = flowStats.find(id);

		if (it != flowStats.end()) {

			FlowMonitor::FlowStats stats = it->second;

			mecStats.AddDelay(
					stats.delaySum.GetMilliSeconds() * 1.0 / stats.rxPackets);
			mecStats.AddJitter(
					stats.jitterSum.GetMilliSeconds() * 1.0
							/ (stats.rxPackets - 1));

			mecStats.AddThroughput(
					stats.rxBytes * 8.0
							/ (stats.timeLastRxPacket.GetSeconds()
									- stats.timeFirstTxPacket.GetSeconds())
							/ 1024 / 1024);

			mecStats.AddHop(1 + (stats.timesForwarded));
			mecStats.AddLoss(
					stats.lostPackets / (stats.rxPackets + stats.lostPackets));
		}

	}

	return mecStats;
}

void MecFlowMonitor::CheckPairFlows(uint32_t id,
		MecFlowContainer &flowContainer, MecPolicyContainer &policyContainer,
		bool isDownload) {

	std::ostringstream msg;

	MecFlowStats mecStats = GetCurrentStats(flowContainer, policyContainer);

	msg << id << ";" << mecStats.GetAvgDelay() << ";"
			<< mecStats.GetAvgThroughput() << ";" << mecStats.GetHops() << ";"
			<< mecStats.GetAvgLoss() << ";" << mecStats.GetAvgJitter();

	MecLogger::LoggingFlowMonitor(msg.str(), isDownload);

}

void MecFlowMonitor::Monitor() {

	if (!Simulator::IsFinished()) {

		m_flowMonitor->CheckForLostPackets();

		for (MecPairCommunicationContainer::const_iterator it =
				m_pairCommunicationContainer.CBegin();
				it != m_pairCommunicationContainer.CEnd(); it++) {

			Ptr<MecPairCommunication> pair = (*it);

			CheckPairFlows(pair->GetId(), pair->GetDownloadFlows(),
					pair->GetDownloadPolicies(), true);
			CheckPairFlows(pair->GetId(), pair->GetUploadFlows(),
					pair->GetUploadPolicies(), false);

		}

		ScheduleNextMonitorCheck(
				MecConstantsHelper::FLOW_MONITOR_NEXT_CHECK_TIME);

	}
}

void MecFlowMonitor::ScheduleNextMonitorCheck(Time nextTime) {
	if (m_monitorCheckEvent.IsRunning()) {

		std::cout
				<< "MecSdnController::ScheduleNextMonitorCheck Event will be canceled"
				<< std::endl;

		m_monitorCheckEvent.Cancel();
	}

	m_monitorCheckEvent = Simulator::Schedule(
			MecConstantsHelper::FLOW_MONITOR_NEXT_CHECK_TIME,
			&MecFlowMonitor::Monitor, this);
}

MecFlowStats::MecFlowStats() {
}

MecFlowStats::~MecFlowStats() {
}

void MecFlowStats::AddDelay(double value) {
	m_delays.push_back(value);
}

void MecFlowStats::AddJitter(double value) {
	m_jitter.push_back(value);
}

void MecFlowStats::AddThroughput(double value) {
	m_throughput.push_back(value);
}

void MecFlowStats::AddHop(double value) {
	m_hop.push_back(value);
}
void MecFlowStats::AddLoss(double value) {
	m_loss.push_back(value);
}

double MecFlowStats::GetAvgDelay() const {
	return std::accumulate(m_delays.begin(), m_delays.end(), 0.0)
			/ m_delays.size();
}

double MecFlowStats::GetAvgJitter() const {
	return std::accumulate(m_jitter.begin(), m_jitter.end(), 0.0)
			/ m_jitter.size();
}

double MecFlowStats::GetAvgThroughput() const {
	return std::accumulate(m_throughput.begin(), m_throughput.end(), 0.0)
			/ m_throughput.size();
}

double MecFlowStats::GetHops() const {
	return std::accumulate(m_hop.begin(), m_hop.end(), 0.0);
}

double MecFlowStats::GetAvgLoss() const {
	return std::accumulate(m_loss.begin(), m_loss.end(), 0.0) / m_loss.size();
}

}/* namespace ns3 */
