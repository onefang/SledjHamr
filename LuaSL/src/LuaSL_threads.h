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

/* scheduler function return constants */
#define	LUAPROC_SCHED_OK		0
#define LUAPROC_SCHED_SOCKET_ERROR	-1
#define LUAPROC_SCHED_SETSOCKOPT_ERROR	-2
#define LUAPROC_SCHED_BIND_ERROR	-3
#define LUAPROC_SCHED_LISTEN_ERROR	-4
#define LUAPROC_SCHED_FORK_ERROR	-5
#define LUAPROC_SCHED_PTHREAD_ERROR	-6
#define LUAPROC_SCHED_INIT_ERROR	-7


/* message channel pointer type */
typedef struct stchannel *channel;

/* lua process pointer type */
typedef struct stluaproc *luaproc;


void luaprocInit(void);

/* create a new worker pthread */
int sched_create_worker( void );

int newProc(const char *code, int file, void *data);
const char *sendToChannel(const char *chname, const char *message, luaproc *dst, channel *chn);

/* join all worker threads and exit */
void sched_join_workerthreads( void );

#endif
