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

mapi_hfn_list_av1 nocolour_hfnlist[] = {
	{ "can_send", (hookfn) h_can_send },
	{ NULL, NULL }
};

static unsigned int mymode;

static int
_modinit(void)
{
	mymode = cflag_add('c', chm_simple);
	if (mymode == 0)
		return -1;

	return 0;
}

static void
_moddeinit(void)
{
	cflag_orphan('c');
}

DECLARE_MODULE_AV1(chm_nocolour, _modinit, _moddeinit, NULL, NULL, nocolour_hfnlist, "SporksNet coding committee");

static void
h_can_send(void *vdata)
{
	char text[BUFSIZE];
	hook_data_channel_approval *data = (hook_data_channel_approval *) vdata;

	if(data->chptr->mode.mode & mymode && (!ConfigChannel.exempt_cmode_c || !is_any_op(data->msptr)))
	{
		strip_colour((char *)data->data, text, BUFSIZE);
		if (EmptyString(text))
		{
			if(data->cmd == COMMAND_PRIVMSG)
				sendto_one(data->client, form_str(ERR_NOTEXTTOSEND), me.name, data->client->name);
			data->approved = CAN_SEND_NO_NONOTIFY;
			return;
		}

		/* Copy it */
		rb_strlcpy(data->data, text, BUFSIZE);
	}

	return;
}

