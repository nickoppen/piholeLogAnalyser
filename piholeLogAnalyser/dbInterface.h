#pragma once

#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <iomanip>

#include <mariadb/conncpp.hpp>

#include "piholeLogAnalyserDefs.h"

using namespace std;
using namespace sql;
//using namespace std::chrono_literals;

// The standard template library does time very badly.
// This is the best way around the lack of functionality relating to file date/time that I've found
//template <typename TP>

//std::time_t to_time_t(TP tp)
//{
//	using namespace std::chrono;
//	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
//	return system_clock::to_time_t(sctp);
//}

class dbInterface
{
private:
	unique_ptr<Connection> _conn;
	unique_ptr<PreparedStatement> _insertStatement;
	ofstream* _errorLogger;
	bool _isADryRun;

public:
	dbInterface(bool dryRun, ofstream * errLogger)
	{
		_isADryRun = dryRun;
		_errorLogger = errLogger;
	}

#ifdef DEBUG		// for testing only - not intended to be used in production code
	dbInterface()
	{
		_isADryRun = true;
		_errorLogger = nullptr;
	}
	void setupForTest(ofstream* errorFile)
	{
		_errorLogger = errorFile;
	}
#endif // DEBUG

	~dbInterface()
	{

	}

	bool ckeckDbServer(cliArgs* args)		/// add database name, ip address and port
	{
		if (open(args))
		{
			close();
			(*_errorLogger) << "Check succeeded.\n";// << endl;
			return  true;
		}

		(*_errorLogger) << "Check failed." << endl;
		return false; // failed to open
	}

	bool open(cliArgs *args)		/// add database name, ip address and port
	{
		// there is a 256 byte memory leak here somewhere
		//Do not delete driver - a null pointer error occurs
		Driver* driver = sql::mariadb::get_driver_instance();

		// Configure Connection
		SQLString url("jdbc:mariadb://"+args->serverIPAddress+":"+args->serverPortNumber+"/"+args->databaseName);
		Properties properties({
								{"user", (args->dbUserName).c_str()},
								{"password", (args->dbUserPwd).c_str()}
							});
		properties["CONNECT_TIMEOUT"] = "10"; // seconds

		// Establish Connection
		try
		{
			unique_ptr<sql::Connection> connectShon(driver->connect(url, properties));
			_conn = move(connectShon);
		}
		catch (exception & ex)
		{
			(*_errorLogger) << ex.what() << endl;
			return false;
		}

		// Prepare the addLogEntry statement for repeated use
		string sql = "CALL addLogEntry(?, ?, ?, ?)";
		unique_ptr<PreparedStatement> statement(_conn->prepareStatement(sql));
		_insertStatement = move(statement);
		
		return true;
	}

	void close()
	{
		//if (!stmnt->isClosed())
		//	stmnt->close();
		if (!_conn->isClosed())
			_conn->close();
	}

	void mostRecentDateTime(chrono::year_month_day & maxDate, chrono::hh_mm_ss<std::chrono::seconds> & maxTime)
	{
		// default values in case the query fails or returns no results
		maxDate = chrono::year_month_day{ chrono::year{ 1970 }, chrono::month{ 1 }, chrono::day{ 1 } };
		maxTime = chrono::hh_mm_ss<std::chrono::seconds>{ chrono::seconds{ 0 } };

		string sql = "CALL maxDateYYYYMMDDHourMinSec;";
		unique_ptr<sql::PreparedStatement> stmnt(_conn->prepareStatement(sql));		

		try
		{
			sql::ResultSet* res = stmnt->executeQuery();
			res->first();
			while (!res->isAfterLast())
			{
				int yy = res->getInt(1);
				unsigned int mm = res->getInt(2);
				unsigned int dd = res->getInt(3);
				int hour = res->getInt(4);
				int min = res->getInt(5);
				int sec = res->getInt(6);

				std::chrono::day ddd = std::chrono::day{ dd };
				std::chrono::month mmm = std::chrono::month{ mm };
				std::chrono::year yyy = std::chrono::year{ yy };
				maxDate = yyy/mmm/ddd;
				maxTime = chrono::hh_mm_ss<std::chrono::seconds>(chrono::hours(hour) + chrono::minutes(min) + chrono::seconds(sec));
				res->next();	// should move past EOF
			}
			stmnt->getMoreResults();	// flush the rest of the buffer
			res->close();
			delete res;
		}
		catch (exception & ex)
		{
			(*_errorLogger) << "Error on executeQuery in maxDate :" << ex.what() << endl;
		}

		return;
	}

