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
 * Use ecore threads instead of raw pthreads.
 *   Ecore threads pretty much wraps pthreads on posix, but has Windows support to.
 * In general use EFL where it is useful.
 * One fixed unique message channel per script.
 *   Probably one fixed unique message channel per object, which each script in the object shares.
 *     But might be better to handle that C side anyway.
 *   No need for channel.c / .h, we are not using that sort of arbitrary channels.
 *   FIFO queue on message channels, seems the C socket queue is not enough.
 *     On the other hand, could just peel messages of the socket queue, then shove them on the scripts queue.
 * Better integration with LuaSL.
 *   Merge the luaproc structure with the script structure.
 * Merge in the edje Lua code, and keep an eye on that, coz we might want to actually add this to edje Lua in the future.
 * Get rid of luaproc.lua, should not need it.
 * Use my coding standards, or EFL ones.  Pffft.
 *
 */

#include "LuaSL.h"

#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>


/*********
* globals
*********/

/* global channel lua_State mutex */
pthread_mutex_t mutex_channel_lstate = PTHREAD_MUTEX_INITIALIZER;

/* global lua_State where channel hash table will be stored */
lua_State *chanls = NULL;

/* message channel */
struct stchannel {
	Eina_Clist send;
	Eina_Clist recv;
	pthread_mutex_t *mutex;
	pthread_cond_t *in_use;
};



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

/* maximum lua processes to recycle */
int recyclemax = 0;

/* lua process */
struct stluaproc {
	Eina_Clist node;
	lua_State *lstate;
	int stat;
	int args;
	channel chan;
	int destroyworker;
	void *data;
	Ecore_Cb callback;
};



/******************************
* library functions prototypes
******************************/
/* create a new lua process */
static int luaproc_create_newproc( lua_State *L );
/* send a message to a lua process */
static int luaproc_send( lua_State *L );
/* receive a message from a lua process */
static int luaproc_receive( lua_State *L );
/* create a new channel */
static int luaproc_create_channel( lua_State *L );
/* destroy a channel */
static int luaproc_destroy_channel( lua_State *L );
/* wait until all luaprocs have finished and exit */
static int luaproc_exit( lua_State *L );
/* create a new worker */
static int luaproc_create_worker( lua_State *L );
/* destroy a worker */
static int luaproc_destroy_worker( lua_State *L );
/* set amount of lua processes that should be recycled (ie, reused) */
static int luaproc_recycle_set( lua_State *L );
/* send a message back to the main loop */
static int luaproc_send_back( lua_State *L );

/* luaproc function registration array - main (parent) functions */
static const struct luaL_reg luaproc_funcs_parent[] = {
	{ "newproc", luaproc_create_newproc },
	{ "exit", luaproc_exit },
	{ "createworker", luaproc_create_worker },
	{ "destroyworker", luaproc_destroy_worker },
	{ "recycle", luaproc_recycle_set },
	{ "sendback", luaproc_send_back },
	{ NULL, NULL }
};

/* luaproc function registration array - newproc (child) functions */
static const struct luaL_reg luaproc_funcs_child[] = {
	{ "newproc", luaproc_create_newproc },
	{ "send", luaproc_send },
	{ "receive", luaproc_receive },
	{ "newchannel", luaproc_create_channel },
	{ "delchannel", luaproc_destroy_channel },
	{ "createworker", luaproc_create_worker },
	{ "destroyworker", luaproc_destroy_worker },
	{ "recycle", luaproc_recycle_set },
	{ "sendback", luaproc_send_back },
	{ NULL, NULL }
};




/* initialize channel table */
void channel_init( void ) {
	chanls = luaL_newstate();
	lua_newtable( chanls );
	lua_setglobal( chanls, "channeltb" );
}

