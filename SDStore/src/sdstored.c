#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <signal.h>

typedef struct line {
    char* operation;
    int limit;
    int using;
} *Line;


typedef struct operation {
    char* fifo;
    char* source;
    char* destination;
    char** operations;
    int opN;
} *Operation;



Operation toOperation (char* proc, char* file){
    Operation ret = (Operation)malloc(sizeof(*ret));
    ret->fifo = malloc(16);
    strcpy(ret->fifo,proc);
    ret->source = strdup(strsep(&file," "));
    ret->destination = strdup(strsep(&file," "));
    ret->operations = malloc(256);
    int index = 0;
    while(file != NULL){
        ret->operations[index++] = strdup(strsep(&file," "));
    }
    ret->opN = index;
    return ret;
}


Line toLine(char* line){
    Line ret = (Line)malloc(sizeof(*ret));
    ret->operation = strdup(strsep(&line," "));
    ret->limit = atoi(strdup(strsep(&line," ")));
    ret->using = 0;

    return ret;
}


//debug

void printLines(Line* lines, int size){
    for (int i = 0; i<size; i++){
        printf("%s ",lines[i]->operation);
        printf("%d ",lines[i]->limit);
        printf("%d\n",lines[i]->using);
    }
    printf("\n");
}


void printOperations(Operation* ops){
    for (int i = 0; ops[i] != NULL; i++){
        printf("%s ",ops[i]->fifo);
        printf("%s ",ops[i]->source);
        printf("%s\n",ops[i]->destination);
    }
    printf("---------------------------\n");
}


void printLine(Line* lines, int index){
    printf("%s ",lines[index]->operation);
    printf("%d\n",lines[index]->limit);
}


void printOperation(Operation op){
    printf("%s ",op->fifo);
    printf("%s ",op->source);
    printf("%s\n",op->destination);
}

//

void sig_handler(int signum){
    if(signum == SIGTERM){
    kill(0,SIGTERM);
    }
}

ssize_t readln(int fd, char *line, size_t size) {
    ssize_t i = 0;

    while(read(fd, &line[i], 1) && line[i++] != '\n');

    return i;
}

char* bytes_read(char* source){

    int status;
    int piped[2];
    char* buf = malloc(32);
    if(pipe(piped) == -1){
        perror("Error creating fifo");
        sig_handler(SIGTERM);
    }

    if(fork()==0){
        close(piped[0]);

        if (dup2(piped[1],1) == -1){
            perror("ERROR");
            sig_handler(SIGTERM);
        }
        close(piped[1]);
        execlp("wc","wc","-c",source,NULL);

        perror("EXEC KILLED");
        sig_handler(SIGTERM);
    }
    wait(&status);
    close(piped[1]);
    read(piped[0],buf,32);
    close(piped[0]);

    char* num = strdup(strsep(&buf," "));

    return num;
}

void setLine(Line* lines, int arr[]){
    for (int i = 0; i<7; i++){
        lines[i]->using = arr[i];
    }
}


char* complete_exec(int size, char** commands, int source, char* destin, char* argv){
    int destination;
    if ((destination = open(destin,O_CREAT | O_TRUNC | O_WRONLY,0666)) < 0){
        perror("ERROR");
        sig_handler(SIGTERM);
    }

    if(size == 1){

        char* arg = malloc(32);
        strcat(strcat(arg,argv),commands[0]);
        pid_t pid = fork();

        if (!pid){
            if(dup2(source,0) == -1){
                perror("ERROR");
                sig_handler(SIGTERM);
            }
            close(source);

            if (dup2(destination,1) == -1){
                perror("ERROR");
                sig_handler(SIGTERM);
            }
            close(destination);

            execlp(arg,commands[0],NULL);

            perror("EXEC KILLED");
            sig_handler(SIGTERM);
        }

    wait(NULL);
    }

    else{

        int piped[size-1][2];
        

        for(int i=0; i<size; i++){
            if (i==0){

                if (pipe(piped[i]) == -1){
                    perror("ERROR CREATING PIPE");
                    sig_handler(SIGTERM);
                }
                
                char* arg = malloc(32);
                strcat(strcpy(arg,argv),commands[i]);
                pid_t pid = fork();
                if (!pid){
                    if (dup2(source,0) == -1){
                        perror("ERROR");
                        sig_handler(SIGTERM);
                    }
                    close(source);

                    close(piped[0][0]);

                    if(dup2(piped[0][1],1)==-1){
                        perror("ERROR");
                        sig_handler(SIGTERM);
                    }
                    close(piped[0][1]);

                    execlp(arg,commands[0],NULL);

                    perror("EXEC KILLED");
                    sig_handler(SIGTERM);
                }
                close(piped[0][1]);
            }

            else if(i<size-1){

                if (pipe(piped[i]) == -1){
                    perror("ERROR CREATING PIPE");
                    sig_handler(SIGTERM);
                }

                char* arg = malloc(32);
                strcat(strcpy(arg,argv),commands[i]);
                pid_t pid = fork();
                if (!pid){

                    if(dup2(piped[i-1][0],0) == -1){
                        perror("ERROR");
                        sig_handler(SIGTERM);
                    }
                    close(piped[i-1][0]);

                    close(piped[i][0]);

                    if(dup2(piped[i][1],1)==-1){
                        perror("ERROR");
                        sig_handler(SIGTERM);
                    }
                    close(piped[i][1]);

                    execlp(arg,commands[i],NULL);

                    perror("EXEC KILLED");
                    sig_handler(SIGTERM);
                }
                close(piped[i-1][0]);
                close(piped[i][1]);
            }
            else{
            
                char* arg = malloc(32);
                strcat(strcpy(arg,argv),commands[i]);
                pid_t pid = fork();
                if(!pid){
                    close(piped[i-1][1]);

                    if(dup2(piped[i-1][0],0) == -1){
                        perror("ERROR");
                        sig_handler(SIGTERM);
                    }
                    close(piped[i-1][0]);

                    if(dup2(destination,1) == -1){
                        perror("ERROR");
                        sig_handler(SIGTERM);
                    }
                    close(destination);

                    execlp(arg,commands[i],NULL);

                    perror("EXEC KILLED");
                    sig_handler(SIGTERM);
                }
                close(piped[i-1][0]);
                close(piped[i][1]);
            }
        }
    }

    for(int i=0; i<size; i++){
        wait(NULL);
    }

    char* result = bytes_read(destin);

    return result;
}


