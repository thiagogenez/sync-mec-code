/*
 * mec-policy-routing-helper.h
 *
 *  Created on: 19 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_POLICY_ROUTING_HELPER_H_
#define MEC_POLICY_ROUTING_HELPER_H_

#include "ns3/object-factory.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

namespace ns3 {

class MecPolicyRoutingHelper: public Ipv4RoutingHelper {
public:
	MecPolicyRoutingHelper();
//	virtual ~MecPolicyRoutingHelper();

	MecPolicyRoutingHelper(const MecPolicyRoutingHelper &);
	MecPolicyRoutingHelper* Copy(void) const;
	virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;

private:
	MecPolicyRoutingHelper &operator =(const MecPolicyRoutingHelper &);
	ObjectFactory m_agentFactory; //!< Object factory
};

}
/* namespace ns3 */

#endif /* MEC_POLICY_ROUTING_HELPER_H_ */
