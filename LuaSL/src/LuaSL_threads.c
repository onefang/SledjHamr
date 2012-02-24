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
 * Use eina lists instead of the rolled your own lists.
 *   Looks like a FIFO double linked list.
 * Use ecore threads instead of raw pthreads.
 *   Ecore threads pretty much wraps pthreads on posix, but has Windows support to.
 * In general use EFL where it is useful.
 * One fixed unique message channel per script.
 *   Probably one fixed unique message channel per object, which each script in the object shares.
 *   But might be better to handle that C side anyway.
 * FIFO queue on message channels, seems the C socket queue is not enough.
 *   On the other hand, could just peel messages the socket queue, then shove them on the scripts queue.
 * Better integration with LuaSL.
 *   Merge the luaproc structure with the script structure.
 * Merge in the edje Lua code, and keep an eye on that, coz we might want to actually add this to edje Lua in the future.
 * Use my coding standards, or EFL ones.  Pffft.
 *
 */