	bool updateTblCommon()
	{
		string sql = "CALL appendToCommon;";
		unique_ptr<sql::PreparedStatement> stmnt(_conn->prepareStatement(sql));
		//sql::ConnectOptionsMap connection_properties;
		//unsigned int timeout = 180;
		//connection_properties["MYSQL_OPT_WRITE_TIMEOUT"] = timeout; // max_connection_time on the server is set to 0 (no timeout)???
		//_conn->c

		try
		{
			if (!_isADryRun)
			{
				ResultSet * res = stmnt->executeQuery();
				res->close();
				delete res;		// to prevent a memory leak
			}
			else
				cout << "(Dry Run) " << sql << endl;
		}
		catch (exception& ex)
		{
			(*_errorLogger) << "Error on executeQuery in updateTblCommon :" << ex.what() << endl;
			return false;
		}
		return true;
	}

	bool updateLevelOfInterestFromDate(std::chrono::year_month_day fromDate)
	{
		string sql = "CALL updateAllLOIFromDate('" + std::format("{:%F}", fromDate) + "');";
		unique_ptr<sql::PreparedStatement> stmnt(_conn->prepareStatement(sql));

		try
		{
			if (!_isADryRun)
			{
				stmnt->setQueryTimeout(600);
				ResultSet* res = stmnt->executeQuery();
				res->close();
				delete res;		// to prevent a memory leak
			}
			else
				cout << "(Dry Run) " << sql << endl;
		}
		catch (exception& ex)
		{
			(*_errorLogger) << "Error on executeQuery in updateAllLOIFromDate :" << ex.what() << endl;
			return false;
		}
		return true;
	}

	bool insertLogEntry(std::chrono::year_month_day * dayOfQuery, std::chrono::hh_mm_ss<std::chrono::seconds> * timeOfQuery, string domain, int status, int callerSubNetID)
	{
		string formatedDateTime = std::format("{:%F}", *dayOfQuery) + " " + std::format("{:%T}", *timeOfQuery);
		//string sql = "CALL addLogEntry('" + domain + "', " + std::to_string(callerSubNetID) + ", " + std::to_string(status) + ", '" + formatedDateTime + "')";
		//TODO: check formateDate + formatTime

		try
		{
			if (!_isADryRun)  // for testing
			{
				_insertStatement->setString(1, domain);
				_insertStatement->setInt(2, callerSubNetID);
				_insertStatement->setInt(3, status);
				_insertStatement->setString(4, formatedDateTime);

				sql::ResultSet * res = _insertStatement->executeQuery();
				res->close();
				delete res;		// to prevent a memory leak
			}
			else
				cout << "(Dry Run) addLogEntry: " << domain << " at: " << formatedDateTime << " status: " << status << " from: " << callerSubNetID << endl;
		}
		catch (exception& ex)
		{
			(*_errorLogger) << "Error on executeQuery in insertLogEntry :" << ex.what() << endl;
			return false;
		}

		return true;
	}

	bool recordLogFile(long fileSize, const filesystem::file_time_type fileDateTime, string * logKey)
	{
		string time_c_str = format("{:%F %T}", fileDateTime);
		(*logKey) = std::to_string(fileSize) + ", '" + time_c_str;
		string sql = "CALL recordLogFile(" + *logKey + "')";
		unique_ptr<sql::PreparedStatement> stmnt(_conn->prepareStatement(sql));

		try
		{
			if (!_isADryRun)
			{
				ResultSet* res = stmnt->executeQuery();
				res->close();
				delete res;		// to prevent a memory leak
			}
			else
				cout << "(Dry Run) " << sql << endl;
			return true;
		}
		catch (exception &ex)
		{
			(*_errorLogger) << "Record Log File Error (insert): " << ex.what() << endl;
			// return the sql error code for main to distinguish duplicates (to skip the file) from other errors (abort)
			return false;
		}
		catch (int e)
		{
			(*_errorLogger) << "Record Log File Error (insert): " << e << endl;
			return false;
		}	
	}

	bool updateLogFileRecord(string * logKey, int logRowsProcessed, string comment)//long fileSize, filesystem::file_time_type fileDateTime, int logRowsProcessed, string comment)
	{
		// UPDATE will not fail if fileSize or fileDateTime has changed since RecordLogFile was called
		//unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("CALL updateLogFileRecord(" + std::to_string(fileSize) + ", '" + formatDateTime(fileDateTime) + "', " + std::to_string(logRowsProcessed) + ", '" + comment + "')"));
		string sql = "CALL updateLogFileRecord(" + *logKey + "', " + std::to_string(logRowsProcessed) + ", '" + comment + "')";
		unique_ptr<PreparedStatement> stmnt(_conn->prepareStatement(sql));

		try
		{
			if (!_isADryRun)
			{
				ResultSet* res = stmnt->executeQuery();
				res->close();
				delete res;		// to prevent a memory leak
			}
			else
				cout << "(Dry Run) " << sql << endl;
			return true;
		}
		catch (exception &ex)
		{
			(*_errorLogger) << "Record Log File Error (update): " << ex.what() << endl;
			return false;
		}
	}

	const bool willExecuteDbCommands()
	{
		return !_isADryRun;
	}


};

