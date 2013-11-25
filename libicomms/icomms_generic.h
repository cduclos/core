/*
   Copyright (C) CFEngine AS

   This file is part of CFEngine 3 - written and maintained by CFEngine AS.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 3.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of CFEngine, the applicable Commercial Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#ifndef ICOMMS_GENERIC_H
#define ICOMMS_GENERIC_H

#include <imessage.h>
/* Generic communications interface */
typedef struct ICommsInterface ICommsInterface;
typedef struct {
    uint32_t sent;
    uint32_t received;
} ICommsInterfaceStats;

#define ICOMMS_INTERFACE_DEFAULT_TIMEOUT    10

/**
  @brief Creates a new communication interface.
  @param name On Unix, path of the socket. On windows, name of the named pipe.
  @return A communication interface or NULL in case of error.
  */
ICommsInterface *ICommsInterfaceNew(const int sd);
/**
  @brief Destroys a communication interface.
  @param interface Communication interface to be destroyed.
  */
void ICommsInterfaceDestroy(ICommsInterface **interface);
/**
  @brief Socket descriptor.
  @param interface ICommsInterface to query.
  @return The socket descriptor or -1 in case of error.
  */
int ICommsInterfaceSocketDescriptor(const ICommsInterface *interface);
/**
  @brief Statistics about the given interface.
  @param interface ICommsInterface to query.
  @param stats Pointer to a ICommsInterfaceStats to store the current statistics.
  @return 0 if successful and -1 in case of error.
  */
int ICommsInterfaceStatistics(const ICommsInterface *interface, ICommsInterfaceStats *stats);
/**
  @brief Timeout used by this interface.
  @param interface ICommsInterface to query.
  @return The timeout in seconds or -1 in case of error.
  */
int ICommsInterfaceTimeout(const ICommsInterface *interface);
/**
  @brief Sets the timeout used by this interface.
  @param interface ICommsInterface to query.
  @param timeout New timeout.
  */
void ICommsInterfaceSetTimeout(ICommsInterface *interface, const int timeout);
/**
  @brief Writes data using the interface.
  @param interface ICommsInterface to operate on.
  @param buffer Data to be written.
  @param length Length of the data.
  @return The number of bytes written or -1 in case of error.
  */
int ICommsInterfaceWrite(ICommsInterface *interface, IMessage *message);
/**
  @brief Reads data using the interface.
  @param interface ICommsInterface to operate on.
  @param buffer Buffer to store the data read from the interface.
  @param length Length of the data to be read.
  @return The number of bytes read or -1 in case of error.
  */
int ICommsInterfaceRead(ICommsInterface *interface, IMessage **message);

#endif // ICOMMS_GENERIC_H
