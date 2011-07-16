/*
 * SporksIRCD: A somewhat useful ircd.
 * m_forcejoin.c: Force joins a user
 *
 * Copyright (C) 2006 Charybdis development team
 * Copyright (C) 2010 Elizabeth Jennifer Myers. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"
#include "hash.h"
#include "s_conf.h"
#include "s_user.h"
#include "s_serv.h"
#include "numeric.h"

static int
_modinit(void)
{
	/* add the usermode to the available slot */
	user_modes['x'] = find_umode_slot();
	construct_umodebuf();

	return 0;
}

static void
_moddeinit(void)
{
	/* disable the umode and remove it from the available list */
	user_modes['x'] = 0;
	construct_umodebuf();
}

static void check_umode_change(void *data);
static void check_new_user(void *data);
mapi_hfn_list_av1 ip_cloaking_hfnlist[] = {
	{ "umode_changed", (hookfn) check_umode_change },
	{ "new_local_user", (hookfn) check_new_user },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(ip_cloaking, _modinit, _moddeinit, NULL, NULL,
			ip_cloaking_hfnlist, "$Revision: 3526 $");

static void
distribute_hostchange(struct Client *client)
{
	if (irccmp(client->host, client->orighost))
		sendto_one_numeric(client, RPL_HOSTHIDDEN, "%s :is now your hidden host",
			client->host);
	else
		sendto_one_numeric(client, RPL_HOSTHIDDEN, "%s :hostname reset",
			client->host);

	sendto_server(NULL, NULL,
		CAP_EUID | CAP_TS6, NOCAPS, ":%s CHGHOST %s :%s",
		use_id(&me), use_id(client), client->host);
	sendto_server(NULL, NULL,
		CAP_TS6, CAP_EUID, ":%s ENCAP * CHGHOST %s :%s",
		use_id(&me), use_id(client), client->host);
	if (irccmp(client->host, client->orighost))
		SetDynSpoof(client);
	else
		ClearDynSpoof(client);
}

static void
do_host_cloak_ip(const char *inbuf, char *outbuf)
{
	/* None of the characters in this table can be valid in an IP */
	char chartable[] = "ghijklmnopqrstuvwxyz";
	char *tptr;
	uint32_t accum = fnv_hash((const unsigned char*) inbuf, 32);
	int sepcount = 0;
	int totalcount = 0;
	int ipv6 = 0;

	rb_strlcpy(outbuf, inbuf, HOSTLEN + 1);

	if (strchr(outbuf, ':'))
	{
		ipv6 = 1;

		/* Damn you IPv6... 
		 * We count the number of colons so we can calculate how much
		 * of the host to cloak. This is because some hostmasks may not
		 * have as many octets as we'd like.
		 *
		 * We have to do this ahead of time because doing this during
		 * the actual cloaking would get ugly
		 */
		for (tptr = outbuf; *tptr != '\0'; tptr++)
			if (*tptr == ':')
				totalcount++;
	}
	else if (!strchr(outbuf, '.'))
		return;

	for (tptr = outbuf; *tptr != '\0'; tptr++) 
	{
		if (*tptr == ':' || *tptr == '.')
		{
			sepcount++;
			continue;
		}

		if (ipv6 && sepcount < totalcount / 2)
			continue;

		if (!ipv6 && sepcount < 2)
			continue;

		*tptr = chartable[(*tptr + accum) % 20];
		accum = (accum << 1) | (accum >> 31);
	}
}

static void
do_host_cloak_host(const char *inbuf, char *outbuf)
{
	char b26_alphabet[] = "abcdefghijklmnopqrstuvwxyz";
	char *tptr;
	uint32_t accum = fnv_hash((const unsigned char*) inbuf, 32);

	rb_strlcpy(outbuf, inbuf, HOSTLEN + 1);

	/* pass 1: scramble first section of hostname using base26 
	 * alphabet toasted against the FNV hash of the string.
	 *
	 * numbers are not changed at this time, only letters.
	 */
	for (tptr = outbuf; *tptr != '\0'; tptr++)
	{
		if (*tptr == '.')
			break;

		if (isdigit(*tptr) || *tptr == '-')
			continue;

		*tptr = b26_alphabet[(*tptr + accum) % 26];

		/* Rotate one bit to avoid all digits being turned odd or even */
		accum = (accum << 1) | (accum >> 31);
	}

	/* pass 2: scramble each number in the address */
	for (tptr = outbuf; *tptr != '\0'; tptr++)
	{
		if (isdigit(*tptr))
			*tptr = '0' + (*tptr + accum) % 10;

		accum = (accum << 1) | (accum >> 31);
	}	
}

static void
check_umode_change(void *vdata)
{
	hook_data_umode_changed *data = (hook_data_umode_changed *)vdata;
	struct Client *source_p = data->client;

	if (!MyClient(source_p))
		return;

	/* didn't change +x umode, we don't need to do anything */
	if (!((data->oldumodes ^ source_p->umodes) & user_modes['x']))
		return;

	if (source_p->umodes & user_modes['x'])
	{
		if (IsIPSpoof(source_p) || source_p->localClient->mangledhost == NULL || (IsDynSpoof(source_p) && strcmp(source_p->host, source_p->localClient->mangledhost)))
		{
			source_p->umodes &= ~user_modes['x'];
			return;
		}
		if (strcmp(source_p->host, source_p->localClient->mangledhost))
		{
			rb_strlcpy(source_p->host, source_p->localClient->mangledhost, HOSTLEN + 1);
			distribute_hostchange(source_p);
		}
		else /* not really nice, but we need to send this numeric here */
			sendto_one_numeric(source_p, RPL_HOSTHIDDEN, "%s :is now your hidden host",
				source_p->host);
	}
	else if (!(source_p->umodes & user_modes['x']))
	{
		if (source_p->localClient->mangledhost != NULL &&
				!strcmp(source_p->host, source_p->localClient->mangledhost))
		{
			rb_strlcpy(source_p->host, source_p->orighost, HOSTLEN + 1);
			distribute_hostchange(source_p);
		}
	}
}

static void
check_new_user(void *vdata)
{
	struct Client *source_p = (void *)vdata;

	if (IsIPSpoof(source_p))
	{
		source_p->umodes &= ~user_modes['x'];
		return;
	}
	source_p->localClient->mangledhost = rb_malloc(HOSTLEN + 1);
	if (!irccmp(source_p->orighost, source_p->sockhost))
		do_host_cloak_ip(source_p->orighost, source_p->localClient->mangledhost);
	else
		do_host_cloak_host(source_p->orighost, source_p->localClient->mangledhost);
	if (IsDynSpoof(source_p))
		source_p->umodes &= ~user_modes['x'];
	if (source_p->umodes & user_modes['x'])
	{
		rb_strlcpy(source_p->host, source_p->localClient->mangledhost, sizeof(source_p->host));
		if (irccmp(source_p->host, source_p->orighost))
			SetDynSpoof(source_p);
	}
}
