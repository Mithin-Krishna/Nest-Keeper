#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/resident.h" 

struct DiskRecord {
    char block;
    char flatNo[10];
    char name[50];
    char phone[15];
};

void saveNodePreOrder(struct ResidentNode* root, FILE* file) 
{
    if (root == NULL) 
    {
        return;
    }
    struct DiskRecord record;
    record.block = root->block;
    strcpy(record.flatNo, root->flatNo);
    strcpy(record.name, root->info.name);
    strcpy(record.phone, root->info.phone);
    fwrite(&record, sizeof(struct DiskRecord), 1, file);
    saveNodePreOrder(root->left, file);
    saveNodePreOrder(root->right, file);
}

void saveAllData(struct ResidentNode* root) 
{
    FILE* file = fopen("database.dat", "wb");
    if (file == NULL) 
    {
        printf("[ERROR] Could not open database.dat for saving!\n");
        return;
    }
    saveNodePreOrder(root, file);
    fclose(file);
    printf("[SYSTEM] Success: All data securely locked in 'database.dat'.\n");
}

struct ResidentNode* loadAllData() 
{
    FILE* file = fopen("database.dat", "rb");
    struct ResidentNode* root = NULL; // Start with an empty tree
    if (file == NULL) 
    {
        printf("[SYSTEM] No existing 'database.dat' found. Starting fresh database.\n");
        return NULL;
    }
    struct DiskRecord temp;
    while (fread(&temp, sizeof(struct DiskRecord), 1, file) == 1) 
    {
        root = insertResident(root, temp.block, temp.flatNo, temp.name, temp.phone); 
    }
    fclose(file);
    printf("[SYSTEM] Boot-up Sequence: Data securely loaded from 'database.dat'.\n");
    return root; 
}