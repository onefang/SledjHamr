/* This code is heavily based on luaproc.
 *
 * The luaproc copyright notice and license is -

 ***************************************************

Copyright 2008 Alexandre Skyrme, Noemi Rodriguez, Roberto Ierusalimschy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

 ****************************************************
 *
 * Additions and changes Copyright 2012 by David Seikel, using the above license.
 */


/* This is a redesign of luaproc.  The design goals and notes -
 *
 * In general use EFL where it is useful.
 * One fixed unique message channel per script.
 *   No need for channel.c / .h, we are not using that sort of arbitrary channels.
 *   FIFO queue on message channels, seems the C socket queue is not enough.
 *     On the other hand, could just peel messages of the socket queue, then shove them on the scripts queue.
 *   Probably one fixed unique message channel per object, which each script in the object shares.
 *     But might be better to handle that C side anyway.
 * Better integration with LuaSL.
 *   Merge the luaproc structure with the script structure.
 * Use ecore threads instead of raw pthreads.
 *   Ecore threads pretty much wraps pthreads on posix, but has Windows support to.
 * Merge in the edje Lua code, and keep an eye on that, coz we might want to actually add this to edje Lua in the future.
 * Use my coding standards, or EFL ones.  Pffft.
 *
 */

#include "LuaSL.h"


#define CHANNEL_MAX_NAME_LENGTH 255

#define CHANNEL_DESTROYED 0

/* ready process queue insertion status */
#define LUAPROC_SCHED_QUEUE_PROC_OK	0
#define LUAPROC_SCHED_QUEUE_PROC_ERR	-1

/* scheduler default number of worker threads */
#define LUAPROC_SCHED_DEFAULT_WORKER_THREADS	1

/* process is idle */
#define LUAPROC_STAT_IDLE		0
/* process is ready to run */
#define LUAPROC_STAT_READY		1
/* process is blocked on send */
#define LUAPROC_STAT_BLOCKED_SEND	2
/* process is blocked on receive */
#define LUAPROC_STAT_BLOCKED_RECV	3


/* message channel */
struct stchannel {
	Eina_Clist send;
	Eina_Clist recv;
	pthread_mutex_t *mutex;
	pthread_cond_t *in_use;
};

/* lua process */
struct stluaproc {
	Eina_Clist node;
	lua_State *lstate;
	int status;
	int args;
	channel chan;
	void *data;
};


/*********
* globals
*********/

/* global channel lua_State mutex */
pthread_mutex_t mutex_channel_lstate = PTHREAD_MUTEX_INITIALIZER;

/* global where channels will be stored */
Eina_Hash *channels;

/* ready process list */
Eina_Clist lpready;

/* ready process queue access mutex */
pthread_mutex_t mutex_queue_access = PTHREAD_MUTEX_INITIALIZER;

/* wake worker up conditional variable */
pthread_cond_t cond_wakeup_worker = PTHREAD_COND_INITIALIZER;

/* active luaproc count access mutex */
pthread_mutex_t mutex_lp_count = PTHREAD_MUTEX_INITIALIZER;

/* no active luaproc conditional variable */
pthread_cond_t cond_no_active_lp = PTHREAD_COND_INITIALIZER;

/* number of active luaprocs */
int lpcount = 0;

/* no more lua processes flag */
int no_more_processes = FALSE;

/* channel operations mutex */
pthread_mutex_t mutex_channel = PTHREAD_MUTEX_INITIALIZER;

/* recycle list mutex */
pthread_mutex_t mutex_recycle_list = PTHREAD_MUTEX_INITIALIZER;

/* recycled lua process list */
Eina_Clist recyclelp;


/******************************
* library functions prototypes
******************************/
/* send a message to a lua process */
static int luaproc_send( lua_State *L );
/* receive a message from a lua process */
static int luaproc_receive( lua_State *L );
/* create a new channel */
static int luaproc_create_channel( lua_State *L );
/* send a message back to the main loop */
static int luaproc_send_back( lua_State *L );

/* luaproc function registration array - main (parent) functions */
static const struct luaL_reg luaproc_funcs_parent[] = {
	{ "sendback", luaproc_send_back },
	{ NULL, NULL }
};

