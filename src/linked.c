#include "../include/linked.h"
#include <stdlib.h>
#include <stdio.h>



void printList(struct Node* n)
{
    while (n != NULL) {
        printf(" %s ", n->data->name);
        n = n->next;
    }
}

struct Node* createLinkedList(){
    return NULL;
}

struct Node* createNode(BYTE TypeVal, char name[], DWORD Nao_usado[2], DWORD inodeNumber)
{
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));

    newNode->data->TypeVal = TypeVal;
    strcpy(newNode->data->name, name);
    memcpy(newNode->data->Nao_usado, Nao_usado, 2 * sizeof(DWORD));
    newNode->datainodeNumber = inodeNumber;

    return newNode;
}

struct Node* appendToList(struct Node* n, struct Node* newNode)
{
    struct Node* aux_n = n;
    if (aux_n != NULL)
    {
        while(aux_n->next != NULL)
        {
            aux_n = aux_n->next;
        }
        aux_n->next = newNode;
        return n;
    }
    else
    {
        return newNode;
    }
}

struct Node* removeFromList(struct Node* n, char* nodeName)
{
    struct Node* aux_n = n;
    if (aux_n != NULL)
    {
        while(aux_n->next != NULL)
        {
            if (strcmp(aux_n->next->data->name, nodeName))
            {
                aux_n->next = aux_n->next->next;
                free(aux_n->next);
                return n;
            }
        }
        return n;
    }
    else
    {
        return NULL;
    }
}

DIRENT2* searchList(struct Node* n, char* nodeName)
{
    struct Node* aux_n = n;
    if (aux_n != NULL)
    {
        while(aux_n->next != NULL)
        {
            if (strcmp(aux_n->next->data->name, nodeName))
            {
                return aux_n->next->data;
            }
        }
        return NULL;
    }
    else
    {
        return NULL;
    }
}
