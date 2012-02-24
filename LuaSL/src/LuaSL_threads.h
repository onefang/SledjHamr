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

#ifndef __LUASL_THREADS_H__
#define __LUASL_THREADS_H__

//#include <Ecore.h>


#define CHANNEL_MAX_NAME_LENGTH 255

#define CHANNEL_DESTROYED 0

/* message channel pointer type */
typedef struct stchannel *channel;



/* scheduler function return constants */
#define	LUAPROC_SCHED_OK						0
#define LUAPROC_SCHED_SOCKET_ERROR				-1
#define LUAPROC_SCHED_SETSOCKOPT_ERROR			-2
#define LUAPROC_SCHED_BIND_ERROR				-3
#define LUAPROC_SCHED_LISTEN_ERROR				-4
#define LUAPROC_SCHED_FORK_ERROR				-5
#define LUAPROC_SCHED_PTHREAD_ERROR				-6
#define LUAPROC_SCHED_INIT_ERROR				-7

/* ready process queue insertion status */
#define LUAPROC_SCHED_QUEUE_PROC_OK		0
#define LUAPROC_SCHED_QUEUE_PROC_ERR	-1

/* scheduler listener service default hostname and port */
#define LUAPROC_SCHED_DEFAULT_HOST "127.0.0.1"
#define LUAPROC_SCHED_DEFAULT_PORT 3133

/* scheduler default number of worker threads */
#define LUAPROC_SCHED_DEFAULT_WORKER_THREADS	1



/* process is idle */
#define LUAPROC_STAT_IDLE			0
/* process is ready to run */
#define LUAPROC_STAT_READY			1
/* process is blocked on send */
#define LUAPROC_STAT_BLOCKED_SEND	2
/* process is blocked on receive */
#define LUAPROC_STAT_BLOCKED_RECV	3
/* process is finished */
#define LUAPROC_STAT_FINISHED		4

/* lua process pointer type */
typedef struct stluaproc *luaproc;





/* initialize channels */
void channel_init( void );

/* create new channel */
channel channel_create( const char *cname );

/* destroy a channel */
int channel_destroy( channel chan, const char *chname );

/* search for and return a channel with a given name */
channel channel_search( const char *cname );

/* return a channel's send queue */
Eina_Clist *channel_get_sendq( channel chan );

/* return a channel's receive queue */
Eina_Clist *channel_get_recvq( channel chan );

/* return a channel's mutex */
pthread_mutex_t *channel_get_mutex( channel chan );

/* return a channel's conditional variable */
pthread_cond_t *channel_get_cond( channel chan );






/* initialize local scheduler */
int sched_init_local( int numworkers );

/* initialize socket enabled scheduler */
int sched_init_socket( int numworkers, const char *host, int port );

/* exit scheduler */
void sched_exit( void );

/* move process to ready queue (ie, schedule process) */
int sched_queue_proc( luaproc lp );

/* join all worker threads and exit */
void sched_join_workerthreads( void );

/* increase active luaproc count */
void sched_lpcount_inc( void );

/* decrease active luaproc count */
void sched_lpcount_dec( void );

/* create a new worker pthread */
int sched_create_worker( void );






void luaprocInit(void);
void luaprocRegister(lua_State *L);
int newProc(const char *code, int file, Ecore_Cb callback, void *data);


/* return a process' status */
int luaproc_get_status( luaproc lp );

/* set a process' status */
void luaproc_set_status( luaproc lp, int status );

/* return a process' state */
lua_State *luaproc_get_state( luaproc lp );

/* return the number of arguments expected by a given a process */
int luaproc_get_args( luaproc lp );

/* set the number of arguments expected by a given process */
void luaproc_set_args( luaproc lp, int n );

/* create luaproc (from scheduler) */
luaproc luaproc_create_sched( char *code );

/* register luaproc's functions in a lua_State */
void luaproc_register_funcs( lua_State *L );

/* allow registering of luaproc's functions in c main prog */
void luaproc_register_lib( lua_State *L );

/* queue a luaproc that tried to send a message */
void luaproc_queue_sender( luaproc lp );

const char *sendToChannel(const char *chname, const char *message, luaproc *dst, channel *chn);

/* queue a luaproc that tried to receive a message */
void luaproc_queue_receiver( luaproc lp );

/* unlock a channel's access */
void luaproc_unlock_channel( channel chan );

/* return a luaproc's channel */
channel luaproc_get_channel( luaproc lp );

/* return status (boolean) indicating if worker thread should be destroyed after luaproc execution */
int luaproc_get_destroyworker( luaproc lp );

/* return status (boolean) indicating if lua process should be recycled */
luaproc luaproc_recycle_pop( void );

/* add a lua process to the recycle list */
int luaproc_recycle_push( luaproc lp );


#endif