/* luaproc function registration array - newproc (child) functions */
static const struct luaL_reg luaproc_funcs_child[] = {
	{ "send", luaproc_send },
	{ "receive", luaproc_receive },
	{ "newchannel", luaproc_create_channel },
	{ "sendback", luaproc_send_back },
	{ NULL, NULL }
};


/* queue a lua process sending a message without a matching receiver */
static void luaproc_queue_sender(luaproc lp)
{
    eina_clist_add_tail(&(lp->chan->send), &(lp->node));
}

/* dequeue a lua process sending a message with a receiver match */
static luaproc luaproc_dequeue_sender(channel chan)
{
    luaproc lp;

    if ((lp = (luaproc) eina_clist_head(&(chan->send))))
	eina_clist_remove(&(lp->node));

    return lp;
}

/* queue a luc process receiving a message without a matching sender */
static void luaproc_queue_receiver(luaproc lp)
{
    eina_clist_add_tail(&(lp->chan->recv), &(lp->node));
}

/* dequeue a lua process receiving a message with a sender match */
static luaproc luaproc_dequeue_receiver(channel chan)
{
    luaproc lp;

    if ((lp = (luaproc) eina_clist_head(&(chan->recv))))
	eina_clist_remove(&(lp->node));

    return lp;
}

/* unlock access to a channel */
static void luaproc_unlock_channel(channel chan)
{
    /* get exclusive access to operate on channels */
    pthread_mutex_lock(&mutex_channel);
    /* unlock channel access */
    pthread_mutex_unlock(chan->mutex);
    /* signal channel not in use */
    pthread_cond_signal(chan->in_use);
    /* free access to operate on channels */
    pthread_mutex_unlock(&mutex_channel);
}

/* increase active lua process count */
static void sched_lpcount_inc(void)
{
    pthread_mutex_lock(&mutex_lp_count);
    lpcount++;
    pthread_mutex_unlock(&mutex_lp_count);
}

/* decrease active lua process count */
static void sched_lpcount_dec(void)
{
    pthread_mutex_lock(&mutex_lp_count);
    lpcount--;
    /* if count reaches zero, signal there are no more active processes */
    if (lpcount == 0)
	pthread_cond_signal(&cond_no_active_lp);
    pthread_mutex_unlock(&mutex_lp_count);
}

/* worker thread main function */
static void *workermain( void *args ) {

	luaproc lp;
	int procstat;

	/* detach thread so resources are freed as soon as thread exits (no further joining) */
	pthread_detach( pthread_self( ));

	/* main worker loop */
	while ( 1 ) {

		/* get exclusive access to the ready process queue */
		pthread_mutex_lock( &mutex_queue_access );

		/* wait until instructed to wake up (because there's work to do or because its time to finish) */
		while (( eina_clist_count( &lpready ) == 0 ) && ( no_more_processes == FALSE )) {
			pthread_cond_wait( &cond_wakeup_worker, &mutex_queue_access );
		}

		/* pop the first node from the ready process queue */
		if ((lp = (luaproc) eina_clist_head(&lpready)))
		    eina_clist_remove(&(lp->node));
		else {
			/* free access to the process ready queue */
			pthread_mutex_unlock( &mutex_queue_access );
			/* finished thread */
			pthread_exit( NULL );
		}

		/* free access to the process ready queue */
		pthread_mutex_unlock( &mutex_queue_access );

		/* execute the lua code specified in the lua process struct */
		procstat = lua_resume(lp->lstate, lp->args);

		/* reset the process argument count */
		lp->args = 0;

		/* check if process finished its whole execution, then recycle it */
		if ( procstat == 0 ) {

			pthread_mutex_lock(&mutex_recycle_list);
			eina_clist_add_tail(&recyclelp, &(lp->node));
			pthread_mutex_unlock(&mutex_recycle_list);
			sched_lpcount_dec();

		}

		/* check if process yielded */
		else if ( procstat == LUA_YIELD ) {

			/* if so, further check if yield originated from an unmatched send/recv operation */
			if ( lp->status == LUAPROC_STAT_BLOCKED_SEND ) {
				/* queue blocked lua process on corresponding channel */
				luaproc_queue_sender( lp );
				/* unlock channel access */
				luaproc_unlock_channel(lp->chan);
			}

			else if ( lp->status == LUAPROC_STAT_BLOCKED_RECV ) {
				/* queue blocked lua process on corresponding channel */
				luaproc_queue_receiver( lp );
				/* unlock channel access */
				luaproc_unlock_channel(lp->chan);
			}

			/* or if yield resulted from an explicit call to coroutine.yield in the lua code being executed */
			else {
				/* get exclusive access to the ready process queue */
				pthread_mutex_lock( &mutex_queue_access );
				/* re-insert the job at the end of the ready process queue */
				eina_clist_add_tail(&lpready, &(lp->node));
				/* free access to the process ready queue */
				pthread_mutex_unlock( &mutex_queue_access );
			}
		}

		/* check if there was any execution error (LUA_ERRRUN, LUA_ERRSYNTAX, LUA_ERRMEM or LUA_ERRERR) */
		else {
			/* print error message */
			fprintf( stderr, "close lua_State (error: %s)\n", luaL_checkstring(lp->lstate, -1 ));
			/* close lua state */
			lua_close(lp->lstate);
			/* decrease active lua process count */
			sched_lpcount_dec();
		}
	}
}

