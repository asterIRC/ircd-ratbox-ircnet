/*  modules/m_operspy.c
 *  Copyright (C) 2003-2005 ircd-ratbox development team
 *  Copyright (C) 2003 Lee Hardy <lee@leeh.co.uk>
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
 * $Id: m_operspy.c 142 2010-01-23 19:22:37Z karel.tuma $
 */

#include "stdinc.h"
#include "struct.h"
#include "send.h"
#include "ircd.h"
#include "parse.h"
#include "modules.h"
#include "s_log.h"
#include "s_newconf.h"

static int ms_operspy(struct Client *client_p, struct Client *source_p,
		      int parc, const char *parv[]);
static int mo_operspy(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);

struct Message operspy_msgtab = {
	"OPERSPY", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, mg_ignore, mg_ignore, mg_ignore, {ms_operspy, 2}, {mo_operspy, 2}}
};

mapi_clist_av2 operspy_clist[] = { &operspy_msgtab, NULL };

DECLARE_MODULE_AV2(operspy, NULL, NULL, operspy_clist, NULL, NULL, "$Revision: 142 $");

/* ms_operspy()
 *
 * parv[1] - operspy command
 * parv[2] - optional params
 */
static int
ms_operspy(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(parc < 4)
	{
		report_operspy(source_p, parv[1], parc < 3 ? NULL : parv[2]);
	}
	/* buffer all remaining into one param */
	else
	{
		report_operspy(source_p, parv[1], array_to_string(&parv[2], parc-2, 0));
	}

	return 0;
}

/* mo_operspy()
 *
 * parv[1] - operspy command
 * parv[2] - optional params
 */
static int
mo_operspy(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	char *pbuf;
	if (!IsOperSpy(source_p) || operspy)
		return 0;

	operspy = 1;
	pbuf = array_to_string(&parv[1], parc-1, 0);
	parse(client_p, pbuf, pbuf + strlen(pbuf));
	operspy = 0;
	return 0;
}

