#include <setjmp.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <cmockery.h>
#include <libutils/linkedlist.h>

// Simple initialization test
static void test_initList(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);
}

// This function is just an example function for the destroyer
#include <stdio.h>
void testDestroyer(void *element) {
    // We know the elements are just char *
    // However we cannot free them because they are stack allocated
    // so we just print them
    char *s = (char *)element;
    printf("element: %s \n", s);
}

static void test_destroyer(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);
    assert_int_equal(list->destroy, NULL);

    // Assign the destroyer function
    assert_int_equal(LinkedList_setDestroyer(list, testDestroyer), 0);

    char element0[] = "this is a test string";
    char element1[] = "another test string";
    char element2[] = "yet another test string";
    char element3[] = "and one more test string";
    char element4[] = "non existing element";

    // We add element0 to the list.
    assert_int_equal(LinkedList_add(list, element0), 0);
    // We add element1 to the list.
    assert_int_equal(LinkedList_add(list, element1), 0);
    // We add element2 to the list.
    assert_int_equal(LinkedList_add(list, element2), 0);
    // We add element3 to the list.
    assert_int_equal(LinkedList_add(list, element3), 0);

    // Now we try to destroy the list.
    assert_int_equal(LinkedList_destroy(&list), 0);
}

static void test_addToList(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);
    assert_int_equal(list->destroy, NULL);

    char element0[] = "this is a test string";
    char element1[] = "another test string";
    void *listPointer = NULL;
    void *firstPointer = NULL;
    void *lastPointer = NULL;

    // We add element0 to the list.
    assert_int_equal(LinkedList_add(list, element0), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    firstPointer = list->mFirst;
    assert_int_not_equal(list->mList, NULL);
    listPointer = list->mList;
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    lastPointer = list->mLast;
    assert_int_equal(list->mNodeCount, 1);
    assert_int_equal(list->mState, 1);

    // We add element1 to the list.
    assert_int_equal(LinkedList_add(list, element1), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == firstPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 2);
    assert_int_equal(list->mState, 2);

    // Now we try to destroy the list. This should fail because the list is not empty
    assert_int_equal(LinkedList_destroy(&list), -1);

    // Yes, we are leaking memory here but we shouldn't be using the remove function until we
    // know that it works.
}

static void test_appendToList(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);
    assert_int_equal(list->destroy, NULL);

    char element0[] = "this is a test string";
    char element1[] = "another test string";
    void *element0tPointer = NULL;

    // We add element0 to the list.
    assert_int_equal(LinkedList_append(list, element0), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    element0tPointer = list->mFirst;
    assert_int_not_equal(list->mList, NULL);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == list->mFirst);
    assert_int_equal(list->mNodeCount, 1);
    assert_int_equal(list->mState, 1);

    // We add element1 to the list.
    assert_int_equal(LinkedList_append(list, element1), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_int_not_equal(list->mList, NULL);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(element0tPointer == list->mList);
    assert_true(element0tPointer == list->mFirst);
    assert_int_equal(list->mNodeCount, 2);
    assert_int_equal(list->mState, 2);

    // Now we try to destroy the list. This should fail because the list is not empty
    assert_int_equal(LinkedList_destroy(&list), -1);

    // Yes, we are leaking memory here but we shouldn't be using the remove function until we
    // know that it works.
}

