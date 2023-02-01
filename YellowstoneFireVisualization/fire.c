//
// fire.c
// Forest Fire Sim - Stage 4 - Fire Mapping
// Created by Dr. Carl Albing and Michael Opheim on 1/15/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <ctype.h> // for toupper() method

// Goal: grid[switch][colNum][rowNum]; 2 grids for swapping generations
char **grid[2];

// How big is the whole sim
int simSize;

// Hold dimensions of the grid (including the halo)
int haloSize;

// Dimensions of the grid without the halo
int gridSize;

int rank; // MPI rank
int world; // MPI world size
int nSize; // Square root of world

// Text file(s) to be created
FILE *fptr;

/*
 * Create both grids: [0] and [1]
 */
void initGrid() {
    
    int gridID, col;

    // Allocate memory for the grid
    for(gridID = 0; gridID <= 1 ; gridID++) {
        grid[gridID] = calloc((size_t)haloSize, sizeof(char *));
        for (col = 0; col < haloSize; col++) {
            grid[gridID][col] = calloc((size_t)haloSize, sizeof(char));
        }
    }
}

/*
 * Copy the grid between grids [0] and [1] based on user input
 */
void copyGrid(int src, int dest)  {
    for (int col = 0; col < haloSize; col++) {
        for (int row = 0; row < haloSize; row++) {
            grid[dest][col][row] = grid[src][col][row];
        }
    }
}

/*
 * makeForest  - Fill the grid with data
 *     density - the forest density
 *     fireProbability - the likelihood that a fire occurs at a specific cell
 *
 * Remember: it's a square grid, so start/end are same for x and y
 */
void makeForest(double density, double fireProbability) {
    
    // A variable to store a random double
    double randomDouble;
    
    // A variable to store tree age
    int treeAge;
    
    for (int i = 1; i < gridSize + 1; i++) {
        for (int j = 1; j < gridSize + 1; j++) {
            randomDouble = ((double)rand())/RAND_MAX;
            
            // Start adding trees to the grid at the designated density
            if (randomDouble <= density) {
                treeAge = (rand() % 5) + 1; // Bias towards old growth
                grid[0][i][j] = ((treeAge > 3) ? 3 : treeAge);
                
                // Check if there's a fire
                if (randomDouble <= fireProbability) {
                    
                    // If there is a fire, make the cell's tree value negative
                    grid[0][i][j] = -grid[0][i][j];
                }
            } else {
                grid[0][i][j] = 0; // Cells without trees are set to 0
            }
        }
    }
}

/*
 * Send RHS data to right neighbor
 */
void sendRight() {
    
    // Make sure there is a right neighbor
    if (rank % nSize != (nSize - 1)) {
        
        // Send an array of data
        char buffer[gridSize];
        for (int i = 0; i < gridSize; i++) {
            buffer[i] = grid[1][gridSize][i + 1];
        }
        MPI_Send(buffer, gridSize, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD);
    }
}

/*
 * Get LHS halo data from the message traveling right
 */
void recvRight() {
    
    // Make sure there is a right neighbor
    if ((rank % nSize) != 0) {
        
        // Receive and implement an array of data
        char buffer[gridSize];
        MPI_Recv(buffer, gridSize, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < gridSize; i++) {
            grid[1][0][i + 1] = buffer[i];
        }
    }
}

/*
 * Send LHS data to the left neighbor
 */
void sendLeft() {
    
    // Make sure there is a left neighbor
    if (rank % nSize != 0) {
        
        // Send an array of data
        char buffer[gridSize];
        for (int i = 0; i < gridSize; i++) {
            buffer[i] = grid[1][1][i + 1];
        }
        MPI_Send(buffer, gridSize, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD);
    }
}

/*
 * Get RHS halo data from the message traveling left
 */
void recvLeft() {
    
    // Make sure there is a left neighbor
    if ((rank % nSize) != nSize - 1) {
        
        // Receive and implement an array of data
        char buffer[gridSize];
        MPI_Recv(buffer, gridSize, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < gridSize; i++) {
            grid[1][gridSize + 1][i + 1] = buffer[i];
        }
    }
}

