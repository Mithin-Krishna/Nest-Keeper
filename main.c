#include <stdio.h>
#include <stdlib.h>
#include "headers/house.h"    
#include "headers/resident.h" 
#include "headers/payment.h"

int main() {
    setupBlocks(); 
    struct ResidentNode* root = NULL; // Prepare the empty Resident tree

    int choice;
    char block;
    char flatNo[10];
    int bhk;
    char name[50];
    char phone[15];

    printf("==================================================\n");
    printf("   WELCOME TO THE COMMUNITY MANAGEMENT SYSTEM\n");
    printf("==================================================\n");

    while (1) {
        printf("\n--- Main Menu ---\n");
        printf("1. Add a New Flat (Physical Building)\n");
        printf("2. Search for Empty Flats by BHK\n");
        printf("3. Add a New Resident (Move-in)\n");
        printf("4. Search for a Resident by Flat Number\n");
        printf("5. Delete a Resident (Move-out)\n");
        printf("6. View Full Resident Directory\n");
        printf("7. Add Flat to Payment Queue (Owes Money)\n");
        printf("8. Process Next Payment (Clear from Queue)\n");
        printf("9. View Pending Payment Queue\n");
        printf("10. Exit System\n");
        printf("Enter your choice (1-10): ");
        
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("\nEnter Block (A-E): ");
                scanf(" %c", &block); 
                printf("Enter Flat Number (e.g., 101): ");
                scanf("%s", flatNo);
                printf("Enter BHK (1, 2, or 3): ");
                scanf("%d", &bhk);
                
                addFlat(flatNo, block, bhk);
                break;

            case 2:
                printf("\nEnter Block to Search (A-E): ");
                scanf(" %c", &block);
                printf("Enter desired BHK: ");
                scanf("%d", &bhk);
                
                searchBHK(block, bhk);
                break;

            case 3:
                printf("\nEnter Block (A-E): ");
                scanf(" %c", &block);
                printf("Enter Flat Number: ");
                scanf("%s", flatNo);
                printf("Enter Resident Name: ");
                scanf(" %[^\n]s", name); 
                printf("Enter Phone Number: ");
                scanf("%s", phone);
                
                root = insertResident(root, block, flatNo, name, phone);
                printf("Successfully registered %s to Flat %s!\n", name, flatNo);
                break;

            case 4:
                printf("\nEnter Block (A-E): ");
                scanf(" %c", &block);
                printf("Enter Flat Number to Search: ");
                scanf("%s", flatNo);
                
                struct ResidentNode* found = searchResident(root, block, flatNo);
                if (found != NULL) {
                    printf("\n--- Resident Found ---\n");
                    printf("Name:  %s\n", found->info.name);
                    printf("Phone: %s\n", found->info.phone);
                } else {
                    printf("No resident found in Block %c, Flat %s.\n", block, flatNo);
                }
                break;

            case 5:
                printf("\nEnter Block (A-E): ");
                scanf(" %c", &block);
                printf("Enter Flat Number to Delete: ");
                scanf("%s", flatNo);
                root = deleteResident(root, block, flatNo);
                printf("If resident existed, they have been removed from the system.\n");
                break;

            case 6:
                printf("\n================ RESIDENT DIRECTORY ================\n");
                if (root == NULL) {
                    printf("The directory is currently empty.\n");
                } else {
                    inorderTraversal(root);
                }
                printf("====================================================\n");
                break;

                case 7:
                float amount;
                printf("\nEnter Block (A-E): ");
                scanf(" %c", &block);
                printf("Enter Flat Number: ");
                scanf("%s", flatNo);
                printf("Enter Amount Owed: ");
                scanf("%f", &amount);
                
                enqueue(block, flatNo, amount);
                break;

            case 8:
                printf("\nProcessing next payment in queue...\n");
                dequeue();
                break;

            case 9:
                displayPaymentQueue();
                break;

            case 10:
                printf("\nShutting down system. Cleaning up memory...\n");
                freeTree(root); 
                printf("Goodbye!\n");
                exit(0);

            default:
                printf("Invalid choice! Please select a number between 1 and 7.\n");
        }
    }
    return 0;
}