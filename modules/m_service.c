/*
 *  ircd-ratbox: A slightly useful ircd.
 *  m_service.c: IRCNet SERVICE/SQUERY support.
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
#include "s_stats.h"


static	int	m_servlist(struct Client *, struct Client *, int,const char **);
static	int	m_squery(struct Client *, struct Client *, int,const char **);
static	int	ms_service(struct Client *, struct Client *, int,const char **);

struct Message service_msgtab = {
	"SERVICE", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_ignore, mg_ignore, {ms_service, 5}, mg_ignore, mg_ignore}
};

struct Message servlist_msgtab = {
	"SERVLIST", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, { m_servlist, 0 }, mg_ignore, mg_ignore, mg_ignore, { m_servlist, 0 }}
};

struct Message squery_msgtab = {
	"SQUERY", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, { m_squery, 2}, mg_ignore, mg_ignore, mg_ignore, { m_squery, 2 }}
};

mapi_clist_av2 service_clist[] = { &service_msgtab, &servlist_msgtab, &squery_msgtab, NULL };

DECLARE_MODULE_AV2(service, NULL, NULL, service_clist, NULL, NULL, "$Revision$");


/*
 * Returns list of all matching services.
 * parv[1] - string to match names against
 * parv[2] - type of service
 */
static	int	m_servlist(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	rb_dlink_node	*ptr;
	const char	*mask = parc > 1 ? parv[1] : "*";
	int	type = 0;

	if (parc > 2)
		type = strtol(parv[2], NULL, 0);

	RB_DLINK_FOREACH(ptr, svc_list.head)
	{
		struct Client *sp = ptr->data;

		if ((!type || (type == sp->tsinfo)) && match(mask, sp->name))
			sendto_one_numeric(source_p, RPL_SERVLIST, form_str(RPL_SERVLIST), 
				   sp->name, sp->servptr->name, sp->host,
				   sp->tsinfo, sp->hopcount, sp->info);
	}
	sendto_one_numeric(source_p, RPL_SERVLISTEND, form_str(RPL_SERVLISTEND), mask, type);
	return 0;
}


/*
 * Send query to a service.
 * parv[1] - string to match name against
 * parv[2] - string to send to service
 */
static int	m_squery(struct Client *client_p, struct Client *source_p, int parc, char const *parv[])
{
	const char *name = parv[1];
	struct Client *svc = find_client(name);

	/* find service matching parv[1]. in case no full name
	 * is given, get the one with least hopcount */
	if (MyConnect(source_p) && !svc)
	{
		rb_dlink_node *ptr;
		int l = strlen(name);

		RB_DLINK_FOREACH(ptr, svc_list.head)
		{
			struct Client *target_svc = ptr->data;

			if (!ircncmp(target_svc->name, name, l) && target_svc->name[l] == '@' &&
				(!svc || (target_svc->hopcount < svc->hopcount)))
					svc = target_svc;
		}
	}

	if (!svc)
		sendto_one(source_p, form_str(ERR_NOSUCHSERVICE),me.name, source_p->name, parv[1]);
	else
		sendto_one(svc, ":%s SQUERY %s :%s", parv[0], svc->name, parv[2]);
	return 0;
}

/*
 * m_service
 *	parv[0] = sender prefix
 *	parv[1] = service name
 *	parv[2] = distribution mask
 *	parv[3] = service type
 *	parv[4]	= info
 *
 */
static int	ms_service(struct Client *client_p, struct Client *source_p, int parc, char const *parv[])
{
	const char	*name, *dist, *info;
	rb_dlink_node	*ptr;
	int	type;
	struct	Client *svc;

	/* Copy parameters into better documenting variables */
	name = parv[1];
	dist = parv[2];
	type = strtol(parv[3], NULL, 0);
	info = parv[4];

	/* First some sanity */
	if (!IsServer(client_p)) {
		sendto_one(client_p, "ERROR :Server doesn't support services");
		return 0;
	}

	if (!IsServer(source_p))
	{
		sendto_realops_flags(UMODE_ALL, L_ALL,
                       	    "ERROR: SERVICE:%s without SERVER:%s from %s",
				    name, source_p->name,
				    client_p->name);
		return 0;
	}

	/* These are protocol errors, KILL wouldn't be of much help .. */
	if (!strchr(name, '@'))
	{
		sendto_realops_flags(UMODE_ALL, L_ALL,
                       	    "ERROR: Incomplete service name %s from %s via %s", name, source_p->name, client_p->name);
		return exit_client(NULL, client_p, &me, "Incomplete service name");
	}

	if (!match(dist, me.name) && !match(dist, me.id))
	{
		sendto_realops_flags(UMODE_ALL, L_ALL,
                       	    "ERROR: SERVICE:%s DIST:%s from %s", name,
				    dist, client_p->name);
		return exit_client(NULL, client_p, &me, "Distribution code mismatch");
	}

	/* Reintroducing the same service is fine, as their name always contain their server.
           This way it is possible to change it's type/info/distmask without ever quitting.
           Note that 2.11 will just go ahead and happily register the same service fullname again */
	svc = find_any_client(name);

	/* Services are just clients as not much info is needed to keep about them */
	if (!svc)
	{
		svc = make_client(source_p);

		svc->name = rb_strdup(name);
		svc->servptr = source_p;
		svc->from = client_p;
		svc->hopcount = source_p->hopcount;

		add_to_hash(HASH_CLIENT, svc->name, svc);

		rb_dlinkAdd(svc, &svc->lnode, &svc->servptr->serv->users);
		rb_dlinkAdd(svc, &svc->node, &svc_list);

		ServerStats.is_services++;
		SetService(svc);
	} else if (svc->servptr != source_p) {
		/* got the same service name from somewhere else, this cant be good. */
		sendto_server(NULL, NULL, CAP_IRCNET, NOCAPS,
			":%s KILL %s :Service %s collision (old from %s [%s], new from %s [%s])",
			me.id, svc->name, svc->servptr->name, svc->servptr->id, source_p->name, source_p->id);
		exit_client(NULL, svc, source_p, "Service name collision");
		return 0;
	}

	/* Looks good, set the info */
	rb_strlcpy(svc->host, dist, HOSTLEN);
	rb_strlcpy(svc->info, info, sizeof(svc->info));
	svc->tsinfo = type;

	/* And spam */
	sendto_realops_flags(UMODE_EXTERNAL, L_ALL,
			"Received SERVICE %s from %s via %s (%s %d %s)",
			svc->name, source_p->name, client_p->name,
			svc->host, svc->hopcount, svc->info);

	/* No global server list here as 2.11 can't handle the fact we want
	   to target some mask across them */
	RB_DLINK_FOREACH(ptr, serv_list.head) {
		struct Client *target_p = ptr->data;

		if(IsMe(target_p) || target_p == client_p)
			continue;

		if (!match(dist, target_p->name) && !match(dist, target_p->id))
			continue;

		if (!IsCapable(target_p, CAP_IRCNET))
			continue;

		sendto_one(target_p, ":%s SERVICE %s %s %d :%s",
			source_p->id, svc->name, svc->host, (int)svc->tsinfo, svc->info);
	}
	return 0;
}


