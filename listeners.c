/*
* listeners.c
*
* NMEA Server Application
*
*
* Copyright 2000-2004 Charles L. Taylor.
*
* 26 Jun 2004: Includes changes by Jon Miner to eliminate the need to wait
*              for the operating system to release the socket when restarting
*              the server.
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
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "nmead.h"


/* Local structure definitions */
union sock {
    struct sockaddr s;
    struct sockaddr_in i;
};


typedef struct {
    connection_t * conn;
    connectionmgr_t * cmgr;
    int socketfd;
} listenerinfo_t;


extern int verbose;
extern int port;


/* Forward reference */
void * listenproc (void * arg);



/*
* multilisten
*
* Await and accept connections from listener applications.
*
* Parameters:
*     cmgr : pointer to connectionmgr_t : A pointer to the connection manager
*                                         object shared between the talker
*                                         and listener threads.
*
* Return Value:
*     The function does not return a value.
*
* Remarks:
*
*/
void multilisten (connectionmgr_t * cmgr)
{
    union sock sock, work, peer;
    int wsd, sd;
    socklen_t addlen, peerlen;
    listenerinfo_t * listenerinfo;
    pthread_t * listener;
    pthread_attr_t attr;
    int threadresult;
    int so_reuse = 1;

    time_t now;

    char buff[BUFSIZ];

    int rv;

    signal (SIGPIPE, SIG_IGN);    /* Watch return codes for pipe signal */

    rv = pthread_attr_init(&attr);
    if (rv != 0) {
        perror ("attr_init");
        exit (1);
    }

    rv = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (rv != 0) {
        perror ("attr_set");
        exit (1);
    }

    sd = socket (AF_INET,SOCK_STREAM,0);
    if (sd == -1) {
        perror ("socket");
        exit (1);
    }
    bzero ((char *) &sock, sizeof (sock));
    sock.i.sin_family = AF_INET;
    sock.i.sin_port = htons (port);
    sock.i.sin_addr.s_addr = htonl (INADDR_ANY);
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*)&so_reuse,sizeof(so_reuse));
    rv = bind (sd, &(sock.s), sizeof (struct sockaddr));
    if (rv == -1) {
        perror ("bind");
        exit (1);
    }
    rv = listen (sd,2);
    if (rv == -1) {
        perror ("listen");
        exit (1);
    }

    do {          
       addlen=sizeof(work.s);
       memset(&work.s,0,addlen);
       wsd = accept (sd, &(work.s), &addlen);
       if (wsd == -1 ) {
          perror("accept");
          exit(1);
        }
        peerlen = sizeof (struct sockaddr);
        getpeername (wsd, &(peer.s), &peerlen);
        time (&now);
        if (verbose >= 10)
            printf ("Connection from %s at %s",
                 inet_ntoa (peer.i.sin_addr),
                 ctime (&now));

        /* Note that the listener thread proc will delete this memory
           unless an error occurs before the thread is started */
        listenerinfo = (listenerinfo_t *) calloc (1, sizeof (listenerinfo_t));
        listenerinfo->conn = newconnection ();
        if (addconnection (cmgr, listenerinfo->conn) != 0) {
            destroyconnection (listenerinfo->conn);
            free (listenerinfo);
            sprintf (buff, "*** Too many connections\r\n");
            write (wsd, buff, strlen (buff));
            close (wsd);
            break;
        }
        listenerinfo->cmgr = cmgr;
        listenerinfo->socketfd = wsd;

        listener = (pthread_t *) malloc (sizeof (pthread_t));
        threadresult = pthread_create (listener, &attr, listenproc, (void *) listenerinfo);

    } while (1);
}




/*
* listenproc_cleanup
*
* Cleanup handler for the listener process.
*
* Parameters:
*     arg : pointer : A pointer to a listenerinfo_t structure containing
*                     information required by the thread.
*
* Return Value:
*     The function returns NULL.
*
* Remarks:
*     This routine was originally intended to work with the pthread_cleanup_push
*     and pthread_cleanup_pop mechanism, but support for this mechanism isn't
*     widely supported in the environments in which this code was tested.
*     Therefore this routine is called explicitly by listenproc.
*
*/
static void * listenproc_cleanup (void * arg)
{
    listenerinfo_t * myli = (listenerinfo_t *) arg;

    if (verbose >= 10)
        printf ("listenproc_cleanup: shutting down connection\n");

    removeconnection (myli->cmgr, myli->conn);
    destroyconnection (myli->conn);
    close (myli->socketfd);
    free (myli);

    return NULL;
}




/*
* listenproc
*
* Handles a connection with a listener application.
*
* Parameters:
*     arg : pointer : A pointer to a listenerinfo_t structure containing
*                     information required by the thread.
*
* Return Value:
*     The function returns NULL.
*
* Remarks:
*
*/
void * listenproc (void * arg)
{
    char buff[MSGELEMENTLENGTH];
    listenerinfo_t * li = (listenerinfo_t *) arg;
    listenerinfo_t * myli
        = (listenerinfo_t *) calloc (1, sizeof (listenerinfo_t));
    int byteswritten;
    int result;

    if ((li == NULL) || (myli == NULL)) return NULL;

    myli->conn = li->conn;
    myli->cmgr = li->cmgr;
    myli->socketfd = li->socketfd;

    /* Destroy the object passed from the parent thread */
    free (li);

    /* Read and send data as long as the connection is open */
    do {
        if (verbose >= 100)
            printf ("listenproc: reading from buffer %p\n",
                myli->conn->msgbuffer);

        result = getmsg (myli->conn->msgbuffer, buff, MSGELEMENTLENGTH);
        if (result != 0) {
            if (verbose >= 100)
                printf ("listenproc: nonzero result for buffer %p\n",
                    myli->conn->msgbuffer);
            usleep (10);
            continue;
        }
        byteswritten = write (myli->socketfd, buff, strlen (buff));
        if (byteswritten < 0) {
            /* The connection is closed.  Remove the connection
               object and drop this socket. */
            if (verbose >= 100)
                printf ("listenproc: connection closed\n");
            break;
        }

    } while (1);

    listenproc_cleanup ((void *) myli);

    return NULL;
}

