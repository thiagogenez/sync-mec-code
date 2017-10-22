/*
 * mec-flow-monitor.h
 *
 *  Created on: 13 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_FLOW_MONITOR_H_
#define MEC_FLOW_MONITOR_H_

#include "ns3/ptr.h"
#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/node-container.h"

#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

#include "ns3/mec-flow-container.h"
#include "ns3/mec-policy-container.h"
#include "ns3/mec-pair-communication-container.h"

namespace ns3 {

class MecFlowStats;

class MecFlowMonitor: public Application {
public:

	MecFlowMonitor(NodeContainer allHost,
			MecPairCommunicationContainer& pairCommunicationContainer);

	virtual ~MecFlowMonitor();

	// Inherited from Application base class.
	// Called at time specified by Start
	virtual void StartApplication();
	// Called at time specified by Stop
	virtual void StopApplication();

private:

	//Management of the flow monitor
	MecFlowStats GetCurrentStats(
			MecFlowContainer& flowContainer,
			MecPolicyContainer& policyContainer);
	void Monitor();
	void ScheduleNextMonitorCheck(Time nextTime);

	void CheckPairFlows(uint32_t id, MecFlowContainer &flowContainer,
			MecPolicyContainer &policyContainer, bool isDownload);

	FlowId GetStatsLastFlow(Ptr<MecFlow> flow,
			std::map<FlowId, FlowMonitor::FlowStats>& flowStats,
			Ptr<Ipv4FlowClassifier> classing);

	//FlowMonitor purposes
	Ptr<FlowMonitor> m_flowMonitor;
	FlowMonitorHelper m_flowMonitorHelper;

	//Mec Communication container
	MecPairCommunicationContainer m_pairCommunicationContainer;

	EventId m_monitorCheckEvent;

};

class MecFlowStats {
public:

	MecFlowStats();
	virtual ~MecFlowStats();

	void AddDelay(double);
	void AddJitter(double);
	void AddThroughput(double);
	void AddHop(double);
	void AddLoss(double);

	double GetAvgDelay() const;
	double GetAvgJitter() const;
	double GetAvgThroughput() const;
	double GetHops() const;
	double GetAvgLoss() const;

private:
	std::vector<double> m_delays;
	std::vector<double> m_jitter;
	std::vector<double> m_throughput;
	std::vector<double> m_hop;
	std::vector<double> m_loss;
};

} /* namespace ns3 */

#endif /* MEC_FLOW_MONITOR_H_ */
