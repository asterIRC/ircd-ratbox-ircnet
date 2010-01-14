/*
 *  ircd-ratbox: A slightly useful ircd.
 *  uid.c: base36 uid handling, as it was too generic to be tossed somewhere
 *	   else.
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
 */

#include "match.h"
#include "uid.h"


/*
 * check_anid
 *	Check that given string is valid ID and return its length.
 */
int check_anid(const char *anid)
{
	int len = 1;

	if (!IsDigit(anid[0]))
		return 0;

	while (*++anid) {
		if (!IsIdChar(*anid))
			return -1;
		len++;
	}
	return len;
}

/*
 * check_uid
 *	Check that user ID is in the allowed range
 */
int check_uid(const char *uid)
{
	int l = check_anid(uid);

	if (l < 0)
		return 0;
	return (l >= MINUIDLEN && l <= MAXUIDLEN);
}

/*
 * check_sid
 *	Check that server ID is in the allowed range
 */
int check_sid(const char *uid)
{
	int l = check_anid(uid);

	if (l < 0)
		return 0;
	return (l >= MINSIDLEN && l <= MAXSIDLEN);
}


/*
 * generate_uid
 *	Create base36 uid
 * inputs
 *	target buf, desired len and number l
 */
char *generate_uid(char *buf, int len, unsigned l)
{
	static unsigned char alphabet[36] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int i;
	for (i = len-1; i >= 0; i--){
		buf[i] = alphabet[l % 36];
		l /= 36;
	}
	buf[len] = 0;
	return buf;
}


