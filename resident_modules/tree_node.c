#include <stdlib.h>
#include <string.h>
#include "../headers/resident.h"

struct ResidentNode* createNode(char block, char flatNo[], char name[], char phone[]) {
    struct ResidentNode* newNode = (struct ResidentNode*)malloc(sizeof(struct ResidentNode));
    
    newNode->block = block;
    strcpy(newNode->flatNo, flatNo);
    strcpy(newNode->info.name, name);
    strcpy(newNode->info.phone, phone);

    newNode->left = NULL;
    newNode->right = NULL;

    return newNode;
}