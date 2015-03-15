#include <iostream>
#include <string.h>
#include <fstream>
#include "Block.h"
using namespace std;

class Cache
{
public:
	Block base;
	Cache(void);
	void add(unsigned char*  key, unsigned char* value, int length);
	Block* hit(unsigned char*  key);
	int code(char x);
	void load(int num,int type);
	void save(fstream& out);
	void peyma(string s,Block* b,fstream& out);
	char decode(int x);
	void loadPeyma(unsigned char* &s, int i,Block* b);
	~Cache(void);
};

