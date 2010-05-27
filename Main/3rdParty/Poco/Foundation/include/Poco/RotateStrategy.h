//
// RotateStrategy.h
//
// $Id: //poco/1.3/Foundation/include/Poco/RotateStrategy.h#3 $
//
// Library: Foundation
// Package: Logging
// Module:  FileChannel
//
// Definition of the RotateStrategy class and subclasses.
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


#ifndef Foundation_RotateStrategy_INCLUDED
#define Foundation_RotateStrategy_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/Exception.h"
#include "Poco/LogFile.h"
#include "Poco/StringTokenizer.h"
#include "Poco/DateTimeParser.h"
#include "Poco/NumberParser.h"


namespace Poco {


class Foundation_API RotateStrategy
	/// The RotateStrategy is used by LogFile to determine when
	/// a file must be rotated.
{
public:
	RotateStrategy();
	virtual ~RotateStrategy();

	virtual bool mustRotate(LogFile* pFile) = 0;
		/// Returns true if the given log file must
		/// be rotated, false otherwise.
		
private:
	RotateStrategy(const RotateStrategy&);
	RotateStrategy& operator = (const RotateStrategy&);
};


template <class DT>
class RotateAtTimeStrategy: public RotateStrategy
	/// The file is rotated at specified [day,][hour]:minute
{
public:
	RotateAtTimeStrategy(const std::string& rtime):
		_day(-1), 
		_hour(-1), 
		_minute(0)
	{
		if (rtime.empty()) 
			throw InvalidArgumentException("Rotation time must be specified.");

		if ((rtime.find(',') != rtime.npos) && (rtime.find(':') == rtime.npos)) 
			throw InvalidArgumentException("Invalid rotation time specified.");

		StringTokenizer timestr(rtime, ",:", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
		int index = 0;

		switch (timestr.count())
		{
		case 3: // day,hh:mm
			{
				std::string::const_iterator it = timestr[index].begin();
				_day = DateTimeParser::parseDayOfWeek(it, timestr[index].end());
				++index;
			}
		case 2: // hh:mm
			_hour = NumberParser::parse(timestr[index]);
			++index;
		case 1: // mm
			_minute = NumberParser::parse(timestr[index]);
			break;
		default:
			throw InvalidArgumentException("Invalid rotation time specified.");
		}
		getNextRollover();
	}
	
	~RotateAtTimeStrategy()
	{
	}
	
	bool mustRotate(LogFile* pFile)
	{
		if (DT() >= _threshold)
		{
			getNextRollover();
			return true;
		}
		return false;
	}

private:
	void getNextRollover()
	{
		Timespan tsp(0, 0, 1, 0, 1000); // 0,00:01:00.001
		do
		{
			_threshold += tsp;
		}
		while (!(_threshold.minute() == _minute &&
		        (-1 == _hour || _threshold.hour() == _hour) && 
		        (-1 == _day  || _threshold.dayOfWeek() == _day)));
		// round to :00.0 seconds
		_threshold.assign(_threshold.year(), _threshold.month(), _threshold.day(), _threshold.hour(), _threshold.minute());
	}

	DT  _threshold;
	int _day;
	int _hour;
	int _minute;
};


class Foundation_API RotateByIntervalStrategy: public RotateStrategy
	/// The file is rotated when the log file 
	/// exceeds a given age.
	///
	/// For this to work reliably across all platforms and file systems
	/// (there are severe issues on most platforms finding out the real
	/// creation date of a file), the creation date of the file is
	/// written into the log file as the first entry.
{
public:
	RotateByIntervalStrategy(const Timespan& span);
	~RotateByIntervalStrategy();
	bool mustRotate(LogFile* pFile);

private:
	Timespan _span;
	Timestamp _lastRotate;
	static const std::string ROTATE_TEXT;
};


class Foundation_API RotateBySizeStrategy: public RotateStrategy
	/// The file is rotated when the log file
	/// exceeds a given size.
{
public:
	RotateBySizeStrategy(UInt64 size);
	~RotateBySizeStrategy();
	bool mustRotate(LogFile* pFile);

private:
	UInt64 _size;
};


} // namespace Poco


#endif // Foundation_RotateStrategy_INCLUDED
