// Reader 1 - Readers Preference
// Alex Healion

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
using namespace std;

#define SEMKEY 7000												// Defines the SEMKEY, this will be the same for all 4 programs
#define SHM_KEY 4200											// Defines the SHM_KEY, shared with readers

struct sembuf p0sembuf, v0sembuf, p1sembuf, v1sembuf;			// Create objects of sembuf struct (built-in)
main()
{
	// Semaphore Setup

	int semid, pause, shmid, *readerCountPtr;

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

	// Reading

	ifstream TestFile1;											// Create input stream
	string line;

	shmid = shmget(SHM_KEY, 1, 0777|IPC_CREAT);					// Sets shmid to shared memory initialised in Writer1
	readerCountPtr = (int*)shmat(shmid, 0, 0);					// Attaches shared memeory via shmid. Pointer initialised to 0 in Writer1

	while(1)
	{
		cout<<"Paused before down op...";						//
		pause = getchar();										// Pause for testing operation
		
		semop(semid, &p1sembuf, 1);								// Reserve the shared reader counter using the readerCount semaphore
		*(readerCountPtr+1) = *(readerCountPtr+1) + 1;			// Increments the reader counter
		if(*(readerCountPtr+1) == 1)							// If first reader
		{
			semop(semid, &p0sembuf, 1);							// Reserve the shared file using the main semaphore (Down op)
		}
		semop(semid, &v1sembuf, 1);								// Releases the shared reader counter using the readerCount semaphore
		
		TestFile1.open("File1.txt", ios::in);					// Opens the text file to input from
		if(TestFile1.is_open())									// If the file is open
		{
			while(getline(TestFile1,line))						// Reads the text file line by line
			{
				cout<<"Line: "<<line<<endl;						// Prints text
				pause = getchar();								// Pause after each line
			}
		}
		else
		{
			cout<<"Error...File is not open\n";
		}
		TestFile1.close();										// Closes the input stream from the text file
				
		semop(semid, &p1sembuf, 1);								// Reserve the shared reader counter using the readerCount semaphore
		*(readerCountPtr+1) = *(readerCountPtr+1) - 1;			// Decrements the reader counter
		if(*(readerCountPtr+1) == 0)							// If last reader
		{
			semop(semid, &v0sembuf, 1);							// Release the shared file using the main semaphore (Up op)
		}
		semop(semid, &v1sembuf, 1);								// Releases the shared reader counter using the readerCount semaphore
		
	}

}














