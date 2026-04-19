#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
//#include <cstring>
#include <format>
#include <regex>
#include <chrono>

#include "piholeLogAnalyserDefs.h"
#include "dbInterface.h"
#include "grok.h"
#include "DNSQuery.h"

using namespace std;

string nowAsString()
{
    auto now = std::chrono::system_clock::now();
    auto zt = std::chrono::zoned_time{ std::chrono::current_zone(), now };
    return std::format("{:%F %T}", zt);
}

int appendFoundFilesInDirectoryToList(const filesystem::directory_entry file, regex rx, list<filesystem::path> * fileList)
{
    if (regex_match(file.path().filename().c_str(), rx))
    {
        fileList->push_front(file.path());   
        return 1;
    }
    else
        return 0;
}

int searchDirectoryForMatchingFilesAndAppendToList(cliArgs* args, list<filesystem::path> * fileList, ofstream * errorLogger)
{
    filesystem::path logPath(args->searchDirName);
    if (filesystem::is_directory(logPath))
    {
        int foundFiles = 0;
        filesystem::path logFile(args->searchDirName);

        if (args->recurse)
            for (const auto& file : filesystem::recursive_directory_iterator(logPath))
                foundFiles += appendFoundFilesInDirectoryToList(file, args->rx, fileList);
        else
            for (const auto& file : filesystem::directory_iterator(logPath))
                foundFiles += appendFoundFilesInDirectoryToList(file, args->rx, fileList);

        return foundFiles;
    }
    else
    {
        (*errorLogger) << logPath << " is not a directory" << endl;
        return 0;
    }
}

void checkCommandLineArgs(cliArgs* args, string errorOutputFile)
{
    ofstream errorLogger(errorOutputFile.c_str());// check if path to errorOutputFile exits
    dbInterface tempDb(args->dryRun, &errorLogger);

    if (tempDb.ckeckDbServer(args))
        cout << "The database is good" << endl;
    else
        cout << "Could not open the database (see " << errorOutputFile << ")" << endl;

	cout << "The grok string is: " << args->grokString << endl;

    try
    {
        list<filesystem::path> filenameList;
        if (searchDirectoryForMatchingFilesAndAppendToList(args, &filenameList, &errorLogger) == 0)
            cout << "There are no files matching " << args->fileSpec << " (" << args->rxFileSpec << ") " << " in " << args->searchDirName << endl;
        else
        {
            unsigned long int i = filenameList.size();

            cout << "Found "<< i << " files:" << endl;
            
            for (std::list<filesystem::path>::iterator filenameInList = filenameList.begin(); filenameInList != filenameList.end(); ++filenameInList)
            {
                cout << "Matched: " << (*filenameInList) << endl;
            }
        }
    }
    catch (filesystem::filesystem_error& fse)
    {
        cout << "Could not find custom search directory " + args->searchDirName + " (error: " << fse.what() << ")" << endl;
    }

    //// check that the grok custom pattern file is available
    try
    {
        filesystem::path customPatternFile(args->customPatternFilename);
        if (filesystem::is_empty(customPatternFile))
            cout << "The custom pattern file exists but is empty." << endl;
        else
            cout << "The custom pattern file looks good." << endl;

    }
    catch (filesystem::filesystem_error& fse)
    {
        cout << "Error for " + args->customPatternFilename + " (error: " << fse.what() << ")" << endl;
    }

    if (tempDb.willExecuteDbCommands())
        cout << "The system will update the database (NOT a dry run)." << endl;
    else
        cout << "The system will NOT update the database (a dry run)." << endl;

    errorLogger.close();

#ifdef DEBUG
    cout << "This is the DEBUG build" << endl;
#else
	cout << "This is the RELEASE build" << endl;
#endif // DEBUG

    cout << "Exiting after check..." << endl;

}

