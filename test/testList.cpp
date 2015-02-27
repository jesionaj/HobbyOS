// 2015 Adam Jesionowski

#include <stdlib.h>
#include <iostream>
#include "CppUTest/TestHarness.h"
#include "list.h"
#include "utils.h"

// TODO: Test new node is in list method

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
	    free(first);
	}
};

TEST(List, RootOk)
{
   POINTERS_EQUAL(first, root);
}

TEST(List, AppendOne)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	POINTERS_EQUAL(root, second);
	POINTERS_EQUAL(first, second->next);
	POINTERS_EQUAL(second, first->prev);

	free(second);
}

TEST(List, AppendTwo)
{
	List_t* second = makeNode(NULL);
	List_t* third = makeNode(NULL);
	AppendToList(&root, second);
	AppendToList(&root, third);

	POINTERS_EQUAL(root, third);
	POINTERS_EQUAL(first, third->next->next);
	POINTERS_EQUAL(third, first->prev->prev);

	free(second);
	free(third);
}

TEST(List, RemoveRoot)
{
	RemoveFromList(&root, first);
	POINTERS_EQUAL(root, NULL);
}

TEST(List, RemoveLast)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	RemoveFromList(&root, first);
	POINTERS_EQUAL(root, second);
	POINTERS_EQUAL(root->next, NULL);
}

TEST(List, RemoveFirst)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	RemoveFromList(&root, second);
	POINTERS_EQUAL(root, first);
	POINTERS_EQUAL(root->next, NULL);
}

TEST(List, RemoveFirstMethod)
{
	List_t* second = makeNode(NULL);
	AppendToList(&root, second);

	RemoveFront(&root);
	POINTERS_EQUAL(root, first);
	POINTERS_EQUAL(root->next, NULL);
}

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

