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

#include "XmsgImMgrCfg.h"

shared_ptr<XmsgImMgrCfg> XmsgImMgrCfg::cfg; 

XmsgImMgrCfg::XmsgImMgrCfg()
{

}

shared_ptr<XmsgImMgrCfg> XmsgImMgrCfg::instance()
{
	return XmsgImMgrCfg::cfg;
}

shared_ptr<XmsgImMgrCfg> XmsgImMgrCfg::load(const char* path)
{
	XMLDocument doc;
	if (doc.LoadFile(path) != 0)
	{
		LOG_ERROR("load config file failed, path: %s", path)
		return nullptr;
	}
	XMLElement* root = doc.RootElement();
	if (root == NULL)
	{
		LOG_ERROR("it a empty xml file? path: %s", path)
		return nullptr;
	}
	shared_ptr<XmsgImMgrCfg> cfg(new XmsgImMgrCfg());
	cfg->cfgPb.reset(new XmsgImMgrCfgPb());
	cfg->cgt = ChannelGlobalTitle::parse(Misc::strAtt(root, "cgt"));
	if (cfg->cgt == nullptr)
	{
		LOG_ERROR("load config file failed, cgt format error: %s", Misc::strAtt(root, "cgt").c_str())
		return nullptr;
	}
	cfg->cfgPb->set_cgt(cfg->cgt->toString());
	if (!cfg->loadLogCfg(root))
		return nullptr;
	if (!cfg->loadXscServerCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeH2nCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeN2hCfg(root))
		return nullptr;
	if (!cfg->loadNeMsgPrefixCfg(root))
		return nullptr;
	if (!cfg->loadMiscCfg(root))
		return nullptr;
	LOG_INFO("load config file successful, cfg: %s", cfg->toString().c_str())
	XmsgImMgrCfg::setCfg(cfg);
	return cfg;
}

void XmsgImMgrCfg::setCfg(shared_ptr<XmsgImMgrCfg> cfg)
{
	XmsgImMgrCfg::cfg = cfg;
}

shared_ptr<XscTcpCfg> XmsgImMgrCfg::xscServerCfg()
{
	shared_ptr<XscTcpCfg> tcpCfg(new XscTcpCfg());
	tcpCfg->addr = this->cfgPb->xsctcpcfg().addr();
	tcpCfg->worker = this->cfgPb->xsctcpcfg().worker();
	tcpCfg->peerLimit = this->cfgPb->xsctcpcfg().peerlimit();
	tcpCfg->peerMtu = this->cfgPb->xsctcpcfg().peermtu();
	tcpCfg->peerRcvBuf = this->cfgPb->xsctcpcfg().peerrcvbuf();
	tcpCfg->peerSndBuf = this->cfgPb->xsctcpcfg().peersndbuf();
	tcpCfg->lazyClose = this->cfgPb->xsctcpcfg().lazyclose();
	tcpCfg->tracing = this->cfgPb->xsctcpcfg().tracing();
	tcpCfg->heartbeat = this->cfgPb->xsctcpcfg().heartbeat();
	tcpCfg->n2hZombie = this->cfgPb->xsctcpcfg().n2hzombie();
	tcpCfg->n2hTransTimeout = this->cfgPb->xsctcpcfg().n2htranstimeout();
	tcpCfg->n2hTracing = this->cfgPb->xsctcpcfg().n2htracing();
	tcpCfg->h2nReConn = this->cfgPb->xsctcpcfg().h2nreconn();
	tcpCfg->h2nTransTimeout = this->cfgPb->xsctcpcfg().h2ntranstimeout();
	return tcpCfg;
}

bool XmsgImMgrCfg::loadLogCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("log");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <log>")
		return false;
	}
	XmsgImMgrCfgLog* log = this->cfgPb->mutable_log();
	log->set_level(Misc::toUpercase(Misc::strAtt(node, "level")));
	log->set_output(Misc::toUpercase(Misc::strAtt(node, "output")));
	return true;
}

bool XmsgImMgrCfg::loadXscServerCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("xsc-server");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <xsc-server>");
		return false;
	}
	XmsgImMgrCfgXscTcpServer* tcpCfg = this->cfgPb->mutable_xsctcpcfg();
	tcpCfg->set_addr(Misc::strAtt(node, "addr"));
	tcpCfg->set_worker(Misc::hexOrInt(node, "worker"));
	tcpCfg->set_peerlimit(Misc::hexOrInt(node, "peerLimit"));
	tcpCfg->set_peermtu(Misc::hexOrInt(node, "peerMtu"));
	tcpCfg->set_peerrcvbuf(Misc::hexOrInt(node, "peerRcvBuf"));
	tcpCfg->set_peersndbuf(Misc::hexOrInt(node, "peerSndBuf"));
	tcpCfg->set_lazyclose(Misc::hexOrInt(node, "lazyClose"));
	tcpCfg->set_tracing("true" == Misc::strAtt(node, "tracing"));
	tcpCfg->set_heartbeat(Misc::hexOrInt(node, "heartbeat"));
	tcpCfg->set_n2hzombie(Misc::hexOrInt(node, "n2hZombie"));
	tcpCfg->set_n2htranstimeout(Misc::hexOrInt(node, "n2hTransTimeout"));
	tcpCfg->set_n2htracing("true" == Misc::strAtt(node, "n2hTracing"));
	tcpCfg->set_h2nreconn(Misc::hexOrInt(node, "h2nReConn"));
	tcpCfg->set_h2ntranstimeout(Misc::hexOrInt(node, "h2nTransTimeout"));
	return true;
}

bool XmsgImMgrCfg::loadXmsgNeH2nCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-h2n");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n>")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_h2n();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "alg", ne->mutable_alg());
		Misc::strAtt(node, "cgt", ne->mutable_cgt());
		auto cgt = ChannelGlobalTitle::parse(ne->cgt());
		if (ne->neg().empty() || ne->addr().empty() || ne->alg().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <ne-group-h2n><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->h2n().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n><ne>")
		return false;
	}
	return true;
}

bool XmsgImMgrCfg::loadXmsgNeN2hCfg(XMLElement* root)
{
	return true;
}

bool XmsgImMgrCfg::loadNeMsgPrefixCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("ne-msg-prefix");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <ne-msg-prefix>")
		return false;
	}
	auto neMsgPrefix = this->cfgPb->mutable_nemsgprefix();
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		string key = Misc::strAtt(node, "prefix");
		string val = Misc::strAtt(node, "neg");
		if (key.empty() || val.empty())
		{
			LOG_ERROR("load config failed, node: <ne-msg-prefix>")
			return false;
		}
		XmsgMisc::insertKv(neMsgPrefix, key, val);
		node = node->NextSiblingElement("ne");
	}
	if (neMsgPrefix->empty())
	{
		LOG_ERROR("load config failed, node: <ne-msg-prefix>")
		return false;
	}
	return true;
}

bool XmsgImMgrCfg::loadMiscCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("misc");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <misc>")
		return false;
	}
	auto misc = this->cfgPb->mutable_misc();
	Misc::strAtt(node, "adminCgt", misc->mutable_admincgt());
	return true;
}

string XmsgImMgrCfg::toString()
{
	return this->cfgPb->ShortDebugString();
}

XmsgImMgrCfg::~XmsgImMgrCfg()
{

}

