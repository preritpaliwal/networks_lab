#include <stdio.h>
#include <string.h>

int main(){
    FILE *fp = fopen("test.txt","r");
    char *data = "testing if this works2\n";
    if(fp==NULL){
        printf("could not open file..!!\n");
    }
    else{
        printf("file opened\n");
        char buffer[50];
        while(fgets(buffer,50,fp)!=NULL){
            printf("%s",buffer);
        }
        fclose(fp);
        printf("file closed\n");

    }
    return 0;
}