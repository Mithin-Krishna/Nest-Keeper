#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/resident.h"

int getHeight(struct ResidentNode* node) {
    if (node == NULL) return 0;
    return node->height;
}

int getMax(int a, int b) {
    return (a > b) ? a : b;
}

int compareFlats(char block1, char flatNo1[], char block2, char flatNo2[]) {
    if (block1 < block2) return -1;
    if (block1 > block2) return 1;
    return strcmp(flatNo1, flatNo2);
}

struct ResidentNode* rightRotate(struct ResidentNode* y) {
    struct ResidentNode* x = y->left;
    struct ResidentNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = getMax(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = getMax(getHeight(x->left), getHeight(x->right)) + 1;

    return x;
}

struct ResidentNode* leftRotate(struct ResidentNode* x) {
    struct ResidentNode* y = x->right;
    struct ResidentNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = getMax(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = getMax(getHeight(y->left), getHeight(y->right)) + 1;

    return y;
}

int getBalance(struct ResidentNode* node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

struct ResidentNode* createNode(char block, char flatNo[], char name[], char phone[]) {
    struct ResidentNode* newNode = (struct ResidentNode*)malloc(sizeof(struct ResidentNode));
    newNode->block = block;
    strcpy(newNode->flatNo, flatNo);
    strcpy(newNode->info.name, name);
    strcpy(newNode->info.phone, phone);
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->height = 1;
    return newNode;
}

struct ResidentNode* insertResident(struct ResidentNode* root, char block, char flatNo[], char name[], char phone[]) {
    if (root == NULL) return createNode(block, flatNo, name, phone);
    int cmp = compareFlats(block, flatNo, root->block, root->flatNo);
    if (cmp < 0) {
        root->left = insertResident(root->left, block, flatNo, name, phone);
    } else if (cmp > 0) {
        root->right = insertResident(root->right, block, flatNo, name, phone);
    } else {
        return root; 
    }
    root->height = 1 + getMax(getHeight(root->left), getHeight(root->right));
    int balance = getBalance(root);
    if (balance > 1 && compareFlats(block, flatNo, root->left->block, root->left->flatNo) < 0)
        return rightRotate(root);
    if (balance < -1 && compareFlats(block, flatNo, root->right->block, root->right->flatNo) > 0)
        return leftRotate(root);
    if (balance > 1 && compareFlats(block, flatNo, root->left->block, root->left->flatNo) > 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && compareFlats(block, flatNo, root->right->block, root->right->flatNo) < 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    return root; 
}

struct ResidentNode* searchResident(struct ResidentNode* root, char block, char flatNo[]) {
    while (root != NULL) {
        int cmp = compareFlats(block, flatNo, root->block, root->flatNo);
        
        if (cmp < 0) {
            root = root->left;  
        } 
        else if (cmp > 0) {
            root = root->right; 
        } 
        else {
            return root;        
        }
    }
    return NULL; 
}

struct ResidentNode* findMinNode(struct ResidentNode* node) {
    struct ResidentNode* current = node;
    while (current && current->left != NULL) current = current->left;
    return current;
}

struct ResidentNode* deleteResident(struct ResidentNode* root, char block, char flatNo[]) {
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
                *root = *temp; 
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

    root->height = 1 + getMax(getHeight(root->left), getHeight(root->right));

    int balance = getBalance(root);

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

void inorderTraversal(struct ResidentNode* root) {
    if (root != NULL) {
        inorderTraversal(root->left);
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