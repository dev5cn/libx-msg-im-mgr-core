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

#include <libx-msg-common-dat-struct-cpp.h>
#include "XmsgNe.h"
#include "../XmsgImMgrCfg.h"

XmsgNe::XmsgNe(shared_ptr<XscTcpServer> tcpServer, const string& peer, const string& neg, const string& pwd, const string& alg) :
		XmsgImTcpH2N(tcpServer, peer)
{
	this->neg = neg;
	this->pwd = pwd;
	this->alg = alg;
}

void XmsgNe::estab()
{
	LOG_INFO("got a connection from %s, peer: %s", this->neg.c_str(), this->peer.c_str())
	shared_ptr<XmsgNeAuthReq> req(new XmsgNeAuthReq());
	req->set_neg(X_MSG_IM_MGR);
	req->set_cgt(XmsgImMgrCfg::instance()->cfgPb->cgt());
	req->set_salt(Crypto::gen0aAkey256());
	req->set_sign(Crypto::sha256ToHexStrLowerCase(XmsgImMgrCfg::instance()->cfgPb->cgt() + req->salt() + this->pwd));
	req->set_alg(this->alg);
	SptrOob oob(new list<pair<uchar, string>>());
	oob->push_back(make_pair<>(XSC_TAG_INTERCEPT, "enable"));
	auto ne = static_pointer_cast<XmsgNe>(this->shared_from_this());
	this->begin(req, [req, ne](SptrXiti trans)
	{
		if(trans->ret != RET_SUCCESS)
		{
			LOG_ERROR("auth with %s failed, req: %s, ret: %04X, desc: %s, this: %s", ne->neg.c_str(), req->ShortDebugString().c_str(), trans->ret, trans->desc.c_str(), ne->toString().c_str())
			ne->close();
			return;
		}
		auto rsp = static_pointer_cast<XmsgNeAuthRsp>(trans->endMsg);
		auto nu = shared_ptr<XmsgNeUsr>(new XmsgNeUsr(ne->neg, rsp->cgt(), ne));
		ne->setXscUsr(nu);
		XmsgNeMgr::instance()->add(nu);
		LOG_INFO("auth with %s successful, req: %s, rsp: %s", ne->neg.c_str(), req->ShortDebugString().c_str(), rsp->ShortDebugString().c_str())
	}, oob);
}

void XmsgNe::evnDisc()
{
	LOG_ERROR("%s connection lost, peer: %s, we will try again later", this->neg.c_str(), this->toString().c_str())
	shared_ptr<XscUsr> usr = this->usr.lock();
	if (usr != nullptr)
	{
		usr->evnDisc();
		this->setXscUsr(nullptr);
	}
	this->connect();
}

XscMsgItcpRetType XmsgNe::itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	return XscMsgItcpRetType::DISABLE;
}

string XmsgNe::toString()
{
	return this->peer;
}

XmsgNe::~XmsgNe()
{

}
