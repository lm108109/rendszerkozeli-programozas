#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#define BUFSIZE 1024                 // Max length of buffer
#define PORT_NO 3333                 // The port on which the server is listening
#define IP_ADDRESS "127.0.0.1"

void SignalHandler(int sig)
{
    if(sig == SIGINT){
        printf("\n\nInterrupt: Ctrl+C was pressed.\n");
        exit(1);
    }
    else if(sig == SIGALRM){
        fprintf(stderr,"ERROR: timeout - no response from the server.\n");
        exit(2);
    }
    else if(sig == SIGUSR1){
        fprintf(stderr,"ERROR: sending via files option doesn't available\n");
        exit(3);
    }
}

void help()
{
    printf("Usage: ./main [OPTION]\n");
    printf("Options:\n");
    printf("  --version\n");
    printf("  --help\n");
    printf("  -send\n");
    printf("  -receive\n");
    printf("  -file\n");
    printf("  -socket\n");
    exit(0);
}

int Measurement(int **Values)
{
    // a negyedórás rész:
    time_t T1;
    int T2 = time(&T1);
    struct tm *T4;
    T4 = localtime(&T1);

    int minutes = ((*T4).tm_min % 15) * 60;
    // printf("minutes: %d", minutes);
    int seconds = (*T4).tm_sec;
    int time1 = minutes + seconds;
    int size;
    // printf("\n sec: %d sec", time1);

    if (time1 < 100) {
        size = 100;
    }
    else{
        size = time1;
    }

    // random:
    srand(time(NULL));
    double random_n;
    int x = 0;

    *Values = malloc(sizeof(int) * size); 
    for (int i = 0; i < size; i++) {
        random_n =(double)rand()/((unsigned)RAND_MAX+1);
        // printf("%d: %lf\t",i, random_n);
        if (i == 0)
            (*Values)[i] = x;
        else {
            if (random_n <= 0.428571){
                x = x + 1;
                (*Values)[i] = x;
            }
            else if(random_n > 0.428571 && random_n <= 0.7834097096774193) {
                x = x - 1;
                (*Values)[i] = x;
            }
            else {
                 x = x + 0;
                 (*Values)[i] = x;
            }
        }
        // printf("Values[%d]: %d\n", i, (*Values)[i]);
    }
    return size;
}

