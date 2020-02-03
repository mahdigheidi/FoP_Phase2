#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include "cJSON.c"
#include "cJSON.h"

#define PORT 12345
#define MAX 1024

bool checkAuth(char* s)
{
    char path[] = "../Online/" , this[MAX] = {0};
    strcpy(this , s);
    strcat(path , this);
    strcat(path , ".txt");
    FILE* fp;
    if(!(fp = fopen(path , "r")))
        return 0;
    return 1;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAX] = {0};
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 12345
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 12345
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    read(new_socket , buffer, MAX);
    char firstCommand[MAX] = {0};
    int ptr;
    for(int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ' ') {
            ptr = i + 1;
            break;
        }
        firstCommand[i] = buffer[i];
    }
    if(strcmp(firstCommand , "register") == 0){
        char user[MAX] = {0} , pass[MAX] = {0};
        int ind = 0;
        while(true){
            if(buffer[ptr] == ','){
                ptr += 2;
                break;
            }
            user[ind++] = buffer[ptr++];
        }
        ind = 0;
        for(int i = ptr; i < strlen(buffer); i++)
            pass[ind++] = buffer[i];
        FILE* fp;
        char path[] = "../Resources/Users/";
        strcat(path , user);
        strcat(path , ".user.json");
        cJSON *ans = cJSON_CreateObject();
        if (fp = fopen(path, "r")){
            fclose(fp);
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("user already exists");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
        } else {
            fp = fopen(path, "w");
            cJSON *userAdd = cJSON_CreateObject();
            cJSON *username = cJSON_CreateString(user);
            cJSON *password = cJSON_CreateString(pass);
            cJSON_AddItemToObject(userAdd, "username", username);
            cJSON_AddItemToObject(userAdd, "password", password);
            char *string = cJSON_Print(userAdd);
            fprintf(fp, "%s", string);
            fclose(fp);
            cJSON *type = cJSON_CreateString("Successful");
            cJSON *content = cJSON_CreateString("");
            cJSON_AddItemToObject(ans, "type", type);
            cJSON_AddItemToObject(ans, "content", content);
        }
        memset(buffer, '\0', sizeof(buffer));
        strcpy(buffer, cJSON_Print(ans));
        printf("%s" , buffer);
    }
    else if(strcmp(firstCommand , "login") == 0){
        char user[MAX] = {0} , pass[MAX] = {0} , auth[MAX] = {0};
        int ind = 0;
        while(true){
            if(buffer[ptr] == ','){
                ptr += 2;
                break;
            }
            user[ind++] = buffer[ptr++];
        }
        ind = 0;
        for(int i = ptr; i < strlen(buffer); i++)
            pass[ind++] = buffer[i];
        FILE* fp;
        char path[] = "../Resources/Users/";
        strcat(path , user);
        strcat(path , ".user.json");
        cJSON* ans = cJSON_CreateObject();
        if (!(fp = fopen(path, "r+"))) {
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
        }
        else{
            char f[1000] = {0};
            int p = 0;
            char ch;
            while((ch = fgetc(fp)) != EOF){
                f[p++] = ch;
            }
            cJSON *get = cJSON_Parse(f);
            cJSON* password = cJSON_GetObjectItemCaseSensitive(get , "password");
            if(strcmp(password->valuestring , pass) != 0){
                cJSON* type = cJSON_CreateString("error");
                cJSON* content = cJSON_CreateString("wrong password");
                cJSON_AddItemToObject(ans , "type" , type);
                cJSON_AddItemToObject(ans , "content" , content);
            }
            else{
                cJSON* type = cJSON_CreateString("AuthToken");
                memset(auth , 0 , sizeof(auth));
                srand(time(0));
                for(int i = 0; i < 33; i++)
                    auth[i] = ('a' + (rand() % 26));
                FILE* f;
                char masir[] = "../Online/";
                strcat(masir , auth);
                strcat(masir, ".txt");
                f = fopen(masir , "w+");
                fprintf(f , "%s", user);
                cJSON* content = cJSON_CreateString(auth);
                cJSON_AddItemToObject(ans , "type" , type);
                cJSON_AddItemToObject(ans , "content" , content);
            }
            fclose(fp);
        }
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , s);
    }
    else if(strcmp(firstCommand , "create") == 0){
        char chname[MAX] = {0}, authTok[MAX] = {0};
        int ptr , cnt = 0;
        for(int i = 15; ; i++) {
            if (buffer[i] == ',') {
                ptr = i + 2;
                break;
            }
            chname[cnt++] = buffer[i];
        }
        strcpy(authTok , buffer + ptr);
        FILE* fp;
        cJSON* ans = cJSON_CreateObject();
        char path[] = "../Resources/Channels/User/";
        strcat(path , chname);
        strcat(path , ".txt");
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            goto this;
        }
        if(fp = fopen(path, "r+")){
            fclose(fp);
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("channel already exists");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            goto this;
        }
        fp = fopen(path , "w+");
        cJSON* type = cJSON_CreateString("Successful");
        cJSON* content = cJSON_CreateString("channel created");
        cJSON_AddItemToObject(ans , "type" , type);
        cJSON_AddItemToObject(ans , "content" , content);
        fprintf(fp , "%s" , authTok);
        this:
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , s);
    }
    else if(strcmp(firstCommand , "join") == 0){
        char chname[MAX] = {0} , authTok[MAX] = {0};
        int ptr , cnt = 0;
        cJSON* ans = cJSON_CreateObject();
        for(int i = 13; ; i++){
            if(buffer[i] == ','){
                ptr = i + 2;
                break;
            }
            chname[cnt++] = buffer[i];
        }
        strcpy(authTok , buffer + ptr);
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            goto that;
        }
        FILE* fp;
        char path[] = "../Resources/Channels/User/";
        strcat(path , chname);
        strcat(path , ".txt");
        if(!(fp = fopen(path, "r+"))){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("channel does not exists");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            goto that;
        }
        fprintf(fp , "%s" , authTok);
        fclose(fp);
        char p[] = "../Online/";
        strcat(p , authTok);
        strcat(p , ".txt");
        fp = fopen(p , "a");
        fprintf(fp , "%s" , chname);
        fclose(fp);
        cJSON* type = cJSON_CreateString("Successful");
        cJSON* content = cJSON_CreateString("added to channel");
        cJSON_AddItemToObject(ans , "type" , type);
        cJSON_AddItemToObject(ans , "content" , content);
        that:
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , s);
    }
    else if(strcmp(firstCommand , "send") == 0){
        char message[MAX] = {0} , authTok[MAX] = {0};
        int cnt = 0;
        for(int i = 5;;i++){
            if(buffer[i] == ','){
                strcpy(authTok , buffer + i + 2);
                break;
            }
            message[cnt++] = buffer[i];
        }
        cJSON* ans = cJSON_CreateObject();
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            goto those;
        }
        char path[] = "../Online/" , user[MAX] = {0};
        strcat(path , authTok);
        strcat(path , ".txt");
        FILE* fp;
        fp = fopen(path , "r");
        fscanf(fp , "%s" , user);
        fclose(fp);
        fp = fopen("../msg.txt" , "w");
        fprintf(fp , "%s %s" , user , message);
        fclose(fp);
        those:
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , s);
    }
    else if(strcmp(firstCommand , "refresh") == 0){
        cJSON* ans = cJSON_CreateObject();
        char authTok[MAX] = {0};
        strcpy(authTok , buffer + 8);
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            goto these;
        }
        FILE* fp;
        fp = fopen("../msg.txt" , "r");
        char name[MAX] = {0} , msg[MAX] = {0} , f[MAX] = {0} , h[MAX] = {0} , c;
        int n = 0;
        cJSON* type = cJSON_CreateString("List");
        cJSON_AddItemToObject(ans , "type" , type);
        cJSON* messages = cJSON_CreateArray();
        cJSON_AddItemToObject(ans , "content" , messages);
        while ((c = fgetc(fp)) != EOF)
        {
            if(c == '\n') {
                sscanf(f , "%s %s" , name , msg);
                cJSON* arObj = cJSON_CreateObject();
                cJSON* sender = cJSON_CreateString(name);
                cJSON* text = cJSON_CreateString(msg);
                cJSON_AddItemToObject(arObj, "sender" , sender);
                cJSON_AddItemToObject(arObj, "content" , text);
                cJSON_AddItemToArray(messages , arObj);
                memset(f , 0 , sizeof(f));
                n = 0;
                continue;
            }
            f[n++] = (char) c;
        }
        these:
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer, s);
        printf("%s" , buffer);
        /*برای پاک شدن همه مسیج ها بعد از رفرش کردن یکبار فایل را با دستور w نیز باز میکنیم تا محتوای فایل پاک شود*/
        fclose(fp);
        fp = fopen("../msg.txt" , "w");
        fclose(fp);
    }
    else if(strcmp(firstCommand , "channel") == 0){
        char authTok[MAX] = {0};
        strcpy(authTok , buffer + 16);
        cJSON* ans = cJSON_CreateObject();
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
        }
        else{
            char user[MAX] = {0} , chname[MAX] = {0};
            char path[] = "../Online/";
            strcat(path , authTok);
            strcat(path , ".txt");
            FILE* fp = fopen(path , "r");
            fscanf(fp , "%s\n%s" , user , chname);
            fclose(fp);
            char p[] = "../Resources/Channels/User/" , c , name[MAX] = {0};
            int n = 0;
            strcat(p , chname);
            strcat(p , ".txt");
            fp = fopen(p , "r");
            cJSON* type = cJSON_CreateString("List");
            cJSON* content = cJSON_CreateArray();
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
            while ((c = fgetc(fp)) != EOF)
            {
                if(c == '\n'){
                    cJSON* nam = cJSON_CreateString(name);
                    cJSON_AddItemToArray(content , nam);
                    memset(name , 0 , sizeof(name));
                    n = 0;
                    continue;
                }
                name[n++] = c;
            }
        }
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , ans);
    }
    else if(strcmp(firstCommand , "leave") == 0){
        char authTok[MAX] = {0};
        strcpy(authTok , buffer + 6);
        cJSON* ans = cJSON_CreateObject();
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
        }
        else{
            char path[] = "../Online/" , user[MAX] = {0} , chname[MAX] = {0};
            strcat(path, authTok);
            strcat(path, ".txt");
            FILE* fp;
            fp = fopen(path , "r");
            fscanf(fp , "%s\n%s" , user, chname);
            fclose(fp);
            fp = fopen(path,  "w");
            fprintf(fp , "%s" , user);
            fclose(fp);
            char p[] = "../Resources/Channels/User/";
            strcat(p, chname);
            strcat(p, ".txt");
            fp = fopen(p , "w");
            fclose(fp);
        }
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , s);
    }
    else if(strcmp(firstCommand , "logout") == 0){
        char authTok[MAX] = {0};
        cJSON* ans = cJSON_CreateObject();
        strcpy(authTok , buffer + 7);
        if(!checkAuth(authTok)){
            cJSON* type = cJSON_CreateString("error");
            cJSON* content = cJSON_CreateString("such user does not exist");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
        }
        else{
            char path[] = "../Online/";
            strcat(path , authTok);
            strcat(path , ".txt");
            remove(path);
            cJSON* type = cJSON_CreateString("successful");
            cJSON* content = cJSON_CreateString("user logged out");
            cJSON_AddItemToObject(ans , "type" , type);
            cJSON_AddItemToObject(ans , "content" , content);
        }
        memset(buffer , 0 , sizeof(buffer));
        char* s = cJSON_Print(ans);
        strcpy(buffer , s);
    }
    send(new_socket , buffer , strlen(buffer) , 0);
    return 0;
}