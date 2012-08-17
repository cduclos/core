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

#include "errors.h"
#include "logging.h"

/*
 * This function calls exit(...) so use it with extreme care
 */
void fatalError(char *s, ...)
{
#if 0
    CfLock best_guess;

    if (s)
    {
        va_list ap;
        char buf[CF_BUFSIZE] = "";

        va_start(ap, s);
        vsnprintf(buf, CF_BUFSIZE - 1, s, ap);
        va_end(ap);
        CfOut(cf_error, "", "Fatal CFEngine error: %s", buf);
    }

    if (strlen(CFLOCK) > 0)
    {
        best_guess.lock = xstrdup(CFLOCK);
        best_guess.last = xstrdup(CFLAST);
        best_guess.log = xstrdup(CFLOG);
        YieldCurrentLock(best_guess);
    }

    unlink(PIDFILE);
    EndAudit();
    GenericDeInitialize();
    exit(1);
#endif
}