/* move process to ready queue (ie, schedule process) */
static int sched_queue_proc( luaproc lp ) {

	/* get exclusive access to the ready process queue */
	pthread_mutex_lock( &mutex_queue_access );

	/* add process to ready queue */
	eina_clist_add_tail(&lpready, &(lp->node));

	lp->status = LUAPROC_STAT_READY;

	/* wake worker up */
	pthread_cond_signal( &cond_wakeup_worker );
	/* free access to the process ready queue */
	pthread_mutex_unlock( &mutex_queue_access );

	return LUAPROC_SCHED_QUEUE_PROC_OK;
}

/* synchronize worker threads */
void sched_join_workerthreads( void ) {

	pthread_mutex_lock( &mutex_lp_count );

	/* wait until there is no more active lua processes */
	while( lpcount != 0 ) {
		pthread_cond_wait( &cond_no_active_lp, &mutex_lp_count );
	}
	/* get exclusive access to the ready process queue */
	pthread_mutex_lock( &mutex_queue_access );
	/* set the no more active lua processes flag to true */
	no_more_processes = TRUE;
	/* wake ALL workers up */
	pthread_cond_broadcast( &cond_wakeup_worker );
	/* free access to the process ready queue */
	pthread_mutex_unlock( &mutex_queue_access );

// We don't need this, as we only get here during shutdown.  Linking this to EFL results in a hang otherwise anyway.
	/* wait for (join) worker threads */
//	pthread_exit( NULL );

	pthread_mutex_unlock( &mutex_lp_count );

}

/* create a new worker pthread */
int sched_create_worker( void ) {

	pthread_t worker;

	/* create a new pthread */
	if ( pthread_create( &worker, NULL, workermain, NULL ) != 0 ) {
		return LUAPROC_SCHED_PTHREAD_ERROR;
	}

	return LUAPROC_SCHED_OK;
}



void newProc(const char *code, int file, script *data)
{
    int ret;
    luaproc lp;

    // Try to recycle a Lua state, otherwise create one from scratch.
    pthread_mutex_lock(&mutex_recycle_list);
    /* pop list head */
    if ((lp = (luaproc) eina_clist_head(&recyclelp)))
	eina_clist_remove(&(lp->node));
    pthread_mutex_unlock(&mutex_recycle_list);

    if (lp == NULL)
    {
	lua_State *lpst = luaL_newstate();

	/* store the luaproc struct in its own Lua state */
	lp = (luaproc) lua_newuserdata(lpst, sizeof(struct stluaproc));
	lp->lstate = lpst;
	lua_setfield(lp->lstate, LUA_REGISTRYINDEX, "_SELF");
	luaL_openlibs(lp->lstate);
	luaL_register(lp->lstate, "luaproc", luaproc_funcs_child);
	eina_clist_element_init(&(lp->node));
    }

    lp->status = LUAPROC_STAT_IDLE;
    lp->args = 0;
    lp->chan = NULL;

    /* load process' code */
    if (file)
	ret = luaL_loadfile(lp->lstate, code);
    else
	ret = luaL_loadstring(lp->lstate, code);

    /* in case of errors, destroy Lua process */
    if (ret != 0)
    {
	lua_close(lp->lstate);
	lp = NULL;
    }

    if (lp)
    {
	lp->data = data;
	sched_lpcount_inc();

	/* schedule luaproc */
	if (sched_queue_proc(lp) != LUAPROC_SCHED_QUEUE_PROC_OK)
	{
	    printf( "[luaproc] error queueing Lua process\n" );
	    sched_lpcount_dec();
	    lua_close(lp->lstate);
	}
    }
}

