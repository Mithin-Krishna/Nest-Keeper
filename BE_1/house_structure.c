#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Flat {
    char flatNo[10];      
    char block;           
    int bhk;              
    int status;          
    struct Flat* next;    
};

struct Flat* communityBlocks[5];
void setupBlocks() 
{
    for (int i = 0; i < 5; i++) 
    {
        communityBlocks[i] = NULL;
    }
}

void addFlat(char flatNo[], char block, int bhk) 
{
    struct Flat* newFlat = (struct Flat*)malloc(sizeof(struct Flat));

    strcpy(newFlat->flatNo, flatNo);
    newFlat->block = block;
    newFlat->bhk = bhk;
    newFlat->status = 0;
        
    int index = 0;
    if (block == 'A') 
    { 
        index = 0; 
    }
    else if (block == 'B') 
    { 
        index = 1; 
    }
    else if (block == 'C') 
    {  
        index = 2; 
    }
    else if (block == 'D') 
    { 
        index = 3; 
    }
    else if (block == 'E') 
    {   
        index = 4; 
    }
    else 
    {
        printf("Invalid Block!\n");
        return;
    }
    
    newFlat->next = communityBlocks[index];
    communityBlocks[index] = newFlat;
    
    printf("Successfully added Flat %s to Block %c!\n", flatNo, block);
}