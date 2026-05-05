#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/resident.h"


#pragma pack(push, 1)
struct DiskRecord {
    char block;
    char flatNo[10];
    char name[50];
    char phone[15];
};
#pragma pack(pop)

void appendTransaction(char block, char flatNo[], char name[], char phone[]) {
    FILE* file = fopen("database.dat", "ab");
    if (file == NULL) {
        return;
    }

    struct DiskRecord record;
    record.block = block;
    strcpy(record.flatNo, flatNo);
    strcpy(record.name, name);
    strcpy(record.phone, phone);

    fwrite(&record, sizeof(struct DiskRecord), 1, file);
    fclose(file);
}

struct ResidentNode* loadAllData() {
    struct ResidentNode* root = NULL;
    
    // "rb" mode reads the transaction log from top to bottom
    FILE* file = fopen("database.dat", "rb");
    if (file == NULL) {
        return NULL; // No history found, starting fresh
    }

    struct DiskRecord record;
    
    while (fread(&record, sizeof(struct DiskRecord), 1, file)) {
        if (strcmp(record.name, "DELETED") == 0) {
            root = deleteResident(root, record.block, record.flatNo);
        } else {
            root = insertResident(root, record.block, record.flatNo, record.name, record.phone);
        }
    }
    
    fclose(file);
    return root;
}
