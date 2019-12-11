// Lab6.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	parallelToSerial port1;
	port1 = 7;
	port1.display();
	port1 += 1;
	port1.display();
	port1 = 67;
	port1.display();
	port1 += 51;
	port1.display();
	int wait=0;
	while(wait == 0)
	{
	}

	return 0;
}

