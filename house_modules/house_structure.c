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
        printf("Invalid Block!\n");
        free(newFlat); // Prevent memory leak on fail
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
        printf("[ERROR] Could not save flats snapshot.\n");
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