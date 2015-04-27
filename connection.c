/*
* connection.c
*
* NMEA Server Application
*
*
* Copyright 2000-2004 Charles L. Taylor.
*
* 26 Jun 2004: Includes FreeBSD compatibility changes by Martin Mersberger.
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

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "nmead.h"


extern int verbose;


/*
* newconnection
*
* Creates and initializes a new connection_t object.
*
* Parameters:
*     None.
*
* Return Value:
*     The function returns a pointer to a new connection_t object, or
*     NULL if an error occurred.
*
* Remarks:
*
*/
connection_t * newconnection (void)
{
    key_t qkey;
    connection_t * c
        = (connection_t *) calloc (1, sizeof (connection_t));

    if (c == NULL) return NULL;

    /* Use the address of the newly-allocated memory block as the
       key ID. */
    qkey = ftok (".", (int) c);

    c->msgbuffer = newmsgbuffer ();
    if (c->msgbuffer == NULL) {
        if (verbose >= 10)
            printf ("Failed to create message buffer for id = 0x%08lx\n",
                (unsigned long) c);
        free (c);
        return NULL;
    }
    if (verbose >= 100)
        printf ("Created message buffer for id = 0x%08lx\n",
            (unsigned long) c);

    return c;
}




/*
* destroyconnection
*
* Destroys a connection object.
*
* Parameters:
*     conn : pointer to connection_t : A pointer to a connection object.
*
* Return Value:
*     The function does not return a value.
*
*/
void destroyconnection (connection_t * conn)
{
    destroymsgbuffer (conn->msgbuffer);

    free (conn);

    return;
}




/*
* newconnectionmgr
*
* Creates and initializes a new connectionmgr_t object.
*
* Parameters:
*     None.
*
* Return Value:
*     The function returns a pointer to a connectionmgr_t object, or
*     NULL if the function fails.
*
* Remarks:
*
*/
connectionmgr_t * newconnectionmgr (void)
{
    connectionmgr_t * c
        = (connectionmgr_t *) calloc (1, sizeof (connectionmgr_t));

    if (c == NULL) return NULL;

    sem_init (&c->semaccess, 0, 1);

    return c;
}




/*
* destroyconnectionmgr
*
* Destroys an existing connectionmgr_t object.
*
* Parameters:
*     cmgr : pointer to connectionmgr_t : A pointer to the object to
*                                         be destroyed.
*
* Return Value:
*     The function does not return a value.
*
* Remarks:
*     The function does not destroy any connection objects.  This is
*     the responsibility of the listener threads.
*
*/
void destroyconnectionmgr (connectionmgr_t * cmgr)
{
    sem_destroy (&cmgr->semaccess);

    free (cmgr);

    return;
}




/*
* addconnection
*
* Adds a connection_t object to a connectionmgr_t object.
*
* Parameters:
*     cmgr : pointer to connectionmgr_t : A pointer to the object to
*                                         contain the connection_t object.
*     conn : pointer to connection_t    : A pointer to the connection
*                                         object to be added.
*
* Return Value:
*     The function returns zero if successful.  In the event of an error,
*     the function returns an appropriate error code.
*
* Remarks:
*
*/
int addconnection (connectionmgr_t * cmgr, connection_t * conn)
{
    int result = 0;

    if ((cmgr == NULL) || (conn == NULL)) return ADDCONNECTION_ERROR;

    sem_wait (&cmgr->semaccess);

    if (cmgr->nconn >= MAXCONNECTIONS)
        result = TOO_MANY_CONNECTIONS;
    else {
        conn->next = cmgr->head;
        cmgr->head = conn;
        cmgr->nconn++;
    }

    sem_post (&cmgr->semaccess);
    return result;

}




/*
* removeconnection
*
* Removes a connection_t object from a connectionmgr_t object.
*
* Parameters:
*     cmgr : pointer to connectionmgr_t : A pointer to the object
*                                         containing the connection_t object.
*     conn : pointer to connection_t    : A pointer to the connection
*                                         object to be removed.
*
* Return Value:
*     The function returns zero if successful.  In the event of an error,
*     the function returns an appropriate error code.
*
* Remarks:
*
*/
int removeconnection (connectionmgr_t * cmgr, connection_t * conn)
{
    int result = 0;
    connection_t * c, * c0;

    if ((cmgr == NULL) || (conn == NULL)) return ADDCONNECTION_ERROR;

    sem_wait (&cmgr->semaccess);

    if (cmgr->nconn > 0) {

        for (c = cmgr->head, c0 = NULL; c != NULL; c0 = c, c = c->next)
            if (c == conn)
                break;

        if (c != NULL) {

            if (c == cmgr->head)
                cmgr->head = c->next;
            else
                c0->next = c->next;
            cmgr->nconn--;

        }
    }

    sem_post (&cmgr->semaccess);
    return result;
}




/*
* writetoconnections
*
* Writes a string to the message queue for each connection in the
* connectionmgr_t object.
*
* Parameters:
*     cmgr : pointer to connectionmgr_t : A pointer to the connection manager.
*     buf  : pointer to character       : The string to be disseminated.
*
* Return Value:
*
* Remarks:
*
*/
int writetoconnections (connectionmgr_t * cmgr, const char * buf)
{
    connection_t * c;

    sem_wait (&cmgr->semaccess);

    if (cmgr->nconn > 0) {

        for (c = cmgr->head; c != NULL; c = c->next) {
            if (putmsg (c->msgbuffer, buf) != 0) {
                if (verbose >= 100)
                    printf (
                      "writetoconnections: dropped message to buffer %p\n",
                       c->msgbuffer);
            }
        }

    }

    sem_post (&cmgr->semaccess);
    return 0;
}



