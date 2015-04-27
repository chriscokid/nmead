/*
* msgbuffer.c
*
* NMEA Server Application
*
* Buffer management functions for passing text messages between threads.
*
* This replaces the NMEA Server's previous usage of struct msgbuf and
* related functions, which have had portability problems from OS to OS.
*
* Copyright 2002-2004 Charles L. Taylor.
*
* 26 Jul 2004: Includes changes by Carl Worth.
*
*/

/*
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
 *          		    NO WARRANTY
 *
 *    BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO
 *  WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE
 *  LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT
 *  HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT
 *  NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *  FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE
 *  QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
 *  PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY
 *  SERVICING, REPAIR OR CORRECTION.
 *
 *    IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN
 *  WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY
 *  MODIFY AND/OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE
 *  LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL,
 *  INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR
 *  INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF
 *  DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU
 *  OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY
 *  OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msgbuffer.h"


extern int verbose;


/*
* newmsgbuffer
*
* Allocates and initializes a new msgbuffer structure.
*
* Parameters:
*     None.
*
* Return Value:
*     The function returns an initialized msgbuffer structure.
*
* Remarks:
*
*/
msgbuffer * newmsgbuffer (void)
{
    msgbuffer * b = calloc (1, sizeof (msgbuffer));
    if (b == NULL)
        return NULL;

    b->readindex = 0;
    b->writeindex = 0;
    sem_init (&b->semaccess, 0, 1);

    return b;
}




/*
* destroymsgbuffer
*
* Destroys a msgbuffer structure.
*
* Parameters:
*     buf : msgbuffer * : A pointer to the structure to be destroyed.
*
* Return Value:
*     The function does not return a value.
*
* Remarks:
*
*/
void destroymsgbuffer (msgbuffer * buf)
{
    sem_destroy (&buf->semaccess);
    free (buf);
    return;
}




/*
* putmsg
*
* Stores a message in the msgbuffer structure.
*
* Parameters:
*     buf : msgbuffer * : A pointer to the structure to be given the message.
*     msg : char *      : A pointer to the message to be stored.
*
* Return Value:
*     The function returns zero if successful, nonzero if not.
*
* Remarks:
*
*/
int putmsg (msgbuffer * buf, const char * msg)
{
    int nextwriteindex;
    int result = 0;

    sem_wait (&buf->semaccess);

    nextwriteindex = (++buf->writeindex % MSGBUFFERELEMENTS);
    if (nextwriteindex == buf->readindex) {
        if (verbose >= 100) {
            printf ("Cannot add message; buffer is full\n");
        }
        result = -1;
    }
    else {
        if (verbose >= 100) {
            printf ("Adding message\n");
        }
        strncpy (buf->element[nextwriteindex], msg, MSGELEMENTLENGTH - 1);
        buf->writeindex = nextwriteindex;
    }

    sem_post (&buf->semaccess);

    return result;
}




/*
* getmsg
*
* Retrieves the next message from the msgbuffer structure.
*
* Parameters:
*     buf    : msgbuffer * : A pointer to the structure from which the message
*                            is to be retrieved.
*     msg    : char *      : A pointer to a memory location into which the
*                            retrieved message is to be stored.
*     length : int         : Number of available bytes in the memory location.
*
* Return Value:
*     The function returns zero if successful, nonzero if not.
*
* Remarks:
*
*/
int getmsg (msgbuffer * buf, char * msg, int length)
{
    int result = 0;

    sem_wait (&buf->semaccess);

    if (buf->readindex == buf->writeindex) {
        if (verbose >= 100) {
            printf ("Cannot read message; buffer is empty\n");
        }
        result = -1;
    }
    else {
        if (verbose >= 100) {
            printf ("Reading message\n");
        }
        strncpy (msg, buf->element[buf->readindex], length);
        buf->readindex = (++buf->readindex % MSGBUFFERELEMENTS);
    }

    sem_post (&buf->semaccess);

    return result;
}