void readArgs(int argc, char** argv, cliArgs * args)
{
    bool checkOnCompletion = false;

    args->dryRun = false;
    args->recurse = false;
    args->searchDirName = "/var/log/pihole";
    args->fileSpec = "pihole.log.1";
    args->rxFileSpec = "pihole\\.log\\.1";
    args->rx = regex(args->rxFileSpec);
    args->errorPathFilename = "./loadError.txt";
    args->maxExecTimeInMilliseconds = 0;
    args->customPatternFilename = "./grokCustom.txt";
    args->dbUserName = "logUser";
    args->dbUserPwd = "oppen20:Log";
    args->serverIPAddress = "192.168.1.110";
    args->serverPortNumber = "3306";
    args->databaseName = "dbPiholeLog";
    args->grokString = "%{LOGTIME:Timestamp:datetime} %{LOGPROG:Prog}: ((%{LOGACTIONFROM:ActionFrom} %{LOGDOMAIN:DomainFrom} %{LOGDIRECTIONFROM:DirectionFrom} %{LOGEOLFROM:EndOfLineFrom})|(%{LOGACTIONTO:ActionTo} %{LOGDOMAIN:DomainTo} %{LOGDIRECTIONTO:DirectionTo} %{LOGEOLTO:EndOfLineTo})|(%{LOGACTIONIS:ActionIs} %{LOGDOMAIN:DomainIs} %{LOGDIRECTIONIS:DirectionIs} %{LOGEOLIS:EndOfLineIs}))";
    args->grokStringFilename = "./grokString.txt";
    string arg;

    for (int i = 1; i < argc; i++)  // argv[0] is the executable name
    {
        arg = argv[i];
        if (arg.compare("-r") == 0)
            args->recurse = true;

        else if (arg.compare("-d") == 0)
            args->searchDirName = string(argv[++i]);

        else if (arg.compare("-f") == 0)
        {
            args->fileSpec = string(argv[++i]);

            regex rxMeta("\\[|\\]|\\.|\\:|\\+|\\,|\\^|\\{|\\}|\\~");            // all regex special characters that can appear in a linux filename
            args->rxFileSpec = regex_replace(args->fileSpec, rxMeta, R"(\$&)");

            rxMeta.assign("\\?");                                               // replace ? with a regex .
            args->rxFileSpec = regex_replace(args->rxFileSpec, rxMeta, R"(.)");
            rxMeta.assign("\\*");                                               // replace * with regex any number of valid file characters
            args->rxFileSpec = regex_replace(args->rxFileSpec, rxMeta, R"(([0-9]|[a-z]|[A-Z]|-|_|\[|\]|\.|\:|\+|\,|\^|\{|\}|\~)*)");    

            args->rx = regex(args->rxFileSpec);      
        }

        else if (arg.compare("-p") == 0)
            args->dryRun = true;

        else if (arg.compare("-t") == 0)
            args->maxExecTimeInMilliseconds = stol(string(argv[++i]));

        else if (arg.compare("-gp") == 0)
            args->customPatternFilename = string(argv[++i]);

        else if (arg.compare("-gs") == 0)
        {
            args->grokStringFilename = string(argv[++i]);
			ifstream grokStringFile(args->grokStringFilename);
            if (grokStringFile.is_open())
            {   
                string grokStringFromFile;
                getline(grokStringFile, grokStringFromFile);
                if (grokStringFromFile.empty())
                {
                    cout << "The grok string file " << args->grokStringFilename << " is empty (exiting)" << endl;
                    exit(1);
                }
                else
					args->grokString = grokStringFromFile;
            }
           else
            {
                cout << "Could not open grok string file " << args->grokStringFilename << " (exiting)" << endl;
                exit(1);
			}
        }

        else if (arg.compare("-e") == 0)
            args->errorPathFilename = string(argv[++i]);

        else if (arg.compare("-check") == 0)
            checkOnCompletion = true;

        else if (arg.compare("-user") == 0)
            args->dbUserName = string(argv[++i]);

        else if (arg.compare("-pwd") == 0)
            args->dbUserPwd = string(argv[++i]);

        else if (arg.compare("-ip") == 0)
            args->serverIPAddress = string(argv[++i]);

        else if (arg.compare("-port") == 0)
            args->serverPortNumber = string(argv[++i]);

        else if (arg.compare("-db") == 0)
            args->databaseName = string(argv[++i]);

        else if (arg.compare("-test") == 0)
            args->testRxString = string(argv[++i]);

        else
        {
            cout << "Invalid argument: " << arg << endl;
            cout << endl;
            cout << "-d Directory : directory to be scanned (default: /var/log/pihole)" << endl;
            cout << "-r : scan recursively (recurse == false)" << endl;
            cout << "-f 'Filename pattern' : load a specific log file with wildcards ? and * (default: pihole.log.1)" << endl;
            cout << "-p : Pretend to add to the database (default: false)" << endl;
            cout << "-t milliseconds : max exec time for regex (default == 0 or no time limit)" << endl;
            cout << "-gp File name : specify the location of the grok pattern file (default: ./grokCustom.txt)" << endl;
            cout << "-gs File name : The file containing the applications grok string (default: ./grokString.txt)" << endl;
            cout << "-e Full path to the load error file" << endl;
            cout << "-user username : log in to the database with the given username" << endl;
            cout << "-pwd pwd : log into the database with the given password" << endl;
            cout << "-ip IP Address : look for the database at the given IP address or url" << endl;
            cout << "-port portNum : connect to the database on the given port" << endl;
            cout << "-db dbName : connect to the database with the given name" << endl;
            cout << "-check : checks the database, directory and file spec for log files the grok custom pattern file and then exits (output in -e load error file)" << endl;
            cout << "-h or -help : print this text" << endl;
            exit(1);
        }
    }
    if (checkOnCompletion)
    {
        checkCommandLineArgs(args, args->errorPathFilename);
        exit(0);
    }
}

