#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"
#include "s_conf.h"
#include "s_user.h"
#include "s_serv.h"
#include "numeric.h"
#include "chmode.h"
#include "match.h"
#include "inline/stringops.h"

static void h_can_send(void *vdata);

mapi_hfn_list_av1 blockcaps_hfnlist[] = {
	{ "can_send", (hookfn) h_can_send },
	{ NULL, NULL }
};

static unsigned int mymode;

static int
_modinit(void)
{
	mymode = cflag_add('G', chm_simple);
	if (mymode == 0)
		return -1;
	
	return 0;
}

static void
_moddeinit(void)
{
	cflag_orphan('G');
}

DECLARE_MODULE_AV1(chm_blockcaps, _modinit, _moddeinit, NULL, NULL, blockcaps_hfnlist, "SporksNet coding committee");

static void
h_can_send(void *vdata)
{
	char text[BUFSIZE];
	size_t contor;
	int caps = 0;
	int len = 0;

	hook_data_channel_approval *data = (hook_data_channel_approval *) vdata;

	if (data->chptr->mode.mode & mymode &&
	   (!ConfigChannel.exempt_cmode_G || !is_any_op(data->msptr)))
	{
		strip_unprintable((char *)data->data, text, BUFSIZE);
		if(strlen(text) < 10)
			return;

		for(contor = 0; contor < strlen(text); contor++)
		{
			if(IsUpper(text[contor]) && !isdigit(text[contor]))
				caps++; 
			len++;
		}
		/* Added divide by 0 check --alxbl 
		 * Make the check work portably --Elizabeth */
		if(len > 0)
		{
			if(((caps * 100) / len) >= ConfigChannel.caps_threshold)
			{
				if(!(data->cmd == COMMAND_PRIVMSG))
					sendto_one_numeric(data->client, 404,
						"%s :Cannot send to channel - Your message contains more than %d%% capital letters (+G set)", 
						data->chptr->chname, ConfigChannel.caps_threshold);
				data->approved = CAN_SEND_NO_NONOTIFY;
				return;
			}
		}
	}

	return;
}

