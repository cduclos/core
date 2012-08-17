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

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>
#include "linkedlist_p.h"

/*
 * This provides a double-linked list implementation.
 * The implementation of the list is kept intentionally opaque,
 * so we can change it in the future.
 * Specifically I have in mind reference-counting, copy-on-write
 * and shallow-copies.
 */
struct LinkedList {
    // Number of nodes
    int mNodeCount;
    // Incremental number that keeps track of the state of the list
    unsigned int mState;
    // Nodes
    LinkedListNode *mList;
    // Link to the first element
    LinkedListNode *mFirst;
    // Link to the last element
    LinkedListNode *mLast;
    // This function can be used to destroy the elements at destruction time
    void (*destroy)(void *element);
};
typedef struct LinkedList LinkedList;
struct LinkedListIterator {
    LinkedListNode *mCurrent;
    LinkedList *mOrigin;
    unsigned int mState;
};
typedef struct LinkedListIterator LinkedListIterator;

int LinkedList_init(LinkedList **list);
int LinkedList_setDestroyer(LinkedList *list, void (*destroy)(void *));
int LinkedList_destroy(LinkedList **list);
int LinkedList_add(LinkedList *list, void *payload);
int LinkedList_append(LinkedList *list, void *payload);
int LinkedList_remove(LinkedList *list, void *payload);
int LinkedListIterator_get(LinkedList *list, LinkedListIterator **iterator);
int LinkedListIterator_release(LinkedListIterator **iterator);
int LinkedListIterator_first(LinkedListIterator *iterator);
int LinkedListIterator_last(LinkedListIterator *iterator);
int LinkedListIterator_next(LinkedListIterator *iterator);
int LinkedListIterator_previous(LinkedListIterator *iterator);
int LinkedListIterator_data(const LinkedListIterator *iterator, void **payload);

#endif // LINKEDLIST_H
