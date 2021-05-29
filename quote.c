#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#define MAX 1000
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg) \
               do { perror(msg); exit(EXIT_FAILURE); } while (0)


int day, port;
char *QOTD;
const char *pathToQOTDfile;
pthread_mutex_t quoteLock=PTHREAD_MUTEX_INITIALIZER;
pthread_t checkForNewDayThread, connectionHandlerThread;

int line_count(const char* file){
    int count=0;
    int currentChar;
    FILE* fd=fopen(file,"r");
    if(fd==NULL){
        perror("Error opening the quotes file");
        exit(EXIT_FAILURE);
    }
    while(true){
        currentChar=fgetc(fd);
        switch (currentChar){
            case '\n':{
                count++;
                break;
            }
            case EOF:{
                fclose(fd);
                return count;
            }
            default:
                continue;
        }
    }
}

char* read_random_quote_from_file(const char* filePath){
   int numOfQuotes=line_count(filePath);
   int lineNumberOfQOTD=rand()%numOfQuotes;
   int lineCounter=0;
   char* lineptr=NULL; //will contain the address of the buffer containing the quote
   size_t n=0; //calling getline with lineptr=NULL and n=0 makes it automatically allocate the right amount of memory
   FILE* fd=fopen(filePath, "r");
    if(fd==NULL){
        perror("Error opening the quotes file");
        exit(EXIT_FAILURE);
    }
   while(lineCounter<lineNumberOfQOTD){ //seek the file until it reaches the line we want to read
       if(fgetc(fd)=='\n') lineCounter++;
   }
    getline(&lineptr, &n, fd);    //stores the line read from fd into a buffer and sets lineptr to the buffer's address
    fclose(fd);
    return lineptr;
}

void * connection_thread_code(void* port_ptr){    //Code for the thread to handle connections
    struct sockaddr_in address;
    int server_fd, new_socket, opt = 1, addrlen = sizeof(address), port=*((int*) port_ptr);

    free(port_ptr);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 1717
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))==-1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    // Forcefully attaching socket to the port 1717
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 100) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %i\n", port);

    while(1) {  //connection handler loop
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&quoteLock);
        send(new_socket, QOTD, strlen(QOTD), 0);
        pthread_mutex_unlock(&quoteLock);
        close(new_socket);
    }
}

int main(int argc, char const *argv[])
{
    int thread1, thread2, join;
    int* port=malloc(sizeof(int));

    if(port==NULL){
	perror("Couldn't allocate memory to store listening port");
	exit(EXIT_FAILURE);
    }

    time_t time1=time(NULL); //number of seconds elapsed since epoch
    struct tm tm = *localtime(&time1);
    day=tm.tm_mday; //The day of the month, in the range 1 to 31

    switch(argc){   //Read settings from terminal or use defaults
        case 1:{ //No input from user, use defaults
            *port=1717;
            pathToQOTDfile=strdup("q.txt");
            break;
        }
        case 3:{    //file and port specified by user
            *port=atoi(argv[2]);
            pathToQOTDfile=argv[1];
            break;
        }
        default:{
            fprintf(stderr,"Bad arguments\n");
            fprintf(stderr,"Usage:\n%s [path_to_quotes_file] [port]\nBy default reads qoutes from a file called \"q.txt\" in the current directory and listens on port 1717.\nNOTE: the standard port of QOTD protocol is 17.\nOnce you specify an argument all arguments became mandatory.\nThe file containing the quotes must be a txt file containig one quote per line and ending with exactly one newline\n", argv[0]);
            exit(EXIT_FAILURE);
            //Not necessary to call break since we call exit
        }

    }

	printf("Running as user %s", getlogin());
	printf("\n");
	printf("The quotes file appears to contain %i quotes\n", line_count(pathToQOTDfile));

        srand(time1);  //To randomize quotes

    QOTD = read_random_quote_from_file(pathToQOTDfile);   //No need to acquire lock here since no thread is even started
    connection_thread_code(port);

    return 0;
}
