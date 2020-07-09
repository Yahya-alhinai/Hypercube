#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 
#include <mpi.h>
#include <math.h>

void merge (  int n, int m, int *a, int *b, int *c);

int HQuicksort(int *Abuf, int *lenA,  int *list, int N, MPI_Comm comm) {
  int myid;
  int nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  if (nprocs == 1) return 0;
  else if (!(nprocs == 2 || nprocs == 4 || nprocs == 8 || nprocs == 16 || nprocs == 32) ) return 1;

  MPI_Request request = MPI_REQUEST_NULL;
  MPI_Status status;
  int recv_N;
  int NN = *lenA;
  int * MIN;
  int * MAX;
  int MIN_N;
  int MAX_N;
  int *A;
  int *c;
  int k;
  int sendTo;
  int q;
  
  //The following array is to indicates which pivot index associated with which process and the level of hypercube
  int sel [4][16] = {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                      {1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2},
                      {3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6},
                      {7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14}};
  int step = 16/nprocs;

  int j =0;
  for (int i = nprocs/2; i > 0; i = i/2)
  {
    // Find The pivot index
    k = 0;
    q = sel[j][myid * step];
    while (Abuf[k] <= list[q] && k < NN) k++;
    

    //Splits the sorted array into two arrays (Low array, High array) using the pivot index 
    MIN_N = k;
    MAX_N = NN-k;
    MIN = (int *) malloc(MIN_N*sizeof(int));
    MAX = (int *) malloc(MAX_N*sizeof(int));
    memcpy(MIN, Abuf, MIN_N*sizeof(int));
    memcpy(MAX, &Abuf[k], MAX_N*sizeof(int));

    //Send the selected array to the inteded 
    sendTo = (myid ^ i);
    if (myid < sendTo) MPI_Isend(MAX, MAX_N, MPI_INT, sendTo, 0, comm, &request); 
    else MPI_Isend(MIN, MIN_N, MPI_INT, sendTo, 0, comm, &request);

    //Get the size of the array sent to this processor
    MPI_Probe(sendTo, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &recv_N);

    //revceve the array that send to this processor with its prober size
    A = (int *) malloc(recv_N*sizeof(int));
    MPI_Irecv(A, recv_N, MPI_INT, sendTo, 0, comm, &request);
    MPI_Wait(&request, &status);
    
    //The folowing part is to merge the part receved with the other array that was not sent
    if (myid < sendTo)
    {
      NN = MIN_N+recv_N;
      c = (int *) malloc(NN*sizeof(int));
      merge(recv_N, MIN_N, A, MIN, c);
    }
    else
    {
      NN = MAX_N+recv_N;
      c = (int *) malloc(NN*sizeof(int));
      merge(recv_N, MAX_N, A, MAX, c);
    }

    //Set the new array with its new size
    memcpy(Abuf,c,NN*sizeof(int));
    *lenA = NN;
    j++;

  }

  free(c);
  free(A);
  free(MIN);
  free(MAX);

  return(0);
}
