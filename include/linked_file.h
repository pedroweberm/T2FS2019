#include "t2disk.h"

struct Node {
    fileData* data;
    struct Node* next;
};

typedef struct fileStruct
{
    record fileRecord;
    FILE2 handle;
    int currentPointer;
} fileData;

void printList(struct Node* n);

struct Node* createLinkedList();

struct Node* createNode(BYTE TypeVal, char name[], DWORD Nao_usado[2], DWORD inodeNumber);

struct Node* appendToList(struct Node* n, struct Node* newNode);

struct Node* removeFromList(struct Node* n, char* nodeName);

fileData* searchList(struct Node* n, int current_pointer);