static void test_removeFromList(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);
    assert_int_equal(list->destroy, NULL);
    assert_int_equal(list->compare, NULL);
    assert_int_equal(list->copy, NULL);


    // Assign the compare function
    assert_int_equal(LinkedList_setCompare(list, strcmp), 0);
    // Assign the destroyer function
    assert_int_equal(LinkedList_setDestroyer(list, testDestroyer), 0);

    char element0[] = "this is a test string";
    char element1[] = "another test string";
    char element2[] = "yet another test string";
    char element3[] = "and one more test string";
    char element4[] = "non existing element";
    void *listPointer = NULL;
    void *firstPointer = NULL;
    void *secondPointer = NULL;
    void *thirdPointer = NULL;
    void *lastPointer = NULL;

    // We add element0 to the list.
    assert_int_equal(LinkedList_add(list, element0), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    firstPointer = list->mFirst;
    assert_int_not_equal(list->mList, NULL);
    listPointer = list->mList;
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    lastPointer = list->mLast;
    assert_int_equal(list->mNodeCount, 1);
    assert_int_equal(list->mState, 1);

    // We add element1 to the list.
    assert_int_equal(LinkedList_add(list, element1), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == firstPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    secondPointer = list->mList;
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 2);
    assert_int_equal(list->mState, 2);

    // We add element2 to the list.
    assert_int_equal(LinkedList_add(list, element2), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == firstPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    thirdPointer = list->mList;
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 3);
    assert_int_equal(list->mState, 3);

    // We add element3 to the list.
    assert_int_equal(LinkedList_add(list, element3), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == firstPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 4);
    assert_int_equal(list->mState, 4);

    // We remove the non existing element
    assert_int_equal(LinkedList_remove(list, element4), -1);
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == firstPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 4);
    assert_int_equal(list->mState, 4);

    // Remove element1 which is in the middle of the list
    assert_int_equal(LinkedList_remove(list, element1), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == firstPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 3);
    assert_int_equal(list->mState, 5);

    // Remove element3 which is at the beginning of the list
    assert_int_equal(LinkedList_remove(list, element3), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == secondPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == lastPointer);
    assert_int_equal(list->mNodeCount, 2);
    assert_int_equal(list->mState, 6);

    // Remove element0 which is at the end of the list
    assert_int_equal(LinkedList_remove(list, element0), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == secondPointer);
    assert_int_not_equal(list->mList, NULL);
    assert_false(list->mList == listPointer);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == thirdPointer);
    assert_int_equal(list->mNodeCount, 1);
    assert_int_equal(list->mState, 7);

    // Remove element2 which is the only element on the list
    assert_int_equal(LinkedList_remove(list, element2), 0);
    // Now we check the list
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 8);

    // Now we destroy the list.
    assert_int_equal(LinkedList_destroy(&list), 0);
}

static void test_destroyList(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);

    // Now we destroy the list
    assert_int_equal(LinkedList_destroy(&list), 0);
    assert_int_equal(list, NULL);
}