/* moves values between lua states' stacks */
static void luaproc_movevalues( lua_State *Lfrom, lua_State *Lto ) {

	int i;
	int n = lua_gettop( Lfrom );

	/* move values between lua states' stacks */
	for ( i = 2; i <= n; i++ ) {
		lua_pushstring( Lto, lua_tostring( Lfrom, i ));
	}
}

/* return the lua process associated with a given lua state */
static luaproc luaproc_getself( lua_State *L ) {
	luaproc lp;
	lua_getfield( L, LUA_REGISTRYINDEX, "_SELF" );
	lp = (luaproc )lua_touserdata( L, -1 );
	lua_pop( L, 1 );
	return lp;
}

/* create a new channel */
static int luaproc_create_channel(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    channel chan;

    /* get exclusive access to operate on channels */
    pthread_mutex_lock(&mutex_channel);

    /* check if channel exists */
    if (eina_hash_find(channels, name) != NULL)
    {
	/* free access to operate on channels */
	pthread_mutex_unlock(&mutex_channel);
	/* return an error to lua */
	lua_pushnil(L);
	lua_pushstring(L, "channel already exists");
	return 2;
    }

    /* get exclusive access to the channel table */
    pthread_mutex_lock(&mutex_channel_lstate);

    /* create a new channel */
    chan = (channel) calloc(1, sizeof(struct stchannel));
    eina_clist_init(&(chan->send));
    eina_clist_init(&(chan->recv));
    chan->mutex  = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( chan->mutex, NULL );
    chan->in_use = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(chan->in_use, NULL);
    eina_hash_add(channels, name, chan);

    /* let others access the channel table */
    pthread_mutex_unlock(&mutex_channel_lstate);

    /* free access to operate on channels */
    pthread_mutex_unlock(&mutex_channel);

    lua_pushboolean(L, TRUE);
    return 1;
}

/* send a message to a lua process */
static int luaproc_send_back( lua_State *L ) {

	luaproc self;
	const char *message = luaL_checkstring( L, 1 );

	self = luaproc_getself( L );
	if (self && self->data)
	{
	    scriptMessage *sm = calloc(1, sizeof(scriptMessage));

	    if (sm)
	    {
		sm->script = self->data;
		strcpy((char *) sm->message, message);
		ecore_main_loop_thread_safe_call_async(scriptSendBack, sm);
	    }
	}

	return 0;
}

/* error messages for the sendToChannel function */
const char *sendToChannelErrors[] =
{
    "non-existent channel",
    "error scheduling process"
};

// TODO - If these come in too quick, then messages might get lost.  Also, in at least one case, it locked up this thread I think.

/* send a message to a lua process */
const char *sendToChannel(const char *chname, const char *message, luaproc *dst, channel *chn)
{
    const char *result = NULL;
    channel chan;
    luaproc dstlp;

    /* get exclusive access to operate on channels */
    pthread_mutex_lock(&mutex_channel);

    /* wait until channel is not in use */
    while( ((chan = eina_hash_find(channels, chname)) != NULL) && (pthread_mutex_trylock(chan->mutex) != 0 ))
    {
	pthread_cond_wait(chan->in_use, &mutex_channel);
    }

    /* free access to operate on channels */
    pthread_mutex_unlock(&mutex_channel);

    /* if channel is not found, return an error */
    if (chan == NULL)
	return sendToChannelErrors[0];

    /* try to find a matching receiver */
    dstlp = luaproc_dequeue_receiver(chan);

    /* if a match is found, send the message to it and (queue) wake it */
    if (dstlp != NULL)
    {
	/* push the message onto the receivers stack */
	lua_pushstring( dstlp->lstate, message);

	dstlp->args = lua_gettop(dstlp->lstate) - 1;

	if (sched_queue_proc(dstlp) != LUAPROC_SCHED_QUEUE_PROC_OK)
	{
	    /* unlock channel access */
	    luaproc_unlock_channel(chan);

	    /* decrease active luaproc count */
	    sched_lpcount_dec();

	    /* close lua_State */
	    lua_close(dstlp->lstate);
	    return sendToChannelErrors[1];
	}

	/* unlock channel access */
	luaproc_unlock_channel(chan);
    }
    else if (dst)
	dst = &dstlp;

    if (chn)
	chn = &chan;
    return result;
}

