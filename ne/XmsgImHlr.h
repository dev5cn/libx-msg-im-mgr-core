/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NE_XMSGIMHLR_H_
#define NE_XMSGIMHLR_H_

#include "XmsgNe.h"

class XmsgImHlr: public XmsgNe
{
public:
	static unordered_map<string, shared_ptr<XmsgImMsgStub>> __h2nMsgs__;
public:
	static bool regH2N(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign = FOREIGN_FORBIDDEN); 
	XmsgImHlr(shared_ptr<XscTcpServer> tcpServer, const string &peer, const string& pwd, const string& alg);
	virtual ~XmsgImHlr();
};

#endif 
