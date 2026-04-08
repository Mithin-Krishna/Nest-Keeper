#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/house.h"

struct Flat* communityBlocks[5]; 

void setupBlocks() 
{
    for (int i = 0; i < 5; i++) {
        communityBlocks[i] = NULL;
    }
}

void addFlat(char flatNo[], char block, int bhk) 
{
    struct Flat* newFlat = (struct Flat*)malloc(sizeof(struct Flat));
    strcpy(newFlat->flatNo, flatNo);
    newFlat->block = block;
    newFlat->bhk = bhk;
    newFlat->status = 0;
        
    int index = 0;
    if (block == 'A') index = 0;
    else if (block == 'B') index = 1;
    else if (block == 'C') index = 2;
    else if (block == 'D') index = 3;
    else if (block == 'E') index = 4;
    else {
        printf("Invalid Block!\n");
        free(newFlat); // Prevent memory leak on fail
        return;
    }
    
    newFlat->next = communityBlocks[index];
    communityBlocks[index] = newFlat;
    
    printf("Successfully added Flat %s to Block %c!\n", flatNo, block);
}

int doesFlatExist(char block, char flatNo[]) 
{
    int index = block - 'A';
    if (index < 0 || index > 4) return 0; // Invalid block
    
    struct Flat* current = communityBlocks[index];
    while (current != NULL) {
        if (strcmp(current->flatNo, flatNo) == 0) {
            return 1; // Found it!
        }
        current = current->next;
    }
    return 0; // Does not exist
}

void updateFlatStatus(char block, char flatNo[], int newStatus) 
{
    int index = block - 'A';
    if (index < 0 || index > 4) return;
    
    struct Flat* current = communityBlocks[index];
    while (current != NULL) {
        if (strcmp(current->flatNo, flatNo) == 0) {
            current->status = newStatus;
            return;
        }
        current = current->next;
    }
}