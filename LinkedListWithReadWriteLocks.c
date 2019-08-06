/*
* LinkedListWithReadWriteLocks
*
* Compile: gcc -g -Wall -o LinkedListWithReadWriteLocks LinkedListWithReadWriteLocks.c
* Run : LinkedListWithReadWriteLocks <n> <m> <threadCount> <mMember> <mIsert> <mDelete>
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#define MAX_RANDOM_NUMBER 65535
#define MAX_THREAD_COUNT 1024
#define MEMBER 0
#define INSERT 1
#define DELETE 2

int n; // number of nodes in the linked list
int m; // number of random operations in the linked list
int threadCount = 0;
int sampleSize = 65;    // number of samples considered

// Fractions of each operations
float mMemberFrac, mInsertFrac, mDeleteFrac;

// operations count
float mMember = 0;
float mInsert = 0;
float mDelete = 0;

struct list_node_s *head = NULL;
pthread_rwlock_t rwlock;

// node definition
struct list_node_s
{
    int data;
    struct list_node_s *next;
};

int member (int value, struct list_node_s* head_p);

int insert (int value, struct list_node_s** head_pp);

int delete (int value, struct list_node_s** head_pp);

void getArgs (int argc, char *argv[]);

void *threadExecute (void *id);

void deleteLinkedList (struct list_node_s** head_pp);

float calculateSTD (double data[], float mean);

int generateLocalNumberOfOperations (int noOfOperations, int threadCount, int id);
 
int main (int argc, char *argv[])
{
    double timeArray[sampleSize];  // store total time taken for each sample
    double totalTime = 0.0;
    float mean= 0.0;
    float std = 0.0;

    getArgs(argc, argv);
    mMember = mMemberFrac * m;
    mInsert = mInsertFrac * m;
    mDelete = mDeleteFrac * m;

    for (int j = 0; j < sampleSize; j++)
    {
        pthread_t* threadHandler = malloc(sizeof(pthread_t)* threadCount);
        clock_t startTime, endTime;

        int *threadID = (int *)malloc(sizeof(int) * threadCount);

        // Linked list generation with non-repeating random numbers
        int i = 0;
        for (; i < n; i++)
        {
            if (!insert(rand() % MAX_RANDOM_NUMBER, &head))
                i--;
        }

        //initializing read-write lock
        pthread_rwlock_init(&rwlock, NULL);

        startTime = clock();

        // thread creation
        i = 0;
        while (i < threadCount)
        {
            threadID[i] = i;
            pthread_create(&threadHandler[i], NULL, (void *) threadExecute, (void *) &threadID[i]);
            i++;
        }

        // thread join 
        i = 0;
        while (i < threadCount)
        {
            pthread_join(threadHandler[i], NULL);
            i++;
        }

        endTime = clock();

        // destroying the read-write lock
        pthread_rwlock_destroy(&rwlock);

        timeArray[j] = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
        totalTime += timeArray[j];
        deleteLinkedList(&head);

    }

    mean = totalTime / sampleSize;  // mean time calculation
    std = calculateSTD(timeArray, mean);    // std calculation

    printf ("Mean : %f\n", mean);
    printf ("STD : %f\n", std);

    return 0;
    
}

int member (int value, struct list_node_s* head_p)
{
    struct list_node_s* curr_p = head_p;

    while (curr_p != NULL && curr_p->data < value)
        curr_p = curr_p->next; 
    
    if (curr_p == NULL || curr_p->data > value)
    {
        return 0;
    }
    else
    {
       return 1; 
    }
    
};

int insert (int value, struct list_node_s** head_pp)
{
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;

    while (curr_p != NULL && curr_p->data <value)
    {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data >value)
    {
        temp_p = malloc(sizeof(struct list_node_s));
        temp_p->data = value;
        temp_p->next = curr_p;

        if (pred_p == NULL)
            *head_pp = temp_p;
        else
            pred_p->next = temp_p;
        
        return 1;
    }
    else
    {
        return 0;
    }    
};

int delete (int value, struct list_node_s** head_pp)
{
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;

    while (curr_p != NULL && curr_p->data < value)
    {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value)
    {
        if (pred_p == NULL)
        {
            *head_pp = curr_p->next;
            free(curr_p);
        }
        else
        {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;   
    }
    else
    {
        return 0;
    }  
};

void getArgs (int argc, char *argv[])
{
    if(argc != 7)
    {
        printf("Enter LinkedListWithReadWriteLocks <n> <m> <threadCount> <mMember> <mInsert> <mDelete> \n");
        exit(0);
    }

    n = (int) strtol(argv[1], (char **) NULL, 10);
    m = (int) strtol(argv[2], (char **) NULL, 10);
    threadCount = (int) strtol(argv[3], (char **) NULL, 10);

    mMemberFrac = (float) atof(argv[4]);
    mInsertFrac = (float) atof(argv[5]);
    mDeleteFrac = (float) atof(argv[6]);

    // validating thread count
    if (threadCount <= 0 || threadCount > MAX_THREAD_COUNT)
    {
        printf ("Thread count is incorrect \n");
        exit(0);
    }

    // arg validation
    if (n <= 0 || m <= 0 || mMemberFrac + mInsertFrac + mDeleteFrac != 1.0)
    {
        printf ("Please give the command with the arguments: ./serial_linked list <n> <m> <mMember> <mInsert> <mDelete> \n");

        if (n <= 0)
            printf ("Value you entered for n is incorrect! \n");
        
        if (m <= 0)
            printf ("Value you entered for n is incorrect! \n");

        if (mMemberFrac + mInsertFrac + mDeleteFrac != 1.0)
            printf ("mMember + mInsert + mDelete should equals to 1 \n");
        
        exit(0);
    }
};

void* threadExecute(void* thread_id)
{
    int id = *(int *) thread_id;

    // generate local no of member operationswithout loss
    int localMemberCount = generateLocalNumberOfOperations(mMember, threadCount, id);
    int localInsertCount = generateLocalNumberOfOperations(mInsert, threadCount, id);
    int localDeleteCount = generateLocalNumberOfOperations(mDelete, threadCount, id);

    int local_m = localMemberCount + localInsertCount + localDeleteCount;

    int totalCount = 0;
    int memberCount = 0;
    int insertCount = 0;
    int deleteCount = 0;

    int i = 0;
    while(totalCount < local_m)
    {
        int randomValue = rand() % MAX_RANDOM_NUMBER;   //generate random number for operations
        int randomOperation = rand() % 3;  //generate random operation type

        if(randomOperation == MEMBER)
        {
            if (memberCount < localMemberCount)
            {
                pthread_rwlock_rdlock(&rwlock);
                member(randomValue, head);
                memberCount++;
                pthread_rwlock_unlock(&rwlock);
            }   
        }
        else if(randomOperation == INSERT)
        {
            if (insertCount < localInsertCount)
            {
                pthread_rwlock_rdlock(&rwlock);
                insert(randomValue, &head);
                insertCount++;
                pthread_rwlock_unlock(&rwlock);
            }    
        }
        else if(randomOperation == DELETE)
        {
            if (deleteCount < localDeleteCount)
            {
                pthread_rwlock_rdlock(&rwlock);
                delete(randomValue, &head);
                deleteCount++;
                pthread_rwlock_unlock(&rwlock);
            }
        }

        totalCount = memberCount + insertCount + deleteCount;
        i++;
    }
    return NULL;
}

float calculateSTD (double data[], float mean)
{
    float standardDeviation = 0.0; 

    for (int i = 0; i < sampleSize; ++i)
        standardDeviation += pow(data[i] - mean, 2);

    return sqrt(standardDeviation / sampleSize);
}

void deleteLinkedList (struct list_node_s** head_pp)
{ 
   struct list_node_s* current = *head_pp; 
   struct list_node_s* next; 
  
   while (current != NULL)  
   { 
       next = current->next; 
       free(current); 
       current = next; 
   } 
   
   *head_pp = NULL; 
}

int generateLocalNumberOfOperations (int noOfOperations, int threadCount, int id)
{
    int localOperationCount = 0;

    if (noOfOperations % threadCount == 0|| noOfOperations % threadCount <= id)
    {
        localOperationCount = noOfOperations / threadCount;
    }
    else if (noOfOperations % threadCount > id)
    {
        localOperationCount = noOfOperations / threadCount + 1;
    }

    return localOperationCount;
}