std::chrono::duration<double> processLogFile(string pathFileName, grokplusplus::grok* grk, dbInterface * db, ofstream* errorLogger, int * linesInserted, unsigned long timeLimitInMilliseconds)
{
    std::chrono::duration<double> insertTimeForThisFileInSeconds;// = std::chrono::duration::zero;
    ifstream logFile(pathFileName);

    if (logFile.is_open())
    {
        dnsQuery currentQuery(db, errorLogger);
        bool dnsServerReplied = false;
        string logLine;
        string action = "";
        string actionTruncated = "";
        tm timeOfCall;
        memset(&timeOfCall, 0, sizeof(tm));
        string domainFrom = "";
        string callerIP = "";
        string valueAsString = "";
        char timeFormat[] = "%b %d %H:%M:%S";
        grokplusplus::grokResult* grkRes;

        while (!logFile.eof())
        {
            getline(logFile, logLine, '\n');
            grkRes = grk->parse(logLine, timeLimitInMilliseconds);
            if (grkRes->timedOut)
            {
                (*errorLogger) << "Time out: >" << logLine << "<" << endl;
                continue;
            }

            if (!grkRes->matched)
            {
                (*errorLogger) << "Line not matched: >" << logLine << "<" << endl;
                continue;
            }

            action = (*grkRes)["ActionFrom"].valueAsString() + (*grkRes)["ActionTo"].valueAsString() + (*grkRes)["ActionIs"].valueAsString();
            actionTruncated = action.substr(0, 5);

            if (actionTruncated.compare("query") == 0)      /// covers all varieties
            {
                if ((*grkRes)["Timestamp"].value(&timeOfCall, &valueAsString, timeFormat))   // piholeLogDateTimeFormat))
                {
                    if ((*grkRes)["DomainFrom"].value(&domainFrom))
                    {
                        if ((*grkRes)["EndOfLineFrom"].value(&callerIP))
                            currentQuery.setRequest(timeOfCall, domainFrom, callerIP);
                        else
                            (*errorLogger) << "Error at EndOfLineFrom: " << callerIP << endl;
                    }
                    else
                        (*errorLogger) << "Error at DomainFrom: " << domainFrom << endl;
                }
                else
                    (*errorLogger) << "Error at TimeStamp: " << valueAsString << endl;

                continue;
            }
            
            if ((actionTruncated.compare("forwa") == 0) ||
                (actionTruncated.compare("cache") == 0) ||
                (actionTruncated.compare("exact") == 0) ||
                (actionTruncated.compare("gravi") == 0) ||
                (actionTruncated.compare("regex") == 0))
            {
                currentQuery.blockerAction(action);
                if (dnsServerReplied)
                {
                    insertTimeForThisFileInSeconds += currentQuery.insertIntoDb(linesInserted);
                }
                continue;
            }
            
            if (actionTruncated.compare("reply") == 0)
            {
                if (!dnsServerReplied)      // store the first log entry that recieved a replay and then store from then on
                {
                    insertTimeForThisFileInSeconds += currentQuery.insertIntoDb(linesInserted);
                    dnsServerReplied = true;
                }
                continue;
            }

            if ((actionTruncated.compare("confi") == 0) ||
                (actionTruncated.compare("speci") == 0) ||
                (actionTruncated.compare("Apple") == 0) ||
                (actionTruncated.compare("Rate-") == 0))
            {
                continue;   // Ignore these actions
            }

            (*errorLogger) << "Unknown: " << endl;
        }
    }
    else
        (*errorLogger) << "File: " << pathFileName << " failed to open." << endl;

    return insertTimeForThisFileInSeconds;
}

