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
  versions of CFEngine, the applicable Commerical Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#include "tls.h"
#include "cf3.defs.h"
#include "alloc.h"
#include "logging.h"
#include "misc_lib.h"

int SendTransaction(ConnectionInfo *connection, char *buffer, int len, char status);
int ServerStartTLS(ConnectionInfo *connection)
{
    int result = 0;
    char buffer[CF_BUFSIZE];
    snprintf(buffer, CF_BUFSIZE, "ACK");

    /*
     * We prepare everything before sending the ACK
     */
    int sd = connection->physical.sd;
    connection->physical.tls = (TLSInfo *)xmalloc(sizeof(TLSInfo));
    connection->physical.tls->method = TLSv1_server_method();
    connection->physical.tls->context = SSL_CTX_new(connection->physical.tls->method);

    if (!connection->physical.tls->context)
    {
        Log(LOG_LEVEL_CRIT, "Unable to create the SSL context");
        free (connection->physical.tls);
        return -1;
    }

    connection->physical.tls->ssl = SSL_new(connection->physical.tls->context);

    if (!connection->physical.tls->ssl)
    {
        Log(LOG_LEVEL_CRIT, "Unable to create the SSL object");
        SSL_CTX_free (connection->physical.tls->context);
        free (connection->physical.tls);
        return -1;
    }

    /*
     * Now we are ready to tell the client to try the TLS initialization.
     */
    ConnectionInfo info;
    info.type = CFEngine_Classic;
    info.physical.sd = sd;
    result = SendTransaction(&info, buffer, 0, CF_DONE);
    if (result == -1)
    {
        Log(LOG_LEVEL_ERR, "Unable to send transaction, aborting connection. (send: %s)", GetErrorStr());
        /*
         * It is not as easy as closing the socket. We need to come up with a proper
         * way to bring down the connection.
         */
    }

    SSL_set_fd(connection->physical.tls->ssl, sd);

    /*
     * Now we wait for the client to send us the TLS request.
     */
    int total_tries = 0;
    do {
        result = SSL_accept(connection->physical.tls->ssl);
        if (result <= 0)
        {
            /*
             * Identify the problem and if possible try to fix it.
             */
            int error = SSL_get_error(connection->physical.tls->ssl, result);
            if ((SSL_ERROR_WANT_WRITE == error) || (SSL_ERROR_WANT_READ == error))
            {
                Log(LOG_LEVEL_DEBUG, "Recoverable error in TLS handshake, trying to fix it");
                /*
                 * We can try to fix this.
                 * This error means that there was not enough data in the buffer, using select
                 * to wait until we get more data.
                 */
                fd_set rfds;
                struct timeval tv;
                int tries = 0;

                do {
                    SET_DEFAULT_TLS_TIMEOUT(tv);
                    FD_ZERO(&rfds);
                    FD_SET(connection->physical.sd, &rfds);

                    result = select(connection->physical.sd+1, &rfds, NULL, NULL, &tv);
                    if (result > 0)
                    {
                        /*
                         * Ready to receive data
                         */
                        break;
                    }
                    else
                    {
                        Log(LOG_LEVEL_DEBUG, "select(2) timed out, retrying (tries: %d)", tries);
                        ++tries;
                    }
                } while (tries <= DEFAULT_TLS_TRIES);
            }
            else
            {
                /*
                 * Unrecoverable error
                 */
                Log(LOG_LEVEL_DEBUG, "Unrecoverable error in TLS handshake (error: %d)", error);
                SSL_free (connection->physical.tls->ssl);
                SSL_CTX_free (connection->physical.tls->context);
                free (connection->physical.tls);
                return -1;
            }
        }
        else
        {
            /*
             * TLS channel established, start talking!
             */
            Log (LOG_LEVEL_INFO, "TLS connection established");
            connection->type = CFEngine_TLS;
            break;
        }
        ++total_tries;
    } while (total_tries <= DEFAULT_TLS_TRIES);
    return 0;
}

