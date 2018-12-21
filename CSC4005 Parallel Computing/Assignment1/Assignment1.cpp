#include <cstdio>
#include <mpi.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#define maxSize 100000

using namespace std;

//Functions designed for this program
int MPI_OddEven_Sort(int n, int *a, int root, MPI_Comm comm);
void print_state(int rank, int *local_a, int localSize);


int main(int argc, char **argv) {
    /* Get the input data from the terminal*/
    // int n = argc-1;
    // int a[n];
    // for (int i=0; i<n; i++) a[i] = atof(argv[i+1]);

    //Needed when during the testing of randomly generated numbers
    // int n=maxSize;
    // int a[maxSize];

    /* Read Input File to Get Data */
    string data; //Store data in every line
    int number;
    int a[maxSize];

    ifstream infile;
    infile.open(argv[argc-1]);
    int n=0;
    while(!infile.eof()){
      getline(infile,data);
      stringstream stringin(data);
      while (stringin >> a[n++]);
      n--;
    }
    infile.close();

    /* starts MPI */
    MPI_Init(&argc, &argv);

    MPI_OddEven_Sort(n, a, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}


int MPI_OddEven_Sort(int n, int *a, int root, MPI_Comm comm)
{
    /* Record Execution Time */
    double beginTime, endTime;
    beginTime = MPI_Wtime();

    int rank, numProcessor, i;
    int *right; //Mark the value of the rightmost floating number in each process
    int *left; //Mark the value of the leftmost floating number in each process
    int *local_a;
    int *sizePt; //Pointer of subarray size
    int localSize; //Store the array size for each process
    int offset=0;
    int *offsetPt;
    int evenRank;
    int oddRank;
    int startIndex; //Mark the position of the first value in each processor with regard to the input 
    int endIndex; //Mark the position of the last value in each processor with regard to the input 

//Get rank and size of comm
    MPI_Comm_rank(comm, &rank); //&rank = address of rank
    MPI_Comm_size(comm, &numProcessor);

      /* To Generate Random Number for Testing */
      // if (rank==0){
      //   cout << "To sort "<<maxSize << " of numbers."<<endl;
      //   for (int i=0;i<maxSize;i++){
      //     a[i]=rand();
      //   }
      // }

//The case when there's only one processor
    if (numProcessor ==1){
      printf("Name: Wenbo Luo\n");
      printf("ID: 115010054\n");

      printf("Sequence: ");
      for (int i=0;i<n;i++) {
        cout<<a[i]<<" ";
      }
      cout << endl;
      int sort_finish_single=0;
      int odd_even_single =1;
      while (sort_finish_single!=1){
        sort_finish_single=1;
        for (int i=odd_even_single;i<n-1;i=i+2){
          if (a[i]>a[i+1]){
            sort_finish_single=0;
            swap(a[i],a[i+1]);
          }
        }
        if (sort_finish_single==1){
          cout << "Rank 0 outputs: ";
          for (int i=0;i<n;i++){
            cout << a[i]<<" ";
          }
          cout << endl;
          cout << "Final: ";
          for (int i=0;i<n;i++){
            cout << a[i] <<" ";
          }
          cout << endl;
        }
        odd_even_single = !odd_even_single;
      }
      endTime = MPI_Wtime();
      if (rank == 0) cout << "Execution Time: " <<endTime-beginTime<<endl;
      return MPI_SUCCESS;
    }

    if (rank ==0){
      printf("Name: Wenbo Luo\n");
      printf("ID: 115010054\n");

      printf("Sequence: ");
      for (int i=0;i<n;i++) {
        cout<<a[i]<<" ";
      }
      cout << endl;
    }

  sizePt = (int *) calloc(numProcessor, sizeof(int)); 
  offsetPt = (int *) calloc(numProcessor, sizeof(int)); 
  for (int i=0;i<numProcessor;i++){
    if (rank ==i && i!=numProcessor-1) {
      localSize = n/numProcessor;
      offset = i * localSize;
    }
    if (rank ==i && i==numProcessor-1){
      int numLeft = n%numProcessor;
      int temp = n/numProcessor;
      localSize = numLeft+temp;
      offset = i*temp;
    }
    int normal_size = n/numProcessor;
    if (i!=numProcessor-1) sizePt[i] = normal_size;
    else sizePt[i] = normal_size+(n%numProcessor);
    offsetPt[i] = i*normal_size;
    local_a = (int *) calloc(localSize, sizeof(int));
  }

// scatter the array a to local_a to distrubte tasks to each process
   MPI_Scatterv(a,sizePt,offsetPt, MPI_INT, local_a, localSize, MPI_INT,
        root, comm);

//Number the odd ranks and the even ranks
  if (rank%2==0){
    left = &local_a[0]; //Mark the leftmost value
    right = &local_a[localSize-1]; //Mark the rightmost value
    startIndex=offsetPt[rank];
    endIndex=offsetPt[rank]+sizePt[rank]-1;
    oddRank=rank+1;
    evenRank=rank-1;
  }
  else{
    left = &local_a[0]; //Mark the leftmost value
    right = &local_a[localSize-1]; //Mark the rightmost value
    startIndex=offsetPt[rank];
    endIndex=offsetPt[rank]+sizePt[rank]-1;
    oddRank=rank-1;
    evenRank=rank+1;
  }

  int sort_finish=0; //If sort_finish == 1, indicating that the array is already sorted
  int odd_even=1;
  while(sort_finish!=1){
    sort_finish=1;
    int buffer;

    if (odd_even ==1){
        //Odd sort
        //For the processors in the middle
        if (rank!=0 && rank != numProcessor-1){
          if (endIndex%2==1) MPI_Send(right,1,MPI_INT, rank+1, 3, MPI_COMM_WORLD);
          if (startIndex%2==0) MPI_Send(left,1, MPI_INT, rank-1, 4, MPI_COMM_WORLD);
          if (endIndex%2==1){
            MPI_Recv(&buffer,1,MPI_INT, rank+1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (*right>buffer){
              *right=buffer;
              sort_finish=0;
            }
          }
          if (startIndex%2==0){
            MPI_Recv(&buffer,1,MPI_INT, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (*left<buffer){
              *left=buffer;
              sort_finish=0;
            }
          }
        }
        //For the first processsor
        if (rank==0 && endIndex%2==1){
          MPI_Send(right,1,MPI_INT, rank+1, 3, MPI_COMM_WORLD);
          MPI_Recv(&buffer, 1, MPI_INT, rank+1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          if (*right>buffer){
            *right=buffer;
            sort_finish=0;
          }
        }
        //For the last processor
        if (rank == numProcessor-1 && startIndex%2 ==0){
          MPI_Send(left, 1, MPI_INT, rank-1, 4, MPI_COMM_WORLD);
          MPI_Recv(&buffer, 1, MPI_INT, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          if (*left<buffer){
            *left=buffer;
            sort_finish=0;
          }
        }
      }
      //Even sort
      else {
        if (rank!=0 && rank!=numProcessor-1){
          if (endIndex%2==1) MPI_Send(right,1,MPI_INT, rank+1, 3, MPI_COMM_WORLD);
          if (startIndex%2==0) MPI_Send(left,1, MPI_INT, rank-1, 4, MPI_COMM_WORLD);
          if (endIndex%2==0){
            MPI_Recv(&buffer,1, MPI_INT, rank+1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (*right>buffer){
              *right=buffer;
              sort_finish=0;
            }
          }
          if (startIndex%2==1){
            MPI_Recv(&buffer,1, MPI_INT, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (*left<buffer){
              *left=buffer;
              sort_finish=0;
            }
          }
        }
        if (rank==0 && endIndex%2==0){
          MPI_Send(right,1,MPI_INT, rank+1, 3, MPI_COMM_WORLD);
          MPI_Recv(&buffer, 1, MPI_INT, rank+1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          if (*right>buffer){
            *right = buffer;
            sort_finish=0;
          }

        }
        if (rank ==numProcessor-1 && startIndex%2==1){
          MPI_Send(left, 1, MPI_INT, rank-1, 4, MPI_COMM_WORLD);
          MPI_Recv(&buffer, 1, MPI_INT, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          if (*left<buffer){
            *left = buffer;
            sort_finish=0;
          }
        }
      }

      //Sort inside processors
      for (int i=odd_even;i<localSize-1;i=i+2){
        if (local_a[i]>local_a[i+1]){
          swap(local_a[i],local_a[i+1]);
          sort_finish=0;
        }
      }

      //Broadcase between process to see if the array is sorted
      if (rank==0){
        int other_condition;
        for (int i=1;i<numProcessor;i++){
          MPI_Recv(&other_condition, 1, MPI_INT, i, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          sort_finish=sort_finish && other_condition;
        }
        for (int i=1;i<numProcessor;i++) MPI_Send(&sort_finish,1, MPI_INT, i, 6, MPI_COMM_WORLD);
      }
      else MPI_Send(&sort_finish,1, MPI_INT, 0, 5, MPI_COMM_WORLD);

      if (rank!=0) MPI_Recv(&sort_finish,1, MPI_INT, 0, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // gather local_a to a
      if (sort_finish==1) {
        //Print the output for each rank separately
        for (int i=0;i<numProcessor;i++){
          if (rank==i){
            print_state(rank,local_a,localSize);
          }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gatherv(local_a, localSize, MPI_INT, a, sizePt, offsetPt, MPI_INT,
             root, MPI_COMM_WORLD);
        //Use rank 0 to output the result

        if (rank==0) {
          printf("Final: ");
          for (int i=0;i<n;i++){
            cout<< a[i] <<" ";
          }
          cout<<endl;
        }
      }
      odd_even = !odd_even; //change between odd-even and even-odd
  }
    MPI_Barrier(MPI_COMM_WORLD);
    endTime = MPI_Wtime();
    if (rank == 0) cout << "Execution Time: " <<endTime-beginTime<<endl;
    return MPI_SUCCESS;
}

//Print the output for each rank
void print_state(int rank, int *local_a, int localSize){
  printf("Rank %d outputs: ",rank);
  for (int i=0;i<localSize;i++){
            cout << local_a[i]<<" ";
  }
  cout << endl;
}