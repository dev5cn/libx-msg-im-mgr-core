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

#include "XmsgAp.h"
#include "../XmsgImMgrCfg.h"
#include "../usr/XmsgImClient.h"

unordered_map<string, shared_ptr<XmsgImMsgStub>> XmsgAp::__h2nMsgs__;

XmsgAp::XmsgAp(shared_ptr<XscTcpServer> tcpServer, const string &peer, const string& pwd, const string& alg) :
		XmsgNe(tcpServer, peer, X_MSG_AP, pwd, alg)
{
	for (auto& it : XmsgAp::__h2nMsgs__)
		this->msgMgr->msgs[it.first] = it.second;
}

XscMsgItcpRetType XmsgAp::itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_BEGIN)
		return this->itcp4begin(wk, channel, pdu);
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_UNIDIRECTION)
		return this->itcp4unidirection(wk, channel, pdu);
	return XscMsgItcpRetType::DISABLE;
}

XscMsgItcpRetType XmsgAp::itcp4begin(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	string cgt, plat, did, ccid;
	if (!this->checkRequiredPars(pdu, cgt, plat, did, ccid))
		return XscMsgItcpRetType::EXCEPTION;
	auto ap = static_pointer_cast<XmsgAp>(this->shared_from_this());
	SptrXitp trans(new XmsgImTransPassive(ap, pdu));
	trans->dtid = pdu->transm.trans->stid;
	trans->addOob(XSC_TAG_UID, ccid);
	string itcp;
	if (trans->getOob(XSC_TAG_INTERCEPT, itcp))
		trans->addOob(XSC_TAG_INTERCEPT, itcp);
	SptrCgt admin = ChannelGlobalTitle::parse(cgt);
	if (admin == nullptr || cgt != XmsgImMgrCfg::instance()->cfgPb->misc().admincgt())
	{
		LOG_WARN("it`s not administrator, plat: %s, ccid: %s, cgt: %s", plat.c_str(), ccid.c_str(), cgt.c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassStart(trans, pdu);
		trans->endDesc(RET_FORBIDDEN, "your are not administrator");
		return XscMsgItcpRetType::FORBIDDEN;
	}
	string dest;
	if (pdu->transm.getOob(XSC_TAG_CLIENT_OOB, dest)) 
	{
		trans->rawMsg.reset(new x_msg_im_trans_raw_msg());
		trans->rawMsg->msg = pdu->transm.trans->msg;
		trans->rawMsg->dat.assign((char*) pdu->transm.trans->dat, pdu->transm.trans->dlen);
		SptrCgt destCgt = ChannelGlobalTitle::parse(dest);
		if (destCgt == nullptr)
		{
			LOG_WARN("oob cgt format error, dest: %s, plat: %s, ccid: %s, cgt: %s", dest.c_str(), plat.c_str(), ccid.c_str(), cgt.c_str())
			XmsgImLog::getLog(trans->channel.get())->transPassStart(trans, pdu);
			trans->endDesc(RET_FORBIDDEN, "oob format error.");
			return XscMsgItcpRetType::FORBIDDEN;
		}
		return this->forward4begin4dest(wk, channel, pdu, cgt, plat, did, ccid, trans, destCgt);
	}
	if (pdu->transm.trans->msg.find("XmsgImMgrRaw") != 0) 
	{
		trans->rawMsg.reset(new x_msg_im_trans_raw_msg());
		trans->rawMsg->msg = pdu->transm.trans->msg;
		trans->rawMsg->dat.assign((char*) pdu->transm.trans->dat, pdu->transm.trans->dlen);
		return this->forward4begin(wk, channel, pdu, cgt, plat, did, ccid, trans);
	}
	XmsgImLog::getLog(trans->channel.get())->transPassStart(trans, pdu);
	shared_ptr<XmsgImMsgStub> stub = this->msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im message stub for: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		trans->endDesc(RET_FORBIDDEN, "unsupported message: %s", pdu->transm.trans->msg.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	trans->beginMsg = stub->newBegin(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (trans->beginMsg == nullptr)
	{
		LOG_DEBUG("can not reflect a begin message from data, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		trans->endDesc(RET_EXCEPTION, "request message format error: %s", pdu->transm.trans->msg.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	SptrClient client(new XmsgImClient(admin, plat, did, ccid, ap));
	auto usr = static_pointer_cast<XmsgNeUsr>(this->usr.lock()); 
	((void (*)(shared_ptr<XmsgNeUsr> nu, SptrClient client, SptrXitp trans, shared_ptr<Message> req)) (stub->cb))(usr, client, trans, trans->beginMsg);
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgAp::itcp4unidirection(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	string cgt, plat, did, ccid;
	if (!this->checkRequiredPars(pdu, cgt, plat, did, ccid))
		return XscMsgItcpRetType::EXCEPTION;
	auto ap = static_pointer_cast<XmsgAp>(this->shared_from_this());
	SptrXitup trans(new XmsgImTransUnidirectionPass(ap, pdu));
	trans->addOob(XSC_TAG_UID, ccid);
	string itcp;
	if (trans->getOob(XSC_TAG_INTERCEPT, itcp))
		trans->addOob(XSC_TAG_INTERCEPT, itcp);
	SptrCgt admin = ChannelGlobalTitle::parse(cgt);
	if (admin == nullptr || cgt != XmsgImMgrCfg::instance()->cfgPb->misc().admincgt())
	{
		LOG_WARN("it`s not administrator, plat: %s, ccid: %s, cgt: %s", plat.c_str(), ccid.c_str(), cgt.c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	string dest;
	if (pdu->transm.getOob(XSC_TAG_CLIENT_OOB, dest)) 
	{
		trans->rawMsg.reset(new x_msg_im_trans_raw_msg());
		trans->rawMsg->msg = pdu->transm.trans->msg;
		trans->rawMsg->dat.assign((char*) pdu->transm.trans->dat, pdu->transm.trans->dlen);
		SptrCgt destCgt = ChannelGlobalTitle::parse(dest);
		if (destCgt == nullptr)
		{
			LOG_WARN("oob cgt format error, dest: %s, plat: %s, ccid: %s, cgt: %s", dest.c_str(), plat.c_str(), ccid.c_str(), cgt.c_str())
			XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
			return XscMsgItcpRetType::FORBIDDEN;
		}
		return this->forward4unidirection4dest(wk, channel, pdu, cgt, plat, did, ccid, trans, destCgt);
	}
	if (pdu->transm.trans->msg.find("XmsgImMgrRaw") != 0) 
	{
		trans->rawMsg.reset(new x_msg_im_trans_raw_msg());
		trans->rawMsg->msg = pdu->transm.trans->msg;
		trans->rawMsg->dat.assign((char*) pdu->transm.trans->dat, pdu->transm.trans->dlen);
		return this->forward4unidirection(wk, channel, pdu, cgt, plat, did, ccid, trans);
	}
	XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
	shared_ptr<XmsgImMsgStub> stub = this->msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im message stub for: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	trans->uniMsg = stub->newUnidirection(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (trans->uniMsg == nullptr)
	{
		LOG_DEBUG("can not reflect a unidirection message from data, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
	SptrClient client(new XmsgImClient(admin, plat, did, ccid, ap));
	auto usr = static_pointer_cast<XmsgNeUsr>(this->usr.lock()); 
	((void (*)(shared_ptr<XmsgNeUsr> nu, SptrClient client, SptrXitup trans, shared_ptr<Message> req)) (stub->cb))(usr, client, trans, trans->uniMsg);
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgAp::forward4begin(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitp trans)
{
	XmsgImLog::getLog(trans->channel.get())->transPassStart(trans, pdu);
	string ne = XmsgAp::getNeGroup(pdu->transm.trans->msg);
	if (ne.empty())
	{
		LOG_ERROR("unexpected x-msg-im-mgr message, plat: %s, did: %s, ccid: %s, cgt: %s, msg: %s", plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str(), pdu->transm.trans->msg.c_str())
		trans->endDesc(RET_FORBIDDEN, "unexpected message: %s", pdu->transm.trans->msg.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	auto usr = XmsgNeMgr::instance()->get(ne);
	if (usr == nullptr)
	{
		LOG_ERROR("can not found any network element to process this message: %s, may be all lost?, plat: %s, did: %s, ccid: %s, cgt: %s", pdu->transm.trans->msg.c_str(), plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str())
		trans->endDesc(RET_EXCEPTION, "can not found any network element to process this message: %s, may be all lost?", pdu->transm.trans->msg.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgImChannel::cast(usr->channel)->begin(pdu->transm.trans->msg, trans->rawMsg->dat, [trans](SptrXiti itrans)
	{
		trans->endRaw(itrans->ret, itrans->desc, itrans->rawMsg);
	}, nullptr, true, trans);
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgAp::forward4begin4dest(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitp trans, SptrCgt dest)
{
	XmsgImLog::getLog(trans->channel.get())->transPassStart(trans, pdu);
	auto ne = XmsgNeMgr::instance()->findByCgt(dest->toString());
	if (ne == nullptr)
	{
		LOG_DEBUG("can not found network element for destination channel global title: %s, plat: %s, did: %s, ccid: %s, cgt: %s", dest->toString().c_str(), plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str())
		trans->endDesc(RET_NOT_FOUND, "can not found network element for destination channel global title: %s", dest->toString().c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgImChannel::cast(ne->channel)->begin(pdu->transm.trans->msg, trans->rawMsg->dat, [trans](SptrXiti itrans)
	{
		trans->endRaw(itrans->ret, itrans->desc, itrans->rawMsg);
	}, nullptr, true, trans);
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgAp::forward4unidirection(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitup trans)
{
	XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
	string ne = XmsgAp::getNeGroup(pdu->transm.trans->msg);
	if (ne.empty())
	{
		LOG_ERROR("unexpected x-msg-im-mgr message, plat: %s, did: %s, ccid: %s, cgt: %s, msg: %s", plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str(), pdu->transm.trans->msg.c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	auto nu = XmsgNeMgr::instance()->get(ne);
	if (nu == nullptr)
	{
		LOG_ERROR("can not found any network element to process this message: %s, may be all lost?, plat: %s, did: %s, ccid: %s, cgt: %s", pdu->transm.trans->msg.c_str(), plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgImChannel::cast(nu->channel)->unidirection(pdu->transm.trans->msg, trans->rawMsg->dat, nullptr, trans);
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgAp::forward4unidirection4dest(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu, const string& cgt, const string& plat, const string& did, const string& ccid, SptrXitup trans, SptrCgt dest)
{
	XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
	auto nu = XmsgNeMgr::instance()->findByCgt(dest->toString());
	if (nu == nullptr)
	{
		LOG_DEBUG("can not found network element for destination channel global title: %s, plat: %s, did: %s, ccid: %s, cgt: %s", dest->toString().c_str(), plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgImChannel::cast(nu->channel)->unidirection(pdu->transm.trans->msg, trans->rawMsg->dat, nullptr, trans);
	return XscMsgItcpRetType::SUCCESS;
}

string XmsgAp::getNeGroup(const string& msgName)
{
	for (auto& it : XmsgImMgrCfg::instance()->cfgPb->nemsgprefix())
	{
		if (Misc::startWith(it.first, msgName))
			return it.second;
	}
	return "";
}

bool XmsgAp::checkRequiredPars(shared_ptr<XscProtoPdu> pdu, string& cgt, string& plat, string& did, string& ccid)
{
	if (!pdu->transm.getOob(XSC_TAG_CGT, cgt)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client channel global title, this: %s", this->toString().c_str())
		return false;
	}
	if (!pdu->transm.getOob(XSC_TAG_PLATFORM, plat)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client platform information, this: %s", this->toString().c_str())
		return false;
	}
	if (!pdu->transm.getOob(XSC_TAG_DEVICE_ID, did)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client device information, this: %s", this->toString().c_str())
		return false;
	}
	if (!pdu->transm.getOob(XSC_TAG_UID, ccid)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client channel id, this: %s", this->toString().c_str())
		return false;
	}
	return true;
}

bool XmsgAp::regH2N(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign)
{
	shared_ptr<XmsgImMsgStub> stub(new XmsgImMsgStub(begin, end, uni, cb, auth, foreign));
	if (XmsgAp::__h2nMsgs__.find(stub->msg) != XmsgAp::__h2nMsgs__.end())
	{
		LOG_ERROR("duplicate message: %s", stub->msg.c_str())
		return false;
	}
	LOG_TRACE("reg h2n messsage: %s", stub->toString().c_str())
	XmsgAp::__h2nMsgs__[stub->msg] = stub;
	return true;
}

XmsgAp::~XmsgAp()
{

}