int SendTLS(SSL *ssl, const char *buffer, int length)
{
    if (!ssl || !buffer || (length < 0))
    {
        return -1;
    }
    /*
     * Technically speaking, the buffer is either sent completely or not sent at all.
     * Therefore it is not needed to count how many bytes we have sent, OpenSSL does that
     * for us.
     */
    int total_tries = 0;
    int sent = 0;
    do {
        sent = SSL_write(ssl, buffer, length);
        if (sent <= 0)
        {
            int error = SSL_get_error(ssl, sent);
            Log(LOG_LEVEL_DEBUG, "SSL_write failed, retrying (tries: %d)", total_tries);
            if ((SSL_ERROR_WANT_READ == error) || (SSL_ERROR_WANT_WRITE == error))
            {
                /*
                 * We need to retry the operation using exactly the same arguments.
                 * We will use select(2) to wait until the underlying socket is ready.
                 */
                int fd = SSL_get_fd(ssl);
                if (fd < 0)
                {
                    Log(LOG_LEVEL_DEBUG, "Could not get fd from SSL");
                    return -1;
                }
                fd_set wfds;
                struct timeval tv;
                int result = 0;
                int tries = 0;

                do {
                    SET_DEFAULT_TLS_TIMEOUT(tv);
                    FD_ZERO(&wfds);
                    FD_SET(fd, &wfds);

                    result = select(fd+1, NULL, &wfds, NULL, &tv);
                    if (result > 0)
                    {
                        /*
                         * Ready to send data
                         */
                        break;
                    }
                    else
                    {
                        Log(LOG_LEVEL_DEBUG, "select(2) timed out, retrying (tries: %d)", tries);
                        ++tries;
                    }
                } while (tries <= DEFAULT_TLS_TRIES);
            }
            else
            {
                /*
                 * Any other error is fatal.
                 */
                Log(LOG_LEVEL_DEBUG, "Fatal error on SSL_write (error: %d)", error);
                return -1;
            }
        }
        else
        {
            /*
             * We sent more than 0 bytes so we are done.
             */
            Log(LOG_LEVEL_DEBUG, "Sent %d bytes using TLS", sent);
            break;
        }
        ++total_tries;
    } while (total_tries <= DEFAULT_TLS_TRIES);
    return sent;
}

int ReceiveTLS(SSL *ssl, char *buffer, int length)
{
    if (!ssl || !buffer || (length < 0))
    {
        return -1;
    }
    int total_tries = 0;
    int received = 0;
    do {
        received = SSL_read(ssl, buffer, length);
        if (received <= 0)
        {
            int error = SSL_get_error(ssl, received);
            Log(LOG_LEVEL_DEBUG, "SSL_read failed, retrying (tries: %d)", total_tries);
            if ((SSL_ERROR_WANT_READ == error) || (SSL_ERROR_WANT_WRITE == error))
            {
                /*
                 * We need to retry the operation using exactly the same arguments.
                 * We will use select(2) to wait until the underlying socket is ready.
                 */
                int fd = SSL_get_fd(ssl);
                if (fd < 0)
                {
                    Log(LOG_LEVEL_DEBUG, "Could not get fd from SSL");
                    return -1;
                }
                fd_set rfds;
                struct timeval tv;
                int result = 0;
                int tries = 0;

                do {
                    SET_DEFAULT_TLS_TIMEOUT(tv);
                    FD_ZERO(&rfds);
                    FD_SET(fd, &rfds);

                    result = select(fd+1, &rfds, NULL, NULL, &tv);
                    if (result > 0)
                    {
                        /*
                         * Ready to receive data
                         */
                        break;
                    }
                    else
                    {
                        Log(LOG_LEVEL_DEBUG, "select(2) timed out, retrying (tries: %d)", tries);
                        ++tries;
                    }
                } while (tries <= DEFAULT_TLS_TRIES);
            }
            else
            {
                /*
                 * Any other error is fatal.
                 */
                Log(LOG_LEVEL_DEBUG, "Fatal error on SSL_read (error: %d)", error);
                return -1;
            }
        }
        else
        {
            /*
             * We received more than 0 bytes so we are done.
             */
            Log(LOG_LEVEL_DEBUG, "Received %d bytes using TLS", received);
            break;
        }
        ++total_tries;
    } while (total_tries <= DEFAULT_TLS_TRIES);
    buffer[received] = '\0';
    return received;
}
