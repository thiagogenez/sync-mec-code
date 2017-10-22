/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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

#include "mec-v4-ping-helper.h"
#include "ns3/mec-v4-ping.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"

#include "ns3/pointer.h"
#include "ns3/callback.h"

namespace ns3 {

MecV4PingHelper::MecV4PingHelper(Ipv4Address remote) {
	m_factory.SetTypeId("ns3::MecV4Ping");
	m_factory.Set("Remote", Ipv4AddressValue(remote));
}

void MecV4PingHelper::SetAttribute(std::string name,
		const AttributeValue &value) {
	m_factory.Set(name, value);
}

ApplicationContainer MecV4PingHelper::Install(NodeContainer c,
		Callback<void, uint32_t, double> pongerCallback, Time interval) {

	ApplicationContainer apps;

	for (uint32_t i = 0; i < c.GetN(); i++) {
		Ptr<Node> node = c.Get(i);
		apps.Add(InstallPriv(node, pongerCallback, interval));
	}

	return apps;
}

Ptr<Application> MecV4PingHelper::InstallPriv(Ptr<Node> node,
		Callback<void, uint32_t, double> pongerCallback, Time interval) {

	SetAttribute("Pong", CallbackValue(pongerCallback));
	SetAttribute("Interval", TimeValue(interval));

	Ptr<MecV4Ping> app = m_factory.Create<MecV4Ping>();

	node->AddApplication(app);

	return app;
}

} // namespace ns3
