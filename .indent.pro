/* $Id: .indent.pro 238 2005-09-21 05:26:03Z nenolod $ */

/* copy this file to the source dir then run indent file.c */

--gnu-style

/* This is the indent before the brace not inside the block. */
--brace-indent0

/* Indent case: by 2 and braces inside case by 0(then by 0)... */
--case-brace-indentation0
--case-indentation2

--indent-level8

/* Put while() on the brace from do... */
--cuddle-do-while

/* Disable an annoying format... */
--no-space-after-function-call-names

/* Disable an annoying format... */
--dont-break-procedure-type

/* Disable an annoying format... */
--no-space-after-casts

--line-length200

/* typedefs */
-T mapi_clist_av1
-T hook
-T hook_data
-T hook_data_int
-T hook_data_channel
-T hook_data_channel_activity
-T hook_data_channel_approval
-T hook_data_client_approval
-T hook_data_client_exit
-T hook_data_umode_changed
-T ExtbanFunc
-T ChannelModeFunc
-T DCF
-T ilogfile
-T PrivilegeFlags
-T Info
-T HandlerType
-T MessageHandler
-T conf_parm_t
-T HEADER
-T temp_list
-T tgchange
-T ssl_ctl_t
-T rb_socklen_t
-T comm_event_cb_t
-T comm_event_id
-T log_cb
-T restart_cb
-T die_cb
-T rb_bh
-T rb_bh_usage_cb
-T rb_fde_t
-T PF
-T CNCB
-T DUMPCB
-T ACCB
-T ACPRE
-T prng_seed_t
-T EVH
-T rb_helper
-T rb_helper_cb
-T buf_line_t
-T buf_head_t
-T rb_prefix_t
-T rb_patricia_node_t
-T rb_patricia_tree_t
-T rawbuf_t
-T rawbuf_head_t
-T rb_dlink_node
-T rb_dlink_list
