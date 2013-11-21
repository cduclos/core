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

#include <icomms_generic.h>
#include <logging.h>
#include <sys/select.h>
#include <alloc.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>

/*
 * For now only Unix is supported. Windows has a similar mechanism called
 * named pipes but with a completely different interface.
 */

struct ICommsInterface {
    struct sockaddr_un *unix_socket;
    socklen_t unix_socket_length;
    int timeout;
    int physical;
    uint32_t sent;
    uint32_t received;
};

/*
 * These methods are implemented in imessage.c but not exported on the public API.
 */
void *IMessageSerialize(IMessage *message);
IMessage *IMessageDeserialize(IMessageRequests request, void *buffer);
void IMessageDestroySerialization(void *serialization);
/* This structure is required to get the data out of the wire, forward declaring it. */
struct IMessageHeader {
    IMessageRequests request;
    pid_t sender;
};

ICommsInterface *ICommsInterfaceNew(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    /* Make sure we don't chew more than what we can swallow. */
    struct sockaddr_un test;
    if (strlen(path) > sizeof(test.sun_path))
    {
        return NULL;
    }

    ICommsInterface *interface = xcalloc(1, sizeof(ICommsInterface));
    interface->unix_socket = xcalloc(1, sizeof(struct sockaddr_un));
    interface->unix_socket->sun_family = AF_UNIX;
    strcpy(interface->unix_socket->sun_path, path);
    interface->unix_socket_length = sizeof(struct sockaddr_un);
    interface->timeout = ICOMMS_INTERFACE_DEFAULT_TIMEOUT;
    interface->physical = -1;

    return interface;
}

void ICommsInterfaceDestroy(ICommsInterface **interface)
{
    if (!interface || !*interface)
    {
        return;
    }
    free ((*interface)->unix_socket);
    free(*interface);
    *interface = NULL;
}

void *ICommsInterfaceLowLevelInterface(const ICommsInterface *interface)
{
    if (!interface || !interface->unix_socket)
    {
        return NULL;
    }
    return interface->unix_socket;
}

unsigned int ICommsInterfaceLowLevelSize(const ICommsInterface *interface)
{
    if (!interface || !interface->unix_socket)
    {
        return 0;
    }
    return interface->unix_socket_length;
}

int ICommsInterfaceTimeout(const ICommsInterface *interface)
{
    if (!interface)
    {
        return -1;
    }
    return interface->timeout;
}

void ICommsInterfaceSetTimeout(ICommsInterface *interface, const int timeout)
{
    if (!interface || (timeout < 0))
    {
        return;
    }
    interface->timeout = timeout;
}

int ICommsInterfaceWrite(ICommsInterface *interface, IMessage *message)
{
    if (!interface || !message)
    {
        return -1;
    }
    if (interface->physical < 0)
    {
        return -1;
    }
    /* Serialize the message */
    struct msghdr *msg = IMessageSerialize(message);
    if (!msg)
    {
        return -1;
    }

    /* Use select to check if a write would block, if so then do not write. */
    int result = 0;
    struct timeval tv;
    tv.tv_sec = interface->timeout;
    tv.tv_usec = 0;

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(interface->physical, &wfds);

    result = select(interface->physical + 1, NULL, &wfds, NULL, &tv);
    if (result != 1)
    {
        /* There is only one fd, so any other result is a problem */
        Log(LOG_LEVEL_INFO, "Could not send internal message");
        goto finish;
    }

    result = sendmsg(interface->physical, msg, 0);
    if (result < 0)
    {
        Log(LOG_LEVEL_INFO, "Could not send internal message");
        goto finish;
    }
    interface->sent += result;
finish:
    IMessageDestroySerialization(msg);
    return result;
}

int ICommsInterfaceRead(ICommsInterface *interface, IMessage **message)
{
    if (!interface || !message)
    {
        return -1;
    }
    if (interface->physical < 0)
    {
        return -1;
    }
    /* Recover the message from the wire. */
    /* Use select to check if a read would block, if so then do not read. */
    int result = 0;
    struct timeval tv;
    tv.tv_sec = interface->timeout;
    tv.tv_usec = 0;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(interface->physical, &rfds);

    result = select(interface->physical + 1, &rfds, NULL, NULL, &tv);
    if (result != 1)
    {
        /* There is only one fd, so any other result is a problem */
        Log(LOG_LEVEL_INFO, "Could not receive internal message");
        return -1;
    }

    struct msghdr msg;
    struct iovec vec[2];

    struct IMessageHeader header;
    vec[0].iov_base = &header;
    vec[0].iov_len = sizeof(struct IMessageHeader);

    char buffer[IMESSAGE_MAX_PAYLOAD];
    vec[1].iov_base = buffer;
    vec[1].iov_len = IMESSAGE_MAX_PAYLOAD;

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = vec;
    msg.msg_iovlen = 2;

#if defined(HAVE_MSGHDR_MSG_CONTROL)
    char ccmsg[CMSG_SPACE(sizeof(interface->physical))];
    msg.msg_control = ccmsg;
    msg.msg_controllen = sizeof(ccmsg);
#elif defined(HAVE_MSGHDR_ACCRIGHTS)
    int *potential = xmalloc(sizeof(int));
    msg.msg_accrights    = (char *)potential;
    msg.msg_accrightslen = sizeof(int);
#else
#error "Your platform does not support this operation, this code should not be compiled!"
#endif

    result = recvmsg(interface->physical, &msg, 0);
    if (result < 0)
    {
        Log(LOG_LEVEL_INFO, "Could not receive internal message");
#if defined(HAVE_MSGHDR_ACCRIGHTS)
        free(potential);
#endif
        return -1;
    }
    interface->received += result;

    if (header.request == IMessage_ShareOwnership)
    {
        /* There is no deserialization call, we do that manually. */
#if defined(HAVE_MSGHDR_MSG_CONTROL)
        struct cmsghdr *cmsg = NULL;
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg->cmsg_type != SCM_RIGHTS)
        {
            Log (LOG_LEVEL_ERR, "Message is of type ShareOwnership but data is of another type");
            return -1;
        }
        *message = IMessageNew(IMessage_ShareOwnership, (int*)CMSG_DATA(cmsg));
#elif defined(HAVE_MSGHDR_ACCRIGHTS)
        *message = IMessageNew(IMessage_ShareOwnership, potential);
#else
#error "Your platform does not support this operation, this code should not be compiled!"
#endif
    }
    else
    {
        /* All the other request can free "potential", so it is easier to do it this way. */
#if defined(HAVE_MSGHDR_ACCRIGHTS)
        free(potential);
#endif
        if (header.request == IMessage_WriteText)
        {
            /* We request deserialization */
            *message = IMessageDeserialize(IMessage_WriteText, buffer);
            if (!(*message))
            {
                return -1;
            }
        }
    }
    return result;
}
