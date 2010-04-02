//
// LoggingFactoryTest.cpp
//
// $Id: //poco/1.3/Foundation/testsuite/src/LoggingFactoryTest.cpp#2 $
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "LoggingFactoryTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/LoggingFactory.h"
#include "Poco/Instantiator.h"
#include "Poco/Channel.h"
#include "Poco/ConsoleChannel.h"
#if defined(_WIN32)
#include "Poco/WindowsConsoleChannel.h"
#endif
#include "Poco/FileChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/Formatter.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Message.h"
#include "Poco/AutoPtr.h"
#include "Poco/Exception.h"
#include <memory>


using Poco::LoggingFactory;
using Poco::Channel;
using Poco::ConsoleChannel;
using Poco::FileChannel;
using Poco::SplitterChannel;
using Poco::Formatter;
using Poco::PatternFormatter;
using Poco::Message;
using Poco::AutoPtr;
using Poco::Instantiator;


namespace
{
	class CustomChannel: public Channel
	{
	public:
		void log(const Message& msg)
		{
		}
	};
	
	class CustomFormatter: public Formatter
	{
		void format(const Message& msg, std::string& text)
		{
		}
	};
}


LoggingFactoryTest::LoggingFactoryTest(const std::string& name): CppUnit::TestCase(name)
{
}


LoggingFactoryTest::~LoggingFactoryTest()
{
}


void LoggingFactoryTest::testBuiltins()
{
	LoggingFactory& fact = LoggingFactory::defaultFactory();
	
	AutoPtr<Channel> pConsoleChannel = fact.createChannel("ConsoleChannel");
#if defined(_WIN32)
	assert (dynamic_cast<Poco::WindowsConsoleChannel*>(pConsoleChannel.get()) != 0);
#else
	assert (dynamic_cast<ConsoleChannel*>(pConsoleChannel.get()) != 0);
#endif

	AutoPtr<Channel> pFileChannel = fact.createChannel("FileChannel");
	assert (dynamic_cast<FileChannel*>(pFileChannel.get()) != 0);
	
	AutoPtr<Channel> pSplitterChannel = fact.createChannel("SplitterChannel");
	assert (dynamic_cast<SplitterChannel*>(pSplitterChannel.get()) != 0);
	
	try
	{
		AutoPtr<Channel> pUnknownChannel = fact.createChannel("UnknownChannel");
		fail("unknown class - must throw");
	}
	catch (Poco::NotFoundException&)
	{
	}
	
	AutoPtr<Formatter> pPatternFormatter = fact.createFormatter("PatternFormatter");
	assert (dynamic_cast<PatternFormatter*>(pPatternFormatter.get()) != 0);
	
	try
	{
		AutoPtr<Formatter> pUnknownFormatter = fact.createFormatter("UnknownFormatter");
		fail("unknown class - must throw");
	}
	catch (Poco::NotFoundException&)
	{
	}
}


void LoggingFactoryTest::testCustom()
{
	std::auto_ptr<LoggingFactory> fact(new LoggingFactory);
	
	fact->registerChannelClass("CustomChannel", new Instantiator<CustomChannel, Channel>);
	fact->registerFormatterClass("CustomFormatter", new Instantiator<CustomFormatter, Formatter>);

	AutoPtr<Channel> pCustomChannel = fact->createChannel("CustomChannel");
	assert (dynamic_cast<CustomChannel*>(pCustomChannel.get()) != 0);

	AutoPtr<Formatter> pCustomFormatter = fact->createFormatter("CustomFormatter");
	assert (dynamic_cast<CustomFormatter*>(pCustomFormatter.get()) != 0);
}


void LoggingFactoryTest::setUp()
{
}


void LoggingFactoryTest::tearDown()
{
}


CppUnit::Test* LoggingFactoryTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("LoggingFactoryTest");

	CppUnit_addTest(pSuite, LoggingFactoryTest, testBuiltins);
	CppUnit_addTest(pSuite, LoggingFactoryTest, testCustom);

	return pSuite;
}
