//
// SyslogTest.cpp
//
// $Id: //poco/1.3/Net/testsuite/src/SyslogTest.cpp#1 $
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
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


#include "SyslogTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/Net/RemoteSyslogChannel.h"
#include "Poco/Net/RemoteSyslogListener.h"
#include "Poco/Net/DNS.h"
#include "Poco/Thread.h"
#include "Poco/Message.h"
#include "Poco/AutoPtr.h"
#include <list>


using namespace Poco::Net;


class CachingChannel: public Poco::Channel
	/// Caches the last n Messages in memory
{
public:
	typedef std::list<Poco::Message> Messages;
	
	CachingChannel(std::size_t n = 100);
		/// Creates the CachingChannel. Caches n messages in memory

	~CachingChannel();
		/// Destroys the CachingChannel.

	void log(const Poco::Message& msg);
		/// Writes the log message to the cache

	void getMessages(std::vector<Poco::Message>& msg, int offset, int numEntries) const;
		/// Retrieves numEntries Messages starting with position offset. Most recent messages are first.

	std::size_t getMaxSize() const;

	std::size_t getCurrentSize() const;

private:
	CachingChannel(const CachingChannel&);

	Messages   _cache;
	std::size_t _size;
	std::size_t _maxSize;
	mutable Poco::FastMutex _mutex;
};


std::size_t CachingChannel::getMaxSize() const
{
	return _maxSize;
}


std::size_t CachingChannel::getCurrentSize() const
{
	return _size;
}


CachingChannel::CachingChannel(std::size_t n):
	_cache(),
	_size(0),
	_maxSize(n),
	_mutex()
{
}


CachingChannel::~CachingChannel()
{
}


void CachingChannel::log(const Poco::Message& msg)
{
	Poco::FastMutex::ScopedLock lock(_mutex);
	_cache.push_front(msg);
	if (_size == _maxSize)
	{
		_cache.pop_back();
	}
	else
		++_size;
}


void CachingChannel::getMessages(std::vector<Poco::Message>& msg, int offset, int numEntries) const
{
	msg.clear();
	Messages::const_iterator it = _cache.begin();

	while (offset > 0 && it != _cache.end())
		++it;

	while (numEntries > 0 && it != _cache.end())
	{
		msg.push_back(*it);
		++it;
	}
}


SyslogTest::SyslogTest(const std::string& name): CppUnit::TestCase(name)
{
}


SyslogTest::~SyslogTest()
{
}


void SyslogTest::testListener()
{
	Poco::AutoPtr<RemoteSyslogChannel> channel = new RemoteSyslogChannel();
	channel->setProperty("loghost", "localhost:51400");
	channel->open();
	Poco::AutoPtr<RemoteSyslogListener> listener = new RemoteSyslogListener(51400);
	listener->open();
	CachingChannel cl;
	listener->addChannel(&cl);
	poco_assert (cl.getCurrentSize() == 0);
	Poco::Message msg("asource", "amessage", Poco::Message::PRIO_CRITICAL);
	channel->log(msg);
	Poco::Thread::sleep(1000);
	listener->close();
	channel->close();
	poco_assert (cl.getCurrentSize() == 1);
	std::vector<Poco::Message> msgs;
	cl.getMessages(msgs, 0, 10);
	poco_assert (msgs.size() == 1);
	poco_assert (msgs[0].getSource() == "asource");
	poco_assert (msgs[0].getText() == "amessage");
	poco_assert (msgs[0].getPriority() == Poco::Message::PRIO_CRITICAL);
}


void SyslogTest::testOldBSD()
{
	Poco::AutoPtr<RemoteSyslogChannel> channel = new RemoteSyslogChannel();
	channel->setProperty("loghost", "localhost:51400");
	channel->setProperty("format", "bsd");
	channel->open();
	Poco::AutoPtr<RemoteSyslogListener> listener = new RemoteSyslogListener(51400);
	listener->open();
	CachingChannel cl;
	listener->addChannel(&cl);
	poco_assert (cl.getCurrentSize() == 0);
	Poco::Message msg("asource", "amessage", Poco::Message::PRIO_CRITICAL);
	channel->log(msg);
	Poco::Thread::sleep(1000);
	listener->close();
	channel->close();
	poco_assert (cl.getCurrentSize() == 1);
	std::vector<Poco::Message> msgs;
	cl.getMessages(msgs, 0, 10);
	poco_assert (msgs.size() == 1);
	// the source is lost with old BSD messages: we only send the local host name!
	poco_assert (msgs[0].getSource() == Poco::Net::DNS::thisHost().name());
	poco_assert (msgs[0].getText() == "amessage");
	poco_assert (msgs[0].getPriority() == Poco::Message::PRIO_CRITICAL);
}


void SyslogTest::setUp()
{
}


void SyslogTest::tearDown()
{
}


CppUnit::Test* SyslogTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("SyslogTest");

	CppUnit_addTest(pSuite, SyslogTest, testListener);
	CppUnit_addTest(pSuite, SyslogTest, testOldBSD);

	return pSuite;
}
