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

#include "linkedlist.h"

#define IsIteratorValid(iterator) \
    iterator->mState != iterator->mOrigin->mState
#define ChangeListState(list) \
    list->mState++

int LinkedList_init(LinkedList **list)
{
    if (!list)
        return -1;
    *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (!(*list)) {
        // This is unlikely in Linux but other Unixes actually return NULL
        return -1;
    }
    (*list)->mList = NULL;
    (*list)->mFirst = NULL;
    (*list)->mLast = NULL;
    (*list)->mNodeCount = 0;
    (*list)->mState = 0;
    (*list)->destroy = NULL;
    return 0;
}

int LinkedList_setDestroyer(LinkedList *list, void (*destroy)(void *element))
{
    if (!list)
        return -1;
    list->destroy = destroy;
    return 0;
}

int LinkedList_destroy(LinkedList **list)
{
    if (!list || !(*list))
        return -1;
    // If there are elements, we check if we have a destroyer. If not we refuse to delete the list.
    if ((*list)->mNodeCount && !(*list)->destroy)
        return -1;
    // We have a destroyer or the list is empty
    LinkedListNode *node = NULL;
    LinkedListNode *p = NULL;
    for (node = (*list)->mFirst; node; node = node->mNext) {
        if (p)
            free(p);
        (*list)->destroy(node->mPayload);
        p = node;
    }
    if (p)
        free(p);
    free((*list));
    *list = NULL;
    return 0;
}

/*
 * Adds an element to the beginning of the list.
 * Notice that we do not copy the element, so if the original element is free'd there will be
 * a dangling pointer.
 */
int LinkedList_add(LinkedList *list, void *payload)
{
    LinkedListNode *node = NULL;
    if (!list)
        return -1;
    node = (LinkedListNode *)malloc(sizeof(LinkedListNode));
    if (!node) {
        // This is unlikely in Linux but other Unixes actually return NULL
        return -1;
    }
    node->mPayload = payload;
    node->mPrevious = NULL;
    if (list->mList) {
        // We have elements
        node->mNext = list->mList;
        list->mList->mPrevious = node;
    } else {
        // First element
        node->mNext = NULL;
        list->mLast = node;
    }
    list->mList = node;
    list->mFirst = node;
    list->mNodeCount++;
    ChangeListState(list);
    return 0;
}

/*
 * This function adds an element to the end of the list.
 * Again, notice that we do not copy the element, so if the original element is free'd
 * there will be problems
 */
int LinkedList_append(LinkedList *list, void *payload)
{
    LinkedListNode *node = NULL;
    if (!list)
        return -1;
    node = (LinkedListNode *)malloc(sizeof(LinkedListNode));
    if (!node) {
        // This is unlikely in Linux but other Unixes actually return NULL
        return -1;
    }
    node->mNext = NULL;
    node->mPayload = payload;
    if (list->mLast) {
        // We have elements
        node->mPrevious = list->mLast;
        list->mLast->mNext = node;
    } else {
        // First element
        node->mPrevious = NULL;
        list->mList = node;
        list->mFirst = node;
    }
    list->mLast = node;
    list->mNodeCount++;
    ChangeListState(list);
    return 0;
}

/*
 * Removes the first element that matches the payload.
 * It starts looking from the beginning of the list.
 */
int LinkedList_remove(LinkedList *list, void *payload)
{
    if (!list)
        return -1;
    LinkedListNode *node = NULL;
    int found = 0;
    for (node = list->mList; node; node = node->mNext) {
        if (node->mPayload == payload) {
            found = 1;
            break;
        }
    }
    if (!found)
        return -1;
    /*
     * We found the node, we just need to change the pointers.
     * We do not clean the memory used by the payload, but we clean the node element.
     */
    if (node->mNext && node->mPrevious) {
        // Middle of the list
        node->mNext->mPrevious = node->mPrevious;
        node->mPrevious->mNext = node->mNext;
    } else if (node->mNext) {
        // First element of the list
        list->mList = node->mNext;
        list->mFirst = node->mNext;
        node->mNext->mPrevious = NULL;
    } else if (node->mPrevious) {
        // Last element
        node->mPrevious->mNext = NULL;
        list->mLast = node->mPrevious;
    } else {
        // Single element
        list->mList = NULL;
        list->mFirst = NULL;
        list->mLast = NULL;
    }
    free(node);
    list->mNodeCount--;
    ChangeListState(list);
    return 0;
}

int LinkedListIterator_get(LinkedList *list, LinkedListIterator **iterator)
{
    if (!list || !iterator)
        return -1;
    *iterator = (LinkedListIterator *)malloc(sizeof(LinkedListIterator));
    if (!(*iterator)) {
        // This is unlikely in Linux but other Unixes actually return NULL
        return -1;
    }
    (*iterator)->mCurrent = list->mList;
    // Remaining only works in one direction, we need two variables for this.
    (*iterator)->mOrigin = list;
    (*iterator)->mState = list->mState;
    return 0;
}

int LinkedListIterator_release(LinkedListIterator **iterator)
{
    if (!iterator || !(*iterator))
        return -1;
    (*iterator)->mCurrent = NULL;
    free((*iterator));
    *iterator = NULL;
    return 0;
}

int LinkedListIterator_first(LinkedListIterator *iterator)
{
    if (!iterator)
        return -1;
    if (IsIteratorValid(iterator))
        // The list has moved forward, the iterator is invalid now
        return -1;
    iterator->mCurrent = iterator->mOrigin->mFirst;
    return 0;
}

int LinkedListIterator_last(LinkedListIterator *iterator)
{
    if (!iterator)
        return -1;
    if (IsIteratorValid(iterator))
        // The list has moved forward, the iterator is invalid now
        return -1;
    iterator->mCurrent = iterator->mOrigin->mLast;
    return 0;
}

int LinkedListIterator_next(LinkedListIterator *iterator)
{
    if (!iterator)
        return -1;
    if (IsIteratorValid(iterator))
        // The list has moved forward, the iterator is invalid now
        return -1;
    // Ok, check if we are at the end
    if (iterator->mCurrent && iterator->mCurrent->mNext) {
        iterator->mCurrent = iterator->mCurrent->mNext;
    } else
        return -1;
    return 0;
}

int LinkedListIterator_previous(LinkedListIterator *iterator)
{
    if (!iterator)
        return -1;
    if (IsIteratorValid(iterator))
        // The list has moved forward, the iterator is invalid now
        return -1;
    // Ok, check if we are at the end
    if (iterator->mCurrent && iterator->mCurrent->mPrevious) {
        iterator->mCurrent = iterator->mCurrent->mPrevious;
    } else
        return -1;
    return 0;
}

int LinkedListIterator_data(const LinkedListIterator *iterator, void **payload)
{
    if (!iterator && !payload)
        return -1;
    if (IsIteratorValid(iterator))
        // The list has moved forward, the iterator is invalid now
        return -1;
    if (iterator->mCurrent)
        *payload = iterator->mCurrent->mPayload;
    else {
        *payload = NULL;
        return -1;
    }
    return 0;
}
