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

#include <stdlib.h>
#include "refcount.h"

void RefCount_init(RefCount **ref)
{
    if (!ref)
        return;
    *ref = (RefCount *)malloc(sizeof(RefCount));
    (*ref)->mUserCount = 0;
    (*ref)->mUsers = NULL;
    (*ref)->mLast = NULL;
}

void RefCount_destroy(RefCount **ref)
{
    if (ref && *ref) {
        // Don't destroy the refCount if it is still in use by somebody else.
        if ((*ref)->mUserCount > 1) {
            return;
        }
        free(*ref);
        *ref = NULL;
    }
}

int RefCount_attach(RefCount *ref, void *owner)
{
    if (!ref || !owner)
        return -1;
    ref->mUserCount++;
    RefCountNode *node = (RefCountNode *)malloc(sizeof(RefCountNode));
    node->mNext = NULL;
    node->mUser = owner;
    if (ref->mLast) {
        ref->mLast->mNext = node;
        node->mPrevious = ref->mLast;
    } else {
        ref->mUsers = node;
        node->mPrevious = NULL;
    }
    ref->mLast = node;
    return ref->mUserCount;
}

int RefCount_detach(RefCount *ref, void *owner)
{
    if (!ref || !owner)
        return -1;
    RefCountNode *p = NULL;
    int found = 0;
    for (p = ref->mUsers; p; p = p->mNext)
        if (p->mUser == owner) {
            found = 1;
            if (p->mPrevious && p->mNext) {
                p->mPrevious->mNext = p->mNext;
                p->mNext->mPrevious = p->mPrevious;
            } else if (p->mPrevious && !p->mNext) {
                // Last node
                p->mPrevious->mNext = NULL;
                ref->mLast = p->mPrevious;
            } else if (!p->mPrevious && p->mNext) {
                // First node
                ref->mUsers = p->mNext;
                p->mNext->mPrevious = NULL;
            } else {
                // Only one node, we cannot detach from ourselves.
                return 0;
            }
            free(p);
            break;
        }
    if (!found)
        return -1;
    ref->mUserCount--;
    return ref->mUserCount;
}

int RefCount_isShared(RefCount *ref)
{
    if (!ref)
        return 0;
    if (ref->mUserCount == 0)
        return 0;
    return (ref->mUserCount != 1);
}

int RefCount_isEqual(RefCount *a, RefCount *b)
{
    if (a == b)
        return 1;
    if (a && b) {
        // Compare the inner elements
        if (a->mUserCount == b->mUserCount) {
            RefCountNode *na = a->mUsers;
            RefCountNode *nb = b->mUsers;
            while (na && nb) {
                if (na->mUser != nb->mUser) {
                    break;
                }
                na = na->mNext;
                nb = nb->mNext;
            }
            return 1;
        }
    }
    return 0;
}
