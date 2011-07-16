/* extensions/botmode.c
 *   Copyright (C) 2010 ShadowIRCd Development Team <http://shadowircd.net>
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
 */

#include "stdinc.h"

#include "send.h"
#include "client.h"
#include "common.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "parse.h"
#include "modules.h"
#include "s_user.h"

static int _modinit(void)
{
	user_modes['B'] = find_umode_slot();
	construct_umodebuf();

	return 0;
}

static void _moddeinit(void)
{
	user_modes['B'] = 0;
	construct_umodebuf();
}

static void h_svc_whois(hook_data_client *);

mapi_hfn_list_av1 botmode_hfnlist[] = {
	{ "doing_whois",	(hookfn) h_svc_whois },
	{ "doing_whois_global",	(hookfn) h_svc_whois },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(botmode, _modinit, _moddeinit, NULL, NULL, botmode_hfnlist, "$Revision: 1907 $");

static void
h_svc_whois(hook_data_client *data)
{
	if(data->target->umodes & user_modes['B'])
	{
		sendto_one_numeric(data->client, 335,
                                "%s :is a bot",
                                data->target->name);
	}
}
