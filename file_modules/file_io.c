#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/house.h"
#include "../headers/resident.h"
#include "../headers/file_io.h"

void saveFlatsToFile(FILE* file) 
{
    for (int i = 0; i < 5; i++) 
    {
        struct Flat* current = communityBlocks[i];
        while (current != NULL) {
            fprintf(file, "F,%c,%s,%d,%d\n", current->block, current->flatNo, current->bhk, current->status);
            current = current->next;
        }
    }
}

void saveResidentsToFile(struct ResidentNode* root, FILE* file) 
{
    if (root == NULL) return;
    fprintf(file, "R,%c,%s,%s,%s\n", root->block, root->flatNo, root->info.name, root->info.phone);
    saveResidentsToFile(root->left, file);
    saveResidentsToFile(root->right, file);
}

void saveAllData(struct ResidentNode* root) 
{
    FILE* file = fopen("C:\\Users\\Mithin Krishna\\OneDrive\\Desktop\\c-workspace\\SDP\\database.txt", "w");
    if (file == NULL) 
    {
        printf("Error: Could not create database file!\n");
        return;
    }
    saveFlatsToFile(file);
    saveResidentsToFile(root, file);
    fclose(file);
    printf("\n[SYSTEM] All data successfully saved to 'database.txt'!\n");
}

struct ResidentNode* loadAllData(struct ResidentNode* root) 
{
    FILE* file = fopen("C:\\Users\\Mithin Krishna\\OneDrive\\Desktop\\c-workspace\\SDP\\database.txt", "r");
    if (file == NULL) 
    {
        printf("\n[SYSTEM] No existing database found. Starting a fresh system!\n");
        return root; 
    }

    char line[200];
    while (fgets(line, sizeof(line), file)) 
    {
        if (line[0] == 'F') 
        {
            char type, block;
            char flatNo[10];
            int bhk, status;
            sscanf(line, "%c,%c,%[^,],%d,%d", &type, &block, flatNo, &bhk, &status);
            addFlat(flatNo, block, bhk);
            int index = block - 'A';
            if (communityBlocks[index] != NULL) 
            {
                communityBlocks[index]->status = status; 
            }
        } 
        
        else if (line[0] == 'R') 
        {
            char type, block;
            char flatNo[10], name[50], phone[15];
            sscanf(line, "%c,%c,%[^,],%[^,],%[^\n]", &type, &block, flatNo, name, phone);
            root = insertResident(root, block, flatNo, name, phone);
        }
    }
    
    fclose(file);
    printf("\n[SYSTEM] Previous data successfully loaded from 'database.txt'!\n");
    return root; 
}