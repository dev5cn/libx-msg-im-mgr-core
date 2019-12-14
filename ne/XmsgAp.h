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

#ifndef NE_XMSGAP_H_
#define NE_XMSGAP_H_

#include "XmsgNe.h"

class XmsgAp: public XmsgNe
{
public:
	static unordered_map<string, shared_ptr<XmsgImMsgStub>> __h2nMsgs__;
public:
	static string getNeGroup(const string& msgName); 
	static bool regH2N(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign = FOREIGN_FORBIDDEN); 
	XscMsgItcpRetType itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu); 
	XmsgAp(shared_ptr<XscTcpServer> tcpServer, const string &peer, const string& pwd, const string& alg);
	virtual ~XmsgAp();
private:
	XscMsgItcpRetType itcp4begin(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu); 
	XscMsgItcpRetType itcp4unidirection(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu); 
private:
	XscMsgItcpRetType forward4begin(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, const SptrXitp trans); 
	XscMsgItcpRetType forward4begin4dest(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitp trans, SptrCgt dest); 
	XscMsgItcpRetType forward4unidirection(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitup trans); 
	XscMsgItcpRetType forward4unidirection4dest(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitup trans, SptrCgt dest); 
private:
	bool checkRequiredPars(shared_ptr<XscProtoPdu> pdu, string& cgt, string& plat, string& did, string& ccid); 
};

#endif 
