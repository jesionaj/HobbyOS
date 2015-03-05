// 2015 Adam Jesionowski

#include <stdlib.h>
#include <iostream>
#include "CppUTest/TestHarness.h"
#include "list.h"
#include "utils.h"

TEST_GROUP(List)
{
	List_t* root = NULL;
	List_t* first;

	void setup()
	{
	    first = makeNode(NULL);
	    AppendToList(&root, first);
	}

	void teardown()
	{
	    root = NULL;
	    free(first);
	}

	List_t* makeNode(void* owner)
	{
	    List_t* node = (List_t*)malloc(sizeof(List_t));
	    node->next  = NULL;
	    node->owner = owner;
	    node->prev  = NULL;
	    return node;
	}

};

/*
 * We already added a node as root. Check that it was added correctly.
 */
TEST(List, RootOk)
{
   POINTERS_EQUAL(first, root);
}

/*
 * Add another node. This adds it to the front of the list.
 */
TEST(List, AppendOne)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	POINTERS_EQUAL(root, second);           // Root is now the just added node
	POINTERS_EQUAL(second->next, first);    // Make sure we link up to first
	POINTERS_EQUAL(first->prev, second);

	free(second);
}

/*
 * Add two nodes
 */
TEST(List, AppendTwo)
{
	List_t* second = makeNode(NULL);
	List_t* third = makeNode(NULL);
	AppendToList(&root, second);
	AppendToList(&root, third);

	POINTERS_EQUAL(root, third);
	POINTERS_EQUAL(third->next->next, first);
	POINTERS_EQUAL(first->prev->prev, third);

	free(second);
	free(third);
}

/*
 * Remove the root node in a single node list using RemoveFromList
 */
TEST(List, RemoveRoot)
{
	RemoveFromList(&root, first);
	POINTERS_EQUAL(root, NULL);
}

/*
 * Remove the last node in a two item list using RemoveFromList
 */
TEST(List, RemoveLast)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	RemoveFromList(&root, first);
	POINTERS_EQUAL(root, second);
	POINTERS_EQUAL(root->next, NULL);
}

/*
 * Remove the first node in a two item list using RemoveFromList
 */
TEST(List, RemoveFirst)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	RemoveFromList(&root, second);
	POINTERS_EQUAL(root, first);
	POINTERS_EQUAL(root->next, NULL);
}

/*
 * Remove the first node in a two item list using RemoveFront
 */
TEST(List, RemoveFirstMethod)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	RemoveFront(&root);
	POINTERS_EQUAL(root, first);
	POINTERS_EQUAL(root->next, NULL);
}

/*
 * Remove the middle element of a three element list using RemoveFromList
 */
TEST(List, RemoveMid)
{
	List_t* second = makeNode(NULL);
	List_t* third = makeNode(NULL);
	AppendToList(&root, second);
	AppendToList(&root, third);

	RemoveFromList(&root, second);
	POINTERS_EQUAL(root, third);
	POINTERS_EQUAL(root->next, first);
	POINTERS_EQUAL(first->prev, root);
}

/*
 * Add to the end of a list using AppendToEnd
 */
TEST(List, AppendToEnd)
{
	// second->first->third

	List_t* second = makeNode(NULL);
	List_t* third = makeNode(NULL);
	AppendToList(&root, second);
	AppendToEndOfList(&root, third);

	POINTERS_EQUAL(root, second);
	POINTERS_EQUAL(third, second->next->next);
	POINTERS_EQUAL(first, second->next);

	free(second);
	free(third);
}

/*
 * Check whether a node is in the list
 */
TEST(List, IsNodeInList)
{
    List_t* second = makeNode(NULL);
    List_t* third = makeNode(NULL);
    AppendToList(&root, second);
    AppendToList(&root, third);

    CHECK_TRUE(IsNodeInList(&root, first));
    CHECK_TRUE(IsNodeInList(&root, second));
    CHECK_TRUE(IsNodeInList(&root, third));

    free(second);
    free(third);
}

/*
 * Check whether a node is not in the list
 */
TEST(List, NodeNotInList)
{
    List_t* second = makeNode(NULL);
    List_t* third = makeNode(NULL);
    AppendToList(&root, second);

    CHECK_FALSE(IsNodeInList(&root, third));

    free(second);
    free(third);
}

