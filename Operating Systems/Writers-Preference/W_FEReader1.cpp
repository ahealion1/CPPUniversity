// Reader 1 - Writers Preference
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
#define SHM_KEY 4200											// Defines the SHM_KEY, shared with all programs

struct sembuf p0sembuf, v0sembuf, p1sembuf, v1sembuf, p2sembuf, v2sembuf, p3sembuf, v3sembuf;		// Creating sembuf objects
main()
{
	// Semaphore Setup
		
	int semid, pause, shmid, *readerWriterCountPtr;

	semid = semget(SEMKEY, 4, 0777|IPC_CREAT);					// Sets semid for 4 semaphpores using SEMKEY

	p0sembuf.sem_num = 0;										// Main semaphore. P is Down operation
	p0sembuf.sem_op = -1;  										// Defines the operation that this sembuf will perform
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
	
	p2sembuf.sem_num = 2;										// WriterCount semaphore. P is Down operation
	p2sembuf.sem_op = -1; 										
	p2sembuf.sem_flg = SEM_UNDO;								
	v2sembuf.sem_num = 2;										// WriterCount semaphore. V is Up operation
	v2sembuf.sem_op = 1; 										
	v2sembuf.sem_flg = SEM_UNDO;
	
	p3sembuf.sem_num = 3;										// ReaderTry semaphore. P is Down operation
	p3sembuf.sem_op = -1; 										
	p3sembuf.sem_flg = SEM_UNDO;								
	v3sembuf.sem_num = 3;										// ReaderTry semaphore. V is Up operation
	v3sembuf.sem_op = 1; 										
	v3sembuf.sem_flg = SEM_UNDO;
	
	// Reading
	
	ifstream TestFile1;											// Create input stream
	string line;
	
	shmid = shmget(SHM_KEY, 2, 0777|IPC_CREAT);					// Sets shmid for shared memory
	readerWriterCountPtr = (int*)shmat(shmid, 0, 0);			// Attaches shared memeory via shmid. Pointer points to beginning
	*(readerWriterCountPtr+1) = 0;								// Initialising ReaderCount to 0
	*(readerWriterCountPtr+2) = 0;								// Initialising WriterCount to 0
	
	while(1)
	{
		cout<<"Paused before down op...";						//
		pause = getchar();										// Pause for testing operation
		
		semop(semid, &p3sembuf, 1);								// Reserve access using the readerTry semaphore
		semop(semid, &p1sembuf, 1);								// Reserve the shared reader counter using the readerCount semaphore
		*(readerWriterCountPtr+1) = *(readerWriterCountPtr+1) + 1;		// Increment the reader counter
		if(*(readerWriterCountPtr+1) == 1)						// If first reader
		{
			semop(semid, &p0sembuf, 1);							// Reserve the shared file using the main semaphore (Down op)
		}
		semop(semid, &v1sembuf, 1);								// Releases the shared reader counter using the readerCount semaphore
		semop(semid, &v3sembuf, 1);								// Releases access using the readerTry semaphore
		
		TestFile1.open("File1.txt", ios::in);					// Opens the text file to input from
		if(TestFile1.is_open()) 								// If the file is open
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
				
		semop(semid, &p1sembuf, 1); 							// Reserve the shared reader counter using the readerCount semaphore
		*(readerWriterCountPtr+1) = *(readerWriterCountPtr+1) - 1;		// Decrement the reader counter
		if(*(readerWriterCountPtr+1) == 0)						// If last reader
		{
			semop(semid, &v0sembuf, 1);							// Release the shared file using the main semaphore (Up op)
		}
		semop(semid, &v1sembuf, 1);								// Releases the shared reader counter using the readerCount semaphore
		
	}
	
}














