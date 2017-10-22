/*
 * mec-migrations-stats.cc
 *
 *  Created on: 26 Jul 2017
 *      Author: thiagogenez
 */

#include "mec-migrations-stats.h"

namespace ns3 {

MecMigrationsStats::MecMigrationsStats(){
}

MecMigrationsStats::~MecMigrationsStats() {
}

void MecMigrationsStats::IncrementPolicyAnalyse() {
	m_policiesAnalysed++;
}
void MecMigrationsStats::IncrementPolicyMigration() {
	m_polciesMigrated++;
}

void MecMigrationsStats::IncrementVmAnalyse() {
	m_VmAnalysed++;
}

void MecMigrationsStats::IncrementVmMigration() {
	m_VmMigrated++;
}

uint32_t MecMigrationsStats::GetPolciesMigrated() const {
	return m_polciesMigrated;
}

uint32_t MecMigrationsStats::GetPoliciesAnalysed() const {
	return m_policiesAnalysed;
}

uint32_t MecMigrationsStats::GetVmAnalysed() const {
	return m_VmAnalysed;
}

uint32_t MecMigrationsStats::GetVmMigrated() const {
	return m_VmMigrated;
}

} /* namespace ns3 */
