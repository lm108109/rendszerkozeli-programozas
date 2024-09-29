/////////////////////
// Name: Lilla Mezei
// Command: gcc main.c myUtils.c -o chart -lm -fopenmp   AND  ./chart
/////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <time.h>
// #include <unistd.h>
// #include <dirent.h>
// #include <sys/stat.h> 
// #include <ctype.h>
#include "myUtils.h"
#include <signal.h>
#include <unistd.h>
#include<omp.h>


int main(int argc, char* argv[]) {

    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);

    int IS_FILE = 0;
    int IS_SOCKET = 1;
    int IS_SEND = 0;
    int IS_RECEIVE = 1;

    if ((strcmp(argv[0],"./chart")) != 0) { 
        printf("Invalid usage, wrong name\n");
        exit(1);
    }
    /*if (argc == 1)
    {
        help();
    }*/
    if (argc == 2) 
    {
        if ((strcmp(argv[1], "--version"))== 0) {
            // printf("Version: 0.0.1\n");
            // printf("Made by Lilla M.\n");
            // printf("Done: 28.04.2024.\n"); // !!!
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    printf("Version: 0.0.1\n");
                }
                #pragma omp section
                {
                    printf("Made by Lilla M.\n");
                }
                #pragma omp section
                {
                    printf("Done: 28.04.2024.\n");
                }
            }
            exit(0);
        }
        else if ((strcmp(argv[1], "--help")) == 0) {
            help();
        }
        else{
            help();
        }
    }
    else if(argc == 3)
    {   
        int helyes = 0;
        if((strcmp(argv[1], "-file") == 0) || (strcmp(argv[2], "-file") == 0)) {
            helyes++;
        }
        if((strcmp(argv[1], "-socket") == 0) || (strcmp(argv[2], "-socket") == 0)) {
            IS_FILE = 1;
            IS_SOCKET = 0;
            helyes++;
        }
        if((strcmp(argv[1], "-send") == 0)|| (strcmp(argv[2], "-send") == 0)) {
            helyes++;
        }
        if((strcmp(argv[1], "-receive") == 0) || (strcmp(argv[2], "-receive") == 0)) {
            IS_SEND = 1;
            IS_RECEIVE = 0;
            helyes++;
        }
        if(helyes != 2) {
            help();
        }
    }
    else if (argc > 3){
        help();
    }
    //printf("lefutott\n\n");

	// ------------------------------------------------------------------------------------------------
    // if (IS_SOCKET == 0) {
    //     printf("Socket\n");
    // }
    // else if (IS_FILE == 0) {
    //     printf("File\n");
    // }

    // if (IS_SEND == 0) {
    //     printf("Send\n");
    //     // int *Values = NULL;
    //     // int size = Measurement(&Values);
    //     // // for (int i = 0; i < size; i++) {
    //     // //     printf("%d\t",  (Values[i]));
    //     // // }
    //     // // printf("\n\n%d\n", size);
    //     // free(Values);
    //     // //printf("\nFindPid:\n");
    //     // int n = FindPID();
    //     // printf("PID: %d\n", n);

    // }
    // else if (IS_RECEIVE == 0) {
    //     printf("Receive\n");
    // }

    int *Values = NULL;
    int NumValues;
    
    ///////////////////////////
    if(IS_SEND == 0 && IS_FILE == 0) {
        NumValues = Measurement(&Values);

        SendViaFile(Values, NumValues);

        free(Values);
    }
    if(IS_RECEIVE == 0 && IS_FILE == 0) {
        while (1)
        {
            signal(SIGUSR1, ReceiveViaFile);
            pause();
        } 
    }
    if(IS_SEND == 0 && IS_SOCKET == 0) {
        NumValues = Measurement(&Values);

        SendViaSocket(Values, NumValues);

        free(Values);
    }
    if(IS_RECEIVE == 0 && IS_SOCKET == 0) {
        RecieveViaSocket();
    }

    return 0;
}