/* send a message to a lua process */
static int luaproc_send( lua_State *L ) {

	channel chan;
	luaproc dstlp, self;
	const char *chname = luaL_checkstring( L, 1 );
	const char *message = luaL_checkstring( L, 2 );
	const char *result = sendToChannel(chname, message, &dstlp, &chan);

	if (result) {
		lua_pushnil( L );
		lua_pushstring( L, result );
		return 2;
	}

	if ( dstlp == NULL ) {

		self = luaproc_getself( L );

		if ( self != NULL ) {
			self->status = LUAPROC_STAT_BLOCKED_SEND;
			self->chan = chan;
		}

		/* just yield the lua process, channel unlocking will be done by the scheduler */
		return lua_yield( L, lua_gettop( L ));
	}

	lua_pushboolean( L, TRUE );
	return 1;
}

/* receive a message from a lua process */
static int luaproc_receive( lua_State *L ) {

	channel chan;
	luaproc srclp, self;
	const char *chname = luaL_checkstring( L, 1 );

	/* get exclusive access to operate on channels */
	pthread_mutex_lock( &mutex_channel );

	/* wait until channel is not in use */
	while((( chan = eina_hash_find(channels, chname)) != NULL ) && ( pthread_mutex_trylock(chan->mutex) != 0 )) {
		pthread_cond_wait(chan->in_use, &mutex_channel );
	}

	/* free access to operate on channels */
	pthread_mutex_unlock( &mutex_channel );

	/* if channel is not found, free access to operate on channels and return an error to Lua */
	if ( chan == NULL ) {
		lua_pushnil( L );
		lua_pushstring( L, "non-existent channel" );
		return 2;
	}

	/* try to find a matching sender */
	srclp = luaproc_dequeue_sender( chan );

	/* if a match is found, get values from it and (queue) wake it */
	if ( srclp != NULL ) {

		/* move values between Lua states' stacks */
		luaproc_movevalues( srclp->lstate, L );

		/* return to sender indicanting message was sent */
		lua_pushboolean( srclp->lstate, TRUE );
		srclp->args = 1;

		if ( sched_queue_proc( srclp ) != LUAPROC_SCHED_QUEUE_PROC_OK ) {

			/* unlock channel access */
			luaproc_unlock_channel( chan );

			/* decrease active luaproc count */
			sched_lpcount_dec();

			/* close lua_State */
			lua_close( srclp->lstate );
			lua_pushnil( L );
			lua_pushstring( L, "error scheduling process" );
			return 2;
		}

		/* unlock channel access */
		luaproc_unlock_channel( chan );

		return lua_gettop( L ) - 1;
	}

	/* otherwise queue (block) the receiving process (sync) or return immediatly (async) */
	else {

		/* if trying an asynchronous receive, unlock channel access and return an error */
		if ( lua_toboolean( L, 2 )) {
			/* unlock channel access */
			luaproc_unlock_channel( chan );
			/* return an error */
			lua_pushnil( L );
			lua_pushfstring( L, "no senders waiting on channel %s", chname );
			return 2;
		}

		/* otherwise (synchronous receive) simply block process */
		else {
			self = luaproc_getself( L );

			if ( self != NULL ) {
				self->status = LUAPROC_STAT_BLOCKED_RECV;
				self->chan = chan;
			}

			/* just yield the lua process, channel unlocking will be done by the scheduler */
			return lua_yield( L, lua_gettop( L ));
		}
	}
}

void luaprocInit(void)
{
    eina_clist_init(&recyclelp);

    int tid;
    pthread_t worker;

    eina_clist_init(&lpready);
    channels = eina_hash_string_superfast_new(NULL);

    /* create initial worker threads */
    for (tid = 0; tid < LUAPROC_SCHED_DEFAULT_WORKER_THREADS; tid++)
    {
	pthread_create( &worker, NULL, workermain, NULL);
    }
}