/*
 * The right/left halo exchanges for all nodes
 */
void rightLeftExch() {
    
    if ((rank & 1) == 0) { // even ranks
        sendRight();
        recvLeft();
        recvRight();
        sendLeft();
        
    } else { // odd ranks
        recvRight();
        sendLeft();
        sendRight();
        recvLeft();
    }
}

/*
 * Send bottom data to the bottom neighbor
 */
void sendDown() {
    
    // Make sure there is a bottom neighbor
    if (rank + nSize < world) {
        
        // Send an array of data
        char buffer[haloSize];
        for (int i = 0; i < haloSize; i++) {
            buffer[i] = grid[1][i][gridSize];
        }
        MPI_Send(buffer, haloSize, MPI_CHAR, rank + nSize, 0, MPI_COMM_WORLD);
    }
}

/*
 * Get upper halo data from message traveling down
 */
void recvDown() {
    
    // Make sure there is a bottom neighbor
    if (rank >= nSize) {
        
        // Receive and implement an array of data
        char buffer[haloSize];
        MPI_Recv(buffer, haloSize, MPI_CHAR, rank - nSize, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < haloSize; i++) {
            grid[1][i][0] = buffer[i];
        }
    }
}

/*
 * Send upper data to the upper neighbor
 */
void sendUp() {
    
    // Make sure there is an upper neighbor
    if (rank - nSize >= 0) {
        
        // Send an array of data
        char buffer[haloSize];
        for (int i = 0; i < haloSize; i++) {
            buffer[i] = grid[1][i][1];
        }
        MPI_Send(buffer, haloSize, MPI_CHAR, rank - nSize, 0, MPI_COMM_WORLD);
    }
}

/*
 * Get lower halo data from message traveling up
 */
void recvUp() {
    
    // Make sure there is an upper neighbor
    if (world - rank > nSize) {
        
        // Receive and implement an array of data
        char buffer[haloSize];
        MPI_Recv(buffer, haloSize, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < haloSize; i++) {
            grid[1][i][haloSize - 1] = buffer[i];
        }
    }
}

/*
 * The down/up halo exchanges for all nodes
 */
void downUpExch() {
    
    if (((rank / nSize) & 1) == 0) {  // even ranks
        sendDown();
        recvUp();
        recvDown();
        sendUp();
        
    } else { // odd ranks
        recvDown();
        sendUp();
        sendDown();
        recvUp();
    }
}

/*
 * Perform all necessary halo exchanges
 */
void haloExchange() {
    rightLeftExch();
    downUpExch();
}

/*
 * Method that changes the probabilities of fire spreading based on the winds present and determines whether a fire actually spreads
 */
int windCheck(char increaseDirection, char decreaseDirection, int row, int col, char windDirection, float windSpeed){
    
    // Create probability checking variables
    double randomDouble =((double)rand())/RAND_MAX;
    double threshold = 0.33;
    
    // If the wind direction points to the cell, increase the probability
    if (windDirection == increaseDirection) {
        threshold += windSpeed;
        
    // If the wind direction doesn't point to the cell, decrease the probability
    } else if (windDirection == decreaseDirection) {
        threshold -= windSpeed;
    }
    
    // Check if the fire spreads
    if (randomDouble < threshold) {
        grid[1][col][row] = -grid[0][col][row]; // The fire spreads!
        return 1;
    }
    return 0; // The fire does not spread
}

/*
 * Method that checks whether a cell's neighbors are on fire
 */
