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

mapi_hfn_list_av1 noctcp_hfnlist[] = {
	{ "can_send", (hookfn) h_can_send },
	{ NULL, NULL }
};

static unsigned int mymode;

static int
_modinit(void)
{
	mymode = cflag_add('C', chm_simple);
	if (mymode == 0)
		return -1;

	return 0;
}

static void
_moddeinit(void)
{
	cflag_orphan('C');
}

DECLARE_MODULE_AV1(chm_noctcp, _modinit, _moddeinit, NULL, NULL, noctcp_hfnlist, "SporksNet coding committee");

static void
h_can_send(void *vdata)
{
	char *text;
	hook_data_channel_approval *data = (hook_data_channel_approval *) vdata;

	if (!(data->cmd == COMMAND_PRIVMSG))
		return;
	
	if (data->chptr->mode.mode & mymode
		&& (!ConfigChannel.exempt_cmode_C || !is_any_op(data->msptr)))
	{
		text = LOCAL_COPY(data->data);
		if (*text == '\001' && strncasecmp(text + 1, "ACTION ", 7))
		{
			sendto_one_numeric(data->client, 404,
					"%s :Cannot send to channel - CTCPs to this channel are disallowed (+C set)",
					data->chptr->chname);
			data->approved = CAN_SEND_NO_NONOTIFY;
			return;
		}
	}
	else if (rb_dlink_list_length(&data->chptr->locmembers) >
			(unsigned int)(GlobalSetOptions.floodcount / 2))
		data->client->large_ctcp_sent = rb_current_time();
	
	return;
}

