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
#include "inline/stringops.h"

static void h_can_send(void *vdata);

mapi_hfn_list_av1 norepeat_hfnlist[] = {
	{ "can_send", (hookfn) h_can_send },
	{ NULL, NULL }
};

static unsigned int mymode;

static int
_modinit(void)
{
	mymode = cflag_add('K', chm_simple);
	if (mymode == 0)
		return -1;

	return 0;
}

static void
_moddeinit(void)
{
	cflag_orphan('K');
}

DECLARE_MODULE_AV1(chm_norepeat, _modinit, _moddeinit, NULL, NULL, norepeat_hfnlist, "SporksNet coding committee");

static void
h_can_send(void *vdata)
{
	char text[BUFSIZE];
	struct Metadata *md;
	hook_data_channel_approval *data = (hook_data_channel_approval *) vdata;

	if(data->chptr->mode.mode & mymode)
	{
		md = channel_metadata_find(data->chptr, "NOREPEAT");
		if(md && (!ConfigChannel.exempt_cmode_K || !is_any_op(data->msptr)))
		{
			strip_unprintable((char *)data->data, text, BUFSIZE);
			if(!(strcmp(md->value, text)))
			{
				if(data->cmd ==COMMAND_PRIVMSG)
					sendto_one_numeric(data->client, 404,
							"%s :Cannot send to channel - Message blocked due to repeating (+K set)",
							data->chptr->chname);
				data->approved = CAN_SEND_NO_NONOTIFY;
				return;
			}
		}
		channel_metadata_delete(data->chptr, "NOREPEAT", 0);
		channel_metadata_add(data->chptr, "NOREPEAT", text, 0);
	}

	return;
}

