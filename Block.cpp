#include "Block.h"


Block::Block(void)
{
	int i;
	for(i = 0; i < 28; i++)
		next[i] = NULL;
	content = NULL;
}


Block::~Block(void)
{
}
