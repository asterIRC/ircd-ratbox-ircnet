/*
 *  ircd-ratbox: A slightly useful ircd.
 *  m_eob.c: Registers the end of burst of a server.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2005 ircd-ratbox development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 *  $Id$
 */

#include "stdinc.h"
#include "struct.h"
#include "client.h"
#include "match.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "s_serv.h"

static int ms_eob(struct Client *, struct Client *, int, const char **);
static int ms_eoback(struct Client *, struct Client *, int, const char **);

struct Message eob_msgtab = {
	"EOB", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_ignore, mg_ignore, {ms_eob, 0}, mg_ignore, mg_ignore}
};

struct Message eoback_msgtab = {
	"EOBACK", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_ignore, mg_ignore, {ms_eoback, 0}, mg_ignore, mg_ignore}
};

mapi_clist_av2 eob_clist[] = { &eob_msgtab, &eoback_msgtab, NULL };

DECLARE_MODULE_AV2(eob, NULL, NULL, eob_clist, NULL, NULL, "$Revision$");

/*
** ms_eob
**      parv[0] = sender prefix
**      parv[1] = opt. comma separated list of SIDs for which this EOB is
**                also valid
*/
static int
ms_eob(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	char *copy, *state, *id;
	struct Client *target_p;
	int act = 0;

	if(!HasSentEob(source_p))
	{
		if(MyConnect(source_p))
		{
			sendto_realops_flags(UMODE_ALL, L_ALL,
					     "End of burst from %s (%d seconds)",
					     source_p->name,
					     (signed int)(rb_current_time() -
							  source_p->localClient->firsttime));
			sendto_one(source_p, ":%s EOBACK", me.id);
		}
		act = 1;
		SetEob(source_p);
		eob_count++;
	}
	if(parc > 1 && *parv[1] != '\0')
	{
		copy = LOCAL_COPY(parv[1]);
		for(id = rb_strtok_r(copy, ",", &state); id != NULL;
				id = rb_strtok_r(NULL, ",", &state))
		{
			target_p = find_id(id);
			if(target_p != NULL && IsServer(target_p) &&
					target_p->from == client_p &&
					!HasSentEob(target_p))
			{
				SetEob(target_p);
				eob_count++;
				act = 1;
			}
		}
	}
	if(!act)
		return 0;
	sendto_server(client_p, NULL, CAP_IRCNET, NOCAPS, ":%s EOB%s%s",
			source_p->id,
			parc > 1 ? " :" : "", parc > 1 ? parv[1] : "");

	return 0;
}

/*
** ms_eoback
**      parv[0] = sender prefix
*/
static int
ms_eoback(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	/* do nothing */
	return 0;
}
