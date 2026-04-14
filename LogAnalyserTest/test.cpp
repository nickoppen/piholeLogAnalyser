// install gtest as a submodule in the project and link it properly in CMakeLists.txt
// add the subdirectory for gtest in CMakeLists.txt and link the test executable with gtest and gtest_main
// update the CMakeLists.txt to include the test subdirectory and set up the test executable

#include <gtest/gtest.h>
#include <chrono>
#include "DNSQuery.h"
#include "piholeLogAnalyserDefs.h"

struct DNSQueryTests : public ::testing::Test {
	// You can add any common setup or teardown code here if needed

	ofstream errLogger;
	dbInterface db; 
	dnsQuery query;  
	cliArgs args;

	void setupLogTime(int month, int day, int hour, int min, int sec, tm & logTimeOfCall)
	{
		logTimeOfCall.tm_mon = month; //  December == 11, January == 0
		logTimeOfCall.tm_mday = day; // day of the month
		logTimeOfCall.tm_hour = hour;
		logTimeOfCall.tm_min = min;
		logTimeOfCall.tm_sec = sec;
	}

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right before each test).
		errLogger.open("test_error_log.txt", ios::out | ios::app); // Open the error log file for writing
		db.setupForTest(&errLogger); // You would need to create a mock or a stub for this
		query.setupForTest(&db, &errLogger);

		args.databaseName = "dbPiholeLogTest";
		args.dbUserName = "logTestUser";
		args.dbUserPwd = "logTest:123";
		args.serverIPAddress = "192.168.1.110";
		args.serverPortNumber = "3306";
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right before the destructor).

		errLogger.close();
		// remove the open error log file

	}
};

TEST_F(DNSQueryTests, constructorTest)
{
	ASSERT_EQ( query._currentYear, 2026y ) << "Incorrect Year: is not 2026.\n";
	ASSERT_EQ(query._currentMonth, April) << "Check the current month against this test.\n";
}

TEST_F(DNSQueryTests, timeDateSetRequestTestFrom_tm)
{
	tm logTimeOfCall = {};
	setupLogTime(11, 31, 14, 30, 45, logTimeOfCall);
	string targetDomain = "example.com";
	string callerIPAddr = "192.168.1.102";

	query.setRequest(logTimeOfCall, targetDomain, callerIPAddr);
	EXPECT_EQ(query._dateOfQuery->year(), 2025y); // You would need to add assertions here to check the values of _timeOfQuery, _dateOfQuery, _domain, _callerIP, and _callerSubNetID
	EXPECT_EQ(query._dateOfQuery->month(), December); // month is 1 based in year_month_day
	EXPECT_EQ(query._dateOfQuery->day(), 31d) << "log entry from last year fails";

	// the next day
	logTimeOfCall.tm_mon = 0; //  January == 0
	logTimeOfCall.tm_mday = 1; // day of the month, so this represents the 1st
	query.setRequest(logTimeOfCall, targetDomain, callerIPAddr);
	EXPECT_EQ(query._dateOfQuery->year(), 2026y); // You would need to add assertions here to check the values of _timeOfQuery, _dateOfQuery, _domain, _callerIP, and _callerSubNetID
	EXPECT_EQ(query._dateOfQuery->month(), January); // month is 1 based in year_month_day
	EXPECT_EQ(query._dateOfQuery->day(), 01d);

}

TEST_F(DNSQueryTests, timeDateSetRequestTestFrom_string)
{
	string logTimeOfCall = "Jan 15 14:30:45";		// the date time string as it appears in the pihole log file, note that there is no year part to the time stamp on the log record so the setRequest method has to make an assumption about the year
	string targetDomain = "example.com";
	string callerIPAddr = "192.168.1.102";

	query.setRequest(logTimeOfCall, targetDomain, callerIPAddr);
	EXPECT_EQ(query._dateOfQuery->year(), 2026y); 
	EXPECT_EQ(query._dateOfQuery->month(), January);
	EXPECT_EQ(query._dateOfQuery->day(), 15d);
	EXPECT_EQ(query._timeOfQuery->hours().count(), 14); 
	EXPECT_EQ(query._timeOfQuery->minutes().count(), 30);
	EXPECT_EQ(query._timeOfQuery->seconds().count(), 45);

}

TEST_F(DNSQueryTests, blockerActionTest)
{
	query.blockerAction("forwarded");
	ASSERT_EQ(query._status, 0);
	query.blockerAction("gravity blocked");
	ASSERT_EQ(query._status, 1);
	query.blockerAction("cached");
	ASSERT_EQ(query._status, 2);
	query.blockerAction("regex blacklisted");
	ASSERT_EQ(query._status, 3);
	query.blockerAction("exactly blacklisted");
	ASSERT_EQ(query._status, 3);
	query.blockerAction("unknown action");
	ASSERT_EQ(query._status, 127);
}

TEST_F(DNSQueryTests, insertIntoDbTest)
{
	tm logTimeOfCall = {};
	setupLogTime(0, 1, 14, 30, 45, logTimeOfCall);
	string targetDomain = "example.com";
	string callerIPAddr = "192.168.1.102";

	query.setRequest(logTimeOfCall, targetDomain, callerIPAddr);
	query.blockerAction("forwarded");

	query.isInserted = false;
	int linesInserted = 0;
//	if (db.openConnection())
//	{
		std::chrono::duration<double> timeTaken = query.insertIntoDb(&linesInserted);
		EXPECT_EQ(linesInserted, 1);

		query.isInserted = true;
		timeTaken = query.insertIntoDb(&linesInserted);
		EXPECT_EQ(linesInserted, 1); // linesInserted should not have been incremented because the log entry was already marked as inserted
//		db.closeConnection();
//	}
//	else
//	{
//		FAIL() << "Failed to open database connection.";
//	}
}

TEST_F(DNSQueryTests, subNetIDFromIPAddrTest)
{
	tm logTimeOfCall = {};
	logTimeOfCall.tm_hour = 14;
	logTimeOfCall.tm_min = 30;
	logTimeOfCall.tm_sec = 45;
	string targetDomain = "example.com";
	string callerIPAddr = "192.168.1.102";

	query.setRequest(logTimeOfCall, targetDomain, callerIPAddr);
	int subNetID = query.subNetIDFromIPAddr(callerIPAddr);
	ASSERT_EQ(subNetID, 102); // You would need to add assertions here to check the value of subNetID for different callerIPAddr values
}

TEST_F(DNSQueryTests, retrieveLastLogDateTime)
{
	if (db.open(&args))
	{
		std::chrono::year_month_day lastDate;
		std::chrono::hh_mm_ss<std::chrono::seconds> lastTime;
		db.mostRecentDateTime(lastDate, lastTime);
		EXPECT_EQ(lastDate.year(), chrono::year(1970)) << "Retrieved an invalid Year from the database.";
		EXPECT_EQ(lastDate.month(), chrono::month(1)) << "Retrieved an invalid Month from the database.";
		EXPECT_EQ(lastDate.day(), chrono::day(1)) << "Retrieved an invalid Day from the database.";
		EXPECT_EQ(lastTime.hours().count(), 0) << "Retrieved an invalid Hour from the database.";
		EXPECT_EQ(lastTime.minutes().count(), 0) << "Retrieved an invalid Minute from the database.";
		EXPECT_EQ(lastTime.seconds().count(), 0) << "Retrieved an invalid Second from the database.";

		db.close();
	}
	else
	{
		FAIL() << "Failed to open database connection.";
	}
}