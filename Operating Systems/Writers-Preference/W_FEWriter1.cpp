// Writer 1 - Writers Preference
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
	union semun
	{
		int val;
		struct semid_ds *buf;
		ushort array[0];
	} arg;

	semid = semget(SEMKEY, 4, 0777|IPC_CREAT);					// Sets semid for 4 semaphpores using SEMKEY

	arg.val = 1;												// Initial value of semaphores, will be used for all 4 semaphores here
	semctl(semid, 0, SETVAL, arg);  							// Initialises semaphore 0, Main semaphore to val
	semctl(semid, 1, SETVAL, arg);  							// Initialises semaphore 1, readerCount semaphore to val
	semctl(semid, 2, SETVAL, arg);  							// Initialises semaphore 2, writerCount semaphore to val
	semctl(semid, 3, SETVAL, arg);  							// Initialises semaphore 3, readerTry semaphore to val

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
	
	// Writing
	
	ofstream TestFile1;											// Create output stream
	string line;												
	
	shmid = shmget(SHM_KEY, 2, 0777|IPC_CREAT);					// Sets shmid for shared memory
	readerWriterCountPtr = (int*)shmat(shmid, 0, 0);			// Attaches shared memeory via shmid. Pointer points to beginning
	*(readerWriterCountPtr+1) = 0;								// Initialising ReaderCount to 0
	*(readerWriterCountPtr+2) = 0;								// Initialising WriterCount to 0
	
	while(1)
	{
		cout<<"Paused before down op...";						//
		pause = getchar();										// Pause for testing operation
		
		semop(semid, &p2sembuf, 1);								// Reserve the writer counter using the writerCount semaphore
		*(readerWriterCountPtr+2) = *(readerWriterCountPtr+2) + 1;		// Increment the writer counter
		if(*(readerWriterCountPtr+2) == 1)						// If first writer
		{
			semop(semid, &p3sembuf, 1);							// Reserve access using the readerTry semaphore
		}
		semop(semid, &v2sembuf, 1);								// Release the writer counter using the writerCount semaphore
		
		semop(semid, &p0sembuf, 1);								// Reserve the shared file using the main semaphore
		TestFile1.open("File1.txt", ios::out | ios::app);		// Opens the text file to output to, will append new entries
		cout<<"Enter Line: ";									
		getline(cin,line);										// Take in text (allows multiple words per line)
		TestFile1<<line<<"\n";									// Outputs string to text file
		TestFile1.close();										// Closes the output stream to the text file
		semop(semid, &v0sembuf, 1);								// Release the shared file using the main semaphore
		
		semop(semid, &p2sembuf, 1);								// Reserve the writer counter using the writerCount semaphore
		*(readerWriterCountPtr+2) = *(readerWriterCountPtr+2) - 1;		// Decrement the writer counter
		if(*(readerWriterCountPtr+2) == 0)						// If last writer
		{
			semop(semid, &v3sembuf, 1);							// Release access using the readerTry semaphore
		}
		semop(semid, &v2sembuf, 1);								// Release the writer counter using the writerCount semaphore
		
	}
	
}














