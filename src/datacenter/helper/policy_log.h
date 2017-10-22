/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef POLICY_LOG_H
#define POLICY_LOG_H


namespace ns3 {

/* ... */


class PolicyLogAll
{
public:
	//FILE* logVmCostMigration;
	std::ofstream logVmCountOnServer;
	std::ofstream logLinkUtilization;
	
	std::ofstream logPeriodLinkUtilization;

	std::ofstream logSnapshotVmCost;

	//std::ofstream logSnapshotFlowLength;
	std::ofstream logSnapshotLinkUtilization;

	//--------------------------
	std::ofstream logPeriodStat;
	std::ofstream logPeriodAllFlowCost;
	std::ofstream logVmMigration;
	std::ofstream logPolicyMigration;
	std::ofstream logPolicyViolation;

	std::ofstream logAlgPerformance;

	PolicyLogAll()
	{
		/*
		logVmCostMigration.open("scratch/logVmCostMigration.txt", std::ios::trunc);
		logVmCountOnServer.open("scratch/logVmCountOnServer.txt", std::ios::trunc);
		logLinkUtilization.open("scratch/logLinkUtilization.txt", std::ios::trunc);

		logPeriodVmCost.open("scratch/logPeriodVmCost.txt", std::ios::trunc);
		logPeriodLinkUtilization.open("scratch/logPeriodLinkUtilization.txt", std::ios::trunc);
		*/
	};
	
	~PolicyLogAll()
	{
		//fclose (logVmCostMigration);
		
		logVmCountOnServer.close();
		logLinkUtilization.close();
		
		logPeriodLinkUtilization.close();

		logSnapshotVmCost.close();

		//logSnapshotFlowLength.close();
		logSnapshotLinkUtilization.close();

		//-------------------
		logPeriodStat.close();
		logPeriodAllFlowCost.close();
		logVmMigration.close();
		logPolicyMigration.close();
		logPolicyViolation.close();
		
		logAlgPerformance.close();
	};

	void initialLogFiles(bool usePolicy, uint32_t sizeK)
	{
		char filenameTemp[200];

		uint32_t isPolicy;

		if(usePolicy)
		{
			isPolicy = 1;
		}
		else
			isPolicy = 0;
			
		sprintf(filenameTemp, "result/log/logPeriodStat_%d_%d.txt", isPolicy, sizeK);
		logPeriodStat.open(filenameTemp, std::ios::trunc);

		sprintf(filenameTemp, "result/log/logPeriodAllFlowCost_%d_%d.txt", isPolicy,sizeK);
		logPeriodAllFlowCost.open(filenameTemp, std::ios::trunc);

		sprintf(filenameTemp, "result/log/logVmMigration_%d_%d.txt", isPolicy,sizeK);
		logVmMigration.open(filenameTemp, std::ios::trunc);

		sprintf(filenameTemp, "result/log/logPolicyMigration_%d_%d.txt", isPolicy,sizeK);
		logPolicyMigration.open(filenameTemp, std::ios::trunc);

		sprintf(filenameTemp, "result/log/logPolicyViolation_%d_%d.txt", isPolicy,sizeK);
		logPolicyViolation.open(filenameTemp, std::ios::trunc);

		sprintf(filenameTemp, "result/logAlgPerformance.txt");
		logAlgPerformance.open(filenameTemp, std::ios::app);
	};

	void initialLogFiles_bak(bool usePolicy)
	{
		if(usePolicy)
		{			
			//logVmCountOnServer.open("result/log/logVmCountOnServer.txt", std::ios::trunc);
			//logLinkUtilization.open("result/log/logLinkUtilization.txt", std::ios::trunc);
			
			//logPeriodLinkUtilization.open("result/log/logPeriodLinkUtilization.txt", std::ios::trunc);

			//logSnapshotVmCost.open("result/log/logSnapshotVmCost.txt", std::ios::trunc);

			//logSnapshotFlowLength.open("result/log/logSnapshotFlowLength.txt", std::ios::trunc);
			//logSnapshotLinkUtilization.open("result/log/logSnapshotLinkUtilization.txt", std::ios::trunc);

			//-------------------
			logPeriodStat.open("result/log/logPeriodStat.txt", std::ios::trunc);
			logPeriodAllFlowCost.open("result/log/logPeriodAllFlowCost.txt", std::ios::trunc);
			logVmMigration.open("result/log/logVmMigration.txt", std::ios::trunc);
			logPolicyMigration.open("result/log/logPolicyMigration.txt", std::ios::trunc);
			logPolicyViolation.open("result/log/logPolicyViolation.txt", std::ios::trunc);
		}
		else //S-CORE
		{
			//logVmCountOnServer.open("result/log/logVmCountOnServer_score.txt", std::ios::trunc);
			//logLinkUtilization.open("result/log/logLinkUtilization_score.txt", std::ios::trunc);

			//logPeriodLinkUtilization.open("result/log/logPeriodLinkUtilization_nonPolicy.txt", std::ios::trunc);

			//logSnapshotVmCost.open("result/log/logSnapshotVmCost_nonPolicy.txt", std::ios::trunc);

			//logSnapshotFlowLength.open("result/log/logSnapshotFlowLength_nonPolicy.txt", std::ios::trunc);
			//logSnapshotLinkUtilization.open("result/log/logSnapshotLinkUtilization_nonPolicy.txt", std::ios::trunc);

			//-------------------
			logPeriodStat.open("result/log/logPeriodStat_score.txt", std::ios::trunc);
			logPeriodAllFlowCost.open("result/log/logPeriodAllFlowCost_score.txt", std::ios::trunc);
			logVmMigration.open("result/log/logVmMigration_score.txt", std::ios::trunc);
			logPolicyMigration.open("result/log/logPolicyMigration_score.txt", std::ios::trunc);
			logPolicyViolation.open("result/log/logPolicyViolation_score.txt", std::ios::trunc);
		}
	};

};

}

#endif /* POLICY_LOG_H */

