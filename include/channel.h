/*
 *  ircd-ratbox: A slightly useful ircd.
 *  channel.h: The ircd channel header.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2004 ircd-ratbox development team
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 */

#ifndef INCLUDED_channel_h
#define INCLUDED_channel_h

#include "s_conf.h"	/* ConfigChannel bleah */

#define MODEBUFLEN      200

/* Maximum mode changes allowed per client, per server is different */
#define MAXMODEPARAMS   4
#define MAXMODEPARAMSSERV 10

struct Client;

/* mode structure for channels */
struct Mode
{
	unsigned int mode;
	int limit;
	char key[KEYLEN];
	unsigned int join_num;
	unsigned int join_time;
	char forward[LOC_CHANNELLEN + 1];
};

/* channel structure */
struct Channel
{
	rb_dlink_node node;
	struct Mode mode;
	char *mode_lock;
	char *topic;
	char *topic_info;
	time_t topic_time;
	time_t last_knock;	/* don't allow knock to flood */

	rb_dlink_list members;	/* channel members */
	rb_dlink_list locmembers;	/* local channel members */

	rb_dlink_list invites;
	rb_dlink_list banlist;
	rb_dlink_list exceptlist;
	rb_dlink_list invexlist;
	rb_dlink_list quietlist;
	rb_dlink_list censorlist;

	time_t first_received_message_time;	/* channel flood control */
	int received_number_of_privmsgs;
	int flood_noticed;

	unsigned int join_count;  /* joins within delta */
	unsigned int join_delta;  /* last ts of join */

	struct Dictionary *metadata;

	unsigned long bants;
	time_t channelts;
	char *chname;
};

struct membership
{
	rb_dlink_node channode;
	rb_dlink_node locchannode;
	rb_dlink_node usernode;

	struct Channel *chptr;
	struct Client *client_p;
	unsigned int flags;

	unsigned long bants;
};

#define BANLEN 195
#define CENSORLEN 64
struct mode_list_t 
{
	char *maskstr;
	char *who;
	time_t when;
	rb_dlink_node node;
};

struct mode_letter
{
	int mode;
	char letter;
};

struct ChModeChange
{
	char letter;
	const char *arg;
	const char *id;
	int dir;
	int caps;
	int nocaps;
	int mems;
	struct Client *client;
};

struct ChCapCombo
{
	int count;
	int cap_yes;
	int cap_no;
};

typedef void (*ChannelModeFunc)(struct Client *source_p, struct Channel *chptr,
		int alevel, int parc, int *parn,
		const char **parv, int *errors, int dir, char c, long mode_type);

struct ChannelMode
{
	ChannelModeFunc set_func;
	long mode_type;
};

typedef int (*ExtbanFunc)(const char *data, struct Client *client_p,
		struct Channel *chptr, long mode_type);

/* can_send results */
#define CAN_SEND_NO		0
#define CAN_SEND_NONOP		1
#define CAN_SEND_OPV		2
#define CAN_SEND_NO_NONOTIFY	3 /* Can't send, and don't tell the user */

/* channel status flags */

#define CHFL_PEON		0x0000	/* normal member of channel */
#define CHFL_VOICE		0x0001	/* the power to speak */
#define CHFL_HALFOP		0x0002	/* half-operator */
#define CHFL_CHANOP     	0x0004	/* Channel operator */
#define CHFL_ADMIN		0x0008	/* cannot be kicked by ops */
#define CHFL_FOUNDER		0x0010	/* Owner of the channel */

#define CHFL_BANNED		0x0020  /* cached as banned */
#define CHFL_QUIETED		0x0040  /* cached as being +q victim */
#define ONLY_SERVERS		0x0080
#define	ONLY_OPERS		0x0100

#define ALL_MEMBERS		CHFL_PEON
#define ONLY_VOICEDANDUP	(CHFL_FOUNDER|CHFL_ADMIN|CHFL_CHANOP|CHFL_HALFOP|CHFL_VOICE)
#define ONLY_HALFOPSANDUP	(CHFL_FOUNDER|CHFL_ADMIN|CHFL_CHANOP|CHFL_HALFOP)
#define ONLY_CHANOPSANDUP	(CHFL_FOUNDER|CHFL_ADMIN|CHFL_CHANOP)
#define ONLY_ADMINSANDUP	(CHFL_FOUNDER|CHFL_ADMIN)
#define ONLY_FOUNDERS		(CHFL_FOUNDER)

