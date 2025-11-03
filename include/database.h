#ifndef DATABASE_H 
#define DATABASE_H 

// our struct is defined here because out parser functions will be making use of it
typedef struct {
  int id;
  char name[50];
  char prog[50]; 
  float mark;
} StudentRecord;

// TODO: add something to store a collection of records + metadata
// TODO: add functions to interact with our collection of records 

#endif
