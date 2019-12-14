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

#ifndef NE_XMSGNE_H_
#define NE_XMSGNE_H_

#include <libx-msg-common-dat-struct-cpp.h>

class XmsgNe: public XmsgImTcpH2N
{
public:
	string neg; 
	string pwd; 
	string alg; 
public:
	string toString();
	XmsgNe(shared_ptr<XscTcpServer> tcpServer, const string& peer, const string& neg, const string& pwd, const string& alg);
	virtual ~XmsgNe();
public:
	void estab(); 
	void evnDisc(); 
	XscMsgItcpRetType itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu); 
};

#endif 
