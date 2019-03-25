/***************************************************************************
 *   Copyright (C) 2007 by root   *
 *   root@localhost.localdomain   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "SYProtocol.h"

using namespace std;

int main(int argc, char *argv[])
{
//Open USB 
   if(PSOpenDevice( DEVICE_USB,0,0))
	   goto OPEN_scc;
   printf("Open usb fail!\n");

//Open UDisk
   if(PSOpenDevice( DEVICE_UDisk,0,0))
	   goto OPEN_scc;
   printf("Open UDisk fail!\n");
         
//Open Com
  	if(PSOpenDevice( DEVICE_COM,0,57600/9600))
		goto OPEN_scc;
	printf("Open com fail!\n");

   	return 0;
 
OPEN_scc://Open dev success
   printf("Open success\n");

   PSCloseDevice();
   printf("Close success\n");

  return EXIT_SUCCESS;
}