void BMPcreator(int *Values, int NumValues) 
{
    int f = open("chart.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (f < 0)
    {
        fprintf(stderr, "ERROR: Can't open the bmp file.\n");
        exit(4);
    }

    int size;
    // printf("bmp padding x numvalues: %d\n", (NumValues * (NumValues + 32 - (NumValues % 32))) / 8);

    if (NumValues % 32 == 0){
        size = 62 + pow(NumValues, 2) / 8;
    }
    else{
        size = 62 + (NumValues * (NumValues + 32 - (NumValues % 32))) / 8;  //paddingeléssel szorzás
    }
    // printf("size: %d\n", size);

    unsigned char *bmp = (unsigned char*) calloc(size, sizeof(char));  //(unsigned char*) malloc(file, sizeof(char)); 

    bmp[0] = 'B';   //Signature ("BM") 
    bmp[1] = 'M';

    bmp[2] = size;  //File size in bytes --shiftelés
    bmp[3] = size >> 8;
    bmp[4] = size >> 16;
    bmp[5] = size >> 24;

    // bmp[6] = 0;     //Unused     //calloc miatt alapból 0
    // bmp[7] = 0;
    // bmp[8] = 0;
    // bmp[9] = 0;

    int constant = 62;
    bmp[10] = constant;   //Pixel Array offset (62)
    bmp[11] = constant >> 8;
    bmp[12] = constant >> 16;
    bmp[13] = constant >> 24;

    constant = 40;
    bmp[14] = constant;   //DIB Header size (40)
    bmp[15] = constant >> 8;
    bmp[16] = constant >> 16;
    bmp[17] = constant >> 24;

    constant = NumValues;
    bmp[18] = constant;   //Image width in pixels
    bmp[19] = constant >> 8;
    bmp[20] = constant >> 16;
    bmp[21] = constant >> 24;

    //constant = NumValues;
    bmp[22] = constant;   //Image height in pixels
    bmp[23] = constant >> 8;
    bmp[24] = constant >> 16;
    bmp[25] = constant >> 24;

    constant = 1;
    bmp[26] = constant;   //Planes (1)
    bmp[27] = constant >> 8;
    
    // constant = 1;
    bmp[28] = constant;   //Bits / pixel (1)
    bmp[29] = constant >> 8;

    //Compression (0) --> 4 Byte
    //Image size (0)  --> 4 Byte
    constant = 3937;
    bmp[38] = constant;   //Horizontal pixel/meter
    bmp[39] = constant >> 8;
    bmp[40] = constant >> 16;
    bmp[41] = constant >> 24;

    //constant = 3937;
    bmp[42] = constant;   //Vertical pixel/meter
    bmp[43] = constant >> 8;
    bmp[44] = constant >> 16;
    bmp[45] = constant >> 24;

    //Colors in palette (0)  --> 4 Byte
    //Used palette colors (0)  --> 4 Byte
    bmp[54] = 119; // Fisrt color
    bmp[55] = 140;
    bmp[56] = 48;
    bmp[57] = 255;

    bmp[58] = 217;   // Second color
    bmp[59] = 61;
    bmp[60] = 196;
    bmp[61] = 255;

    // innen pixel array -- 62. byte-tól

    int halfNumValues = NumValues / 2;
    int examinedBits = 8; //egyszerre 8 bitet nézünk

    for(int i = 0; i < NumValues; i++)
    {
        int pattern = 0;

        if(Values[i] > (halfNumValues)) //határok
        {
            Values[i] = (NumValues % 2 == 0) ? halfNumValues - 1 : halfNumValues;
        }
        if(Values[i] < -(halfNumValues))
        {
            Values[i] = -(halfNumValues);
        }
        int start = (i / examinedBits) * examinedBits; //adott byte-ban hol kezdődik a bit
        for (int j = start; j < start + examinedBits; j++)
        {
            int position = 8 - (start + examinedBits - j);
            // printf("position: %d\n", position);
            if (Values[i] == Values[j])
            {
                pattern += 1 << 7 - position; // 2^7-poz számolása balra shifteléssel = pow()
            }
        }
        int index = (62 + ((halfNumValues + Values[i]) * (NumValues + (32 - (NumValues % 32)))) / 8) + (i / examinedBits);
        // printf("index = %d\n", index);
        // printf("pattern = %d\n", pattern);
        bmp[index] = pattern;
    }
    
    write(f, bmp, size);
    close(f);
    free(bmp);
}

int FindPID()
{
    int pid = getpid();
    //printf("FindPID, my pid = %d\n", pid);
    FILE *f;
    DIR *d;
    int meret = 300;
    char path[meret];
    char name[meret - 200];
    struct dirent *entry;
    struct stat inode;
    d = opendir("/proc");
    int count = 0;
    int visszaad = -1;

    while((entry=readdir(d))!= NULL){
        if (isdigit((*entry).d_name[0])) {
            snprintf(path, meret, "/proc/%s/status", (*entry).d_name);
            //printf("%s\t",(*entry).d_name);
            //printf("Path: %s\n", path);
            f = fopen(path, "r");
            // printf("Entry name: %s\t", (*entry).d_name);
            // printf("\n");
            // printf("Path: %s\n", path);
            if (f == NULL) {
                printf("Error: Unable to open file\n");
            }
            else{
                fscanf(f, "Name:\t%s\n", name);
                // printf("Name %s\n -----------------\n", name);
                if(strcmp(name, "chart")== 0)        //if(strcmp(name, "bash")== 0)     !!! if(strcmp(name, "fish")== 0) !!!
                {
                    count++;
                    //printf("FindPID, this pid = %d\n", atoi((*entry).d_name));
                    //return atoi((*entry).d_name);       //a PID:\t után lévő szám mindig megegyezik a fájl nevével
                    //printf("if felett, this pid = %d, mypid = %d\n", atoi((*entry).d_name), pid);
                    if (atoi((*entry).d_name) != pid)
                    {   
                        //printf("ifben, this pid = %d, mypid = %d\n", atoi((*entry).d_name), pid);
                        visszaad = atoi((*entry).d_name);
                    }
                }
            }fclose(f);
        }
        
    }
    closedir(d);
    return visszaad;
    //return pid;
}

void SendViaFile(int *Values, int NumValues)
{
    FILE *f;
    char *homePath = getenv("HOME");
    strcat(homePath, "/Measurement.txt");
    //printf("Homepath: %s\n", homePath);
    f = fopen(homePath, "w");
    if (f == NULL) {
        printf("Error: Unable to open file 1\n");
    }
    else{
        for (int i = 0; i < NumValues; i++) {
            fprintf(f, "%d\n", Values[i]);
        }
    }
    fclose(f);

    int getPID = FindPID();
    // printf("PID: %d\n", getPID);
    if (getPID == -1) {
        fprintf(stderr, "Error:No host mode process found.\n");
        exit(5);
    }
    else{
        kill(getPID, SIGUSR1);
    }
}



void ReceiveViaFile(int sig){
    FILE *f;
    // char* homePath = getenv("HOME");
    // strcat(homePath, "/Measurement.txt");
    char* homePath = strcat(getenv("HOME"), "/Measurement.txt");
    // printf("Homepath: %s\n", homePath);
    f = fopen(homePath, "r");
    int size = 100;
    int* ptr = (int*) malloc(size*sizeof(int));

    if (f == NULL) {
        printf("Error: Unable to open file\n");
    }
    else{       
        int i = 1;
        while(!feof(f)){        //fscanf(f, "%d", &num) != EOF
            if (i == size +1 ) {
                size = size + 50;
                ptr = (int*) realloc(ptr, size*sizeof(int));
            }
            fscanf(f, "%d\n", &ptr[i-1]);
            //printf("%d.\t%d\n",i-1, ptr[i-1]);
            ++i;
        }
        homePath[strlen(homePath) - 16] = '\0';
        //printf("homepath: %s\n", homePath);
        fclose(f);
        BMPcreator(ptr, i);
        free(ptr);
    }//homePath = ".";
}

void SendViaSocket(int *Values, int NumValues)
{
    /************************ Declarations **********************/
    int s;                            // socket ID
    int bytes;                        // received/sent bytes
    int flag;                         // transmission flag
    char on;                          // sockopt option
    //char buffer[BUFSIZE];             // datagram buffer area
    unsigned int server_size;         // length of the sockaddr_in server
    struct sockaddr_in server;        // address of server

    /************************ Initialization ********************/
    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server.sin_port        = htons(PORT_NO);
    server_size = sizeof server;

    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( s < 0 ) {
        fprintf(stderr, "Error: Socket creation error.\n");
        exit(6);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Sending data **********************/

    bytes = sendto(s, &NumValues, sizeof(NumValues), flag, (struct sockaddr *) &server, server_size);
    if ( bytes <= 0 ) {
        fprintf(stderr, "Error: Sending error.\n");
        exit(7);
    }

    signal(SIGALRM, SignalHandler);
    alarm(1); //időzítő ------------------------------------------------------------------------------!!!

    /************************ Receive data **********************/
    int recievedData;
    bytes = recvfrom(s, &recievedData, sizeof(recievedData), flag, (struct sockaddr *) &server, &server_size);
    if ( bytes < 0 ) {
        fprintf(stderr, "Error: Receiving error.\n");
        exit(8);
    }
    alarm(0); //időzítő kikapcsolása

    //printf("Received data: %d\n", recievedData);
    // egyeznie kell a sended és recieved datanak

    if (NumValues != recievedData){
        fprintf(stderr, "Error: The sent and received data are not equal.\n");
        exit(9);
    }
    bytes = sendto(s, Values, NumValues*sizeof(int), flag, (struct sockaddr *) &server, server_size);
    int sendbytes= bytes;
    if ( bytes <= 0 ) {
        fprintf(stderr, "Error: Sending error.\n");
        exit(10);
    }
    bytes = recvfrom(s, &recievedData, recievedData, flag, (struct sockaddr *) &server, &server_size);
    if ( bytes < 0 ) {
        fprintf(stderr, "Error: Receiving error.\n");
        exit(11);
    }

    if (NumValues*sizeof(int) != recievedData){
        fprintf(stderr, "Error: The sent and received data are not equal.\n");
        exit(12);
    }


    /************************ Closing ***************************/
    close(s);

}

void RecieveViaSocket(){
    /************************ Declarations **********************/
    int s;
    int bytes;                        // received/sent bytes
    int err;                          // error code
    int flag;                         // transmission flag
    char on;                          // sockopt option
    //char buffer[BUFSIZE];             // datagram buffer area
    unsigned int server_size;         // length of the sockaddr_in server
    unsigned int client_size;         // length of the sockaddr_in client
    struct sockaddr_in server;        // address of server
    struct sockaddr_in client;        // address of client

    /************************ Initialization ********************/
    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; //nev konstan, ipcím ami be van állítva
    server.sin_port        = htons(PORT_NO);
    server_size = sizeof server;
    client_size = sizeof client;
    //    signal(SIGINT, stop);
    //    signal(SIGTERM, stop);

    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0 ); //aljzat létrehozás
    if ( s < 0 ) {
        fprintf(stderr, "ERROR: Socket creation error.\n");
        exit(13);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Binding socket ********************/
    err = bind(s, (struct sockaddr *) &server, server_size); //aljzat összekötés 
    if ( err < 0 ) {
        fprintf(stderr,"ERROR: Binding error.\n");
        exit(14);
    }
    int recievedData;
    int *reservedData;

    while(1){ // Continuous server operation
        /************************ Receive data **********************/ //numvalues
        bytes = recvfrom(s, &recievedData, sizeof(recievedData), flag, (struct sockaddr *) &client, &client_size );
        if ( bytes < 0 ) {
            fprintf(stderr, "ERROR: Receiving error.\n");
            exit(15);
        }
        /************************ Sending data **********************/
        bytes = sendto(s, &recievedData, sizeof(recievedData), flag, (struct sockaddr *) &client, client_size);
        if ( bytes <= 0 ) {
            fprintf(stderr, "ERROR: Sending error.\n");
            exit(16);
        }
        //printf(" Acknowledgement have been sent to client.\n");
        // terület lefoglalása dinamikusan --fel kell szabadítani!!!
        reservedData = (int*) malloc(recievedData*sizeof(int));
        bytes = recvfrom(s, reservedData, recievedData*sizeof(int), flag, (struct sockaddr *) &client, &client_size);
        if ( bytes < 0 ) {
            free(reservedData); //!!!
            fprintf(stderr, "ERROR: Receiving error.\n");
            exit(17);
        }
        int check = bytes;
        bytes = sendto(s, &check, sizeof(check), flag, (struct sockaddr *) &client, client_size);
        if ( bytes <= 0 ) {
            free(reservedData); //!!!
            fprintf(stderr, "ERROR: Sending error.\n");
            exit(18);
        }
        //printf("Received data: %d\n", recievedData);
        BMPcreator(reservedData, recievedData);
        //printf("The data has been received and the BMP file has been created.\n");
        free(reservedData);
    }
}



