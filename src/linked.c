#include "../include/linked.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void printList(struct Node* n)
{
    while (n != NULL)
    {
        printf("%s \n", n->data->name);
        n = n->next;
    }
}

struct Node* createLinkedList()
{
    return NULL;
}

struct Node* createNode(BYTE TypeVal, char name[], DWORD Nao_usado[2], DWORD inodeNumber, int handle, int current_pointer, int isOpen)
{
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));
    struct t2fs_record* record = (struct t2fs_record*) malloc(sizeof(struct t2fs_record));

    record->TypeVal = TypeVal;
    strcpy(record->name, name);
    memcpy(record->Nao_usado, Nao_usado, 2 * sizeof(DWORD));
    record->inodeNumber = inodeNumber;
    newNode->data = record;

    newNode->handle = handle;
    newNode->current_pointer = current_pointer;
    newNode->isOpen = isOpen;
    newNode->next = NULL;

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
            else
            {
                aux_n = aux_n->next;
            }

        }
        return n;
    }
    else
    {
        return NULL;
    }
}

struct Node* searchList(struct Node* n, int current_pointer)
{
    struct Node* aux_n = malloc(sizeof(struct Node));
    aux_n = n;
    int i = 0;
    if (aux_n != NULL)
    {
        while(aux_n != NULL)
        {
            if(i == current_pointer)
            {
                printf("Chegou no indice\n");
                return aux_n;
            }
            else
            {
                i++;
                aux_n = aux_n->next;
            }
        }
        return NULL;
    }
    else
    {
        return NULL;
    }
}

int getIndex(struct Node* n, char* nodeName)
{
    struct Node* aux_n = n;
    int i = 0;
//    printf("sal\n");
//    printf("Node name passado = %s\n", nodeName);
    if (aux_n != NULL)
    {
        while(aux_n)
        {
            if (strcmp(aux_n->data->name, nodeName))
            {
                return i;
            }
            else
            {
                aux_n = aux_n->next;
                i += 1;
            }
        }
        return -1;
    }
    else
    {
        return -1;
    }
}
int getIndexByHandle(struct Node* n, DWORD handle)
{
    struct Node* aux_n = n;
    int i = 0;
    if (aux_n != NULL)
    {
        while(aux_n != NULL)
        {
            if (aux_n->handle == handle)
            {
                return i;
            }
            else
            {
                aux_n = aux_n->next;
                i += 1;
            }
        }
        return -1;
    }
    else
    {
        return -1;
    }


}
