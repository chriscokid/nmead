/*
* nmead.h
*
* NMEA Server Application
*
*
* Copyright 2000-2004 Charles L. Taylor.
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

#ifndef NMEAD_H
#define NMEAD_H

#include <stdio.h>
#include <semaphore.h>
#include "msgbuffer.h"


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Size of a character buffer.  Used in several locations throughout the
   program. */
#define BUFSZ     (1024)


/* Connection structure definitions.
*
*  Each listener thread has an associated connection structure, which is
*  a wrapper around a message queue and a pointer to allow the connection
*  structure to be part of a linked list.
*
*  Each listener has unrestricted access to its corresponding
*  connection structure so that it can access the message queue at its
*  own speed.  The listener is responsible for creating and destroying
*  its connection structure.
*
*  The talker accesses the connection structures only through the parent
*  connections manager, which provides synchronized access for adding and
*  removing connection structures (by listeners) as well as distributing
*  NMEA sentences (by the talker) to each active connection.
*/
typedef struct connection_struct {
    msgbuffer * msgbuffer;
    struct connection_struct * next;
} connection_t;



/* Connection manager structure definitions.
*
*  The connection manager provides synchronized access to the set of
*  active connection structures.  This synchronized access allows
*  connection structures to be added and removed by listener threads,
*  and it allows the talker thread to distribute data to all of the
*  listeners.
*/
#define MAXCONNECTIONS  20

#define TOO_MANY_CONNECTIONS  -2
#define ADDCONNECTION_ERROR   -1

typedef struct {
    connection_t * head;
    int nconn;
    int nextqnum;
    sem_t semaccess;
} connectionmgr_t;



/* Talker info structure.
*
*  This structure is passed to the talker thread and includes the
*  opened serial port device as well as the connection manager
*  structure for disseminating NMEA sentences.
*/
typedef struct {

    FILE * fp;
    connectionmgr_t * cmgr;
    int zip;
    int tickinterval;

}  talkerinfo_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Connection struct creation and destruction */
connection_t * newconnection (void);
void destroyconnection (connection_t * conn);


/* Connection manager creation, destruction, and access */
connectionmgr_t * newconnectionmgr (void);
void destroyconnectionmgr (connectionmgr_t * cmgr);
int addconnection (connectionmgr_t * cmgr, connection_t * conn);
int removeconnection (connectionmgr_t * cmgr, connection_t * conn);
int writetoconnections (connectionmgr_t * cmgr, const char * buffer);


void * talk (void * arg);
void multilisten (connectionmgr_t * ti);


#ifdef __cplusplus
}
#endif


#endif  /* NMEAD_H */


