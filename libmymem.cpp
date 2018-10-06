#include "libmymem.hpp"

/*Initialises all the bucket pointers to NULL and checks the mutex locks if  they are NULL.*/
void initialize(void)
{
	for (int i = 0; i < 11; ++i)
	{
		hashBucket[i] = NULL;
		if (pthread_mutex_init(&locks[i], NULL) != 0)
   		{
        	perror("\n mutex init has failed\n");
        	exit(1);
    	}
	}
}

/*Finds the appropriate bucket for a given memory size as,log2(memory) - 1 */

int findBucket(const int size)
{
	if (size < 1 || size > 8192)
	{
		return -1;
	}

	if (size <= 4)
	{
		return 0;
	}
	else 
	{
		return 1 + findBucket(size/2 + size%2);
	}
}

/*Deletes the provided slab*/
int delSlab(struct slab* ptr)
{
	if (ptr == NULL)
	{
		perror("Delete function called with NULL pointer");
		return 1;
	}

	if (ptr->prevSlab == NULL)
	{
		hashBucket[ptr->bucket] = ptr->nxtSlab;
		if (ptr->nxtSlab != NULL)
		{
			ptr->nxtSlab->prevSlab = NULL;
		}
		munmap(ptr,64*1024);
	}
	else
	{
		ptr->prevSlab->nxtSlab = ptr->nxtSlab;
		if (ptr->nxtSlab != NULL)
		{
			ptr->nxtSlab->prevSlab = ptr->prevSlab;
		}
		munmap(ptr,64*1024);
	}

	return 0;
}

/*Generates and returns a new slab*/
struct slab* genSlab(const int bucket)
{
	void *memoryChunck; 
	if ((memoryChunck = mmap(NULL, 64*1024, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	struct slab* newSlab = (struct slab*)memoryChunck;

	int fragamentSize = pow(2,bucket+2);

	int totObj = (64*1024 - sizeof(struct slab))/(9 + fragamentSize);

	long long temp3 = (long long int)newSlab;

	newSlab->totObj  = totObj;
	newSlab->freeObj = totObj;
	newSlab->fragamentSize = fragamentSize;
	newSlab->memStartLocation = (void *)(temp3 + totObj + sizeof(struct slab));
	newSlab->bucket = bucket;
	newSlab->nxtSlab = NULL;
	newSlab->prevSlab = NULL;

	long long int* temp;
	long long int temp2 = (long long int)newSlab->memStartLocation;

	for (int i = 0; i < totObj; ++i)
	{
		newSlab->bitmap[i] = false;
		// (newSlab->memStartLocation + i*(8 + fragamentSize)) = newSlab;
		temp = (long long int*) (temp2 + i*(8+fragamentSize));
		*temp = (long long int)newSlab;

		if (temp == NULL)
		{
			perror("Unable to set slab pointer value");
			exit(1);
		}
	}


	return newSlab;
}

/*Returns slab to return pointer from*/
struct slab* getSlab(const int bucket)
{
	if (hashBucket[bucket] == NULL)
	{
		hashBucket[bucket] = genSlab(bucket);
		return hashBucket[bucket];
	}

	struct slab* ptr = hashBucket[bucket];

	while(ptr != NULL){
		if (ptr->freeObj > 0)
		{
			break;
		}
		else
		{ 
			if (ptr->nxtSlab != NULL)
			{
				ptr = ptr->nxtSlab; 
			}
			else
			{
				ptr->nxtSlab = genSlab(bucket);
				ptr->nxtSlab->prevSlab = ptr;
				ptr = ptr->nxtSlab;
			}
		}
	}

	return ptr;
}

/*Fetches the memory location to be returned to the user for the asked memory fragment.*/
void* getReturnPtr(struct slab* ptr)
{
	if (ptr == NULL)
	{
		perror("Requested Memoery from NULL Slab");
		exit(1);
	}

	int location = 0;

	for (; location < ptr->totObj; ++location)
	{
		if (!ptr->bitmap[location])
		{
			break;
		}
	}

	ptr->bitmap[location] = true;
	ptr->freeObj--;

	long long int temp = (long long int)ptr->memStartLocation;

	void* result = (void *)(temp + 8 + location*(8 + ptr->fragamentSize));

	return result;
}

/*Takes the size of memory to be returned as input and goes through the hashBucket,finds the memory slab,memory fragment,locks the critical section and returns the memory location of the allocated memory fragment.*/
void* mymalloc(const int size)
{
	if (!initialized)
	{
		initialize();
		initialized = true;
	}

	int bucket = findBucket(size);
	if (bucket == -1)
	{
		perror("Memory request is out of range.");
		exit(1);
	}

	pthread_mutex_lock(&locks[bucket]);
	struct slab* curSlab = getSlab(bucket);

	void* result = getReturnPtr(curSlab);
	pthread_mutex_unlock(&locks[bucket]);
	return result;
}

/*Finds the slab and hence bucket of the memory fragment and deallocates its memory.if the whole slab is deallocated,removes the slab from the linked list while locking the critical section.*/
void myfree(void *ptr)
{
	if (ptr == NULL)
	{
		perror("Illegal request of myfree!");
		exit(1);
	}

	long long int *temp1 = (long long int *)ptr;
	temp1 = (temp1 - 1);
	struct slab *curSlab = (struct slab*)*temp1;

	int bucket = curSlab->bucket;
	
	pthread_mutex_lock(&locks[bucket]);

	long long int temp2 = (long long int)curSlab->memStartLocation;
	long long int location = (long long int)((long long int )ptr - temp2 - 8);
	long long int temp3 = (8 + curSlab->fragamentSize);

	location = location/temp3;

	curSlab->bitmap[location] = false;
	curSlab->freeObj++;

	if (curSlab->freeObj == curSlab->totObj)
	{
		delSlab(curSlab);
	}

	pthread_mutex_unlock(&locks[bucket]);
}

