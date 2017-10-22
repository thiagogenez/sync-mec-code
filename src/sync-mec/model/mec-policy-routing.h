/*
 * mec-policy-routing.h
 *
 *  Created on: 18 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_POLICY_ROUTING_H_
#define MEC_POLICY_ROUTING_H_

#include "ns3/ipv4-global-routing.h"
#include "ns3/mec-backhaul-core-helper.h"
#include "ns3/mec-policy-container.h"

namespace ns3 {

class MecPolicyRouting: public Ipv4GlobalRouting {
public:
	static TypeId GetTypeId();
	MecPolicyRouting();

//	virtual ~MecPolicyRouting();

	virtual void SetIpv4(Ptr<Ipv4> ipv4);

	bool IsPolicyEnable() const;

	void SetMecBackHaulCoreHelper(
			const Ptr<MecBackhaulCoreHelper>& mecBackHaulCoreHelper);
	void SetMecPolicyContainer(MecPolicyContainer* mecPolicyContainer);
	void SetPolicyEnable(bool policyEnable);

private:
	virtual bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
			Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
			MulticastForwardCallback mcb, LocalDeliverCallback lcb,
			ErrorCallback ecb);

	virtual Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
			Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

	void SetStatsTag(Ptr<Packet> packetCopy);

	Ptr<Ipv4> m_ipv4;
	bool m_policyEnable = true;
	Ptr<MecBackhaulCoreHelper> m_backHaulCoreHelper;
	MecPolicyContainer *m_policyContainer;

};

} /* namespace ns3 */

#endif /* MEC_POLICY_ROUTING_H_ */
