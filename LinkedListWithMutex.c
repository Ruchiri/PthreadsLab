/*
* LinkedListWithMutex
*
* Compile: gcc -g -Wall -o LinkedListWithMutex LinkedListWithMutex.c
* Run : LinkedListWithMutex <n> <m> <threadCount> <mMember> <mIsert> <mDelete>
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
int sampleSize = 35;    // number of samples considered

// Fractions of each operations
float mMemberFrac, mInsertFrac, mDeleteFrac;

// total number of operations
int memberCount, insertCount, deleteCount;

// operations count
float mMember = 0;
float mInsert = 0;
float mDelete = 0;

struct list_node_s *head = NULL;
pthread_mutex_t mutex;

// node definition
struct list_node_s
{
    int data;
    struct list_node_s *next;
};

int member (int value, struct list_node_s* head_p);

int insert (int value, struct list_node_s** head_pp);

int delete (int value, struct list_node_s** head_pp);

void getArgs(int argc, char *argv[]);

void *threadExecute();

void deleteLinkedList (struct list_node_s** head_pp);

float calculateSTD (double data[], float mean);

int main (int argc, char *argv[])
{
    struct list_node_s *head = NULL;

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

        // Linked list generation with non-repeat random numbers
        for (int i =0; i < n; i++)
        {
            if (!insert(rand() % MAX_RANDOM_NUMBER, &head))
                i--;
        }

        // initializing the mutex
        pthread_mutex_init(&mutex, NULL);

        startTime = clock();

        // thread creation
        int k = 0;
        while (k < threadCount)
        {
            pthread_create (&threadHandler[k], NULL, (void *) threadExecute, NULL);
            k++;
        }

        // thread join 
        k = 0;
        while (k < threadCount)
        {
            pthread_join (threadHandler[k], NULL);
            k++;
        }

        endTime = clock();

        // destroying the mutex
        pthread_mutex_destroy(&mutex);

        timeArray[j] = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
        totalTime += timeArray[j];
        deleteLinkedList(&head);

        memberCount = 0;
        insertCount = 0;
        deleteCount = 0;

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
        printf("Enter LinkedListWithMutex <n> <m> <threadCount> <mMember> <mInsert> <mDelete> \n");
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

void* threadExecute()
{
    int totalCount = 0;
    while (totalCount < m)
    {
        int randomValue = rand() % MAX_RANDOM_NUMBER;   //generate random number for operations
        int randomOperation = rand() % 3;   //generate random operation type

        if (randomOperation == MEMBER)
        {
            pthread_mutex_lock(&mutex);
            if (memberCount < mMember){
                member(randomValue, head);
                memberCount++;
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (randomOperation == INSERT)
        {
            pthread_mutex_lock(&mutex);
            if (insertCount < mInsert){
                insert(randomValue, &head);
                insertCount++;
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (randomOperation == DELETE)
        {
            pthread_mutex_lock(&mutex);
            if (deleteCount < mDelete){
                insert(randomValue, &head);
                deleteCount++;
            }
            pthread_mutex_unlock(&mutex);
        }

        totalCount = memberCount + insertCount + deleteCount;
        
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