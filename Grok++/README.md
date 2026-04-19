# Grok++ Basic Usage Example

You can use the grok constructor at https://grokconstructor.appspot.com/ to practice constructing your grok string and testing it against sample log lines.

```cpp 
#include "Grok++.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>


int main() 
{
	// Assign the grok string to be passed into the parse function. 
	// Logs change over time so you may need to update your grok string to reflect changes in the log format therefore it is good practice to keep the grok string in a separate file (see grokString.txt) and read it in at runtime. 
	std::string grokString = "%{LOGTIME:Timestamp:datetime} %{LOGPROG:Prog}: ((%{LOGACTIONFROM:ActionFrom} %{LOGDOMAIN:DomainFrom} %{LOGDIRECTIONFROM:DirectionFrom} %{LOGEOLFROM:EndOfLineFrom})|(%{LOGACTIONTO:ActionTo} %{LOGDOMAIN:DomainTo} %{LOGDIRECTIONTO:DirectionTo} %{LOGEOLTO:EndOfLineTo})|(%{LOGACTIONIS:ActionIs} %{LOGDOMAIN:DomainIs} %{LOGDIRECTIONIS:DirectionIs} %{LOGEOLIS:EndOfLineIs}))";
	
	// Create a Grok object with the custom pattern file
	// The custom pattern file contains any custom expansions used in your grok string. 
	// The grok pattern file contains common patterns that you can use in your grok string. It should be in the same directory as your application's executable.
	std::string customPatternFilename = "grokCustom.txt";
	std::ifstream customPatterns(customPatternFilename);
	if (!customPatterns.is_open()) 
	{
		std::cerr << "Error opening custom pattern file: " << customPatternFilename << std::endl;
		return 1;
	}

	grokplusplus::grok grok(grokString, customPatterns);
	// Patterns can also be added programmatically using the addPatterns(map<string, string>) function. The first string in the pair is the name of the pattern (e.g. "LOGEMAIL"") and the second string the pattern ("[a-zA-Z][a-zA-Z0-9_.+-=:]+@%{LOGDOMAIN}").
	grok.parsePatterns();

	// Read the log line to be parsed (in practice you would read this from a file or stream)
	std::string logLine = "Mar 28 00:15:14 dnsmasq[65558]: query[A] 192-168-1-122.tpgi.com.au from 192.168.1.102";	

	// Parse the log line using the grok string	
	// std::regex can sometimes stall when matching against certain patterns and inputs therefore a timeout can be set to prevent this while you are debugging your grok string. 
	// A timeout of 0 means no time limit.
	int timeoutInMilliseconds = 1000; // 1 second timeout
	grokplusplus::grokResult * result = grok.parse(logLine, timeoutInMilliseconds);

	// Print the parsed results
	std::cout << "Parsed Results:" << std::endl;
	if (result->timedOut) 
	{
		std::cout << "Parsing timed out before " << timeoutInMilliseconds << " milliseconds." << std::endl;
		return 1;
	} 
	if (result->match) 
	{
		// Access the parsed values using the names defined in the grok string (grokResult is currently not iterable so you need to know the names of the values you want to access))
		// Use the valueAsString function to get the value as a string 
		std::cout << "Action From: " << (*result)["ActionFrom"].valueAsString() << std::endl;

		tm timeOfCall;
        memset(&timeOfCall, 0, sizeof(tm));
        char timeFormat[] = "%b %d %H:%M:%S";

		// Use the value function to get the value converted to a specific type (e.g. datetime, int, etc.) if you have specified a type in your grok string.
		cout << "Timestamp: " << (*result).value(&timeOfCall, &valueAsString, timeFormat)) << std::endl;

	} 
	else 
	{
		std::cout << "No match found." << std::endl;
	})


	return 0;	
}
	```
