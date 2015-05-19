/*
 * Copyright (C) 2004-2005 Lee Hardy <lee -at- leeh.co.uk>
 * Copyright (C) 2004-2005 ircd-ratbox development team
 *
 * $Id: hook.h 130 2010-01-23 07:52:02Z karel.tuma $
 */
#ifndef INCLUDED_HOOK_H
#define INCLUDED_HOOK_H

typedef struct
{
	char *name;
	rb_dlink_list hooks;
} hook;

typedef void (*hookfn) (void *data);

extern int h_iosend_id;
extern int h_iorecv_id;
extern int h_iorecvctrl_id;

extern int h_burst_client;
extern int h_burst_channel;
extern int h_burst_finished;
extern int h_server_introduced;
extern int h_schan_notice;
extern int h_schan_clean;
extern int h_schan_add;

void init_hook(void);
int register_hook(const char *name);
void add_hook(const char *name, hookfn fn);
void remove_hook(const char *name, hookfn fn);
void call_hook(int id, void *arg);

typedef struct
{
	struct Client *client;
	const void *arg1;
	const void *arg2;
} hook_data;

typedef struct
{
	struct Client *client;
	const void *arg1;
	int arg2;
} hook_data_int;

typedef struct
{
	struct Client *client;
	struct Client *target;
} hook_data_client;

typedef struct
{
	struct Client *client;
	struct Channel *chptr;
} hook_data_channel;

typedef struct
{
	char name[CHANNELLEN];
	char topic[MAX_TOPICLEN];
	char pattern[BUFSIZE];
	int flags;
	int operonly;
	int pass;
} hook_data_schan;

#endif
