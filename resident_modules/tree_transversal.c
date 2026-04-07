#include <stdio.h>
#include <stdlib.h> 
#include "../headers/resident.h"

void inorderTraversal(struct ResidentNode* root) {
    if (root == NULL) return;
    
    inorderTraversal(root->left);
    
    printf("Block: %c | Flat: %s | Resident: %s | Phone: %s\n",
           root->block, root->flatNo, root->info.name, root->info.phone);
           
    inorderTraversal(root->right);
}

void freeTree(struct ResidentNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}