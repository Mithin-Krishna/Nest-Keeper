#ifndef HOUSE_H
#define HOUSE_H

struct Flat {
    char flatNo[10];      
    char block;           
    int bhk;              
    int status;          
    struct Flat* next;    
};

extern struct Flat* communityBlocks[5];

void setupBlocks();
void addFlat(char flatNo[], char block, int bhk);
void searchBHK(char targetBlock, int desiredBhk);

int doesFlatExist(char block, char flatNo[]);
void updateFlatStatus(char block, char flatNo[], int newStatus);

#endif