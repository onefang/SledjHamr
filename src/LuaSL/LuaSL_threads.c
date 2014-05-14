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
 *   Probably one fixed unique message channel per object, which each script in the object shares.
 *     But might be better to handle that C side anyway.
 * Better integration with LuaSL.
 * Use ecore threads instead of raw pthreads.
 *   Ecore threads pretty much wraps pthreads on posix, but has Windows support to.
 * Merge in the edje Lua code, and keep an eye on that, coz we might want to actually add this to edje Lua in the future.
 * Use my coding standards, or EFL ones.  Pffft.
 *
 */

#include "LuaSL.h"


/* ready process queue insertion status */
#define LUAPROC_SCHED_QUEUE_PROC_OK	0
//#define LUAPROC_SCHED_QUEUE_PROC_ERR	-1

/* process is idle */
#define LUAPROC_STAT_IDLE		0
/* process is ready to run */
#define LUAPROC_STAT_READY		1
/* process is blocked on send */
#define LUAPROC_STAT_BLOCKED_SEND	2
/* process is blocked on receive */
#define LUAPROC_STAT_BLOCKED_RECV	3


typedef struct
{
    Eina_Clist node;
    lua_State *L;
} recycled;

/*********
* globals
*********/

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
	{ "sendback", luaproc_send_back },
	{ NULL, NULL }
};


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

	script *lp;
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
		if ((lp = (script *) eina_clist_head(&lpready)))
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
		procstat = lua_resume(lp->L, lp->args);
		/* reset the process argument count */
		lp->args = 0;

		/* check if process finished its whole execution, then recycle it */
		if (procstat == 0)
		{
//		    recycled *trash = malloc(sizeof(recycled));

		    // TODO - Trash stuff trashes memory, fix it.
		    //        Later, it's an optimization we don't need right now.
/*
		    if (trash)
		    {
			trash->L = lp->L;
			pthread_mutex_lock(&mutex_recycle_list);
			eina_clist_add_tail(&recyclelp, &(trash->node));
			pthread_mutex_unlock(&mutex_recycle_list);
		    }
*/
		    sched_lpcount_dec();
		    lua_close(lp->L);
		    if (lp->timer)
			ecore_timer_del(lp->timer);
//		    free(lp);
		}

		/* check if process yielded */
		else if ( procstat == LUA_YIELD ) {

			/* if so, further check if yield originated from an unmatched send/recv operation */
			if (lp->status == LUAPROC_STAT_BLOCKED_SEND)
			{
			}
			else if (lp->status == LUAPROC_STAT_BLOCKED_RECV)
			{
			}
			/* or if yield resulted from an explicit call to coroutine.yield in the lua code being executed */
			else
			{
				/* get exclusive access to the ready process queue */
				pthread_mutex_lock( &mutex_queue_access );
				/* re-insert the job at the end of the ready process queue */
				eina_clist_add_tail(&lpready, &(lp->node));
				/* free access to the process ready queue */
				pthread_mutex_unlock( &mutex_queue_access );
			}
		}
		/* check if there was any execution error (LUA_ERRRUN, LUA_ERRSYNTAX, LUA_ERRMEM or LUA_ERRERR) */
		else
		{
			/* print error message */
			fprintf( stderr, "close lua_State (error: %s)\n", luaL_checkstring(lp->L, -1 ));
			/* close lua state */
			lua_close(lp->L);
			/* decrease active lua process count */
			sched_lpcount_dec();
		}
	}
}

