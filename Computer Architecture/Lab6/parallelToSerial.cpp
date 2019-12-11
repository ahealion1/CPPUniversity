#include "stdafx.h"
#include "parallelToSerial.h"


parallelToSerial::parallelToSerial(void)
{
	reg = 0;
}

parallelToSerial::parallelToSerial(int v)
{
	if(v <= 15)
	{
		reg = v;
	}
	else
	{
		cout<<"Too big fool";
	}
}

parallelToSerial::~parallelToSerial(void)
{

}

parallelToSerial parallelToSerial::operator=(int v)
{
	reg = v;
	return *this;
}

parallelToSerial parallelToSerial::operator=(parallelToSerial obj)
{
	reg = obj.reg;
	return *this;
}

void parallelToSerial::operator+=(int i)
{
	reg = reg << 1;
	reg & 15;
	if(i ==1) reg |= 1;
}

void parallelToSerial::display()
{
	bitset<4> bits(reg);
	cout<< bits;
	cout<<endl;
}