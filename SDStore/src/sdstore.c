#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    int index;

    if((argc == 2 && (!strcmp(argv[1],"status") || !strcmp(argv[1],"sigterm")))  || (argc >= 5 && !strcmp(argv[1],"proc-file"))){

        int fifo;
        if ((fifo = open("c-sFifo",O_WRONLY)) < 0) {
            perror("ERROR");
            exit(-1);
        }

        if(!strcmp(argv[1],"sigterm")){
            write(fifo," sigterm",8);

            close(fifo);
        }

        else {

            char* proc = malloc(16);
            pid_t pid = getpid();
            sprintf(proc,"proc%d",pid);

            if (mkfifo(proc,0666) == -1){
            perror("Error creating fifo");
            exit(-1);
            }
    
            write(fifo,proc,strlen(proc));
            if (!strcmp(argv[1],"status")){
                write(fifo," status",7);

                close(fifo);

                int procc;
                if ((procc = open(proc,O_RDONLY)) < 0){
                    perror("ERROR");
                    exit(-1);
                }

                int arr[7];
                char* buf = malloc(128);

                read(procc,arr,sizeof(arr));
                read(procc,buf,128);
            
                char* tr1 = strsep(&buf,"\n");char* us1 = malloc(8);sprintf(us1,"%d",arr[0]);

                char* tr2 = strsep(&buf,"\n");char* us2 = malloc(8);sprintf(us2,"%d",arr[1]);

                char* tr3 = strsep(&buf,"\n");char* us3 = malloc(8);sprintf(us3,"%d",arr[2]);

                char* tr4 = strsep(&buf,"\n");char* us4 = malloc(8);sprintf(us4,"%d",arr[3]);

                char* tr5 = strsep(&buf,"\n");char* us5 = malloc(8);sprintf(us5,"%d",arr[4]);

                char* tr6 = strsep(&buf,"\n");char* us6 = malloc(8);sprintf(us6,"%d",arr[5]);

                char* tr7 = strsep(&buf,"\n");char* us7 = malloc(8);sprintf(us7,"%d",arr[6]);

                write(1,tr1,strlen(tr1)); write(1," - ",3); write(1,us1,strlen(us1)); write(1," times\n",strlen(" times\n"));
                write(1,tr2,strlen(tr2)); write(1," - ",3); write(1,us2,strlen(us2)); write(1," times\n",strlen(" times\n"));
                write(1,tr3,strlen(tr3)); write(1," - ",3); write(1,us3,strlen(us3)); write(1," times\n",strlen(" times\n"));
                write(1,tr4,strlen(tr4)); write(1," - ",3); write(1,us4,strlen(us4)); write(1," times\n",strlen(" times\n"));
                write(1,tr5,strlen(tr5)); write(1," - ",3); write(1,us5,strlen(us5)); write(1," times\n",strlen(" times\n"));
                write(1,tr6,strlen(tr6)); write(1," - ",3); write(1,us6,strlen(us6)); write(1," times\n",strlen(" times\n"));
                write(1,tr7,strlen(tr7)); write(1," - ",3); write(1,us7,strlen(us7)); write(1," times\n",strlen(" times\n"));

                close(procc);
            }

            else { 
                for (int i = 2; i < argc && (write(fifo," ", 1)) > 0; i++)
                write(fifo,argv[i],strlen(argv[i]));

                close(fifo);
    
                int procc;
                if ((procc = open(proc,O_RDONLY)) < 0){
                    perror("ERROR");
                    exit(-1);
                }

                char* output = malloc(128);

                for (index = 0; (read(procc,&output[index],1)) > 0;index++);

                write(1,output,strlen(output));    

                close(procc);
        
            }
            unlink(proc);
        }
    }

    else{
        perror("Wrong number of arguments!");
        exit(-1);
    }

    return 0;
}