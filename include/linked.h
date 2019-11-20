#include "t2disk.h"

struct Node {
    struct t2fs_record* data;
    struct Node* next;
    int isOpen;
    int handle;
    int current_pointer;
};

void printList(struct Node* n);

struct Node* createLinkedList();

struct Node* createNode(BYTE TypeVal, char name[], DWORD Nao_usado[2], DWORD inodeNumber, int handle, int current_pointer, int isOpen);

struct Node* appendToList(struct Node* n, struct Node* newNode);

struct Node* removeFromList(struct Node* n, char* nodeName);

struct Node* searchList(struct Node* n, int current_pointer);