int neighborCheck(int col, int row, char windDirection, float windSpeed) {
    
    // Keep track of whether the fire was spread
    int onFire = 0;
    
    // East check
    if (grid[0][col + 1][row] < 0) { // If there is a fire in the east...
        onFire = windCheck('W', 'E', row, col, windDirection, windSpeed);
    }
    
    // West check
    if (grid[0][col - 1][row] < 0 && onFire == 0) { // If there is a fire in the west and the fire hasn't already spread to this cell...
        onFire = windCheck('E', 'W', row, col, windDirection, windSpeed);
    }
    
    // North check (Note: North and South orientations are backwards on the grids)
    if (grid[0][col][row - 1] < 0 && onFire == 0) { // If there is a fire in the north and the fire hasn't already spread to this cell...
        onFire = windCheck('S', 'N', row, col, windDirection, windSpeed);
    }
    
    // South check
    if (grid[0][col][row + 1] < 0 && onFire == 0) { // If there is a fire in the south and the fire hasn't already spread to this cell...
        onFire = windCheck('N', 'S', row, col, windDirection, windSpeed);
    }
    
    return onFire;
}

/*
 * Method that writes grid data to a text file
 */
void writeData(int gen, int fprintfreq) {
    
    // Write to a text file based on the user's desired text file writing frequency
    if (gen % fprintfreq == 0) {
        
        // Create grids for message passing
        char tempGrid[gridSize * gridSize];
        char *unsortedGrid = NULL;
        if (rank == 0) {
            unsortedGrid = (char *)malloc(gridSize * gridSize * sizeof(char));
        }
        
        // Add rank data to temporary grids
        for (int col = 1; col < haloSize - 1; col++) {
            for (int row = 1; row < haloSize - 1; row++) {
                tempGrid[(col - 1) * gridSize + (row - 1)] = grid[1][row][col];
            }
        }
        
        // Collect all the rank data
        MPI_Gather(tempGrid, gridSize * gridSize, MPI_CHAR, unsortedGrid, gridSize * gridSize, MPI_CHAR, 0, MPI_COMM_WORLD);
        
        // Recreate the overall grid from all the rank data
        if (rank == 0) {
            
            int sortedGrid[simSize][simSize];
            int endOfGridLine = simSize; // Row length check
            int endOfGridBlock = gridSize; // Grid 'block' (nSize) check
            int endOfCurrentArrayRank = gridSize - 1; // Rank check
            int offset = 0;
            int startOfCurrentLineOffset = 0;
            
            for (int row = 0; row < simSize; row++) {
                for (int col = 0; col < simSize; col++) {
                    
                    sortedGrid[row][col] = 0;
                    
                    if (endOfGridLine == 0) { // We are going to a new line
                        endOfGridBlock--;
                        offset = 0;
                        if(endOfGridBlock == 0){ // If we are at the end of a grid 'block' (i.e. at nSize)
                            endOfGridBlock = gridSize;
                            startOfCurrentLineOffset += nSize - 1;
                        }
                        
                        endOfCurrentArrayRank = gridSize - 1;
                        endOfGridLine = simSize; // Reset number of lines to go through
                    }
                    
                    // Add element to the sorted grid
                    sortedGrid[row][col] = unsortedGrid[(row * gridSize + col)+ (offset + gridSize * gridSize * startOfCurrentLineOffset)];
                    
                    // Reset offset value if necessary
                    if (endOfCurrentArrayRank == 0){
                        endOfCurrentArrayRank = gridSize - 1;
                        offset = gridSize * gridSize - gridSize;
                    }
                    
                    // Keep track of where the pointer is at in the 1D array
                    endOfGridLine--;
                    endOfCurrentArrayRank--;
                }
            }
            
            // Format string for text file
            ssize_t bufSize = snprintf(NULL, 0, "ADD PATH HERE...", gen);
            char* title = malloc(bufSize + 1);
            snprintf(title, bufSize + 1, "ADD PATH HERE...", gen);

            // Create text file for a generation
            fptr = fopen(title, "w");
            for (int col = 0; col < simSize; col++) {
                for (int row = 0; row < simSize; row++) {
                    fprintf(fptr, "%d ", sortedGrid[col][row]);
                }
                fprintf(fptr, "\n");
            }

            fclose(fptr);
            free(title);
            free(unsortedGrid);
        }
    }
}

/*
 * Iterative method for fire spread over time
 */
