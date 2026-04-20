#include <gtest/gtest.h>
#include "grok.h"

using namespace grokplusplus;

// IMPORTANT: Don't forget to copy the grok-patterns file to the same directory as the test executable 

struct GrokTest : public ::testing::Test {

	std::string locationOfGrokStringFile = "/home/user0/grokString.txt";			// Change this path to the location of the test string file on your machine
	std::string locationOfGrokCustomPatternFile = "/home/user0/grokCustom.txt";		// Change this path to the location of the custom pattern file on your machine

	grok * grk;
	grokResult* grkRes;
	ifstream grkCustomPatternFile;
	fstream grkStringFile;

	virtual void SetUp() 
	{
		std::string grokPattern;
		grkStringFile.open(locationOfGrokStringFile, ios::in);			
		if (grkStringFile.is_open())
		{
			getline(grkStringFile, grokPattern);
		}
		else
			throw runtime_error("Unable to open the test string file");

		grkCustomPatternFile.open(locationOfGrokCustomPatternFile, ios::in);	
		if (!grkCustomPatternFile.is_open())
		{
			throw runtime_error("Unable to open the test pattern file");
		}

		grk = new grok(grokPattern, &grkCustomPatternFile);					// In production the grok instance would be a stack variable. It's a pointer here because of the way the test is structured.
		grk->parseGrokString();
	}

	virtual void TearDown() 
	{
		if(grkCustomPatternFile.is_open())
			grkCustomPatternFile.close();
		if (grkStringFile.is_open())		
			grkStringFile.close();
		delete grk;
	}
};

TEST_F(GrokTest, TestGrokPatternDirectionFrom)
{
	string testString = "Mar 28 00:15:21 dnsmasq[65558]: query[A] plex.tv from 192.168.1.110";
	grokResult* grkRes = grk->parse(testString, 1000);
	EXPECT_TRUE(grkRes->matched);

	if (grkRes->matched)
	{
		std::string timestamp = (*grkRes)["Timestamp"].valueAsString();
		EXPECT_EQ(timestamp, "Mar 28 00:15:21");

		std::string actionFrom = (*grkRes)["ActionFrom"].valueAsString();
		EXPECT_EQ(actionFrom, "query[A]");

		std::string domainFrom;
		(*grkRes)["DomainFrom"].value(&domainFrom);
		EXPECT_EQ(domainFrom, "plex.tv");

		std::string eolFrom;
		(*grkRes)["EndOfLineFrom"].value(&eolFrom);
		EXPECT_EQ(eolFrom, "192.168.1.110");
	}
}

TEST_F(GrokTest, TestGrokPatternDirectionTo)
{
	string testString = "Feb 23 11:54:34 dnsmasq[1569]: forwarded api.insight.synology.com to 203.12.160.35";
	grokResult* grkRes = grk->parse(testString, 1000);
	EXPECT_TRUE(grkRes->matched);

	if (grkRes->matched)
	{
		std::string timestamp = (*grkRes)["Timestamp"].valueAsString();
		EXPECT_EQ(timestamp, "Feb 23 11:54:34");

		std::string actionFrom = (*grkRes)["ActionTo"].valueAsString();
		EXPECT_EQ(actionFrom, "forwarded");

		std::string domainTo;
		(*grkRes)["DomainTo"].value(&domainTo);
		EXPECT_EQ(domainTo, "api.insight.synology.com");

		std::string eolTo;
		(*grkRes)["EndOfLineTo"].value(&eolTo);
		EXPECT_EQ(eolTo, "203.12.160.35");
	}
}

TEST_F(GrokTest, TestGrokPatternDirectionIs)
{
	string testString = "Feb 23 11:54:35 dnsmasq[1569]: reply www.speedtest.net.cdn.cloudflare.net is 104.17.148.22";
	grokResult* grkRes = grk->parse(testString, 1000);
	EXPECT_TRUE(grkRes->matched);

	if (grkRes->matched)
	{
		std::string timestamp = (*grkRes)["Timestamp"].valueAsString();
		EXPECT_EQ(timestamp, "Feb 23 11:54:35");

		std::string actionFrom = (*grkRes)["ActionIs"].valueAsString();
		EXPECT_EQ(actionFrom, "reply");

		std::string domainIs;
		(*grkRes)["DomainIs"].value(&domainIs);
		EXPECT_EQ(domainIs, "www.speedtest.net.cdn.cloudflare.net");

		std::string eolIs;
		(*grkRes)["EndOfLineIs"].value(&eolIs);
		EXPECT_EQ(eolIs, "104.17.148.22");
	}
}
