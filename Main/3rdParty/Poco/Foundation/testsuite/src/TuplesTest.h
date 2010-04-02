//
// TuplesTest.h
//
// $Id: //poco/1.3/Foundation/testsuite/src/TuplesTest.h#4 $
//
// Definition of the TuplesTest class.
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


#ifndef TuplesTest_INCLUDED
#define TuplesTest_INCLUDED


#include "Poco/Foundation.h"
#include "CppUnit/TestCase.h"


class TuplesTest: public CppUnit::TestCase
{
public:
	TuplesTest(const std::string& name);
	~TuplesTest();

	void testTuple1();
	void testTuple2();
	void testTuple3();
	void testTuple4();
	void testTuple5();
	void testTuple6();
	void testTuple7();
	void testTuple8();
	void testTuple9();
	void testTuple10();
	void testTuple11();
	void testTuple12();
	void testTuple13();
	void testTuple14();
	void testTuple15();
	void testTuple16();
	void testTuple17();
	void testTuple18();
	void testTuple19();
	void testTuple20();
	void testTupleOrder();
	void testMemOverhead();
	void setUp();
	void tearDown();

	static CppUnit::Test* suite();

private:

	template <class T>
	void testTupleStrictWeak(const T& t1, const T& t2, const T& t3)
	{
		assert (t1 < t2 && !(t2 < t1)); // antisymmetric
		assert (t1 < t2 && t2 < t3 && t1 < t3); // transitive
		assert (!(t1 < t1)); // irreflexive
	}
};


#endif // TuplesTest_INCLUDED
