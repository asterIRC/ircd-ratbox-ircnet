/*
 * Copyright (C) 2010 !ratbox development team
 * m_schan.c: Service channels.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1.Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 2.Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3.The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */

#include "stdinc.h"
#include "struct.h"
#include "send.h"
#include "client.h"
#include "channel.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "hash.h"
#include "parse.h"
#include "hook.h"
#include "modules.h"
#include "match.h"
#include "whowas.h"
#include "monitor.h"
#include "s_serv.h"

static void hfn_schan_notice(hook_data_int *);
static void hfn_schan_add(hook_data_schan *);
static void hfn_schan_clean(hook_data *);

static rb_dlink_list schan_list;

struct schan {
	rb_dlink_node node;
	struct	Channel *chptr;
	int flags;
	int pass;
	char pattern[1];
};

mapi_hfn_list_av2 schan_hfnlist[] = {
	{"schan_notice", (hookfn) hfn_schan_notice},
	{"schan_add", (hookfn) hfn_schan_add},
	{"schan_clean", (hookfn) hfn_schan_clean},
	{NULL, NULL}
};

DECLARE_MODULE_AV2(schan, NULL, NULL, NULL, NULL, schan_hfnlist,
		   "$Revision$");

static void
hfn_schan_notice(hook_data_int * hdata)
{
	rb_dlink_node *ptr;

	RB_DLINK_FOREACH(ptr, schan_list.head)
	{
		struct schan *sch = ptr->data;

		if (IsSCH(sch->chptr) && match_esc(sch->pattern, hdata->arg1) && (hdata->arg2 & sch->flags)) {
			sendto_channel_flags(NULL, 0, &me, sch->chptr, "NOTICE %s :%s", sch->chptr->chname, (char*)hdata->arg1);
			if (!sch->pass)
				break;
		}
	}
}

static void
hfn_schan_add(hook_data_schan * hdata)
{
	struct Channel *chptr;
	struct schan *sch;

	chptr = get_or_create_channel(&me, hdata->name, NULL);

	if (!chptr) return;

	sch = rb_malloc(sizeof(struct schan) + strlen(hdata->pattern));
	sch->chptr = chptr;
	sch->flags = hdata->flags;
	strcpy(sch->pattern, hdata->pattern);

	rb_dlinkAddTail(sch, &sch->node, &schan_list);
	SetChannelSCH(chptr);

	/* no mode for remote channels */
	if (IsRemoteChannel(chptr->chname))
		return;

	if (!EmptyString(hdata->topic))
		set_channel_topic(chptr, hdata->topic, "anonymous!anonymous@.", rb_current_time());

	chptr->mode.mode |= MODE_TOPICLIMIT|MODE_ANONYMOUS|MODE_NOPRIVMSGS|MODE_MODERATED;
	if (hdata->operonly)
		chptr->mode.mode |= MODE_INVITEONLY;
}

static void
hfn_schan_clean(hook_data *dummy)
{
	rb_dlink_node *ptr, *next;

	RB_DLINK_FOREACH_SAFE(ptr, next, schan_list.head)
	{
		struct schan *sch = ptr->data;

		rb_dlinkDelete(&sch->node, &schan_list);
		rb_free(sch);
	}

	RB_DLINK_FOREACH_SAFE(ptr, next, global_channel_list.head)
	{
		struct Channel *chptr = ptr->data;

		if (!IsSCH(chptr))
			continue;

		UnSetChannelSCH(chptr);

		if(rb_dlink_list_length(&chptr->members) <= 0)
			destroy_channel(chptr);
	}

}

