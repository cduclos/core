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

#ifndef REFCOUNT_H
#define REFCOUNT_H

#include "refcount_p.h"

struct RefCount {
    // Normally one unless we are shared.
    unsigned int mUserCount;
    RefCountNode *mUsers;
    RefCountNode *mLast;
};
typedef struct RefCount RefCount;

void RefCount_init(RefCount **ref);
void RefCount_destroy(RefCount **ref);
int RefCount_attach(RefCount *ref, void *owner);
int RefCount_detach(RefCount *ref, void *owner);
int RefCount_isShared(RefCount *ref);
int RefCount_isEqual(RefCount *a, RefCount *b);

#endif // REFCOUNT_H
