#include <iostream>
#include <string.h>
using namespace std;

class Block
{
public:
	int lent;
	unsigned char* content;
	Block* next[28];
	Block(void);
	~Block(void);
};

