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

#ifndef IMESSAGE_H
#define IMESSAGE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <unistd.h>

typedef struct IMessage IMessage;

/**
  @brief This library provides a standard framework for communicating data between processes.

  It is based on message passing between processes over local domain sockets.
  */

typedef enum {
    IMessage_Invalid,       /*!< An invalid code is always needed */
    IMessage_ShareOwnership,/*!< Share an fd between two processes */
    IMessage_WriteText      /*!< Send a text message to another process */
} IMessageRequests;
/**
  @brief In all honesty the payload is a tad smaller because of the encoding.
  */
#define IMESSAGE_MAX_PAYLOAD    1024
/**
  @brief Constructs a new IMessage structure.
  @param request Request to send to other process.
  @param data Request specific data.
  @return A new IMessage or NULL in case of error.
  */
IMessage *IMessageNew(IMessageRequests request, void *data);
/**
  @brief Destroys an IMessage structure.
  @param message IMessage structure to be destroyed.
  @remarks If the message has data, the data is not destroyed. The caller needs
  to destroy the data on its own.
  */
void IMessageDestroy(IMessage **message);
/**
  @brief IMessage data.
  @param message IMessage structure to operate on.
  @return The data available on the message or NULL in case of error/no data.
  */
void *IMessageDataContent(const IMessage *message);
/**
  @brief Length of the IMessage data.
  @param message IMessage structure to operate on.
  @return The length of the data or 0 in case of error.
  */
uint16_t IMessageDataLength(const IMessage *message);
/**
  @brief IMessage request
  @param message IMessage structure to operate on.
  @return The request or IMessage_Invalid in case of error.
  */
IMessageRequests IMessageRequest(const IMessage *message);
/**
  @brief PID of the sender.
  @param message IMessage structure to operate on.
  @return The PID of the sender process or 0 in case of error.
  */
pid_t IMessageSender(const IMessage *message);

#endif // IMESSAGE_H
