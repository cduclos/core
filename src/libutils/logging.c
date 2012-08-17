/*
   Copyright (C) Cfengine AS

   This file is part of Cfengine 3 - written and maintained by Cfengine AS.

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
  versions of Cfengine, the applicable Commerical Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#include "logging.h"

#if 0
/*
 * Common functionality of CfFOut and CfOut.
 */
static void log(FILE *fh, enum cfreport level, const char *errstr, const char *fmt, va_list args)
{
    char buffer[CF_BUFSIZE], output[CF_BUFSIZE];
    Item *mess = NULL;

    if ((fmt == NULL) || (strlen(fmt) == 0))
    {
        return;
    }

    memset(output, 0, CF_BUFSIZE);
    vsnprintf(buffer, CF_BUFSIZE - 1, fmt, args);
    Chop(buffer);
    AppendItem(&mess, buffer, NULL);

    if ((errstr == NULL) || (strlen(errstr) > 0))
    {
        snprintf(output, CF_BUFSIZE - 1, " !!! System reports error for %s: \"%s\"", errstr, GetErrorStr());
        AppendItem(&mess, output, NULL);
    }

    switch (level)
    {
    case cf_inform:

        if (INFORM || VERBOSE || DEBUG)
        {
            LogList(fh, mess, VERBOSE);
        }
        break;

    case cf_verbose:

        if (VERBOSE || DEBUG)
        {
            LogList(fh, mess, VERBOSE);
        }
        break;

    case cf_error:
    case cf_reporting:
    case cf_cmdout:

        LogList(fh, mess, VERBOSE);
        MakeLog(mess, level);
        break;

    case cf_log:

        if (VERBOSE || DEBUG)
        {
            LogList(fh, mess, VERBOSE);
        }
        MakeLog(mess, cf_verbose);
        break;

    default:

        FatalError("Report level unknown");
        break;
    }

    DeleteItemList(mess);
}

void CfFOut(const char *filename, const enum cfreport level, const char *errstr, const char *fmt, ...)
{
    FILE *fp = fopen(filename, "a");
    if (fp == NULL)
    {
        CfOut(cf_error, "fopen", "Could not open log file %s\n", filename);
        fp = stdout;
    }

    va_list ap;
    va_start(ap, fmt);

    log(fp, level, errstr, fmt, ap);

    va_end(ap);

    if (fp != stdout)
    {
        fclose(fp);
    }

}

void CfOut(const enum cfreport level, const char *errstr, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(stdout, level, errstr, fmt, ap);
    va_end(ap);
}
#endif
