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

#include <agent_upgrade.h>
#include <logging.h>
#include <buffer.h>
#include <ip_address.h>
#include <client_code.h>
#include <net.h>

int find_policy_server(IPAddress **address)
{
    Buffer *ip_address_buffer = NULL;
    char buffer[256]; /* Even the longest IP Address will fit here */
    int fd = 0;
    size_t address_size = 0;

    if (!address)
    {
        Log (LOG_LEVEL_ERR, "Could not find the address of the policy server");
        return -1;
    }
    fd = open ("/var/cfengine/policy_server.dat", O_RDONLY);
    if (fd < 0)
    {
        Log (LOG_LEVEL_ERR, "Could not find the address of the policy server");
        return -1;
    }
    address_size = read(fd, buffer, 256);
    ip_address_buffer = BufferNewFrom(buffer, address_size);
    if (!ip_address_buffer)
    {
        Log (LOG_LEVEL_ERR, "Could not find the address of the policy server");
        close (fd);
        return -1;
    }
    *address = IPAddressNew(ip_address_buffer);
    if (!*address)
    {
        Log (LOG_LEVEL_ERR, "Could not find the address of the policy server");
        close (fd);
        BufferDestroy(&ip_address_buffer);
        return -1;
    }
    Log (LOG_LEVEL_INFO, "Using %s as upgrade server", BufferData(ip_address_buffer));
    close (fd);
    BufferDestroy(&ip_address_buffer);
    return 0;
}

int ConnectToServerForUpgrade(const IPAddress *address, const char *os, const char *architecture)
{
    int error = 0;
    FileCopy fc;
    char request[CF_BUFSIZE];
    char reply[CF_BUFSIZE];
    AgentConnection *connection = NULL;

    connection = NewAgentConn(BufferData(IPAddressGetAddress(address)));
    error = ServerConnect(connection, BufferData(IPAddressGetAddress(address)), fc);
    if (error == 0)
    {
        Log (LOG_LEVEL_CRIT, "Unable to contact the server for upgrades [%s]", BufferData(IPAddressGetAddress(address)));
        return -1;
    }
    snprintf(request, CF_BUFSIZE, "HAVE_UPGRADE %s %s %s", Version(), os, architecture);
    error = SendTransaction(&connection->conn_info, request, strlen(request), CF_DONE);
    if (error < 0)
    {
        Log (LOG_LEVEL_CRIT, "Could not send upgrade request");
        Log (LOG_LEVEL_VERBOSE, "[ %s ]", request);
        DisconnectServer(connection);
        return -1;
    }
    error = ReceiveTransaction(&connection->conn_info, reply, NULL);
    if (error < 0)
    {
        Log (LOG_LEVEL_CRIT, "Could not receive reply from server");
        DisconnectServer(connection);
        return -1;
    }
    /* Analyze the reply, if the first two characters are No, then no upgrades are available */
    if ((reply[0] == 'N') && (reply[1] == 'o'))
    {
        Log (LOG_LEVEL_NOTICE, "No upgrades were found");
    }
    else
    {
        /* Download the update */
        Log (LOG_LEVEL_VERBOSE, "Found upgrade [ %s ]", reply);
        struct stat upgrade_info;
        char stattype[] = "no link";
        error = cf_remote_stat(reply, &upgrade_info, stattype, false, connection);
        if (error < 0)
        {
            Log (LOG_LEVEL_CRIT, "Could not stat upgrade package");
            DisconnectServer(connection);
            return -1;
        }
        char *package = xstrdup(reply);
        error = CopyRegularFileNet(reply, package, upgrade_info.st_size, false, connection);
        if (error == 0)
        {
            Log (LOG_LEVEL_CRIT, "Could not download upgrade package");
            DisconnectServer(connection);
            return -1;
        }
        Log (LOG_LEVEL_VERBOSE, "Upgrade downloaded to %s", package);
    }
    DisconnectServer(connection);
    return 0;
}

int CheckForUpgrades(const char *os, const char *architecture)
{
    int result = 0;
    IPAddress *policy_server = NULL;

    Log (LOG_LEVEL_INFO, "Checking for upgrades");
    result = find_policy_server(&policy_server);

    if (result < 0)
    {
        Log (LOG_LEVEL_ERR, "Could not read the address of the policy server");
        return -1;
    }
    Log (LOG_LEVEL_INFO, "Connecting to the server");
    result = ConnectToServerForUpgrade(policy_server, os, architecture);
    if (result < 0)
    {
        Log (LOG_LEVEL_ERR, "Could not connect to server");
        IPAddressDestroy(&policy_server);
        return -1;
    }
    return 0;
}
