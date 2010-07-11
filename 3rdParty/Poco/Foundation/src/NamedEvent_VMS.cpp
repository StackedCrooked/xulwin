//
// NamedEvent_VMS.cpp
//
// $Id: //poco/1.3/Foundation/src/NamedEvent_VMS.cpp#1 $
//
// Library: Foundation
// Package: Processes
// Module:  NamedEvent
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


#include "Poco/NamedEvent_VMS.h"
#include <lib$routines.h>
#include <starlet.h>
#include <descrip.h>
#include <iodef.h>


namespace Poco {


NamedEventImpl::NamedEventImpl(const std::string& name):
	_name(name)
{
	struct dsc$descriptor_s mbxDesc;
	mbxDesc.dsc$w_length  = _name.length();
	mbxDesc.dsc$b_dtype   = DSC$K_DTYPE_T;
	mbxDesc.dsc$b_class   = DSC$K_CLASS_S;
	mbxDesc.dsc$a_pointer = _name.c_str();
	if (sys$crembx(0, &_mbxChan, 0, 0, 0, 0, &mbxDesc, 0, 0) != 1)
		throw SystemException("cannot create named event", _name);
}


NamedEventImpl::~NamedEventImpl()
{
	sys$dassgn(_mbxChan);
}


void NamedEventImpl::setImpl()
{
	char buffer = 0xFF;
	if (sys$qio(0, _mbxChan, IO$_WRITEVBLK, 0, 0, 0, &buffer, sizeof(buffer), 0, 0, 0, 0) != 1)
		throw SystemException("cannot set named event", _name);
}


void NamedEventImpl::waitImpl()
{
	char buffer = 0;
	while (buffer == 0)
	{
		if (sys$qiow(0, _mbxChan, IO$_READVBLK, 0, 0, 0, &buffer, sizeof(buffer), 0, 0, 0, 0) != 1)
			throw SystemException("cannot wait for named event", _name);
	}
}


} // namespace Poco