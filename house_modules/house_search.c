#include <stdio.h>
#include <string.h>
#include "../headers/house.h"

void searchBHK(char targetBlock, int desiredBhk) 
{
    int index = 0;
    if (targetBlock == 'A') 
        index = 0;
    else if (targetBlock == 'B') 
        index = 1;
    else if (targetBlock == 'C') 
        index = 2;
    else if (targetBlock == 'D') 
        index = 3;
    else if (targetBlock == 'E') 
        index = 4;
    else 
    {
        printf("Invalid Block!\n");
        return;
    }
    struct Flat* current = communityBlocks[index];
    int found = 0;
    printf("\n--- Available %d BHK Flats in Block %c ---\n", desiredBhk, targetBlock);
    while (current != NULL) 
    {
        if (current->bhk == desiredBhk && current->status == 0) 
        {
            printf("-> Flat No: %s is AVAILABLE\n", current->flatNo);
            found = 1;
        }
        current = current->next;
    }
    if (found == 0) 
    {
        printf("Sorry, no %d BHK flats available in this block.\n", desiredBhk);
    }
}