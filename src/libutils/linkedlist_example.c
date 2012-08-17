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

#include <stdio.h>
#include "linkedlist.h"

/*
 * This function is an auxiliary function that destroys the elements of the list
 * Since we cannot free argv, we just print them.
 */
void destroy(void *element) {
    char *s = (char *)element;
    printf("destroying: %s \n", s);
}

/*
 * This program shows how to use the linkedlist implementation.
 * The list will store the command line arguments and iterate over them.
 */

int main(int argc, char **argv)
{
    int result = 0;
    int i = 0;
    // Create a linkedlist
    LinkedList *list = NULL;
    result = LinkedList_init(&list);
    if (result < 0) {
        printf("Problems creating the list!\n");
        return 0;
    }
    // Adding elements
    for (i = 0; i < argc; i++) {
        // The difference between add and append is that "add" adds at the beginning
        // of the list while "append" adds to the end of the list.
        result = LinkedList_add(list, argv[i]);
        if (result < 0) {
            printf("Problems adding element: %i \n", i);
            return 0;
        }
    }
    // Traversing the list.
    // Changing the list after the iterator was created invalidates the iterator and it forces to discard it.
    LinkedListIterator *iterator = NULL;
    result = LinkedListIterator_get(list, &iterator);
    if (result < 0) {
        printf("Problems getting an iterator for the list!\n");
        return 0;
    }
    do {
        // Getting the element
        char *element = NULL;
        result = LinkedListIterator_data(iterator, (void **)&element);
        if (result < 0) {
            printf("Not possible to get the data\n");
            return 0;
        }
        printf("Element: %s \n", element);
    } while (LinkedListIterator_next(iterator) == 0);

    /*
     * The list cannot be destroyed until all elements have been destroyed, otherwise
     * we might leak memory. So in order to clean the list we have to assign a function
     * that acts as a destroyer for the elements.
     */
    result = LinkedList_setDestroyer(list, destroy);
    if (result < 0) {
        printf("Couldn't assign destroyer function \n");
        return 0;
    }

    // Destroy the list
    result = LinkedList_destroy(&list);
    if (result < 0) {
        printf("Problems destroying the list! \n");
        return 0;
    }
    return 0;
}
