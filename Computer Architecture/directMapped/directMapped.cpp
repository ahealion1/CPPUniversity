// directMapped.cpp
// 
// Direct Mapped Cache

#include "stdafx.h"


struct cacheEntry
{
	unsigned short upperAddr;													// 16 bit Tags
	bool validFlag;																// Valid Flag
	bool lruFlag;																// Least Recently Used Flag
	char data[4];																// 1 byte of Data, 4 bytes per Cache Line
	char DRAMdata;																// DRAM Data, 1 byte
};

int main()
{
	struct cacheEntry way0[16384], DRAMDataStruct[65536];						// 16K Lines in Cache, 2 cacheEntry objects way0 and DRAM
	unsigned int cpuAddr, DRAMdataLineInt;										// 32 bit CPU Address
	int hitCount, missCount;													// Hit and Miss counters
	string addressLineString, DRAMDataLineString;								// Strings used to hold line read in from .txt file
	unsigned short cpuLower, cpuUpper, byteNo, setNo, cpuLowerMod;				// 16 bit Addresses
	char dataRequested;															// 1 byte of Data sent back to CPU
		
	int lCount = 0;
	ifstream testDRAMDataFile ("testDRAMData.txt");								// Uses ifstream to read line from testDRAMData.txt
	if (testDRAMDataFile.is_open())												// Ensures ifstream file is open
	{
		while (getline(testDRAMDataFile,DRAMDataLineString))					// Will cycle through .txt file returning each line as string
		{
			DRAMdataLineInt = stoi(DRAMDataLineString,nullptr,16);				// Converts string to integer using base 16
			DRAMDataStruct[lCount].DRAMdata = DRAMdataLineInt;					// Building array of DRAM Data
			lCount++;		
		}
		testDRAMDataFile.close();												// Closes ifstream file
	}
	
	cout<<"*******************************************************\n";
	cout<<" 32-bit CPU Address\t\t8-bit Data Bus\n 1-byte per unit\t\t4 units per Cache Line\n 16K Cache Lines/Sets\t\t64KB Cache\n";
	cout<<"*******************************************************\n\n";
		
	cout<<"Direct-Map Cache Design\nTag No.\t  Set No.  Byte No. \tH/M \t  DRAM Line\t  Hits \t Misses    \n";		// Headings
	
	hitCount = 0;																// Reset Hit
	missCount = 0;																// and Miss counters
	ifstream testAddressesFile ("testAddresses.txt");							// Same as previous, will grab CPU Addresses from .txt file
  	if (testAddressesFile.is_open())											// Ensures ifstream file is open
  	{
    	while (testAddressesFile.eof() != 1)									// Will cycle through .txt file passing each line
    	{
			testAddressesFile>>hex>>cpuAddr;									// Sets cpuAddr equal to testAddressesFile line, uses hex modifier for correct output
						
			cpuLower = (cpuAddr & 0x0000ffff);									// Get Lower Address from CPU Address, least significant 16 bits
			cpuUpper = ((cpuAddr & 0xffff0000)>>16);							// Get Upper Address from CPU Address, most significant 16 bits
			byteNo = (cpuLower & 3);											// Byte No. is 2 LSBs of Lower Address
			setNo = ((cpuLower & 0xfffc)>>2);									// Set No. is 14 MSBs of Lower Address
			
			cout<<hex<<cpuUpper;												// Prints Tag No.
			cout<<"\t  "<<setNo;												// Set No.
			cout<<"\t   "<<byteNo;												// And Byte No.
						
			if((cpuUpper == way0[setNo].upperAddr) && (way0[setNo].validFlag == 1))			// True if Upper Address from CPU matches Tag (Comparator) and is Valid
			{
				cout<<"\t\tHit";															// Hit
				dataRequested = way0[setNo].data[byteNo];									// Data is on line 'setNo', element 'byteNo'
				hitCount = hitCount + 1;													// Increment Hit counter
				cout<<"\t\t";
			}
			else																			// If no match between CPU Address and tag...
			{
				cout<<"\t\tMiss";															// Miss
				dataRequested = DRAMDataStruct[cpuLower].DRAMdata;							// Data is read from DRAM on line 'cpuLower'
				way0[setNo].upperAddr = cpuUpper;											// Add cpuUpper Address to Tags
				cpuLowerMod = (setNo<<2);													// cpuLowerMod is cpuLower with last two bits (byteNo) = '00'
				way0[setNo].data[0] = DRAMDataStruct[cpuLowerMod].DRAMdata;					// DRAM Data is then put into cache but it is kept
				way0[setNo].data[1] = DRAMDataStruct[cpuLowerMod+1].DRAMdata;				// as a group of 4 bytes
				way0[setNo].data[2] = DRAMDataStruct[cpuLowerMod+2].DRAMdata;				// e.g. if DRAM Data is on line xxxxxx10, then 00, 01,
				way0[setNo].data[3] = DRAMDataStruct[cpuLowerMod+3].DRAMdata;				// 10 and 11 are all put into cache on appropriate line (setNo)
				way0[setNo].validFlag = 1;													// Valid flag is set to '1'
				missCount = missCount + 1;													// Increment Miss counter
				cout<<"\t  "<<hex<<cpuAddr;													// Prints out DRAM Data Line, CPU address used as index
			}
			
			cout<<"\t  "<<hitCount<<"\t "<<missCount<<"\n";									// Prints out Hit and Miss counters
			
    	}
    	testAddressesFile.close();															// Closes ifstream file
  	}
	
	cout<<endl<<endl;
	system("pause");
	return 0;
}

