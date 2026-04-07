#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/resident.h"

// 1. INSERTION
struct ResidentNode* insertResident(struct ResidentNode* root, char block, char flatNo[], char name[], char phone[]) {
    if (root == NULL) {
        return createNode(block, flatNo, name, phone);
    }

    if (block < root->block) {
        root->left = insertResident(root->left, block, flatNo, name, phone);
    }
    else if (block > root->block) {
        root->right = insertResident(root->right, block, flatNo, name, phone);
    }
    else {
        int cmp = strcmp(flatNo, root->flatNo);
        if (cmp < 0) {
            root->left = insertResident(root->left, block, flatNo, name, phone);
        }
        else if (cmp > 0) {
            root->right = insertResident(root->right, block, flatNo, name, phone);
        }
        else {
            printf("Notice: Adding roommate %s to Block %c, Flat %s\n", name, block, flatNo);
            root->right = insertResident(root->right, block, flatNo, name, phone);
        }
    }
    return root;
}

// 2. SEARCHING
struct ResidentNode* searchResident(struct ResidentNode* root, char block, char flatNo[]) {
    if (root == NULL) return NULL; 

    if (block < root->block) return searchResident(root->left, block, flatNo);
    else if (block > root->block) return searchResident(root->right, block, flatNo);
    else {
        int cmp = strcmp(flatNo, root->flatNo);
        if (cmp == 0) return root; 
        else if (cmp < 0) return searchResident(root->left, block, flatNo);
        else return searchResident(root->right, block, flatNo);
    }
}

// 3. DELETION
struct ResidentNode* findMinNode(struct ResidentNode* node) {
    struct ResidentNode* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

struct ResidentNode* deleteResident(struct ResidentNode* root, char block, char flatNo[]) {
    if (root == NULL) return root;

    if (block < root->block) {
        root->left = deleteResident(root->left, block, flatNo);
    } 
    else if (block > root->block) {
        root->right = deleteResident(root->right, block, flatNo);
    } 
    else {
        int cmp = strcmp(flatNo, root->flatNo);
        if (cmp < 0) {
            root->left = deleteResident(root->left, block, flatNo);
        } 
        else if (cmp > 0) {
            root->right = deleteResident(root->right, block, flatNo);
        } 
        else {
            // Match found! Handle 3 cases.
            if (root->left == NULL) {
                struct ResidentNode* temp = root->right;
                free(root);
                return temp;
            } 
            else if (root->right == NULL) {
                struct ResidentNode* temp = root->left;
                free(root);
                return temp;
            }

            struct ResidentNode* temp = findMinNode(root->right);
            root->block = temp->block;
            strcpy(root->flatNo, temp->flatNo);
            strcpy(root->info.name, temp->info.name);
            strcpy(root->info.phone, temp->info.phone);
            root->right = deleteResident(root->right, temp->block, temp->flatNo);
        }
    }
    return root;
}