int main(int argc, char** argv)
{
    int exitCode = 1;
    cliArgs args;
    readArgs(argc, argv, &args);


    ifstream customPatterns(args.customPatternFilename);
    ofstream errorLogger;
    errorLogger.open(args.errorPathFilename.c_str());

    errorLogger << "Opened: " << nowAsString() << endl;   

    if (customPatterns.is_open())
    {
        list<filesystem::path> fileList;

        if (searchDirectoryForMatchingFilesAndAppendToList(&args, &fileList, &errorLogger) > 0)
        {
            cout << nowAsString() << " : " << fileList.size() << " file(s) found" << endl;

            try
            {
                dbInterface db(args.dryRun, &errorLogger);

                if (db.open(&args))
                {
                    stringstream msg;

                    grokplusplus::grok grk(args.grokString, &customPatterns);

                    /*errorLogger <<*/ grk.parseGrokString();// << endl;

                    std::chrono::duration<double> dbInsertTimeInSeconds;
                    std::chrono::duration<double> fileProcessingTimeInSeconds;
                    std::chrono::duration<double> totalProcessingTimeInSeconds;// = std::chrono::duration::zero;
                    int linesInserted = 0;
                    int filesProcessed = 0;
                    std::filesystem::file_time_type fileLastWriteTime;
                    unsigned long int fileSize;
                    string readLogKey;

                    std::chrono::year_month_day lastDate;     // deleted at the end of this if block
                    std::chrono::hh_mm_ss<std::chrono::seconds> lastTime;
                    db.mostRecentDateTime(lastDate, lastTime);

                    for (list<filesystem::path>::iterator fileIter = fileList.begin(); fileIter != fileList.end(); fileIter++)
                    {
                        fileLastWriteTime = filesystem::last_write_time(*fileIter);
                        fileSize = filesystem::file_size(*fileIter);
                        if (db.recordLogFile(fileSize, fileLastWriteTime, &readLogKey))
                        {
                            auto start = std::chrono::steady_clock::now();
                            dbInsertTimeInSeconds = processLogFile(*fileIter, &grk, &db, &errorLogger, &linesInserted, args.maxExecTimeInMilliseconds);
							auto end = std::chrono::steady_clock::now();
                            fileProcessingTimeInSeconds = end - start;
                            
                            msg << nowAsString() << " File: " << fileIter->filename() << " processing time: " << fileProcessingTimeInSeconds << " seconds,  insert time : " << dbInsertTimeInSeconds.count() << " seconds for " << linesInserted << " lines";
                            db.updateLogFileRecord(&readLogKey, linesInserted, msg.str());

                            filesProcessed++;
                            totalProcessingTimeInSeconds += fileProcessingTimeInSeconds;
                        }
                        else
                        {
                            msg << "It looks like " << fileIter->filename() << " is a duplicate.";
                        }
                        cout << msg.str() << endl;
                        msg.str("");
                    }

                    if (filesProcessed > 0)
                    {
#ifdef DEBUG
                        db.close();

                        cout << endl << "About to run updateTblCommon" << endl;
                        db.open(&args);
#endif // DEBUG
                        auto start = std::chrono::steady_clock::now();
                        db.updateTblCommon();
                        auto end = std::chrono::steady_clock::now();
                        std::chrono::duration<double> duration_seconds = end - start;
                        totalProcessingTimeInSeconds += duration_seconds;

#ifdef DEBUG
                        db.close();
                        cout << endl << "Done with updateTblCommon in " << duration_seconds.count() << " seconds. " << endl;

                        cout << endl << "About to run updateLevelOfInterestFromDate: " << lastDate;
                        db.open(&args);
#endif // DEBUG
                        start = std::chrono::steady_clock::now();
                        db.updateLevelOfInterestFromDate(lastDate);
                        end = std::chrono::steady_clock::now();
                        duration_seconds = end - start;
                        totalProcessingTimeInSeconds += duration_seconds;

#ifdef DEBUG
                        cout << endl << "Done with updateLOI in " << duration_seconds.count() << " seconds. " << endl;
                        cout << endl << "Done with bulk updates" << endl;
#endif // DEBUG

                        cout << nowAsString() << " Files processed: " << filesProcessed << " in " << totalProcessingTimeInSeconds.count() << " seconds." << endl;

                    }

                    db.close();
                    exitCode = 0;
                }
            }
            catch (grokplusplus::grokException& gEx)
            {
                cout << gEx.what() << endl;
            }
            catch (exception& ex)
            {
                cout << ex.what() << endl;
                errorLogger << ex.what() << endl;
            }
            catch (int e)
            {
                cout << e << endl;
                errorLogger << "Error: " << e << endl;
            }
            catch (...)
            {
                cout << "Yeah... dunno!" << endl;
                errorLogger << "Unknown Exception" << endl;
            }
        }
        else
            cout << "No files found in: " << args.searchDirName << endl;

        customPatterns.close();
    }
    else
        cout << "grokCustom.txt not open (or not found)." << endl;

    errorLogger << "About to close" << endl;
    errorLogger.close();

    return exitCode;
}