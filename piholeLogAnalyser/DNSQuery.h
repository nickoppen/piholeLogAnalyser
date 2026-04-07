#pragma once
#include <string>
#include <chrono>
#include <fstream>

#include "dbInterface.h"
#include "grok.h"

using namespace std;
using namespace std::chrono;



class dnsQuery
{

public:
	bool isInserted = false;

private:	// private for production code

#ifdef DEBUG
public:	// public for testing 


	dnsQuery()	
	{
		_dateOfQuery = nullptr;
		_timeOfQuery = nullptr;
	}
	void setupForTest(dbInterface * databaseInterface, ofstream* errorFile)
	{
		_db = databaseInterface;
		_errorLogger = errorFile;
	}
#endif // DEBUG

	std::chrono::year_month_day * _dateOfQuery;
	std::chrono::hh_mm_ss<std::chrono::seconds> * _timeOfQuery;
	dbInterface* _db;
	ofstream* _errorLogger;
	string _domain = "";
	string _callerIP = "";
	int _callerSubNetID;
	int _status;
	static std::chrono::month _currentMonth;	// Initialised to the current month by the setCurrentMonth() function at the bottom of this class
	static std::chrono::year _currentYear;		// Initialised to the current year by the setCurrentYear() function at the bottom of this class

public:
	dnsQuery(dbInterface * databaseInterface, ofstream* errorFile)
	{
		_dateOfQuery = nullptr;
		_timeOfQuery = nullptr;

		_db = databaseInterface;
		_errorLogger = errorFile;
	}
	~dnsQuery()
	{
		if (_dateOfQuery != nullptr)
			delete _dateOfQuery;
		if (_timeOfQuery != nullptr)
			delete _timeOfQuery;
	}	

	void setRequest(tm logTimeOfQuery, string targetDomain, string callerIPAddr)
	{
		_timeOfQuery = new hh_mm_ss<seconds>(hours(logTimeOfQuery.tm_hour) + minutes(logTimeOfQuery.tm_min) + seconds(logTimeOfQuery.tm_sec));

		// There is no year part to the time stamp on the log record so
		// start by assuming that the log record is in the current year
		std::chrono::year yearOfCall = _currentYear;
		std::chrono::month monthOfCall = month(logTimeOfQuery.tm_mon + 1); // tm_mon is 0 based, month is 1 based

		// The log records loaded on 1 Jan are mostly from Dec last year 
		// so assuming the current year is the year of the log line that would put Dec records in the future
		if (monthOfCall > _currentMonth)//static_cast<unsigned int>(_currentMonth));
		{
			yearOfCall -= years{ 1 };
		}

		_dateOfQuery = new year_month_day(yearOfCall, monthOfCall, day(logTimeOfQuery.tm_mday));

		_domain = targetDomain;
		_callerIP = callerIPAddr;
		_callerSubNetID = subNetIDFromIPAddr(callerIPAddr);
		isInserted = false;
	}

	void setRequest(string logTimeOfQuery, string targetDomain, string callerIPAddr)
	{
		tm logTimeOfQueryAs_tm = {};
		strptime(logTimeOfQuery.c_str(), "%b %d %H:%M:%S", &logTimeOfQueryAs_tm);
		setRequest(logTimeOfQueryAs_tm, targetDomain, callerIPAddr);
	}

	void blockerAction(string blockerAction)
	{
//		this->action = (blockerAction);
		if ((blockerAction.compare("forwarded") == 0) || (blockerAction.compare("cached-stale") == 0))
			_status = 0;
		else if (blockerAction.compare("gravity blocked") == 0)
			_status = 1;
		else if (blockerAction.compare("cached") == 0)
			_status = 2;
		else if ((blockerAction.compare("regex blacklisted") == 0) || (blockerAction.compare("exactly blacklisted") == 0))
			_status = 3;
		else
		{
			_status = 127;
			(*_errorLogger) << "Unknown blocker action: >" << blockerAction << "<" << endl;
		}
	}

	std::chrono::duration<double> insertIntoDb(int * linesInserted)
	{
		if (isInserted)
			return std::chrono::steady_clock::now() - std::chrono::steady_clock::now();// std::chrono::duration_values::zero;
		else
		{
			auto start = std::chrono::steady_clock::now();
			_db->insertLogEntry(_dateOfQuery, _timeOfQuery, _domain, _status, _callerSubNetID);
			auto end = std::chrono::steady_clock::now();
			(*linesInserted)++;
			isInserted = true;
			return end - start;
		}
	}

// testing private:
public:
	int subNetIDFromIPAddr(string ip)
	{
		// return the number after the third .
		size_t dotPos = 0;
		dotPos = ip.find(".", dotPos + 1);
		dotPos = ip.find(".", dotPos + 1);
		dotPos = ip.find(".", dotPos + 1);
		return std::stoi(ip.substr(dotPos + 1));
	}

private:
	static std::chrono::month setCurrentMonth()
	{
		const std::chrono::time_point aboutNow{ std::chrono::system_clock::now() };
		const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(aboutNow) };
		return static_cast<std::chrono::month>(ymd.month());
	}

	static std::chrono::year setCurrentYear()
	{
		const std::chrono::time_point aboutNow{ std::chrono::system_clock::now() };
		const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(aboutNow) };
		return static_cast<std::chrono::year>(ymd.year()); static_cast<std::chrono::year>(ymd.year());;
	}

};

std::chrono::month dnsQuery::_currentMonth = dnsQuery::setCurrentMonth();
std::chrono::year dnsQuery::_currentYear = dnsQuery::setCurrentYear();	


