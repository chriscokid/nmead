/*
* main.c
*
* NMEA Server Application
*
*
* Copyright 2000-2004 Charles L. Taylor.
* This file was derived from work published under GPL by Wolfgang Rupprecht
* (salog.c).
* Copyright 1999 Wolfgang Rupprecht.
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

#if 0
#include "config.h"
#else
#define PACKAGE  "nmead"
#define VERSION  "developmental version 01.03.00"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <getopt.h>
#include "nmead.h"


int verbose = 0;
int port = 1155;
long ttybaud = 4800;
char * ttyport = "/dev/gps";


/* Forward references */
int openserial (u_char * tty, int blocksz, long ttybaud);
void usage (void);
void * terminate (int);



/*
* main
*
* Main entry point for the program.  Processes command-line arguments and
* prepares for action.
*
* Parameters:
*     argc : integer     : Number of command-line arguments.
*     argv : array of    : Command-line arguments as an array of zero-terminated
*            pointers to   strings.
*            char
*
* Return Value:
*     The function does not return a value (although its return type is int).
*
* Remarks:
*
*/
int main (int argc, char ** argv)
{
    pthread_t    * talker;
    talkerinfo_t   talkerinfo;
    u_char       * ttyin = ttyport,
                   buf[BUFSZ];
    char         * logfilepath = NULL;
    int            c,
                   gpsfd = -1;
    int            threadresult;
    int            talkerretval;


    while ((c = getopt (argc, argv, "hi:b:v:p:")) != EOF) {
        switch (c) {
        case 'i':		/* gps serial ttyin */
            ttyin = optarg;
            break;

        case 'b':		/* serial ttyin speed */
            ttybaud = atol (optarg);
            break;

        case 'v':		/* verbose */
            verbose = atoi (optarg);
            break;

        case 'p':		/* listener port */
            port = atoi (optarg);
            break;

        case 'h':
        default:
            usage ();
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc > 0) {
        fprintf (stderr, "Extra args on command line.\n");
        for (; argc > 0; argc--) {
            fprintf (stderr, " %s", *argv++);
        }
        fprintf (stderr, "\n");
        usage ();		/* never returns */
    }

    if (verbose >= 1)
        fprintf (stderr, "%s %s\n", PACKAGE, VERSION);

    gpsfd = openserial (ttyin, 1, ttybaud);
    read (gpsfd, buf, BUFSZ);        /* flush */

    talkerinfo.fp = fdopen (gpsfd, "r");
    if (!talkerinfo.fp) {
        perror ("fdopen gps");
        exit (1);
    }
    talkerinfo.tickinterval = 0;

    talkerinfo.cmgr = newconnectionmgr ();

    talker = (pthread_t *) malloc (sizeof (pthread_t));

    threadresult = pthread_create (talker, NULL, talk,
        (void *) &talkerinfo);

    multilisten (talkerinfo.cmgr);

    pthread_join (*talker, (void **) &talkerretval);

    exit(0);
}



/*
* openserial
*
* Open the serial port with the given device name and configure it for
* reading NMEA data from a GPS receiver.
*
* Parameters:
*     tty     : pointer to    : A zero-terminated string containing the device
*               unsigned char   name of the appropriate serial port.
*     blocksz : integer       : Block size for port I/O
*     ttybaud : long          : Baud rate for port I/O
*
* Return Value:
*     The function returns a file descriptor for the opened port if successful.
*     The function returns -1 in the event of an error.
*
* Remarks:
*
*/
int openserial (u_char * tty, int blocksz, long ttybaud)
{
    int             fd;
    struct termios  termios;
    speed_t         ttyspeed;

    fd = open(tty, O_RDWR | O_NONBLOCK
#if !defined(__FreeBSD__) && defined(O_EXLOCK)
              | O_EXLOCK
#endif
        );
    if (fd < 0) {
        fprintf (stderr, "Error: Failed to open %s: %s\n", tty,
            strerror (errno));
        return (-1);
    }
    if (tcgetattr(fd, &termios) < 0) {

        perror("tcgetattr");
        return (-1);
    }
    termios.c_iflag = 0;
    termios.c_oflag = 0;        /* (ONLRET) */
    termios.c_cflag = CS8 | CLOCAL | CREAD;
    termios.c_lflag = 0;
    {
        int             cnt;

        for (cnt = 0; cnt < NCCS; cnt++)
            termios.c_cc[cnt] = -1;
    }
    termios.c_cc[VMIN] = blocksz;
    termios.c_cc[VTIME] = 2;

    switch (ttybaud) {
    case 300:
        ttyspeed = B300;
        break;
    case 1200:
        ttyspeed = B1200;
        break;
    case 2400:
        ttyspeed = B2400;
        break;
    case 4800:
        ttyspeed = B4800;
        break;
    case 9600:
        ttyspeed = B9600;
        break;
    case 19200:
        ttyspeed = B19200;
        break;
    case 38400:
        ttyspeed = B38400;
        break;
    case 115200:
        ttyspeed = B115200;
        break;
    default:
        ttyspeed = B4800;
        break;
    }

    if (cfsetispeed(&termios, ttyspeed) != 0) {
        perror("cfsetispeed");
        return (-1);
    }
    if (cfsetospeed(&termios, ttyspeed) != 0) {
        perror("cfsetospeed");
        return (-1);
    }
    if (tcsetattr(fd, TCSANOW, &termios) < 0) {
        perror("tcsetattr");
        return (-1);
    }
#if 1        			/* WANT_BLOCKING_READ */
    if (fcntl(fd, F_SETFL, 0) == -1) {
        perror("fcntl: set nonblock");
    }
#endif
    return (fd);
}



/*
* usage
*
* Send a usage message to standard error and quit the program.
*
* Parameters:
*     None.
*
* Return Value:
*     The function does not return a value.
*
* Remarks:
*
*/
void usage (void)
{
    fprintf (stderr, "nmead: NMEA server application\n");
    fprintf (stderr, "Usage: nmead [OPTIONS]\n");
    fprintf (stderr, "  Options are:\n");
    fprintf (stderr, "    -b baud_rate  sets serial output baud rate\n");
    fprintf (stderr, "       default/current value is %ld\n", ttybaud);
    fprintf (stderr, "    -i serial_port  sets name of serial input device\n");
    fprintf (stderr, "       default/current value is %s\n", ttyport);
    fprintf (stderr, "       (normally a symbolic link to /dev/ttyxxx)\n");
    fprintf (stderr, "    -p tcp_port  sets port number on which the server will listen\n");
    fprintf (stderr, "       default/current value is %d\n", port);
    fprintf (stderr, "    -v verblevel  turns on extra output\n");
    exit (2);
}


