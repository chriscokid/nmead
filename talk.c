/*
* talk.c
*
* NMEA Server Application
*
*
* Copyright 2000-2004 Charles L. Taylor.
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
#include "nmead.h"


extern int verbose;


/*
* talk
*
* Get data from the GPS receiver and distribute it to any connections that
* are listening.
*
* Parameters:
*     arg : pointer : Pointer to an initialized talkerinfo_t structure.
*
* Return Value:
*     The function does not return a value.
*
* Remarks:
*     This routine is expected to run continuously until the entire
*     application is terminated.
*
*/
void * talk (void * arg)
{
    talkerinfo_t * ti = (talkerinfo_t *) arg;
    char nmeabuf[BUFSZ];

    if (verbose >= 10)
        printf ("talker: started\n");

    if (ti == NULL) {
        fprintf (stderr, "talker: bad argument\n");
        exit (-2);
    }

    if (ti->fp == NULL) {
        fprintf (stderr, "talker: bad fp\n");
        exit (-2);
    }

    while (1) {
        fgets (nmeabuf, BUFSZ, ti->fp);
        if (nmeabuf[0] != '$' && nmeabuf[0] != '!')  /* not a valid NMEA line */
            continue;

        writetoconnections (ti->cmgr, nmeabuf);
        if (verbose >= 200)
            printf ("%s", nmeabuf);

    }

}


