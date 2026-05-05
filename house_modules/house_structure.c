#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/house.h"

struct Flat* communityBlocks[5]; 

#pragma pack(push, 1)
struct FlatDiskRecord {
    char flatNo[10];
    char block;
    int bhk;
    int status;
};
#pragma pack(pop)

void setupBlocks() 
{
    for (int i = 0; i < 5; i++) 
    {
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
    else 
    {
        free(newFlat);
        return;
    }
    
    newFlat->next = communityBlocks[index];
    communityBlocks[index] = newFlat;
    saveFlats();
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
            saveFlats();
            return;
        }
        current = current->next;
    }
}

struct Flat* findFlat(char block, char flatNo[]) {
    int index = block - 'A';
    if (index < 0 || index > 4) return NULL;

    struct Flat* current = communityBlocks[index];
    while (current != NULL) {
        if (strcmp(current->flatNo, flatNo) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void saveFlats() {
    FILE* file = fopen("flats.dat", "wb");
    if (file == NULL) {
        return;
    }

    for (int index = 0; index < 5; index++) {
        struct Flat* current = communityBlocks[index];
        while (current != NULL) {
            struct FlatDiskRecord record;
            strcpy(record.flatNo, current->flatNo);
            record.block = current->block;
            record.bhk = current->bhk;
            record.status = current->status;
            fwrite(&record, sizeof(struct FlatDiskRecord), 1, file);
            current = current->next;
        }
    }

    fclose(file);
}

int updateFlat(char oldBlock, char oldFlatNo[], char newBlock, char newFlatNo[], int newBhk) {
    int oldIndex = oldBlock - 'A';
    int newIndex = newBlock - 'A';
    struct Flat* current;
    struct Flat* previous = NULL;

    if (oldIndex < 0 || oldIndex > 4 || newIndex < 0 || newIndex > 4) {
        return 0;
    }

    current = communityBlocks[oldIndex];
    while (current != NULL) {
        if (strcmp(current->flatNo, oldFlatNo) == 0) {
            break;
        }
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        return 0;
    }

    if ((oldBlock != newBlock || strcmp(oldFlatNo, newFlatNo) != 0) &&
        doesFlatExist(newBlock, newFlatNo)) {
        return 0;
    }

    if (previous == NULL) {
        communityBlocks[oldIndex] = current->next;
    } else {
        previous->next = current->next;
    }

    current->block = newBlock;
    strcpy(current->flatNo, newFlatNo);
    current->bhk = newBhk;
    current->next = communityBlocks[newIndex];
    communityBlocks[newIndex] = current;

    saveFlats();
    return 1;
}

int deleteFlat(char block, char flatNo[]) {
    int index = block - 'A';
    struct Flat* current;
    struct Flat* previous = NULL;

    if (index < 0 || index > 4) {
        return 0;
    }

    current = communityBlocks[index];
    while (current != NULL) {
        if (strcmp(current->flatNo, flatNo) == 0) {
            break;
        }
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        return 0;
    }

    if (previous == NULL) {
        communityBlocks[index] = current->next;
    } else {
        previous->next = current->next;
    }

    free(current);
    saveFlats();
    return 1;
}

void loadFlats() {
    FILE* file = fopen("flats.dat", "rb");
    if (file == NULL) {
        return;
    }

    struct FlatDiskRecord record;
    while (fread(&record, sizeof(struct FlatDiskRecord), 1, file)) {
        struct Flat* newFlat = (struct Flat*)malloc(sizeof(struct Flat));
        strcpy(newFlat->flatNo, record.flatNo);
        newFlat->block = record.block;
        newFlat->bhk = record.bhk;
        newFlat->status = record.status;
        newFlat->next = NULL;

        int index = record.block - 'A';
        if (index < 0 || index > 4) {
            free(newFlat);
            continue;
        }

        newFlat->next = communityBlocks[index];
        communityBlocks[index] = newFlat;
    }

    fclose(file);
}
