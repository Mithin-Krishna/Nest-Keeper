#ifndef RESIDENT_H
#define RESIDENT_H

struct ResidentData {
    char name[50];
    char phone[15];
};

struct ResidentNode {
    char block;                  
    char flatNo[10];             
    struct ResidentData info;   
    int height; 
    struct ResidentNode* left;   
    struct ResidentNode* right;  
};

struct ResidentNode* createNode(char block, char flatNo[], char name[], char phone[]);
struct ResidentNode* insertResident(struct ResidentNode* root, char block, char flatNo[], char name[], char phone[]);
struct ResidentNode* searchResident(struct ResidentNode* root, char block, char flatNo[]);
struct ResidentNode* deleteResident(struct ResidentNode* root, char block, char flatNo[]);
void inorderTraversal(struct ResidentNode* root);
void freeTree(struct ResidentNode* root);

#endif