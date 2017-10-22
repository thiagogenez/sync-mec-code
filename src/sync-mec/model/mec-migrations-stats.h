/*
 * mec-migrations-stats.h
 *
 *  Created on: 26 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_MIGRATIONS_STATS_H_
#define MEC_MIGRATIONS_STATS_H_

#include "ns3/uinteger.h"

namespace ns3 {

class MecMigrationsStats {
public:
	MecMigrationsStats();
	virtual ~MecMigrationsStats();


	void IncrementPolicyAnalyse();
	void IncrementPolicyMigration();
	void IncrementVmAnalyse();
	void IncrementVmMigration();

	uint32_t GetPolciesMigrated() const;
	uint32_t GetPoliciesAnalysed() const;
	uint32_t GetVmAnalysed() const;
	uint32_t GetVmMigrated() const;

private:
	uint32_t m_policiesAnalysed = 0;
	uint32_t m_polciesMigrated = 0;

	uint32_t m_VmAnalysed = 0;
	uint32_t m_VmMigrated = 0;

};

} /* namespace ns3 */

#endif /* MEC_MIGRATIONS_STATS_H_ */