void nextGen(char windDirection, float windSpeed, int fprintfreq) {
    
    // Keep track of changes in the grid
    int changing;
    int gen = 0;
    
    while(1) {
        
        // Make a text file with the data
        writeData(gen, fprintfreq);
        
        gen++;
        
        // Reset 'changing' variable
        changing = 0;
        
        // Make changes to the grid (allow the fire to spread)
        for (int i = 1; i < gridSize + 1; i++) {
            for (int j = 1; j < gridSize + 1; j++) {
                
                // If the cell is on fire, decrease the magnitude of the fire
                if (grid[0][i][j] < 0) {
                    grid[1][i][j]++;
                    changing = 1;
                    
                // If the cell has a tree that's not on fire...
                } else if (grid[0][i][j] > 0) {
                    
                    // Check its neighbors for fires
                    if (neighborCheck(i, j, windDirection, windSpeed) == 1) {
                        changing = 1;
                    }
                } // Skip cells that don't have trees
            }
        }
        
        // Create another 'change' variable for MPI
        int changeCheck = 0;
        
        // See if changes were made on any grid
        MPI_Allreduce(&changing, &changeCheck, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        if (changeCheck == 1) {
            haloExchange();
            copyGrid(1, 0);
        } else {
            break; // End the loop when changes are no longer being made to the grid
        }
    }
}


int main(int argc, char **argv) {
    
    // Variable for the MPI initialization/finalization
    int ierr;
    
    // Instantiate MPI variables
    ierr = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // This will ensure that numbers are actually random when the program is ran
    srand(rank + time(NULL));
    
    double density; // forest density
    double fireProbability; // Later probability that any one cell will have a fire
    char windDirection = 'N';
    float windSpeed = 0;
    int fprintfreq;

    if (argc > 5) {
        simSize = atoi(argv[1]);
        density = atof(argv[2]);
        fireProbability = atof(argv[3]);
        windDirection = *(argv[4]);
        windSpeed = atof(argv[5]);
        fprintfreq = atoi(argv[6]);
        
        // Check for a valid direction
        windDirection = toupper(windDirection);
        if (windDirection != 'N' && windDirection != 'S' && windDirection != 'E' && windDirection != 'W') {
            if (rank == 0) {
                printf("Invalid wind direction\n");
                
            }
            ierr = MPI_Finalize();
            exit(-1);
        }
        
        // Check for a valid wind speed (between 0 and 33 mph)
        if (windSpeed < 0 || windSpeed > 33) {
            if (rank == 0) {
                printf("Wind speed must be between 0 and 33 miles per hour\n");
            }
            ierr = MPI_Finalize();
            exit(-1);
        }
        
        // If the wind speed is valid, convert it to a decimal value for later use
        windSpeed /= 100;
        
    } else {
        printf ("Incorrect number of inputs");
        exit(-1);
    }
    
    // Make equal-sized grids for all the processors
    gridSize = simSize / sqrt(world);
    haloSize = gridSize + 2;
    
    // Keep track of grid dimensions
    nSize = sqrt(world);
    
    // Create a grid
    initGrid();
    
    // Fill the grid with data
    makeForest(density, fireProbability);
    
    // Initial grid instantiations
    copyGrid(0, 1);
    
    // Halo Exchange
    haloExchange();
    
    // Start the time lapse
    nextGen(windDirection, windSpeed, fprintfreq);

    // Sum up the grid after the fire has raged
    int sum = 0;
    for (int col = 1; col < haloSize - 1; col++) {
        for (int row = 1; row < haloSize - 1; row++) {
            if (grid[0][col][row] > 0) {
                sum++;
            }
        }
    }
    
    // A final result variable
    int result = 0;

    // MPI_Reduce to rank 0 all LRHC values
    MPI_Reduce(&sum, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("For a simulation of size %d units^2 with an inital foliage density of **approximately** %0.04lf%% and an initial fire chance of %lf%%, only %lf%% of the forest remains standing after the fire\n", (int)pow(simSize, 2), density * 100, fireProbability * 100, (result / pow((double)simSize, 2)) * 100);
    }
    
    // Terminate MPI
    ierr = MPI_Finalize();
    return 0;
}