/* create new channel */
channel channel_create( const char *cname ) {

	channel chan;

	/* get exclusive access to the channel table */
	pthread_mutex_lock( &mutex_channel_lstate );

	/* create a new channel */
	lua_getglobal( chanls, "channeltb");
	lua_pushstring( chanls, cname );
	chan = (channel )lua_newuserdata( chanls, sizeof( struct stchannel ));
	eina_clist_init(&(chan->send));
	eina_clist_init(&(chan->recv));
	chan->mutex  = (pthread_mutex_t *)malloc( sizeof( pthread_mutex_t ));
	pthread_mutex_init( chan->mutex, NULL );
	chan->in_use = (pthread_cond_t *)malloc( sizeof( pthread_cond_t ));
	pthread_cond_init( chan->in_use, NULL );
	lua_settable( chanls, -3 );
	lua_pop( chanls, 1 );

	/* let others access the channel table */
	pthread_mutex_unlock( &mutex_channel_lstate );

	return chan;
}

/* destroy a channel */
int channel_destroy( channel chan, const char *chname ) {

	/* get exclusive access to the channel table */
	pthread_mutex_lock( &mutex_channel_lstate );

	lua_getglobal( chanls, "channeltb");
	lua_pushstring( chanls, chname );
	lua_pushnil( chanls );
	lua_settable( chanls, -3 );
	lua_pop( chanls, 1 );

	/* let others access the channel table */
	pthread_mutex_unlock( &mutex_channel_lstate );

	return CHANNEL_DESTROYED;
}

/* search for and return a channel with a given name */
channel channel_search( const char *cname ) {

	channel chan;

	/* get exclusive access to the channel table */
	pthread_mutex_lock( &mutex_channel_lstate );

	/* search for channel */
	lua_getglobal( chanls, "channeltb");
	lua_getfield( chanls, -1, cname );
	if (( lua_type( chanls, -1 )) == LUA_TUSERDATA ) {
		chan = (channel )lua_touserdata( chanls, -1 );
	} else {
		chan = NULL;
	}
	lua_pop( chanls, 2 );

	/* let others access channel table */
	pthread_mutex_unlock( &mutex_channel_lstate );

	return chan;
}

/* return a channel's send queue */
Eina_Clist *channel_get_sendq( channel chan ) {
	return &chan->send;
}

/* return a channel's receive queue */
Eina_Clist *channel_get_recvq( channel chan ) {
	return &chan->recv;
}

/* return a channel's mutex */
pthread_mutex_t *channel_get_mutex( channel chan ) {
	return chan->mutex;
}

/* return a channel's conditional variable */
pthread_cond_t *channel_get_cond( channel chan ) {
	return chan->in_use;
}




/* worker thread main function */
void *workermain( void *args ) {

	luaproc lp;
	int procstat;
	int destroyworker;

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
		procstat = lua_resume( luaproc_get_state( lp ), luaproc_get_args( lp ));

		/* reset the process argument count */
		luaproc_set_args( lp, 0 );

		/* check if process finished its whole execution */
		if ( procstat == 0 ) {

			/* check if worker thread should be destroyed */
			destroyworker = luaproc_get_destroyworker( lp );

			/* set process status to finished */
			luaproc_set_status( lp, LUAPROC_STAT_FINISHED );

			/* check if lua process should be recycled and, if not, destroy it */
			if ( luaproc_recycle_push( lp ) == FALSE ) {
				lua_close( luaproc_get_state( lp ));
			}

			/* decrease active lua process count */
			sched_lpcount_dec();

			/* check if thread should be finished after lua process conclusion */
			if ( destroyworker ) {
				/* if so, finish thread */
				pthread_exit( NULL );
			}
		}

		/* check if process yielded */
		else if ( procstat == LUA_YIELD ) {

			/* if so, further check if yield originated from an unmatched send/recv operation */
			if ( luaproc_get_status( lp ) == LUAPROC_STAT_BLOCKED_SEND ) {
				/* queue blocked lua process on corresponding channel */
				luaproc_queue_sender( lp );
				/* unlock channel access */
				luaproc_unlock_channel( luaproc_get_channel( lp ));
			}

			else if ( luaproc_get_status( lp ) == LUAPROC_STAT_BLOCKED_RECV ) {
				/* queue blocked lua process on corresponding channel */
				luaproc_queue_receiver( lp );
				/* unlock channel access */
				luaproc_unlock_channel( luaproc_get_channel( lp ));
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
			fprintf( stderr, "close lua_State (error: %s)\n", luaL_checkstring( luaproc_get_state( lp ), -1 ));
			/* close lua state */
			lua_close( luaproc_get_state( lp ));
			/* decrease active lua process count */
			sched_lpcount_dec();
		}
	}
}

