// QueensMarch.cpp : This project finds all the possible solutions to the "N Queens problem" on a NxN chessboard
//and prints their number, then uses MPI to execute the code in parallel on multiple cores in order to speed it up
#define _USE_LEGACY_TWO_PHASE_LOOKUP

#include<iostream>
#include <mpi.h>
#include <chrono>

using namespace std;
const int maxSize = 16;
int grid[maxSize][maxSize];   //16 maximum chessboard size; can be larger but would take too long to execute anyway
int solutions = 0;

//prints a solution; unused
void print(int n) {
    for (int i = 0; i <= n - 1; i++) {

        for (int j = 0; j <= n - 1; j++) {

            cout << grid[i][j] << " ";

        }
        cout << endl;
    }
    cout << endl;
    cout << endl;
}

//function for checking if a position is safe or not
//row indicates the queen no. and col represents the possible positions on the row
bool isSafe(int col, int row, int n) {
    //check for same column
    for (int i = 0; i < row; i++) {

        if (grid[i][col]) {
            return false;
        }
    }
    //check for upper left diagonal
    for (int i = row, j = col; i >= 0 && j >= 0; i--, j--) {

        if (grid[i][j]) {
            return false;
        }
    }
    //check for upper right diagonal
    for (int i = row, j = col; i >= 0 && j < n; j++, i--) {

        if (grid[i][j]) {
            return false;
        }
    }
    return true;
}
//function to find the position for each queen
bool solve(int n, int row) {
    if (n == row) {
        solutions++; 
        return true;
    }

    //variable res is used for possible backtracking 
    bool res = false;
    for (int i = 0; i <= n - 1; i++) {

        if (isSafe(i, row, n)) {
            grid[row][i] = 1;
            //recursive call solve(n, row+1) for next queen (row+1)
            res = solve(n, row + 1) || res;//if res == false then backtracking will occur by assigning the grid[row][i] = 0       
            grid[row][i] = 0;   
        }
    }
    return res;
}
int main()
{
   
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    int n;
    int rank, size;

    MPI_Init(0, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {  //only the first process asks for input n and then it broadcasts it to everyone
        cout << "Enter the number of queens:" << endl;
        cin >> n;
        while (n > maxSize) {
            cout << "Number of queens should be lower than " << maxSize << ":" << endl;
            cin >> n;
        }
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //starting to count time; this returns a timepoint object
    auto start = std::chrono::high_resolution_clock::now();  

    for (int i = 0; i < n; i++) {    //draws the chessboard

        for (int j = 0; j < n; j++) {
            grid[i][j] = 0;
        }
    }

    for (int i = 0; i < n; i++) {

        if (rank == i % size) {  //destributes workload for processes, starts them off with one queen
            grid[0][i] = 1;
            bool res = solve(n, 1);
            grid[0][i] = 0;
        }
    }

    //sums up the solutions from all processes
    int globalSolutions = 0;
    MPI_Reduce(&solutions, &globalSolutions, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);  


    MPI_Finalize();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
 

    if (rank == 0) {      //the first process prints relevant information
        cout << "Duration: " << duration.count() << " ms" << endl;
        cout << "Number of solutions: " << globalSolutions << endl;

    }

    return 0;
}

 
 