static void test_iterator(void **state)
{
    LinkedList *list = NULL;
    assert_int_equal(LinkedList_init(&list), 0);
    assert_int_not_equal(list, NULL);
    assert_int_equal(list->mFirst, NULL);
    assert_int_equal(list->mList, NULL);
    assert_int_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 0);
    assert_int_equal(list->mState, 0);

    // Assign the compare function
    assert_int_equal(LinkedList_setCompare(list, strcmp), 0);
    // Assign the destroyer function
    assert_int_equal(LinkedList_setDestroyer(list, testDestroyer), 0);

    char element0[] = "this is a test string";
    char element1[] = "another test string";
    char element2[] = "yet another test string";
    char element3[] = "and one more test string";
    void *element0Pointer = NULL;
    void *element1Pointer = NULL;
    void *element2Pointer = NULL;
    void *element3Pointer = NULL;

    // We add element0 to the list.
    assert_int_equal(LinkedList_add(list, element0), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    element0Pointer = list->mFirst;
    assert_int_not_equal(list->mList, NULL);
    assert_true(list->mList == list->mFirst);
    assert_int_not_equal(list->mLast, NULL);
    assert_int_equal(list->mNodeCount, 1);
    assert_int_equal(list->mState, 1);

    // We add element1 to the list.
    assert_int_equal(LinkedList_add(list, element1), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == element0Pointer);
    assert_int_not_equal(list->mList, NULL);
    element1Pointer = list->mList;
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == element0Pointer);
    assert_int_equal(list->mNodeCount, 2);
    assert_int_equal(list->mState, 2);

    // We add element2 to the list.
    assert_int_equal(LinkedList_add(list, element2), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == element1Pointer);
    assert_int_not_equal(list->mList, NULL);
    element2Pointer = list->mList;
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == element0Pointer);
    assert_int_equal(list->mNodeCount, 3);
    assert_int_equal(list->mState, 3);

    // We add element3 to the list.
    assert_int_equal(LinkedList_add(list, element3), 0);
    // Now we check the list
    assert_int_not_equal(list->mFirst, NULL);
    assert_false(list->mFirst == element2Pointer);
    assert_int_not_equal(list->mList, NULL);
    element3Pointer = list->mList;
    assert_int_not_equal(list->mLast, NULL);
    assert_true(list->mLast == element0Pointer);
    assert_int_equal(list->mNodeCount, 4);
    assert_int_equal(list->mState, 4);

    LinkedListIterator *iterator0 = NULL;
    assert_int_equal(LinkedListIterator_get(list, &iterator0), 0);
    // Check the iterator
    assert_int_not_equal(iterator0, NULL);
    assert_int_equal(iterator0->mState, 4);
    assert_true(iterator0->mOrigin == list);
    assert_true(iterator0->mCurrent == list->mFirst);

    // Remove element1 which is in the middle of the list, this will invalidate the iterator
    assert_int_equal(LinkedList_remove(list, element1), 0);
    // Check that the iterator is not valid by trying to advance it
    assert_int_equal(LinkedListIterator_next(iterator0), -1);
    // Destroy the iterator
    assert_int_equal(LinkedListIterator_release(&iterator0), 0);
    assert_int_equal(iterator0, NULL);

    // Create a new iterator and move it
    LinkedListIterator *iterator1 = NULL;
    assert_int_equal(LinkedListIterator_get(list, &iterator1), 0);
    // Check the iterator
    assert_int_not_equal(iterator1, NULL);
    assert_int_equal(iterator1->mState, 5);
    assert_true(iterator1->mOrigin == list);
    assert_true(iterator1->mCurrent == list->mFirst);
    void *value = NULL;
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element3);

    // Advance it
    assert_int_equal(LinkedListIterator_next(iterator1), 0);
    // Check the value, it should be equal to element2
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element2);

    // Advance it, now we are at the last element
    assert_int_equal(LinkedListIterator_next(iterator1), 0);
    // Check the value, it should be equal to element0
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element0);

    // Advance it, should fail and the iterator should stay where it was
    assert_int_equal(LinkedListIterator_next(iterator1), -1);
    // Check the value, it should be equal to element0
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element0);

    // Go back
    assert_int_equal(LinkedListIterator_previous(iterator1), 0);
    // Check the value, it should be equal to element2
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element2);

    // Go back, now we are at the beginning of the list
    assert_int_equal(LinkedListIterator_previous(iterator1), 0);
    // Check the value, it should be equal to element3
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element3);

    // Go back, should fail and the iterator should stay where it was
    assert_int_equal(LinkedListIterator_previous(iterator1), -1);
    // Check the value, it should be equal to element3
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element3);

    // Jump to the last element
    assert_int_equal(LinkedListIterator_last(iterator1), 0);
    // Check the value, it should be equal to element0
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element0);

    // Go back
    assert_int_equal(LinkedListIterator_previous(iterator1), 0);
    // Check the value, it should be equal to element2
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element2);

    // Jump to the first element
    assert_int_equal(LinkedListIterator_first(iterator1), 0);
    // Check the value, it should be equal to element3
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element3);

    // Advance it
    assert_int_equal(LinkedListIterator_next(iterator1), 0);
    // Check the value, it should be equal to element2
    assert_int_equal(LinkedListIterator_data(iterator1, &value), 0);
    assert_true(value == element2);

    // Remove the elements
    assert_int_equal(LinkedList_remove(list, element3), 0);
    assert_int_equal(LinkedList_remove(list, element0), 0);
    assert_int_equal(LinkedList_remove(list, element2), 0);

    // Now we destroy the list.
    assert_int_equal(LinkedList_destroy(&list), 0);
}

int main()
{
    const UnitTest tests[] = {
        unit_test(test_initList)
        , unit_test(test_destroyList)
        , unit_test(test_destroyer)
        , unit_test(test_addToList)
        , unit_test(test_appendToList)
        , unit_test(test_removeFromList)
        , unit_test(test_iterator)
    };

    return run_tests(tests);
}

