#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <dirent.h>

typedef struct{
    unsigned int length;
    char ftitle[30];
}finfo;
#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    char buffer[BUF_SIZE];
    char contents[BUF_SIZE];
    int str_len, i;
    struct sockaddr_in serv_adr;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;
    DIR *dir;
    struct dirent *dp;
    FILE * fp;
    int read_cnt;
    int count=0;
    int num;
    finfo file_info[30];
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    dir = opendir(".");
        if(dir){
            while((dp = readdir(dir))!=NULL){
                if(strcmp(dp->d_name,".")==0||strcmp(dp->d_name,"..")==0) //. .. 스킵
                    continue;
                strcpy(file_info[count].ftitle,dp->d_name); //파일 이름
                
                //파일 크기 확인 위해 fseek와 ftell사용
                fp = fopen(file_info[count].ftitle,"rb");
                fseek(fp,0,SEEK_END); 
                file_info[count].length = ftell(fp);
                fclose(fp);
                count++; //파일 개수 증가
            }
            closedir(dir);
        }else{
            error_handling("directory can't open");
        }
    serv_sock=socket(PF_INET, SOCK_STREAM, 0); 
    if (serv_sock == -1)
        error_handling("socket() error");
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    clnt_adr_sz=sizeof(clnt_adr);
    for (i = 0; i < 5; i++){
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        if (clnt_sock == -1)
            error_handling("accept() error");
        else
            printf("Connected client %d \n", i+1);
        
        write(clnt_sock,&count,sizeof(int)); //파일 개수 전달

        for(int i=0; i<count; i++){ //파일 개수만큼 파일 정보 전송
            send(clnt_sock,&file_info[i],sizeof(finfo),0);
        }
        while(1){
            read(clnt_sock,&num,sizeof(int)); //복사 할 파일 인자값 read
            if(num== -1) break; //클라이언트 종료
            fp = fopen(file_info[num].ftitle,"rb"); //해당 파일 rb모드로 open
            while(1){
                memset(buffer, 0, BUF_SIZE); //버퍼 초기화
                read_cnt = fread((void*)buffer, 1, BUF_SIZE, fp);
                if (read_cnt < BUF_SIZE){
                    write(clnt_sock, buffer, read_cnt);
                    break;
                }
                write(clnt_sock, buffer, BUF_SIZE);
            }
        fclose(fp);
        }
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