#define is_halfop(x)	(ConfigChannel.use_halfop &&		\
			 ((x) && (x)->flags & CHFL_HALFOP))
#define is_admin(x)	(ConfigChannel.use_admin && 		\
			 ((x) && (x)->flags & CHFL_ADMIN))
#define is_founder(x)	(ConfigChannel.use_founder &&		\
			 ((x) && (x)->flags & CHFL_FOUNDER))
#define is_chanop(x)	((x) && (x)->flags & CHFL_CHANOP)
#define is_voiced(x)	((x) && (x)->flags & CHFL_VOICE)
#define is_any_op(x)	(is_chanop(x) || is_halfop(x) || is_admin(x) ||	\
			 is_founder(x))
#define is_op_voiced(x)	(is_any_op(x) || is_voiced(x))
#define can_send_banned(x) ((x) && (x)->flags & (CHFL_BANNED|CHFL_QUIETED))

/* channel modes ONLY */
#define MODE_PRIVATE    0x0001
#define MODE_SECRET     0x0002
#define MODE_MODERATED  0x0004
#define MODE_TOPICLIMIT 0x0008
#define MODE_INVITEONLY 0x0010
#define MODE_NOPRIVMSGS 0x0020
#define MODE_REGONLY	0x0040
#define MODE_EXLIMIT	0x0080  /* exempt from list limits, +b/+e/+I/+q */
#define MODE_PERMANENT  0x0100  /* permanent channel, +P */
#define MODE_OPMODERATE 0x0200  /* send rejected messages to ops */
#define MODE_FREEINVITE 0x0400  /* allow free use of /invite */
#define MODE_FREETARGET 0x0800  /* can be forwarded to without authorization */
#define MODE_DISFORWARD 0x1000  /* disable channel forwarding */
#define MODE_NOKICK	0x2000 /* Disable /kick on this channel */
#define MODE_NONICK	0x4000 /* Disable /nick for anyone on this channel */
#define MODE_NOREJOIN	0x8000 /* Block rejoin immediately after kick */
#define MODE_NOOPERKICK	0x10000 /* disallow kicking opers */


#define CHFL_BAN        0x8000000	/* ban channel flag */
#define CHFL_EXCEPTION  0x10000000	/* exception to ban channel flag */
#define CHFL_INVEX      0x20000000
#define CHFL_QUIET      0x40000000

#define CHFL_CENSOR	0x80000000

/* mode flags for direction indication */
#define MODE_QUERY     0
#define MODE_ADD       1
#define MODE_DEL       -1

#define SecretChannel(x)        ((x) && ((x)->mode.mode & MODE_SECRET))
#define HiddenChannel(x)        ((x) && ((x)->mode.mode & MODE_PRIVATE))
#define PubChannel(x)           ((!x) || ((x)->mode.mode &\
                                 (MODE_PRIVATE | MODE_SECRET)) == 0)

/* channel visible */
#define ShowChannel(v,c)        (PubChannel(c) || IsMember((v),(c)))

#define IsMember(who, chan) ((who && who->user && \
                find_channel_membership(chan, who)) ? 1 : 0)

#define IsChannelName(name) ((name) && (*(name) == '#' || *(name) == '&'))

/* extban function results */
#define EXTBAN_INVALID -1  /* invalid mask, false even if negated */
#define EXTBAN_NOMATCH  0  /* valid mask, no match */
#define EXTBAN_MATCH    1  /* matches */

/* For the CAN_SEND hook stuff */
#define COMMAND_PART	0x0001
#define COMMAND_TOPIC	0x0002
#define COMMAND_PRIVMSG	0x0004
#define COMMAND_NOTICE	0x0008

extern rb_dlink_list global_channel_list;
void init_channels(void);

struct Channel *allocate_channel(const char *chname);
void free_channel(struct Channel *chptr);
struct mode_list_t *allocate_list_item(const char *, const char *);
void free_list_item(struct mode_list_t *bptr);


extern void destroy_channel(struct Channel *);