/* local scheduler initialization */
int sched_init_local( int numworkers ) {

	int tid;
	int workercount = 0;
	pthread_t worker;

	/* initialize ready process list */
//	lpready = list_new();
	eina_clist_init(&lpready);

	/* initialize channels */
	channel_init();

	/* create initial worker threads */
	for ( tid = 0; tid < numworkers; tid++ ) {
		if ( pthread_create( &worker, NULL, workermain, NULL ) == 0 ) {
			workercount++;
		}
	}

	if ( workercount != numworkers ) {
		return LUAPROC_SCHED_INIT_ERROR;
	}

	return LUAPROC_SCHED_OK;
}

/* exit scheduler */
void sched_exit( void ) {

	/* get exclusive access to the ready process queue */
	pthread_mutex_lock( &mutex_queue_access );
	/* free access to the process ready queue */
	pthread_mutex_unlock( &mutex_queue_access );
}

/* move process to ready queue (ie, schedule process) */
int sched_queue_proc( luaproc lp ) {

	/* get exclusive access to the ready process queue */
	pthread_mutex_lock( &mutex_queue_access );

	/* add process to ready queue */
	eina_clist_add_tail(&lpready, &(lp->node));

	/* set process status to ready */
	luaproc_set_status( lp, LUAPROC_STAT_READY );

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

/* increase active lua process count */
void sched_lpcount_inc( void ) {
	pthread_mutex_lock( &mutex_lp_count );
	lpcount++;
	pthread_mutex_unlock( &mutex_lp_count );
}

/* decrease active lua process count */
void sched_lpcount_dec( void ) {
	pthread_mutex_lock( &mutex_lp_count );
	lpcount--;
	/* if count reaches zero, signal there are no more active processes */
	if ( lpcount == 0 ) {
		pthread_cond_signal( &cond_no_active_lp );
	}
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




/*
static void registerlib( lua_State *L, const char *name, lua_CFunction f ) {
	lua_getglobal( L, "package" );
	lua_getfield( L, -1, "preload" );
	lua_pushcfunction( L, f );
	lua_setfield( L, -2, name );
	lua_pop( L, 2 );
}
*/
static void openlibs( lua_State *L ) {
/*
	lua_cpcall( L, luaopen_base, NULL );
	lua_cpcall( L, luaopen_package, NULL );
	registerlib( L, "io", luaopen_io );
	registerlib( L, "os", luaopen_os );
	registerlib( L, "table", luaopen_table );
	registerlib( L, "string", luaopen_string );
	registerlib( L, "math", luaopen_math );
	registerlib( L, "debug", luaopen_debug );
*/
    luaL_openlibs(L);
}

/* return status (boolean) indicating if lua process should be recycled */
luaproc luaproc_recycle_pop( void ) {

	luaproc lp;

	/* get exclusive access to operate on recycle list */
	pthread_mutex_lock( &mutex_recycle_list );

	/* check if there are any lua processes on recycle list */
	if ( eina_clist_count( &recyclelp ) > 0 ) {
		/* pop list head */
		if ((lp = (luaproc) eina_clist_head(&recyclelp)))
		    eina_clist_remove(&(lp->node));
		/* free access to operate on recycle list */
		pthread_mutex_unlock( &mutex_recycle_list );
		/* return associated luaproc */
		return lp;
	}

	/* free access to operate on recycle list */
	pthread_mutex_unlock( &mutex_recycle_list );

	/* if no lua processes are available simply return null */
	return NULL;
}

/* check if lua process should be recycled and, in case so, add it to the recycle list */
int luaproc_recycle_push( luaproc lp ) {

	/* get exclusive access to operate on recycle list */
	pthread_mutex_lock( &mutex_recycle_list );

	/* check if amount of lua processes currently on recycle list is greater than
	   or equal to the maximum amount of lua processes that should be recycled */
	if ( eina_clist_count( &recyclelp ) >= recyclemax ) {
		/* free access to operate on recycle list */
		pthread_mutex_unlock( &mutex_recycle_list );
		/* if so, lua process should NOT be recycled and should be destroyed */
		return FALSE;
	}
	/* otherwise, lua process should be added to recycle list */
	eina_clist_add_tail( &recyclelp, &(lp->node) );
	/* free access to operate on recycle list */
	pthread_mutex_unlock( &mutex_recycle_list );
	/* since lua process will be recycled, it should not be destroyed */
	return TRUE;
}

/* create new luaproc */
static luaproc luaproc_new( const char *code, int destroyflag, int file) {

	luaproc lp;
	int ret;
	/* create new lua state */
	lua_State *lpst = luaL_newstate( );
	/* store the luaproc struct in its own lua state */
	lp = (luaproc )lua_newuserdata( lpst, sizeof( struct stluaproc ));
	lua_setfield( lpst, LUA_REGISTRYINDEX, "_SELF" );

	eina_clist_element_init(&(lp->node));
	lp->lstate = lpst;
	lp->stat = LUAPROC_STAT_IDLE;
	lp->args = 0;
	lp->chan = NULL;
	lp->destroyworker = destroyflag;

	/* load standard libraries */
	openlibs( lpst );

	/* register luaproc's own functions */
	luaL_register( lpst, "luaproc", luaproc_funcs_child );

	/* load process' code */
	if (file)
	    ret = luaL_loadfile( lpst, code );
	else
	    ret = luaL_loadstring( lpst, code );
	/* in case of errors, destroy recently created lua process */
	if ( ret != 0 ) {
		lua_close( lpst );
		return NULL;
	}

	/* return recently created lua process */
	return lp;
}

/* synchronize worker threads and exit */
static int luaproc_exit( lua_State *L ) {
	sched_join_workerthreads();
	return 0;
}

/* create a new worker pthread */
static int luaproc_create_worker( lua_State *L ) {

	if ( sched_create_worker( ) != LUAPROC_SCHED_OK ) {
		lua_pushnil( L );
		lua_pushstring( L, "error creating worker" );
		return 2;
	}

	lua_pushboolean( L, TRUE );
	return 1;
}

/* set amount of lua processes that should be recycled (ie, reused) */
static int luaproc_recycle_set( lua_State *L ) {

	luaproc lp;
	int max = luaL_checkint( L, 1 );

	/* check if function argument represents a reasonable value */
	if ( max < 0 ) {
		/* in case of errors return nil + error msg */
		lua_pushnil( L );
		lua_pushstring( L, "error setting recycle limit to negative value" );
		return 2;
	}

	/* get exclusive access to operate on recycle list */
	pthread_mutex_lock( &mutex_recycle_list );

	/* set maximum lua processes that should be recycled */
	recyclemax = max;

	/* destroy recycle list excessive nodes (and corresponding lua processes) */
	while ( eina_clist_count( &recyclelp ) > max ) {
		if ((lp = (luaproc) eina_clist_head(&recyclelp)))
		    eina_clist_remove(&(lp->node));
		/* close associated lua_State */
		lua_close( lp->lstate );
	}

	/* free access to operate on recycle list */
	pthread_mutex_unlock( &mutex_recycle_list );

	lua_pushboolean( L, TRUE );
	return 1;
}


/* destroy a worker pthread */
static int luaproc_destroy_worker( lua_State *L ) {

	/* new lua process pointer */
	luaproc lp;

	/* create new lua process with empty code and destroy worker flag set to true
	   (ie, conclusion of lua process WILL result in worker thread destruction */
	lp = luaproc_new( "", TRUE, FALSE );

	/* ensure process creation was successfull */
	if ( lp == NULL ) {
		/* in case of errors return nil + error msg */
		lua_pushnil( L );
		lua_pushstring( L, "error destroying worker" );
		return 2;
	}

	/* increase active luaproc count */
	sched_lpcount_inc();

	/* schedule luaproc */
	if ( sched_queue_proc( lp ) != LUAPROC_SCHED_QUEUE_PROC_OK ) {
		printf( "[luaproc] error queueing Lua process\n" );
		/* decrease active luaproc count */
		sched_lpcount_dec();
		/* close lua_State */
		lua_close( lp->lstate );
		/* return nil + error msg */
		lua_pushnil( L );
		lua_pushstring( L, "error destroying worker" );
		return 2;
	}

	lua_pushboolean( L, TRUE );
	return 1;
}

/* recycle a lua process */
static luaproc luaproc_recycle( luaproc lp, const char *code, int file ) {

	int ret;

	/* reset struct members */
	lp->stat = LUAPROC_STAT_IDLE;
	lp->args = 0;
	lp->chan = NULL;
	lp->destroyworker = FALSE;

	/* load process' code */
	ret = luaL_loadstring( lp->lstate, code );

	/* in case of errors, destroy lua process */
	if ( ret != 0 ) {
		lua_close( lp->lstate );
		return NULL;
	}

	/* return recycled lua process */
	return lp;
}


int newProc(const char *code, int file, Ecore_Cb callback, void *data)
{
	/* new lua process pointer */
	luaproc lp;

	/* check if existing lua process should be recycled to avoid new creation */
	lp = luaproc_recycle_pop( );

	/* if there is a lua process available on the recycle queue, recycle it */
	if ( lp != NULL ) {
		lp = luaproc_recycle( lp, code, file );
	}
	/* otherwise create a new one from scratch */
	else {
		/* create new lua process with destroy worker flag set to false
		   (ie, conclusion of lua process will NOT result in worker thread destruction */
		lp = luaproc_new( code, FALSE, file );
	}

	/* ensure process creation was successfull */
	if ( lp == NULL ) {
		return 1;
	}

	/* Stash any data and callback given to us. */
	lp->data = data;
	lp->callback = callback;

	/* increase active luaproc count */
	sched_lpcount_inc();

	/* schedule luaproc */
	if ( sched_queue_proc( lp ) != LUAPROC_SCHED_QUEUE_PROC_OK ) {
		printf( "[luaproc] error queueing Lua process\n" );
		/* decrease active luaproc count */
		sched_lpcount_dec();
		/* close lua_State */
		lua_close( lp->lstate );
		return 2;
	}

	return 0;
}

/* create and schedule a new lua process (luaproc.newproc) */
static int luaproc_create_newproc( lua_State *L ) {

	/* check if first argument is a string (lua code) */
	const char *code = luaL_checkstring( L, 1 );

	switch (newProc(code, FALSE, NULL, NULL))
	{
	    case 1 :
		/* in case of errors return nil + error msg */
		lua_pushnil( L );
		lua_pushstring( L, "error loading code string" );
		return 2;
	    case 2 :
		/* return nil + error msg */
		lua_pushnil( L );
		lua_pushstring( L, "error queuing process" );
		return 2;
	}

	lua_pushboolean( L, TRUE );
	return 1;
}

/* queue a lua process sending a message without a matching receiver */
void luaproc_queue_sender( luaproc lp ) {
	/* add the sending process to this process' send queue */
	eina_clist_add_tail( channel_get_sendq( lp->chan ), &(lp->node));
}

/* dequeue a lua process sending a message with a receiver match */
luaproc luaproc_dequeue_sender( channel chan ) {

	luaproc lp;

	if ( eina_clist_count( channel_get_sendq( chan )) > 0 ) {
		/* get first node from channel's send queue */
		if ((lp = (luaproc) eina_clist_head(channel_get_sendq( chan ))))
		    eina_clist_remove(&(lp->node));
		/* return associated luaproc */
		return lp;
	}

	return NULL;
}

/* queue a luc process receiving a message without a matching sender */
void luaproc_queue_receiver( luaproc lp ) {
	/* add the receiving process to this process' receive queue */
	eina_clist_add_tail( channel_get_recvq( lp->chan ), &(lp->node));
}

/* dequeue a lua process receiving a message with a sender match */
luaproc luaproc_dequeue_receiver( channel chan ) {

	luaproc lp;

	if ( eina_clist_count( channel_get_recvq( chan )) > 0 ) {
		/* get first node from channel's recv queue */
		if ((lp = (luaproc) eina_clist_head(channel_get_recvq( chan ))))
		    eina_clist_remove(&(lp->node));
		/* return associated luaproc */
		return lp;
	}

	return NULL;
}

/* moves values between lua states' stacks */
void luaproc_movevalues( lua_State *Lfrom, lua_State *Lto ) {

	int i;
	int n = lua_gettop( Lfrom );

	/* move values between lua states' stacks */
	for ( i = 2; i <= n; i++ ) {
		lua_pushstring( Lto, lua_tostring( Lfrom, i ));
	}
}

/* return the lua process associated with a given lua state */
luaproc luaproc_getself( lua_State *L ) {
	luaproc lp;
	lua_getfield( L, LUA_REGISTRYINDEX, "_SELF" );
	lp = (luaproc )lua_touserdata( L, -1 );
	lua_pop( L, 1 );
	return lp;
}

/* send a message to a lua process */
static int luaproc_send_back( lua_State *L ) {

	luaproc self;
	const char *message = luaL_checkstring( L, 1 );

	self = luaproc_getself( L );
	if (self && self->callback && self->data)
	{
	    scriptMessage *sm = calloc(1, sizeof(scriptMessage));

	    if (sm)
	    {
		sm->script = self->data;
		strcpy((char *) sm->message, message);
		ecore_main_loop_thread_safe_call_async(self->callback, sm);
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
    while( ((chan = channel_search(chname)) != NULL) && (pthread_mutex_trylock(channel_get_mutex(chan)) != 0 ))
    {
	pthread_cond_wait(channel_get_cond(chan), &mutex_channel);
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
			self->stat = LUAPROC_STAT_BLOCKED_SEND;
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
	while((( chan = channel_search( chname )) != NULL ) && ( pthread_mutex_trylock( channel_get_mutex( chan )) != 0 )) {
		pthread_cond_wait( channel_get_cond( chan ), &mutex_channel );
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
				self->stat = LUAPROC_STAT_BLOCKED_RECV;
				self->chan = chan;
			}

			/* just yield the lua process, channel unlocking will be done by the scheduler */
			return lua_yield( L, lua_gettop( L ));
		}
	}
}

void luaprocInit(void)
{
	/* initialize recycle list */
	eina_clist_init(&recyclelp);

	/* initialize local scheduler */
	sched_init_local( LUAPROC_SCHED_DEFAULT_WORKER_THREADS );
}

void luaprocRegister(lua_State *L)
{
	/* register luaproc functions */
	luaL_register( L, "luaproc", luaproc_funcs_parent );
}

LUALIB_API int luaopen_luaproc( lua_State *L ) {
	luaprocRegister(L);
	luaprocInit();
	return 0;
}

/* return a process' status */
int luaproc_get_status( luaproc lp ) {
	return lp->stat;
}

/* set a process' status */
void luaproc_set_status( luaproc lp, int status ) {
	lp->stat = status;
}

/* return a process' state */
lua_State *luaproc_get_state( luaproc lp ) {
	return lp->lstate;
}

/* return the number of arguments expected by a given process */
int luaproc_get_args( luaproc lp ) {
	return lp->args;
}

/* set the number of arguments expected by a given process */
void luaproc_set_args( luaproc lp, int n ) {
	lp->args = n;
}

/* create a new channel */
static int luaproc_create_channel( lua_State *L ) {

	const char *chname = luaL_checkstring( L, 1 );

	/* get exclusive access to operate on channels */
	pthread_mutex_lock( &mutex_channel );

	/* check if channel exists */
	if ( channel_search( chname ) != NULL ) {
		/* free access to operate on channels */
		pthread_mutex_unlock( &mutex_channel );
		/* return an error to lua */
		lua_pushnil( L );
		lua_pushstring( L, "channel already exists" );
		return 2;
	}

	channel_create( chname );

	/* free access to operate on channels */
	pthread_mutex_unlock( &mutex_channel );

	lua_pushboolean( L, TRUE );

	return 1;

}

/* destroy a channel */
static int luaproc_destroy_channel( lua_State *L ) {

	channel chan;
	luaproc lp;
	pthread_mutex_t *chmutex;
	pthread_cond_t *chcond;
	const char *chname = luaL_checkstring( L, 1 );


	/* get exclusive access to operate on channels */
	pthread_mutex_lock( &mutex_channel );

	/* wait until channel is not in use */
	while((( chan = channel_search( chname )) != NULL ) && ( pthread_mutex_trylock( channel_get_mutex( chan )) != 0 )) {
		pthread_cond_wait( channel_get_cond( chan ), &mutex_channel );
	}

	/* free access to operate on channels */
	pthread_mutex_unlock( &mutex_channel );

	/* if channel is not found, return an error to Lua */
	if ( chan == NULL ) {
		lua_pushnil( L );
		lua_pushstring( L, "non-existent channel" );
		return 2;
	}

	/* get channel's mutex and conditional pointers */
	chmutex = channel_get_mutex( chan );
	chcond  = channel_get_cond( chan );

	/* search for processes waiting to send a message on this channel */
	while (( lp = (luaproc) eina_clist_head( channel_get_sendq( chan ))) != NULL ) {
		eina_clist_remove(&(lp->node));

		/* return an error so the processe knows the channel was destroyed before the message was sent */
		lua_settop( lp->lstate, 0 );
		lua_pushnil( lp->lstate );
		lua_pushstring( lp->lstate, "channel destroyed while waiting for receiver" );
		lp->args = 2;

		/* schedule the process for execution */
		if ( sched_queue_proc( lp ) != LUAPROC_SCHED_QUEUE_PROC_OK ) {

			/* decrease active luaproc count */
			sched_lpcount_dec();

			/* close lua_State */
			lua_close( lp->lstate );
		}
	}

	/* search for processes waiting to receive a message on this channel */
	while (( lp = (luaproc) eina_clist_head( channel_get_recvq( chan ))) != NULL ) {
		eina_clist_remove(&(lp->node));

		/* return an error so the processe knows the channel was destroyed before the message was received */
		lua_settop( lp->lstate, 0 );
		lua_pushnil( lp->lstate );
		lua_pushstring( lp->lstate, "channel destroyed while waiting for sender" );
		lp->args = 2;

		/* schedule the process for execution */
		if ( sched_queue_proc( lp ) != LUAPROC_SCHED_QUEUE_PROC_OK ) {

			/* decrease active luaproc count */
			sched_lpcount_dec();

			/* close lua_State */
			lua_close( lp->lstate );
		}
	}

	/* get exclusive access to operate on channels */
	pthread_mutex_lock( &mutex_channel );
	/* destroy channel */
	channel_destroy( chan, chname );
	/* broadcast channel not in use */
	pthread_cond_broadcast( chcond );
	/* unlock channel access */
	pthread_mutex_unlock( chmutex );
	/* destroy channel mutex and conditional */
	pthread_mutex_destroy( chmutex );
	pthread_cond_destroy( chcond );
	/* free memory used by channel mutex and conditional */
	free( chmutex );
	free( chcond );
	/* free access to operate on channels */
	pthread_mutex_unlock( &mutex_channel );

	lua_pushboolean( L, TRUE );

	return 1;
}

/* register luaproc's functions in a lua_State */
void luaproc_register_funcs( lua_State *L ) {
	luaL_register( L, "luaproc", luaproc_funcs_child );
}

/* return the channel where the corresponding luaproc is blocked at */
channel luaproc_get_channel( luaproc lp ) {
	return lp->chan;
}

/* unlock access to a channel */
void luaproc_unlock_channel( channel chan ) {
	/* get exclusive access to operate on channels */
	pthread_mutex_lock( &mutex_channel );
	/* unlock channel access */
	pthread_mutex_unlock( channel_get_mutex( chan ));
	/* signal channel not in use */
	pthread_cond_signal( channel_get_cond( chan ));
	/* free access to operate on channels */
	pthread_mutex_unlock( &mutex_channel );
}

/* return status (boolean) indicating if worker thread should be destroyed after luaproc execution */
int luaproc_get_destroyworker( luaproc lp ) {
	return lp->destroyworker;
}
