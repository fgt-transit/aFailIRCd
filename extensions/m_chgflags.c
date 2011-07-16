/*
 * m_chgflags.c - Change a user's auth flags on IRC.
 *
 * Use this for whatever purpose. Buy me a bottle of wine if you find it useful
 * enough and you meet me (none of the cheap shit either). Also no beer.
 */

#include "stdinc.h"
#include "modules.h"
#include "numeric.h"
#include "client.h"
#include "ircd.h"
#include "send.h"
#include "s_user.h"
#include "s_serv.h"
#include "s_conf.h"
#include "s_newconf.h"


struct authflag
{
	const char *name;
	int flag;
};

static struct authflag auth_flags[] =
{
	{ "kline_exempt",	FLAGS2_EXEMPTKLINE	},
	{ "spambot_exempt",	FLAGS2_EXEMPTSPAMBOT	},
	{ "shide_exempt",	FLAGS2_EXEMPTSHIDE	},
	{ "jupe_exempt",	FLAGS2_EXEMPTJUPE	},
	{ "resv_exempt",	FLAGS2_EXEMPTRESV	},
	{ "flood_exempt",	FLAGS2_EXEMPTFLOOD	},
	{ NULL,			0			}
};

static int mo_chgflags(struct Client *client_p, struct Client *source_p,
		       int parc, const char *parv[]);
static int me_chgflags(struct Client *client_p, struct Client *source_p,
		       int parc, const char *parv[]);
static void do_chgflags(struct Client *source_p, struct Client *target_p, const char *flags);

struct Message chgflags_msgtab = {
	"CHGFLAGS", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, {me_chgflags, 3}, {mo_chgflags, 3}}
};

mapi_clist_av1 chgflags_clist[] = { &chgflags_msgtab, NULL };

DECLARE_MODULE_AV1(chgflags, NULL, NULL, chgflags_clist, NULL, NULL, "SporksNet coding committee");

/* mo_chgflags - chgflags message handler
 *	parv[1] = user
 *	parv[2] = flags (comma-separated)
 */
static int
mo_chgflags(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;

	if (!IsOperGrant(source_p))
	{
		sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "grant");
		return 0;
	}

	if(EmptyString(parv[2]))
	{
		sendto_one(source_p, form_str(ERR_NEEDMOREPARAMS), me.name,
			   source_p->name, "CHGFLAGS");
		return 0;
	}

	target_p = find_named_person(parv[1]);
	if (target_p == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), parv[1]);
		return 0;
	}

	do_chgflags(source_p, target_p, parv[2]);
	sendto_match_servs(source_p, "*", CAP_ENCAP, NOCAPS,
			   "ENCAP * CHGFLAGS %s %s",
			   get_id(target_p, target_p), parv[2]);
	
	return 0;
}

static int
me_chgflags(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;

	target_p = find_person(parv[1]);
	if (target_p == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), parv[1]);
		return 0;
	}

	/* XXX should it be in the same clustering "class"? */
	if (!find_shared_conf(source_p->username, source_p->host,
			      source_p->servptr->name, SHARED_GRANT))
	{
		sendto_one(source_p, ":%s NOTICE %s :You don't have an appropriate shared"
			   "block to grant auth flags on this server.", me.name, source_p->name);
		return 0;
	}

	do_chgflags(source_p, target_p, parv[2]);

	return 0;
}

static void
do_chgflags(struct Client *source_p, struct Client *target_p, const char *flags)
{
	char *name;
	char *flags2 = LOCAL_COPY(flags);
	char *p = NULL;

	for(name = rb_strtok_r(flags2, ",", &p); name; name = rb_strtok_r(NULL, ",", &p))
	{
		int x;
		int found = 0;

		for(x = 0; auth_flags[x].name != NULL && auth_flags[x].flag != 0; x++)
		{
			if(strcasecmp(name, auth_flags[x].name))
				continue;

			/* Check if the flag is set
			 * XXX probably shouldn't assume flags2 even if flags is full... */
			if((target_p->flags2 & auth_flags[x].flag) == 0)
			{
				/* Flag is not set, let's set it */
				target_p->flags2 |= auth_flags[x].flag;
				if(MyClient(target_p))
					sendto_realops_snomask(SNO_GENERAL, L_NETWIDE,
							"%s is setting auth flag %s on %s",
							get_oper_name(source_p), name,
							target_p->name);
				found = 1;
				break;
			}
			else
			{
				/* Flag is set, let's unset it */
				target_p->flags2 &= ~(auth_flags[x].flag);
				if(MyClient(target_p))
					sendto_realops_snomask(SNO_GENERAL, L_NETWIDE,
							"%s is removing auth flag %s on %s",
							get_oper_name(source_p), name,
							target_p->name);
				found = 1;
				break;
			}
		}

		if(!found && MyClient(target_p))
			sendto_one_notice(source_p, ":flag %s not found", name);
	}
}