extern int can_send(struct Channel *chptr, struct Client *who, 
		    struct membership *, char *msg, unsigned int cmd);
extern int flood_attack_channel(int p_or_n, struct Client *source_p,
				struct Channel *chptr, char *chname);
extern int is_banned(struct Channel *chptr, struct Client *who,
		     struct membership *msptr, const char *, const char *);
extern int is_quieted(struct Channel *chptr, struct Client *who,
		     struct membership *msptr, const char *, const char *);
extern int can_join(struct Client *source_p, struct Channel *chptr, char *key);

extern struct membership *find_channel_membership(struct Channel *, struct Client *);
extern const char *find_channel_status(struct membership *msptr, int combine);
extern int get_channel_access(struct Client *source_p, struct membership *msptr);
extern void add_user_to_channel(struct Channel *, struct Client *, int flags);
extern void remove_user_from_channel(struct membership *);
extern void remove_user_from_channels(struct Client *);
extern void invalidate_bancache_user(struct Client *);

extern void free_channel_list(rb_dlink_list *);
extern void free_channel_censor(rb_dlink_list *);

extern int check_channel_name(const char *name);

extern void channel_member_names(struct Channel *chptr, struct Client *,
				 int show_eon);

extern void del_invite(struct Channel *chptr, struct Client *who);

const char *channel_modes(struct Channel *chptr, struct Client *who);

extern int has_common_channel(struct Client *client1, struct Client *client2);

extern struct Channel *find_bannickchange_channel(struct Client *client_p);

extern struct Channel *find_nonickchange_channel(struct Client *client_p);

extern void check_spambot_warning(struct Client *source_p, const char *name);

extern void check_splitmode(void *);

void set_channel_topic(struct Channel *chptr, const char *topic,
		       const char *topic_info, time_t topicts);

extern void init_chcap_usage_counts(void);
extern void set_chcap_usage_counts(struct Client *serv_p);
extern void unset_chcap_usage_counts(struct Client *serv_p);
extern void send_cap_mode_changes(struct Client *client_p, struct Client *source_p,
				  struct Channel *chptr, struct ChModeChange foo[], int);

void resv_chan_forcepart(const char *name, const char *reason, int temp_time);

extern void set_channel_mode(struct Client *client_p, struct Client *source_p,
            	struct Channel *chptr, struct membership *msptr, int parc, const char *parv[]);
extern void set_channel_mlock(struct Client *client_p, struct Client *source_p,
            	struct Channel *chptr, const char *newmlock, int propagate);

extern struct ChannelMode chmode_table[256];

extern int add_id(struct Client *source_p, struct Channel *chptr, const char *banid,
       rb_dlink_list * list, long mode_type);

extern int del_id(struct Client *source_p, struct Channel *chptr, const char *banid, rb_dlink_list * list, long mode_type);

extern int add_censor(struct Client *source_p, struct Channel *chptr, const char *censorid,
       rb_dlink_list * list, long mode_type);

extern int del_censor(struct Client *source_p, struct Channel *chptr, const char *censorid, 
       rb_dlink_list * list, long mode_type);


extern ExtbanFunc extban_table[256];

extern int match_extban(const char *banstr, struct Client *client_p, struct Channel *chptr, long mode_type);
extern int valid_extban(const char *banstr, struct Client *client_p, struct Channel *chptr, long mode_type);
const char * get_extban_string(void);

extern struct Channel * check_forward(struct Client *source_p, struct Channel *chptr, char *key);
extern void user_join(struct Client * client_p, struct Client * source_p, const char * channels, const char * keys);
extern void do_join_0(struct Client *client_p, struct Client *source_p);
extern int check_channel_name_loc(struct Client *source_p, const char *name);

extern struct Metadata *channel_metadata_add(struct Channel *target, const char *name, const char *value, int propegate);
extern struct Metadata *channel_metadata_time_add(struct Channel *target, const char *name, time_t timevalue, const char *value);
extern void channel_metadata_delete(struct Channel *target, const char *name, int propegate);
extern struct Metadata *channel_metadata_find(struct Channel *target, const char *name);
extern void channel_metadata_clear(struct Channel *target);


extern void send_channel_join(struct Channel *chptr, struct Client *client_p);

#endif /* INCLUDED_channel_h */
