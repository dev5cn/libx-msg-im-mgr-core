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

#include "XmsgImClient.h"
#include "../XmsgImMgrCfg.h"

XmsgImClient::XmsgImClient(SptrCgt cgt, const string& plat, const string& did, const string& ccid, shared_ptr<XmsgAp> apLocal ) :
		Actor(ActorType::ACTOR_ITC, XmsgImClient::assignXscWorker(cgt))
{
	this->plat = plat;
	this->did = did;
	this->ccid = ccid;
	this->cgt = cgt;
	this->apLocal = apLocal;
	this->apForeign = nullptr;
	this->apCcid = XmsgMisc::getApCcid(apLocal->usr.lock()->uid, ccid);
}

XmsgImClient::XmsgImClient(SptrCgt cgt, const string& plat, const string& did, const string& ccid, SptrCgt apForeign ) :
		Actor(ActorType::ACTOR_ITC, XmsgImClient::assignXscWorker(cgt))
{
	this->plat = plat;
	this->did = did;
	this->ccid = ccid;
	this->cgt = cgt;
	this->apForeign = apForeign;
	this->apLocal = nullptr;
	this->apCcid = XmsgMisc::getApCcid(apForeign->toString(), ccid);
}

int XmsgImClient::assignXscWorker(SptrCgt cgt)
{
	return (((int) (cgt->uid.data()[cgt->uid.length() - 1])) & 0x0000FF) % XmsgImMgrCfg::instance()->cfgPb->xsctcpcfg().worker();
}

string XmsgImClient::toString()
{
	string str;
	SPRINTF_STRING(&str, "plat: %s, ccid: %s, cgt: %s", this->plat.c_str(), this->ccid.c_str(), this->cgt->toString().c_str())
	return str;
}

XmsgImClient::~XmsgImClient()
{

}

