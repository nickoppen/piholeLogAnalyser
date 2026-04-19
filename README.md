# Before starting
Insure that you have permission to use sudo.
Install the minimum packages:
            sudo apt update
            sudo apt install build-essential cmake curl g++ gdb git gh make rsync

Log into your github account using gh:
            gh auth login... (however you do this)
Download the source code for this project from using gh:
            gh repo clone nickoppen/piholeLogAnalyser


# Grok++ library
Grok++ is a C++ library for parsing log files using the "Grok" extensions to basic regular expressions. It has nothing to do with the many other uses of the word (https://en.wikipedia.org/wiki/Grok_(disambiguation)). For an introduction see https://www.elastic.co/docs/reference/logstash/plugins/plugins-filters-grok.
This is a implementation of Roman Marusyk's [Grok.Net](https://github.com/Marusyk/grok.net). All credit for the design goes to him.

## To Compile
- Compile to gnu++17 standard or higher
- Copy the sources files and CMakeLists.txt from this project to your project directory
- Include the Grok++ library in your projects top level CMakeLists.txt file (i.e. add_subdirectory ("Grok++"))

## To Use
- Copy [grok-patterns](http://grokconstructor.appspot.com/groklib/grok-patterns) file to the executable's directory
- Develop your top level grok string and custom pattern file (see grokCustom.txt for an example) and pass them into the grok constructor (see example code in Grok++/README.md) to create a grok object that can be used to parse log lines.
- To practice grok string construction and test your patterns use the grok constructor at https://grokconstructor.appspot.com/ 

# PiholeLogAnalyser
An application that uses Grok++ to interpret and load pihole log files into a MariaDB database.
Written in C++20 and developed on piOS and Ubuntu (24.04).

## Prerequisites:
- An accessible, running instance of MariaDB with a database and user with permissions to create tables and insert data (see database schema) and a user with permissions to read and write to the pihole log tables.
- Raspberry Pi OS Bullseye or later or Ubuntu 24.04 or later (other Linux distros may work but have not been tested)
- Ensure that the compiler supports C++20 (std::chrono and std::format) and that the necessary libraries are installed
- MariaDB_connector_cpp available from https://mariadb.com/docs/connectors/mariadb-connector-cpp/install-mariadb-connector-cpp (it is a bit tedious but stick at it)

## To Compile
- Clone the repository using gh as above or download the source code as a zip file and extract it
- Copy the Grok++ directory containing source files and CMakeLists.txt to your project directory
- Use CMake to build the project. For example, you can create a build directory and run the following commands:
            mkdir build
            cd build
            cmake ..
            make

## To Run
- Set up the database as per the database schema
- Root priviledges are needed to read /var/log/pihole/pihole.log.1 (so to run yesterday's log file run the executable with sudo)
- Copy the grok custom pattern file, grokCustom.txt to current directory. Note: if you have scheduled execution using cron then . (current directory) is the users home directory. 
- Run the executable with the -p option and the -check option to check that the database connection, log file specification and grok custom pattern file are all correct and that log files can be found. Output will be written to the load error file (default: ./loadError.txt) and the program will exit. If there are no errors then the load error file will be empty.
- When the check option returns no errors and you are ready to load data into the database run the executable with the -p option to do a test run. This will go through all the motions of loading the data but not actually insert anything into the database (useful for testing and debugging).
- When everthing is working run the executable without the -p option to load the data into the database. 
- Commandline options are:

            -d Directory : directory to be scanned (default: /var/log/pihole)
            -r : scan recursively (recurse == false)
            -f 'Filename pattern' : load a specific log file with wildcards ? and \* (default: pihole.log.1) Note: '' to ensure that the wildcards are not expanded by the shell
            -p : Pretend to add to the database (default: false)
            -t milliseconds : max exec time for regex match function (default == 0 or no time limit)
            -gp File name : specify the location of the grok pattern file (default: ./grokCustom.txt)
            -gs File name : The file containing the applications grok string (default: ./grokString.txt)
            -e Full path to the load error file (default: ./loadError.txt)
            -user username : log in to the database with the given username
            -pwd pwd : log into the database with the given password
            -ip IP Address : look for the database at the given IP address or url
            -port portNum : connect to the database on the given port
            -db dbName : connect to the database with the given name
            -check : checks the database, directory and file spec for log files the grok custom pattern file and then exits (output in load error file)
            -h or -help : print this text

## Automated Mode
To automate the execution of the program you can set up a cron job. For example, to run the program every day at 2am you can add the following line to your crontab file (use `crontab -e` to edit your crontab):

            0 2 * * * /path/to/piholeLogAnalyser -d /var/log/pihole -f 'pihole.log.1' -gp /path/to/grokCustom.txt -gs /path/to/grokString.txt -e /path/to/loadError.txt -user dbUser -pwd dbPassword -ip dbIP -port dbPort -db dbName

I use a shell script to run the program in automated mode and to handle the output of the load error file (it's easier than mucking around with crontab all the time). The cron job looks like this:

            0 2 * * * /path/to/runPiholeLogAnalyser.sh

And the shell script looks like this:

``` bash
# Make a copy of the log file
now=$(date +%F_%T)
localFile="/home/user0/pihole/logs/$now.log"
sudo cp /var/log/pihole/pihole.log.1 $localFile
sudo chown user0 $localFile

sleep 5

# Run the pihole log analyser and redirect output to consoleOutput.txt and cron errors to cronError.txt (note: this is different to the error file that is specified in the commandline options which contains error in the log file written by the application)
sudo /home/user0/projects/piholeLogAnalyser/out/build/linux-release/piholeLogAnalyser/piholeLogAnalyser  >> consoleOutput.txt 2> cronError.txt
```
