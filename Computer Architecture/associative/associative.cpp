// associative.cpp
// 
// Associative Cache

#include "stdafx.h"


struct cacheEntry
{
	unsigned int addr;															// 32 bit Tags
	bool validFlag;																// Valid Flag
	bool lruFlag;																// Least Recently Used Flag
	char data[4];																// 1 byte of Data, 4 bytes per Cache Line
	char DRAMdata;																// DRAM Data, 1 byte
};

void incCache(struct cacheEntry way0Inc[], int len)								// Function to increment the Addresses and Data in the cache
{
	for(int i = len;i>0;i--)													// Runs through way0 starting at the last line
	{
		way0Inc[i] = way0Inc[i-1];												// Sets a line equal to the previous line
	}
	way0Inc[0].addr = 0;														//
	way0Inc[0].data[0] = 0;														// Ensures that when top line is set to line above
	way0Inc[0].data[1] = 0;														// (containing garbage), that tags, data
	way0Inc[0].data[2] = 0;														// and valid flags are set to 0
	way0Inc[0].data[3] = 0;														//
	way0Inc[0].validFlag = 0;													//
}

void loadCache(struct cacheEntry way0Load[], int len, struct cacheEntry DRAMLoad[], unsigned int newAddr)
{																				// Function to load cache with new data from DRAM
	unsigned int newAddrModShort = (newAddr & 0xfffc);							// Shortcut to avoid having to create 4 billion lines in DRAM
																				// Ignores top half of address just for loading from DRAM
	way0Load[0].addr = newAddr;													// Sets Address at top of Tags to new Address
	way0Load[0].data[0] = DRAMLoad[newAddrModShort].DRAMdata;					//
	way0Load[0].data[1] = DRAMLoad[newAddrModShort+1].DRAMdata;					// Loads data in from DRAM but ensures that it is kept in groups
	way0Load[0].data[2] = DRAMLoad[newAddrModShort+2].DRAMdata;					// of 4 bytes. Also ensures that no set of 4 bytes are partially
	way0Load[0].data[3] = DRAMLoad[newAddrModShort+3].DRAMdata;					// overwritten
	way0Load[0].validFlag = 1;													// Valid flag is set to '1'
}

int main()
{
	struct cacheEntry way0[16384], DRAMDataStruct[65536];								// 16K Lines in Cache, 2 cacheEntry objects way0 and DRAM
	unsigned int cpuAddr, cpuAddrShort, DRAMdataLineInt, rowNo, addressesLength;		// 32 bit CPU Address as well as other variables
	int hitCount, missCount;															// Hit and Miss counters
	string addressLineString, DRAMDataLineString;										// String used to hold line read in from .txt file
	unsigned short cpuLower, cpuUpper, byteNo, setNo;									// 16 bit Addresses
	char dataRequested;																	// 1 byte of Data sent back to CPU
	
	int lCount = 0;
	ifstream testDRAMDataFile ("testDRAMData.txt");										// Uses ifstream to read line from testDRAMData.txt
	if (testDRAMDataFile.is_open())														// Ensures ifstream file is open
	{
		while (getline(testDRAMDataFile,DRAMDataLineString))							// Will cycle through .txt file returning each line as string
		{
			DRAMdataLineInt = stoi(DRAMDataLineString,nullptr,16);						// Converts string to integer using base 16
			DRAMDataStruct[lCount].DRAMdata = DRAMdataLineInt;							// Building array of DRAM Data
			lCount++;		
		}
		testDRAMDataFile.close();														// Closes ifstream file
	}
	
	cout<<"*******************************************************\n";
	cout<<" 32-bit CPU Address\t\t8-bit Data Bus\n 1-byte per unit\t\t4 units per Cache Line\n 16K Cache Lines/Sets\t\t64KB Cache\n";
	cout<<"*******************************************************\n\n";
	
	cout<<"Associative Cache Design\nTag No.\t  Set No.  Byte No. \tH/M \t  DRAM Line\t  Hits \t Misses    \n";		// Headings
	
	hitCount = 0;																		// Reset Hit
	missCount = 0;																		// and Miss counters
	addressesLength = 0;
	ifstream testAddressesFile ("testAddresses.txt");									// Same as previous, will grab CPU Addresses from .txt file
  	if (testAddressesFile.is_open())													// Ensures ifstream file is open
  	{
    	while (testAddressesFile.eof() != 1)											// Will cycle through .txt file passing each line
    	{
			testAddressesFile>>hex>>cpuAddr;											// Sets cpuAddr equal to testAddressesFile line, uses hex modifier for correct output
			cpuAddrShort = (cpuAddr & 0xffff);											// Same space-saving shortcut as previous, avoids having
																						// to create 4 billion lines of data in DRAM
																						
			cpuLower = (cpuAddr & 0x0000ffff);											// Get Lower Address from CPU Address, least significant 16 bits
			cpuUpper = ((cpuAddr & 0xffff0000)>>16);									// Get Upper Address from CPU Address, most significant 16 bits
			byteNo = (cpuLower & 3);													// Byte No. is 2 LSBs of Lower Address
			setNo = ((cpuLower & 0xfffc)>>2);											// Set No. is 14 MSBs of Lower Address
			
			cout<<hex<<cpuUpper;														// Prints Tag No.
			cout<<"\t  "<<setNo;														// Set No.
			cout<<"\t   "<<byteNo;														// And Byte No.
			
			for(int i = 0;i<addressesLength+1;i++)										// For loop acting as 32 bit comparators
			{																			// Checks if there is a match in addresses and if so on which row
				if(cpuAddr == way0[i].addr)												// Only runs through rows that have had data added to them
				{
					rowNo = i;
					break;																// Breaks For loop
				}
				else
					rowNo = 0;
			}
			
			if((cpuAddr == way0[rowNo].addr) && (way0[rowNo].validFlag == 1))					// True if CPU Address matches Tag (Comparator) and is Valid
			{
				cout<<"\t\tHit";																// Hit
				byteNo = (way0[rowNo].addr & 0x3);												// byteNo is 2 LSBs of Address
				dataRequested = way0[rowNo].data[byteNo];										// Data is on line 'rowNo', element 'byteNo'
				hitCount = hitCount + 1;														// Increment Hit counter
				cout<<"\t\t";
			}
			else																				// If no match between CPU Address and Tag...
			{
				addressesLength++;																// Increment length of Tag map
				cout<<"\t\tMiss";																// Miss
				dataRequested = DRAMDataStruct[cpuAddrShort].DRAMdata;							// Data is in DRAM on line 'cpuAddrShort'
				cacheEntry *way0Ptr, *DRAMPtr;													// Create pointers of type cacheEntry structs
				way0Ptr = way0;																	// Set pointers to
				DRAMPtr = DRAMDataStruct;														// beginning of structs
				incCache(way0Ptr, addressesLength);												// Run incCache function with way0 and length of Tag map
				loadCache(way0Ptr, addressesLength, DRAMPtr, cpuAddr);							// Run loadCache funct with above plus DRAM and CPU Address
				missCount = missCount + 1;														// Increment Miss counter
				cout<<"\t  "<<hex<<cpuAddr;														// Prints out DRAM Data Line, CPU address used as index
			}
		
			cout<<"\t  "<<hitCount<<"\t "<<missCount<<"\n";										// Prints out Hit and Miss counters
    	}
    	testAddressesFile.close();																// Closes ifstream file
  	}
	
	cout<<endl<<endl;
	system("pause");
	return 0;
}

