#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

struct ResidentNode {
    char flatNo[10];
    char block;
    char name[50];
    struct ResidentNode* left;
    struct ResidentNode* right;
};

struct ResidentNode* createNode(char flatNo[], char block, char name[]) {
    struct ResidentNode* newNode = (struct ResidentNode*)malloc(sizeof(struct ResidentNode));
    
    strcpy(newNode->flatNo, flatNo);
    newNode->block = block;
    strcpy(newNode->name, name);

    newNode->left = NULL;
    newNode->right = NULL;

    return newNode;
}

struct ResidentNode* insert(struct ResidentNode* root, char flatNo[], char block, char name[]) {
    if (root == NULL) {
        return createNode(flatNo, block, name);
    }
    if (block < root->block) {
        root->left = insert(root->left, flatNo, block, name);
    }
    else if (block > root->block) {
        root->right = insert(root->right, flatNo, block, name);
    }
    else {
        int cmp = strcmp(flatNo, root->flatNo);
        if (cmp < 0) {
            root->left = insert(root->left, flatNo, block, name);
        }
        else if (cmp > 0) {
            root->right = insert(root->right, flatNo, block, name);
        }
        else {
            printf("Notice: Adding roommate %s to Block %c, Flat %s\n", name, block, flatNo);
            root->right = insert(root->right, flatNo, block, name);
        }
    }
    return root;
}

void inorderTraversal(struct ResidentNode* root) {
    if (root == NULL)
        return;
    inorderTraversal(root->left);
    printf("Block: %c | Flat: %s | Resident: %s\n",
           root->block, root->flatNo, root->name);
    inorderTraversal(root->right);
}

void freeTree(struct ResidentNode* root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root); // Destroy the physical box
}

int main() {
    struct ResidentNode* root = NULL;

    printf("--- Populating Tree ---\n");
    root = insert(root, "203", 'B', "Ravi");
    root = insert(root, "101", 'A', "Anu");
    root = insert(root, "301", 'C', "Kiran");
    root = insert(root, "102", 'A', "Meena");
    root = insert(root, "201", 'B', "Sita");
    
    root = insert(root, "101", 'A', "Rahul"); 

    printf("\n--- Sorted Residents ---\n");
    inorderTraversal(root);

    freeTree(root);
    return 0;
}