/* move process to ready queue (ie, schedule process) */
static int sched_queue_proc( script *lp ) {

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
int sched_create_worker(void)
{
    pthread_t worker;

    /* create a new pthread */
    if (pthread_create( &worker, NULL, workermain, NULL ) != 0)
	return LUAPROC_SCHED_PTHREAD_ERROR;
    return LUAPROC_SCHED_OK;
}

void newProc(const char *code, int file, script *lp)
{
    int ret;
//    recycled *trash;

    // Try to recycle a Lua state, otherwise create one from scratch.
#if 0	// TODO - something about this causes a crash.
    pthread_mutex_lock(&mutex_recycle_list);
    /* pop list head */
    if ((trash = (recycled *) eina_clist_head(&recyclelp)))
    {
	printf("  Reusing Lua trash.\n");
	eina_clist_remove(&(trash->node));
	lp->L = trash->L;
	free(trash);
    }
    pthread_mutex_unlock(&mutex_recycle_list);
#endif

    if (NULL == lp->L)
    {
	lp->L = luaL_newstate();

	luaL_openlibs(lp->L);
	luaL_register(lp->L, "luaproc", luaproc_funcs_child);
    }

    /* store the script struct in its own Lua state */
    lua_pushlightuserdata(lp->L, lp);
    lua_setfield(lp->L, LUA_REGISTRYINDEX, "_SELF");

    lp->status = LUAPROC_STAT_IDLE;
    lp->args = 0;
    eina_clist_element_init(&(lp->node));
    eina_clist_init(&(lp->messages));

    /* load process' code */
    if (file)
	ret = luaL_loadfile(lp->L, code);
    else
	ret = luaL_loadstring(lp->L, code);

    /* in case of errors, destroy Lua process */
    if (ret != 0)
    {
	lua_close(lp->L);
	lp->L = NULL;
    }

    if (lp->L)
    {
	sched_lpcount_inc();

	/* schedule luaproc */
	if (sched_queue_proc(lp) != LUAPROC_SCHED_QUEUE_PROC_OK)
	{
	    printf( "[luaproc] error queueing Lua process\n" );
	    sched_lpcount_dec();
	    lua_close(lp->L);
	}
    }
}

/* return the lua process associated with a given lua state */
static script *luaproc_getself(lua_State *L)
{
    script *lp;

    lua_getfield(L, LUA_REGISTRYINDEX, "_SELF");
    lp = (script *) lua_touserdata(L, -1);
    lua_pop(L, 1);
    return lp;
}

/* send a message to the client process */
static int luaproc_send_back(lua_State *L)
{
    script *self = luaproc_getself(L);
    const char *message = luaL_checkstring(L, 1);

    if (self)
    {
	scriptMessage *sm = calloc(1, sizeof(scriptMessage));

	if (sm)
	{
	    eina_clist_element_init(&(sm->node));
	    sm->script = self;
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

/* send a message to a lua process */
const char *sendToChannel(gameGlobals *ourGlobals, const char *SID, const char *message)
{
    const char *result = NULL;
    script *dstlp;

    if (!message)
    {
	PE("sendToChannel NULL message to %s", SID);
	return NULL;
    }
//    PD("sendToChannel message to %s -> %s", SID, message);

    /* get exclusive access to operate on channels */
    pthread_mutex_lock(&mutex_channel);

    // Add the message to the queue.
    if ((dstlp = eina_hash_find(ourGlobals->scripts, SID)))
    {
	scriptMessage *sm = NULL;

	if ((sm = malloc(sizeof(scriptMessage))))
	{
	    sm->script = dstlp;
	    strcpy((char *) sm->message, message);
	    eina_clist_add_tail(&(dstlp->messages), &(sm->node));
	}

	/* if it's already waiting, send the next message to it and (queue) wake it */
	if (dstlp->status == LUAPROC_STAT_BLOCKED_RECV)
	{
	    scriptMessage *msg = (scriptMessage *) eina_clist_head(&(dstlp->messages));

	    // See if there's a message on the queue.  Note, this may not be the same as the incoming message, if there was already a queue.
	    if (msg)
	    {
		eina_clist_remove(&(msg->node));
		message = msg->message;
	    }
	    /* push the message onto the receivers stack */
	    lua_pushstring(dstlp->L, message);
	    dstlp->args = lua_gettop(dstlp->L) - 1;
	    if (msg)
		free(msg);

	    if (sched_queue_proc(dstlp) != LUAPROC_SCHED_QUEUE_PROC_OK)
	    {
		sched_lpcount_dec();
		lua_close(dstlp->L);
		result = sendToChannelErrors[1];
	    }
	}
    }

    pthread_mutex_unlock(&mutex_channel);

    return result;
}

/* send a message to a lua process */
static int luaproc_send(lua_State *L)
{
    script *self = luaproc_getself(L);
    const char *result = sendToChannel(self->game, luaL_checkstring(L, 1), luaL_checkstring(L, 2));

    if (result)
    {
	lua_pushnil(L);
	lua_pushstring(L, result);
	return 2;
    }

    lua_pushboolean(L, TRUE);
    return 1;
}

/* receive a message from a lua process */
static int luaproc_receive(lua_State *L)
{
    script *self;
    const char *chname = luaL_checkstring(L, 1);
    scriptMessage *msg;

    // First check if there are queued messages, and grab one.
    self = luaproc_getself(L);
    if ((msg = (scriptMessage *) eina_clist_head(&(self->messages))))
    {
	eina_clist_remove(&(msg->node));
	lua_pushstring(L, msg->message);
	free(msg);
	return lua_gettop(L) - 1;
    }

    /* if trying an asynchronous receive, return an error */
    if ( lua_toboolean( L, 2 ))
    {
	lua_pushnil(L);
	lua_pushfstring(L, "no senders waiting on channel %s", chname);
	return 2;
    }
    /* otherwise (synchronous receive) simply block process */
    self->status = LUAPROC_STAT_BLOCKED_RECV;
    return lua_yield(L, lua_gettop(L));
}

void luaprocInit(void)
{
    eina_clist_init(&recyclelp);
    eina_clist_init(&lpready);
}
