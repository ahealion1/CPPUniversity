// Writer 2 - Readers Preference
// Alex Healion


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
using namespace std;

#define SEMKEY 7000												// Defines the SEMKEY, this will be the same for all 4 programs

struct sembuf p0sembuf, v0sembuf, p1sembuf, v1sembuf;			// Create objects of sembuf struct (built-in)
main()
{
	// Semaphore Setup
	
	int semid, pause;

	semid = semget(SEMKEY, 2, 0777|IPC_CREAT);					// Sets semid for 2 semaphpores using SEMKEY

	p0sembuf.sem_num = 0;										// Main semaphore. P is Down operation
	p0sembuf.sem_op = -1; 										// Defines the operation that this sembuf will perform
	p0sembuf.sem_flg = SEM_UNDO;								// Ensures that a semaphore isn't left 'down' if a process exits
	v0sembuf.sem_num = 0;										// Main semaphore. V is Up operation
	v0sembuf.sem_op = 1;
	v0sembuf.sem_flg = SEM_UNDO;
	
	p1sembuf.sem_num = 1;										// ReaderCount semaphore. P is Down operation
	p1sembuf.sem_op = -1; 
	p1sembuf.sem_flg = SEM_UNDO;
	v1sembuf.sem_num = 1;										// ReaderCount semaphore. V is Up operation
	v1sembuf.sem_op = 1;
	v1sembuf.sem_flg = SEM_UNDO;
	
	// Writing
	
	ofstream TestFile1;											// Create output stream
	string line;
	
	while(1)
	{
		cout<<"Paused before down op...";						//
		pause = getchar();										// Pause for testing operation
		
		semop(semid, &p0sembuf, 1);								// Reserve the shared file using the main semaphore (Down op)
		TestFile1.open("File1.txt", ios::out | ios::app);		// Opens the text file to output to, will append new entries
		cout<<"Enter Line: ";									
		getline(cin,line);										// Take in text (allows multiple words per line)
		TestFile1<<line<<"\n";									// Outputs string to text file
		TestFile1.close();										// Closes the output stream to the text file
		semop(semid, &v0sembuf, 1);								// Release the shared file using the main semaphore (Up op)
		
	}
	
}














