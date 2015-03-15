#include "Cache.h"


Cache::Cache(void)
{
}

void Cache::add(unsigned char*  key, unsigned char* value, int length)
{
	int i;
	Block* b;
	Block* toadd;
	b = &base;

	for(i = 0; key[i] != '\0'; i++)
	{
            if(b->next[code(key[i])] == NULL)
                b->next[code(key[i])] = new Block();
            b = b->next[code(key[i])];
	}

	b->lent = length;
        unsigned char* temp = new unsigned char[length];
	memcpy(temp,(void*)value,length);
        b->content = temp;
        i++;
}

Block* Cache::hit(unsigned char* key)
{
    int i;
    Block* b;
    Block* toadd;
    b = &base;

    for(i = 0; key[i] != '\0'; i++)
    {
        if(b->next[code(key[i])] == NULL)
            return NULL;
        b = b->next[code(key[i])];
    }
    return b;
}

Cache::~Cache(void)
{

}

int Cache::code(char x)
{
	if(x == '.')
		return 26;
	else if(x == '-')
		return 27;
	else
		return ((int)x)-97;
}

char Cache::decode(int x)
{

	if(x == 26)
		return '.';
	else if(x == 27)
		return '-';
	else
		return ((char)(x+97));
}

void Cache::load(int number,int type)
{

/*
    fstream in;
    int i,lent;
    char buffer[60000];
    unsigned char url[1000];

    for(i = 0; i < number; i++)
    {
        in.open("type"+((char)(type+48))+((char)(i+48))+".txt",fstream::in);
        in >> url >> lent;
        add((unsigned char*)url ,(unsigned char*)&buffer[0],lent);
        in.close();
    }

    in.close();
 */
}

void Cache::save(fstream& out)
{
    peyma("",&base,out);
}

void Cache::peyma(string s,Block* b,fstream& out)
{
	int i;
	if(b->content != NULL)
	{
		out << s << endl << b->lent << endl;
		for(i = 0; i < b->lent; i++)
			out << *((b->content)+i);
		out << endl;
	}

	for(i = 0; i < 28; i++)
		if(b->next[i] != NULL)
			peyma(s+(decode(i)),b->next[i],out);
}



