#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

void error_handling(char *message);
typedef struct{
    unsigned int length;
    char ftitle[30];
}finfo;
int main(int argc, char *argv[]){
    int sock;
    char title[BUF_SIZE];
    int str_len;
    int read_cnt;
    int write_cnt=0;
    struct sockaddr_in serv_adr;
    int count=0;
    int num;
    FILE *fp;
    finfo file_info[30];
    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    sock=socket(PF_INET, SOCK_STREAM, 0); 
    if (sock == -1)
        error_handling("socket() error"); 
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));
    if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");
    else
        puts("Connected...........");
    read(sock,&count,sizeof(int)); //파일 개수
    for(int i=0; i<count; i++){ //파일 개수 만큼 파일 정보 받아오기
        read(sock,&file_info[i],sizeof(finfo));
    }
    while(1){
        for(int i=0; i<count; i++){
            printf("%d: %s(크기: %d)\n",i+1,file_info[i].ftitle,file_info[i].length);
        }
        printf("복사할 파일의 번호를 입력하시오(종료 0번): ");
		scanf("%d",&num);
        if(num==0){ //클라이언트 종료
            num=-1;
            write(sock,&num,sizeof(int));
            break;
        }
        if(num>count) //파일 수보다 큰 번호를 입력할 시 에러
            error_handling("잘못된 파일 번호\n");
        num-=1;
        write(sock,&num,sizeof(int)); //복사 할 파일 번호 전송
        fp = fopen(file_info[num].ftitle,"wb");
        char buffer[BUF_SIZE];
        write_cnt=0;
        while (write_cnt<file_info[num].length){
            memset(buffer, 0, BUF_SIZE); //버퍼 초기화
            read_cnt = read(sock, buffer, BUF_SIZE);
            if(read_cnt<0) break;
            fwrite((void*)buffer, 1, read_cnt, fp);
            write_cnt+=read_cnt;
        }
        
        puts("Received file data");
        fclose(fp);
    }
    close(sock);
    return 0;
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
