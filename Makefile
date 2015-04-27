#!/bin/make
#
# Developmental makefile for NMEA Server Application
#
# Copyright 2000-2004 Charles L. Taylor.
#
# 26 Jun 2004: Incorporated changes by Martin Mersberger for compatibility
#              with FreeBSD.
#


#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#          		    NO WARRANTY
#
#    BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO
#  WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE
#  LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT
#  HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT
#  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT
#  NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
#  FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE
#  QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
#  PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY
#  SERVICING, REPAIR OR CORRECTION.
#
#    IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN
#  WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY
#  MODIFY AND/OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE
#  LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL,
#  INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR
#  INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF
#  DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU
#  OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY
#  OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN
#  ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#

CC=arm-linux-gcc

#ifndef $(RM)
#      RM := $(shell which rm)
#endif

OBJS=main.o talk.o listeners.o msgbuffer.o connection.o

ifeq ($(shell uname -s),FreeBSD)
      LIBS  = -lc_r -lz
else
      LIBS  = -lpthread -L/opt/FriendlyARM/toolschain/4.4.3/lib
endif

nmead: $(OBJS)
	$(CC) -o nmead $(OBJS) $(LIBS)


clean:
	$(RM) -f *.o nmead



