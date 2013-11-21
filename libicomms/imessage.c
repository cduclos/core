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

#include <alloc.h>
#include <compiler.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include <imessage.h>
#include <logging.h>

struct IMessageData {
    uint16_t length;
    void *payload;
};

struct IMessageHeader {
    IMessageRequests request;
    pid_t sender;
};

struct IMessage {
    struct IMessageHeader *header;
    struct IMessageData *data;
};

/*
 * Request specific constructors.
 */
struct IMessageData *IMessage_ShareOwnershipRequest(void *data)
{
    /*
     * This request shares a socket between two processes.
     */
    struct IMessageData *imdata = xcalloc(1, sizeof(struct IMessageData));
    imdata->length = (uint16_t)sizeof(int);
    imdata->payload = data;
    return imdata;
}

struct IMessageData *IMessage_WriteTextRequest(void *data)
{
    /*
     * This request sends a text from one process to another.
     */
    char *text = data;
    uint16_t length = (uint16_t)strlen(text);
    if (length > IMESSAGE_MAX_PAYLOAD)
    {
        return NULL;
    }
    struct IMessageData *imdata = xcalloc(1, sizeof(struct IMessageData));
    imdata->length = length;
    imdata->payload = text;
    return imdata;
}

/*
 * This method serializes an IMessage to a struct msghdr so it can be transmitted.
 * The result depends on the type of the request.
 * This method is not part of the public API and it is only supposed to be used
 * by the ICommsSend function.
 */
typedef enum {
    IMessageData_Empty,
    IMessageData_Text
} IMessageDataTypes;

void *IMessageSerialize(IMessage *message)
{
    assert(message);
    struct msghdr *msg = xcalloc(1, sizeof(struct msghdr));
    /*
     * We use the first slot on the IO vector to send our protocol header.
     * The second slot is used to send the data payload.
     */
    struct iovec *vec = xcalloc(2, sizeof(struct iovec));
    struct IMessageHeader *header = message->header;
    assert(header);
    struct IMessageData *data = message->data;
    assert(data);
    vec[0].iov_base = (char *)header;
    vec[0].iov_len = sizeof(struct IMessageHeader);
    /*
     * Every request sends the same amount of data IMESSAGE_MAX_PAYLOAD as the
     * second entry on the vector. If a request does not have data to send,
     * then that space will be filled with zeros.
     * Data is encoded as following:
     * 2B Data type
     * 2B Data length
     * XB Data
     * We initialize a buffer with zeros and each request fills it accordingly.
     * It has to be initialized on the heap or it will disappear after this
     * function is done.
     */
    char *buffer = xmalloc(IMESSAGE_MAX_PAYLOAD);
    memset(buffer, 0, IMESSAGE_MAX_PAYLOAD);
    vec[1].iov_base = buffer;
    vec[1].iov_len = IMESSAGE_MAX_PAYLOAD;
    msg->msg_iov = vec;
    msg->msg_iovlen = 2;

    /* Request specific data, add more cases accordingly. */
    if (header->request == IMessage_ShareOwnership)
    {
        int *fd = data->payload;
#if defined(HAVE_MSGHDR_MSG_CONTROL)
        char ccmsg[CMSG_SPACE(sizeof(*fd))];
        struct cmsghdr *cmsg = NULL;
        msg->msg_control = ccmsg;
        msg->msg_controllen = sizeof(ccmsg);
        cmsg = CMSG_FIRSTHDR(msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(*fd));
        msg->msg_controllen = cmsg->cmsg_len;
        msg->msg_flags = 0;
#elif defined(HAVE_MSGHDR_ACCRIGHTS)
        msg->msg_accrights  = (char *)fd;
        msg->msg_accrightslen = sizeof(*fd);
#else
#error "Your platform does not support this operation, this code should not be compiled!"
#endif
    }
    else if (header->request == IMessage_WriteText)
    {
        uint16_t type = htons(IMessageData_Text);
        uint16_t length = htons(data->length);
        memcpy(buffer, &type, sizeof(type));
        memcpy(buffer, &length, sizeof(length));
        char *p = data->payload;
        /* The -2 is to avoid overwriting the final '\0' */
        strncpy(buffer + sizeof(type) + sizeof(length), p,
                IMESSAGE_MAX_PAYLOAD - 2 - sizeof(type) - sizeof(length));
    }

    return msg;
}

void IMessageDestroySerialization(void *serialization)
{
    assert(serialization);
    struct msghdr *msg = serialization;
    /*
     * 1. Destroy the buffer inside the vector vec[1].
     * 2. Destroy the vector.
     * 3. Destroy the message.
     */
    struct iovec *vec = msg->msg_iov;
    free(vec[1].iov_base);
    free (vec);
    free (msg);
}

IMessage *IMessageDeserialize(IMessageRequests request, void *buffer)
{
    /*
     * Depending on the kind of request how to deserialize the data.
     * For now we only have text requests.
     */
    if (request == IMessage_WriteText)
    {
        /*
         * 1. Get the type of the data.
         * 2. Check that is text.
         * 3. Get the length.
         * 4. Copy it to a buffer.
         * 5. Create the message.
         */
        uint16_t type;
        uint16_t length;
        memcpy(&type, buffer, sizeof(type));
        if (type != IMessageData_Text)
        {
            Log(LOG_LEVEL_ERR, "Data type does not match announced type");
            return NULL;
        }
        memcpy(&length, buffer + sizeof(type), sizeof(length));
        char *text = xmalloc((size_t)length);
        char *p = buffer + sizeof(type) + sizeof(length);
        strncpy(text, p, IMESSAGE_MAX_PAYLOAD);
        return IMessageNew(request, text);
    }
    return NULL;
}

/*
 * Public API
 */
IMessage *IMessageNew(IMessageRequests request, void *data)
{
    if (!data)
    {
        return NULL;
    }

    struct IMessageData *imdata = NULL;
    switch (request)
    {
    case IMessage_ShareOwnership:
        imdata = IMessage_ShareOwnershipRequest(data);
        break;
    case IMessage_WriteText:
        imdata = IMessage_WriteTextRequest(data);
        break;
    default:
        break;
    }

    if (!imdata)
    {
        return NULL;
    }

    IMessage *message = xcalloc(1, sizeof(struct IMessage));
    struct IMessageHeader *header = xcalloc(1, sizeof(struct IMessageHeader));
    header->request = request;
    header->sender = getpid();

    message->header = header;
    message->data = imdata;

    return message;
}

void IMessageDestroy(IMessage **message)
{
    if (!message || !*message)
    {
        return;
    }
    free ((*message)->header);
    free (*message);
    *message = NULL;
}

void *IMessageDataContent(const IMessage *message)
{
    if (!message || !message->data)
    {
        return NULL;
    }
    return message->data->payload;
}

uint16_t IMessageDataLength(const IMessage *message)
{
    if (!message || !message->data)
    {
        return 0;
    }
    return message->data->length;
}

IMessageRequests IMessageRequest(const IMessage *message)
{
    if (!message || !message->header)
    {
        return IMessage_Invalid;
    }
    return message->header->request;
}

pid_t IMessageSender(const IMessage *message)
{
    if (!message || !message->header)
    {
        return 0;
    }
    return message->header->sender;
}
