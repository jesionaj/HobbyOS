// 2015 Adam Jesionowski

#include "config.h"
#include "list.h"

/*
 * Append a node to the front of the list
 */
void AppendToList(List_t** root, List_t* node)
{
    if(*root == NULL)
    {
    	// If root is null, all we need to do is set next to NULL
        node->next = NULL;
    }
    else
    {
    	// If it isn't, then the old root now has this as its prev, and next is the old root
        (*root)->prev = node;
        node->next = *root;
    }

    // prev will always be null as we're now root
    node->prev = NULL;
    *root = node;
}

/*
 * Append a node to the end of the list
 */
void AppendToEndOfList(List_t** root, List_t* node)
{
    List_t* temp;

    if(*root == NULL)
    {
    	// If root is null, we're now root
        node->prev = NULL;
        *root = node;
    }
    else
    {
    	// Look for the end of the list
    	temp = *root;

    	while(temp->next != NULL)
    	{
            temp = temp->next;
    	}

    	// Append to the old end of the list
    	temp->next = node;
    	node->prev = temp;
    }

    // As we're at the end, next is always null
    node->next = NULL;
}

/*
 * Remove a node from the list.
 */
void RemoveFromList(List_t** root, List_t* node)
{
    if(root != NULL)
    {
        if(node->prev == NULL)
        {
            if(node->next == NULL)
            {
                // This node is root and there are no more nodes, set root to null
                *root = NULL;
            }
            else
            {
                // This node is root, set next node to root
                *root = node->next;
                node->next->prev = NULL;
            }
        }
        else
        {
            if(node->next == NULL)
            {
                // If this was the old end of the list, set the node behind it to null
                node->prev->next = NULL;
            }
            else
            {
            	// This node was in the middle of the list, remove references to it
                node->prev->next = node->next;
                node->next->prev = node->prev;
            }
        }
    }
    node->prev = NULL;
    node->next = NULL;
}

/*
 * Shortcut for removing the front node of a list.
 */
void RemoveFront(List_t** root)
{
    RemoveFromList(root, *root);
}

/*
 * Returns true if the node is in the given list
 */
bool IsNodeInList(List_t** root, List_t* node)
{
    List_t* list = *root;
    bool found = false;

    while(list != NULL)
    {
        if(list == node)
        {
            found = true;
            break;
        }

        list = list->next;
    }

    return found;
}
