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
        printf("[ERROR] Could not open database to log transaction.\n");
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
    int loaded = 0;
    
    while (fread(&record, sizeof(struct DiskRecord), 1, file)) {
        // TOMBSTONE CHECK: If the log says they were deleted later on, remove them from the tree!
        if (strcmp(record.name, "DELETED") == 0) {
            root = deleteResident(root, record.block, record.flatNo);
        } else {
            // Otherwise, insert them into our AVL tree
            root = insertResident(root, record.block, record.flatNo, record.name, record.phone);
            loaded++;
        }
    }
    
    fclose(file);
    if (loaded > 0) {
        printf("[SYSTEM] Boot Sequence: Successfully replayed transaction log.\n");
    }
    return root;
}