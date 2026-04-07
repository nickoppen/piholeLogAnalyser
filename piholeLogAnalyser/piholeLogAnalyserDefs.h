#pragma once

// move to the main file
#include <string>
#include <regex>

struct cliArgs
{
    bool dryRun;
    std::string fileSpec;
    std::string rxFileSpec;
    std::regex rx;
    std::string searchDirName;
    bool recurse;
    std::string errorPathFilename;
    unsigned long maxExecTimeInMilliseconds;
    std::string customPatternFilename;
	std::string grokString;
    std::string grokStringFilename;
    std::string dbUserName;
    std::string dbUserPwd;
    std::string serverIPAddress;
    std::string serverPortNumber;
    std::string databaseName;

    std::string testRxString;   // testing
};