int canOperate(Line* lines, Operation op){
    int can = 0;

    for (int i = 0; i<op->opN; i++)
        for (int j = 0; j<7; j++){
            if(!strcmp(op->operations[i],lines[j]->operation) && lines[j]->using > lines[j]->limit){
                can = 1;
                break;
            }
    }
    return can;
}


void incIt(Line* lines, Operation op){
    for (int i = 0; i<op->opN; i++)
        for (int j = 0; j<7; j++){
            if(!strcmp(op->operations[i],lines[j]->operation))
                lines[j]->using++;
        }
}


void decIt(Line* lines, Operation op){
    for (int i = 0; i<op->opN; i++)
        for (int j = 0; j<7; j++){
            if(!strcmp(op->operations[i],lines[j]->operation))
                lines[j]->using--;
        }
}


void write_procs(int procc, char* b_read, char* b_final){
    write(procc,"concluded (bytes-input: ",24);
    write(procc,b_read,strlen(b_read));
    write(procc,", bytes-output: ",16);
    write(procc,b_final,strlen(b_final));
    write(procc,")\n",2);
}


void addOp (Operation* operations, Operation op, int size){
    int i;
    for (i=0; i<size && operations[i] != NULL; i++);
    operations[i] = op;

}


void replace (Operation op, Operation op2){
    op->fifo = op2->fifo;
    op->source = op2->source;
    op->destination = op2->destination;
    for (int i = 0; op2->operations[i]; i++)
        op->operations[i] = op2->operations[i];
    op->opN = op2->opN;
}


void shift (Operation* operations, int index){

    for (; operations[index+1] != NULL; index++){
        replace(operations[index],operations[index+1]);
    }
    operations[index] = NULL;
}


void doExec(Operation ops, Line* lines, char* argv){
    
    if(canOperate(lines,ops)){}

        int procc;
        if((procc = open(ops->fifo,O_WRONLY,0666)) < 0){
            perror("ERROR");
            sig_handler(SIGTERM);
        }
            

        int sourc;
        if ((sourc = open(ops->source,O_RDONLY)) < 0){
            perror("ERROR");
            sig_handler(SIGTERM);
        }

        char* b_read = bytes_read(ops->source);
        char* b_final = complete_exec(ops->opN,ops->operations,sourc,ops->destination,argv);

        write_procs(procc,b_read,b_final);

        decIt(lines,ops);

        close(procc);

}

void doStatus(Line* lines, char* proc){
    char* buf = malloc(128);
    int arr[7];

    for (int i=0; i<7; i++){
        strcat(buf,lines[i]->operation);
        strcat(buf,"\n");
        
        arr[i] = lines[i]->using;
    }


    int procc;
    if((procc = open(proc,O_WRONLY,0666)) < 0){
        perror("ERROR");
        sig_handler(SIGTERM);
    }

    write(procc,arr,sizeof(arr));
    write(procc,buf,128);
}

   
//////////////////////////////////////////////////////////////


int main(int argc, char* argv[]){
    
    if(argc != 3){
        perror("Wrong number of arguments!");
        sig_handler(SIGTERM);
    }

    Line* lines = calloc(7,sizeof(*lines));
    int index = 0;
    char* line = malloc(32);
    
    
    int config;
    if ((config = open(argv[1],O_RDONLY)) < 0){
        perror("ERROR");
        sig_handler(SIGTERM);
    }

    int op = 0;
    while (readln(config,line,128)){
        lines[index] = toLine(line);
        op += lines[index]->limit;
        index++;
    }

    ////////////////////////////////////////

    if (mkfifo("c-sFifo",0666) == -1){
        perror("Error creating fifo");
        sig_handler(SIGTERM);
    }


    while (1){
        int fifo;
        if ((fifo = open("c-sFifo",O_RDONLY)) < 0){
            perror("ERROR");
            sig_handler(SIGTERM);
        }
        
        char* file = malloc(128);
        
        for (int i = 0; (read(fifo,&file[i],1)) > 0;i++);
        close(fifo);

        char* proc = strdup(strsep(&file," "));

        
        if(!strcmp(file,"sigterm")){
            unlink("c-sFifo");
            sig_handler(SIGTERM);
        }

        else if(!strcmp(file,"status")){
            if (!fork()){
            doStatus(lines,proc);
            _exit(0);
            }

            memset(file, 0, 128);
        }

        else {

            Operation ops = malloc(1024); 
            ops = toOperation(proc,file);

            memset(file, 0, 128);
        
         
            incIt(lines,ops);
        

            if (!fork()){
                doExec(ops,lines,argv[2]);
                _exit(0);
            }
        
        
        }
    }
    return 0;
}