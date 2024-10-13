#ifndef UTILS__H
#define UTILS__H





// ---------------- DECLARATIONS ----------------

//useful
typedef unsigned short     ushr;
typedef unsigned int       uint;
typedef unsigned long long ulng;






// ---------------- FUNCTIONS ----------------

//IO
char* readFile(char* filename);
void  writeFile(char* filename, char* content, ulng contentLength);






#endif
