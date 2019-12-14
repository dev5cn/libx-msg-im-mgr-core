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

#ifndef USR_XMSGIMCLIENT_H_
#define USR_XMSGIMCLIENT_H_

#include "../ne/XmsgAp.h"

class XmsgImClient: public Actor
{
public:
	string plat; 
	string did; 
	string apCcid; 
	string ccid; 
	SptrCgt cgt; 
	shared_ptr<XmsgAp> apLocal; 
	SptrCgt apForeign; 
public:
	string toString();
	XmsgImClient(SptrCgt cgt, const string& plat, const string& did, const string& ccid, shared_ptr<XmsgAp> apLocal );
	XmsgImClient(SptrCgt cgt, const string& plat, const string& did, const string& ccid, SptrCgt apForeign );
	virtual ~XmsgImClient();
private:
	static int assignXscWorker(SptrCgt cgt); 
};

typedef shared_ptr<XmsgImClient> SptrClient;

#endif 
