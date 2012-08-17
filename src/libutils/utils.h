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

/*
 * This library is a collection of utilities used in different points of
 * the code. By having them here we can just reuse them without problems.
 *
 * This header file is just a simple collection point for this library.
 * By including this header we get access to the whole functionality of the
 * library. If that is not desired one can access each piece by only including
 * that header file.
 */

#ifndef UTILS_H
#define UTILS_H

#include "bit-operations.h"
#include "compiler.h"
#include "errors.h"
#include "linkedlist.h"
#include "logging.h"

#endif // UTILS_H
