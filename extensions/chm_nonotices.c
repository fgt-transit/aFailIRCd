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

mapi_hfn_list_av1 nonotices_hfnlist[] = {
	{ "can_send", (hookfn) h_can_send },
	{ NULL, NULL }
};

static unsigned int mymode;

static int
_modinit(void)
{
	mymode = cflag_add('T', chm_simple);
	if (mymode == 0)
		return -1;

	return 0;
}

static void
_moddeinit(void)
{
	cflag_orphan('T');
}

DECLARE_MODULE_AV1(chm_nonotices, _modinit, _moddeinit, NULL, NULL, nonotices_hfnlist, "SporksNet coding committee");

static void
h_can_send(void *vdata)
{
	hook_data_channel_approval *data = (hook_data_channel_approval *) vdata;

	if (!(data->cmd == COMMAND_NOTICE))
		return;
	
	if(data->chptr->mode.mode & mymode &&
			(!ConfigChannel.exempt_cmode_T || !is_any_op(data->msptr)))
	{
		sendto_one_numeric(data->client, 404,
				"%s :Cannot send to channel - Notices are disallowed (+T set)",
				data->chptr->chname);
		data->approved = CAN_SEND_NO_NONOTIFY;
	}
	return;
}

