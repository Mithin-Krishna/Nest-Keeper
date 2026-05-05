#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/resident.h"

/* =========================================================
   AVL TREE HELPERS (Height & Rotations)
   ========================================================= */

// Helper to get the height of a node
int getHeight(struct ResidentNode* node) {
    if (node == NULL) return 0;
    return node->height;
}

// Helper to get maximum of two integers
int getMax(int a, int b) {
    return (a > b) ? a : b;
}

// Helper to compare two flats (returns -1 if A < B, 1 if A > B, 0 if equal)
int compareFlats(char block1, char flatNo1[], char block2, char flatNo2[]) {
    if (block1 < block2) return -1;
    if (block1 > block2) return 1;
    return strcmp(flatNo1, flatNo2);
}

// Right Rotate (Balances Left-Heavy Trees)
struct ResidentNode* rightRotate(struct ResidentNode* y) {
    struct ResidentNode* x = y->left;
    struct ResidentNode* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = getMax(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = getMax(getHeight(x->left), getHeight(x->right)) + 1;

    // Return new root
    return x;
}

// Left Rotate (Balances Right-Heavy Trees)
struct ResidentNode* leftRotate(struct ResidentNode* x) {
    struct ResidentNode* y = x->right;
    struct ResidentNode* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    x->height = getMax(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = getMax(getHeight(y->left), getHeight(y->right)) + 1;

    // Return new root
    return y;
}

// Get Balance Factor
int getBalance(struct ResidentNode* node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

/* =========================================================
   0. CREATE NODE
   ========================================================= */
struct ResidentNode* createNode(char block, char flatNo[], char name[], char phone[]) {
    struct ResidentNode* newNode = (struct ResidentNode*)malloc(sizeof(struct ResidentNode));
    newNode->block = block;
    strcpy(newNode->flatNo, flatNo);
    strcpy(newNode->info.name, name);
    strcpy(newNode->info.phone, phone);
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->height = 1; // New nodes are added as leaves, height is 1
    return newNode;
}

/* =========================================================
   1. AVL INSERTION
   ========================================================= */
struct ResidentNode* insertResident(struct ResidentNode* root, char block, char flatNo[], char name[], char phone[]) {
    // 1. Standard BST Insertion
    if (root == NULL) return createNode(block, flatNo, name, phone);

    int cmp = compareFlats(block, flatNo, root->block, root->flatNo);
    if (cmp < 0) {
        root->left = insertResident(root->left, block, flatNo, name, phone);
    } else if (cmp > 0) {
        root->right = insertResident(root->right, block, flatNo, name, phone);
    } else {
        printf("Notice: Updating roommate %s to Block %c, Flat %s\n", name, block, flatNo);
        return root; // No duplicates allowed in this logic structure
    }

    // 2. Update Height of this ancestor node
    root->height = 1 + getMax(getHeight(root->left), getHeight(root->right));

    // 3. Get the balance factor to check if it became unbalanced
    int balance = getBalance(root);

    // 4. If unbalanced, apply the 4 rotation cases:
    
    // Left Left Case
    if (balance > 1 && compareFlats(block, flatNo, root->left->block, root->left->flatNo) < 0)
        return rightRotate(root);

    // Right Right Case
    if (balance < -1 && compareFlats(block, flatNo, root->right->block, root->right->flatNo) > 0)
        return leftRotate(root);

    // Left Right Case
    if (balance > 1 && compareFlats(block, flatNo, root->left->block, root->left->flatNo) > 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    // Right Left Case
    if (balance < -1 && compareFlats(block, flatNo, root->right->block, root->right->flatNo) < 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root; // Return the perfectly balanced node
}

/* =========================================================
   2. SEARCHING (Remains fast $O(\log N)$)
   ========================================================= */
struct ResidentNode* searchResident(struct ResidentNode* root, char block, char flatNo[]) {
    // Loop continues as long as we haven't hit an empty leaf
    while (root != NULL) {
        int cmp = compareFlats(block, flatNo, root->block, root->flatNo);
        
        if (cmp < 0) {
            root = root->left;  // Move left, no new memory allocated!
        } 
        else if (cmp > 0) {
            root = root->right; // Move right, no new memory allocated!
        } 
        else {
            return root;        // Match found!
        }
    }
    return NULL; // Flat not found in the database
}

/* =========================================================
   3. AVL DELETION
   ========================================================= */
struct ResidentNode* findMinNode(struct ResidentNode* node) {
    struct ResidentNode* current = node;
    while (current && current->left != NULL) current = current->left;
    return current;
}

struct ResidentNode* deleteResident(struct ResidentNode* root, char block, char flatNo[]) {
    // 1. Standard BST Deletion
    if (root == NULL) return root;

    int cmp = compareFlats(block, flatNo, root->block, root->flatNo);
    if (cmp < 0) {
        root->left = deleteResident(root->left, block, flatNo);
    } else if (cmp > 0) {
        root->right = deleteResident(root->right, block, flatNo);
    } else {
        // Node found!
        if ((root->left == NULL) || (root->right == NULL)) {
            struct ResidentNode* temp = root->left ? root->left : root->right;
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp; // Copy contents
            }
            free(temp);
        } else {
            struct ResidentNode* temp = findMinNode(root->right);
            root->block = temp->block;
            strcpy(root->flatNo, temp->flatNo);
            strcpy(root->info.name, temp->info.name);
            strcpy(root->info.phone, temp->info.phone);
            root->right = deleteResident(root->right, temp->block, temp->flatNo);
        }
    }

    if (root == NULL) return root;

    // 2. Update Height
    root->height = 1 + getMax(getHeight(root->left), getHeight(root->right));

    // 3. Get Balance Factor
    int balance = getBalance(root);

    // 4. Balance the tree (4 cases)
    if (balance > 1 && getBalance(root->left) >= 0) return rightRotate(root);
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0) return leftRotate(root);
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

/* =========================================================
   4. INORDER TRAVERSAL & 5. FREE TREE
   ========================================================= */
void inorderTraversal(struct ResidentNode* root) {
    if (root != NULL) {
        inorderTraversal(root->left);
        // Swapped the Balance factor back to the Phone Number
        printf("Block: %c | Flat: %-5s | Name: %-20s | Phone: %s\n", 
               root->block, root->flatNo, root->info.name, root->info.phone);
        inorderTraversal(root->right);
    }
}

void freeTree(struct ResidentNode* root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}