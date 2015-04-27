/*
* msgbuffer.h
*
* NMEA Server Application
*
* Buffer structure and function prototypes for passing text messages
* between threads.
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


#ifndef MSGBUFFER_H
#define MSGBUFFER_H

#include <semaphore.h>



#define MSGELEMENTLENGTH    88    /* 80 + cr/lf + padding to dword boundary */
#define MSGBUFFERELEMENTS   20


typedef char msgelement[MSGELEMENTLENGTH];


typedef struct msgbuffer {
    sem_t  semaccess;
    int readindex;
    int writeindex;
    msgelement element[MSGBUFFERELEMENTS];
} msgbuffer;


#ifdef __cplusplus
extern "C" {
#endif


msgbuffer * newmsgbuffer (void);
void destroymsgbuffer (msgbuffer * buf);
int putmsg (msgbuffer * buf, const char * msg);
int getmsg (msgbuffer * buf, char * msg, int length);


#ifdef __cplusplus
}
#endif


#endif  /* MSGBUFFER_H */


