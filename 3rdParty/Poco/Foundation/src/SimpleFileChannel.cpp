//
// SimpleFileChannel.cpp
//
// $Id: //poco/1.3/Foundation/src/SimpleFileChannel.cpp#2 $
//
// Library: Foundation
// Package: Logging
// Module:  SimpleFileChannel
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
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


#include "Poco/SimpleFileChannel.h"
#include "Poco/LogFile.h"
#include "Poco/File.h"
#include "Poco/Message.h"
#include "Poco/Exception.h"
#include <cctype>


namespace Poco {


const std::string SimpleFileChannel::PROP_PATH          = "path";
const std::string SimpleFileChannel::PROP_SECONDARYPATH = "secondaryPath";
const std::string SimpleFileChannel::PROP_ROTATION      = "rotation";


SimpleFileChannel::SimpleFileChannel(): 
	_limit(0),
	_pFile(0)
{
}


SimpleFileChannel::SimpleFileChannel(const std::string& path):
	_path(path),
	_secondaryPath(path + ".0"),
	_limit(0),
	_pFile(0)
{
}


SimpleFileChannel::~SimpleFileChannel()
{
	close();
}


void SimpleFileChannel::open()
{
	FastMutex::ScopedLock lock(_mutex);
	
	if (!_pFile)
	{
		File primary(_path);
		File secondary(_secondaryPath);
		Timestamp pt = primary.exists() ? primary.getLastModified() : 0;
		Timestamp st = secondary.exists() ? secondary.getLastModified() : 0;
		std::string path;
		if (pt >= st)
			path = _path;
		else
			path = _secondaryPath;
		_pFile = new LogFile(path);
	}
}


void SimpleFileChannel::close()
{
	FastMutex::ScopedLock lock(_mutex);

	delete _pFile;
	_pFile = 0;
}


void SimpleFileChannel::log(const Message& msg)
{
	open();

	FastMutex::ScopedLock lock(_mutex);

	if (_limit > 0 && _pFile->size() >= _limit)
	{
		rotate();
	}
	_pFile->write(msg.getText());
}

	
void SimpleFileChannel::setProperty(const std::string& name, const std::string& value)
{
	FastMutex::ScopedLock lock(_mutex);

	if (name == PROP_PATH)
	{
		_path = value;
		if (_secondaryPath.empty())
			_secondaryPath = _path + ".0";
	}
	else if (name == PROP_SECONDARYPATH)
		_secondaryPath = value;
	else if (name == PROP_ROTATION)
		setRotation(value);
	else
		Channel::setProperty(name, value);
}


std::string SimpleFileChannel::getProperty(const std::string& name) const
{
	if (name == PROP_PATH)
		return _path;
	else if (name == PROP_SECONDARYPATH)
		return _secondaryPath;
	else if (name == PROP_ROTATION)
		return _rotation;
	else
		return Channel::getProperty(name);
}


Timestamp SimpleFileChannel::creationDate() const
{
	if (_pFile)
		return _pFile->creationDate();
	else
		return 0;
}

	
UInt64 SimpleFileChannel::size() const
{
	if (_pFile)
		return _pFile->size();
	else
		return 0;
}


const std::string& SimpleFileChannel::path() const
{
	return _path;
}


const std::string& SimpleFileChannel::secondaryPath() const
{
	return _secondaryPath;
}


void SimpleFileChannel::setRotation(const std::string& rotation)
{
	std::string::const_iterator it  = rotation.begin();
	std::string::const_iterator end = rotation.end();
	UInt64 n = 0;
	while (it != end && std::isspace(*it)) ++it;
	while (it != end && std::isdigit(*it)) { n *= 10; n += *it++ - '0'; }
	while (it != end && std::isspace(*it)) ++it;
	std::string unit;
	while (it != end && std::isalpha(*it)) unit += *it++;
	
	if (unit == "K")
		_limit = n*1024;
	else if (unit == "M")
		_limit = n*1024*1024;
	else if (unit.empty())
		_limit = n;
	else if (unit == "never")
		_limit = 0;
	else
		throw InvalidArgumentException("rotation", rotation);
	_rotation = rotation;
}


void SimpleFileChannel::rotate()
{
	std::string newPath;
	if (_pFile->path() == _path)
		newPath = _secondaryPath;
	else
		newPath = _path;
	File f(newPath);
	if (f.exists())
	{
		try
		{
			f.remove();
		}
		catch (...)
		{
		}
	}
	delete _pFile;
	_pFile = new LogFile(newPath);
}


} // namespace Poco
