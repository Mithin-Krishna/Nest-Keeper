#ifndef FILE_IO_H
#define FILE_IO_H
#include "resident.h" 

void saveAllData(struct ResidentNode* root);
struct ResidentNode* loadAllData(struct ResidentNode* root);

#endif