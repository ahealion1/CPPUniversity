#pragma once
#include "stdafx.h"

class parallelToSerial
{
	public:
		parallelToSerial(void);
		parallelToSerial(int);
		~parallelToSerial(void);
		parallelToSerial operator=(int);
		parallelToSerial operator=(parallelToSerial);
		void operator +=(int);
		void display();

	private:
		unsigned char reg;
};

