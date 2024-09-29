#ifndef HEADER_FILE
#define HEADER_FILE

void help();

void SignalHandler(int sig);

int Measurement(int **Values);

double randfrom(double min, double max);

void BMPcreator(int *Values, int NumValues);

int FindPID();

void SendViaFile(int *Values, int NumValues);

void ReceiveViaFile(int sig);

void SendViaSocket(int *Values, int NumValues);

void RecieveViaSocket();

#endif