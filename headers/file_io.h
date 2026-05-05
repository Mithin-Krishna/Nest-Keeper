#ifndef FILE_IO_H
#define FILE_IO_H

#include "resident.h"

struct ResidentNode* loadAllData();
void appendTransaction(char block, char flatNo[], char name[], char phone[]);

#endif