#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ResidentData {
    char name[50];
    char phone[15];
};

struct ResidentNode {
    char flatNo[10];             
    struct ResidentData info;    
    struct ResidentNode* left;   
    struct ResidentNode* right;  
};