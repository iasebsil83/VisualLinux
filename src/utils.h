#ifndef UTILS__H
#define UTILS__H





// ---------------- DECLARATIONS ----------------

//useful
typedef unsigned short     ushr;
typedef unsigned int       uint;
typedef unsigned long long ulng;
typedef struct {
	char* data;
	ulng  length;
} buffer;






// ---------------- FUNCTIONS ----------------

//ednianness
bool isBigEndian();

//IO
void freeBuffer(buffer* content);
buffer* readFile(char* filename);
void  writeFile(char* filename, buffer* content);






#endif
