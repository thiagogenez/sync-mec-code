
#ifndef POLICY_ROUTING_HELPER_H
#define POLICY_ROUTING_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"


namespace ns3 {

/**
 * \brief Helper class that adds Nix-vector routing to nodes.
 *
 * This class is expected to be used in conjunction with 
 * ns3::InternetStackHelper::SetRoutingHelper
 *
 */
class PolicyRoutingHelper : public Ipv4RoutingHelper// Ipv4GlobalRoutingHelper//
{
public:
  /**
   * Construct an PolicyRoutingHelper to make life easier while adding Nix-vector
   * routing to nodes.
   */
  PolicyRoutingHelper ();

  /**
   * \brief Construct an PolicyRoutingHelper from another previously 
   * initialized instance (Copy Constructor).
   */
  PolicyRoutingHelper (const PolicyRoutingHelper &);

  /**
   * \returns pointer to clone of this PolicyRoutingHelper 
   * 
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  PolicyRoutingHelper* Copy (void) const;

  /**
  * \param node the node on which the routing protocol will run
  * \returns a newly-created routing protocol
  *
  * This method will be called by ns3::InternetStackHelper::Install
  */
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

private:
  /**
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  PolicyRoutingHelper &operator = (const PolicyRoutingHelper &);

  ObjectFactory m_agentFactory; //!< Object factory
};
} // namespace ns3

#endif /* POLICY_ROUTING_HELPER_H */
