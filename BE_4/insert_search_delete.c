#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Insertion
struct ResidentNode* insertResident(struct ResidentNode* root, char flatNo[], char name[], char phone[]) \
{
    if (root == NULL) 
    {
        struct ResidentNode* newNode = (struct ResidentNode*)malloc(sizeof(struct ResidentNode));
        strcpy(newNode->flatNo, flatNo);
        strcpy(newNode->info.name, name);
        strcpy(newNode->info.phone, phone);
        newNode->left = NULL;
        newNode->right = NULL;
        return newNode;
    }

    int compareResult = strcmp(flatNo, root->flatNo);

    if (compareResult < 0) 
    {
        root->left = insertResident(root->left, flatNo, name, phone);
    } 
    else if (compareResult > 0) 
    {
        root->right = insertResident(root->right, flatNo, name, phone);
    }
    else 
    {
        printf("Error: Resident already exists in Flat %s!\n", flatNo);
    }
    return root;
}

//Searching
struct ResidentNode* searchResident(struct ResidentNode* root, char flatNo[]) {
    if (root == NULL) 
    {
        return NULL; 
    }

    int compareResult = strcmp(flatNo, root->flatNo);

    //Match
    if (compareResult == 0) 
    {
        return root;
    }
    //Search Left
    else if (compareResult < 0) 
    {
        return searchResident(root->left, flatNo);
    }
    //Search Right
    else 
    {
        return searchResident(root->right, flatNo);
    }
}

//Deletion
struct ResidentNode* findMinNode(struct ResidentNode* node) 
{
    struct ResidentNode* current = node;
    while (current && current->left != NULL) 
    {
        current = current->left;
    }
    return current;
}

struct ResidentNode* deleteResident(struct ResidentNode* root, char flatNo[]) 
{
    if (root == NULL) return root;
    int compareResult = strcmp(flatNo, root->flatNo);

    if (compareResult < 0) 
    {
        root->left = deleteResident(root->left, flatNo);
    } 
    else if (compareResult > 0) 
    {
        root->right = deleteResident(root->right, flatNo);
    } 
    else 
    {
        //Right child or No child
        if (root->left == NULL)
        {
            struct ResidentNode* temp = root->right;
            free(root); 
            return temp;
        } 

        //Left child
        else if (root->right == NULL) 
        {
            struct ResidentNode* temp = root->left;
            free(root);
            return temp;
        }

        //Two children
        struct ResidentNode* temp = findMinNode(root->right);
        strcpy(root->flatNo, temp->flatNo);
        strcpy(root->info.name, temp->info.name);
        strcpy(root->info.phone, temp->info.phone);
        root->right = deleteResident(root->right, temp->flatNo);
    }
    return root;
}