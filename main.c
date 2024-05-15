#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "arpa/inet.h"

#define MAGIC 0xabcdfe01

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t direction;
    uint32_t player_name_len;
    uint8_t player_name[];
}player;

enum direction{
    up,
    right,
    down,
    left
};

typedef struct {
    uint32_t magic;
    uint32_t ptype;
    uint32_t datasize;
}packet;

//Game info
player pacman[4];
int score[4] = {0, 0, 0, 0};
int food_count;
uint8_t* player_name[4];
uint32_t player_name_len[4];
uint32_t frame_timeout = 200000;


//Map
uint8_t** map;

//Threads info
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t key;

//Network info
int is_serv = -1;
int player_count;
int port;
char ip[15];

//Server info
int players_sockets[4];
struct sockaddr_in players_addr[4];
int players_addr_size[4];


//Client info
struct sockaddr_in server_addr;

//////////////////////////////////////////////////////////////
int parse_args(int argc, char** argv){
    int arg;
    while((arg = getopt(argc, argv, "scp:i:m:n:")) != -1){
        switch (arg)
        {
        case 's':
            is_serv = 1;
            break;
        case 'c':
            is_serv = 0;
            break;
        case 'i':
            strcpy(ip, optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'm':
            player_count = atoi(optarg);
            break;
        case 'n':
            player_name[0] = (uint8_t*)malloc(strlen(optarg) + 1);
            strcpy(player_name[0], optarg);
            player_name_len[0] = strlen(player_name[0]);
            break;
        }
    }
    if(is_serv == -1 || argc == 1){
        printf("Use parametrs: \n-s for server\n-c for client\n-n to enter your name\n-p to enter port\n-i to enter ip (for client)\n-m to enter max count of player (for server)\n");
        return 0;
    }
    if(is_serv && (player_count < 1 || player_count > 4)){
        printf("Incorrect max players count (must be 1-4)\n");
        return 0;
    }
    if(player_name_len[0] < 1){
        printf("Incorrect name\n");
        return 0;
    }
    return 1;
}

void check_pos(int x, int y, uint8_t** quarter_map, int** neighbors){
    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){
            neighbors[i][j] = 0;
        }
    }
    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){
            if(quarter_map[i][j] == 0xaa || quarter_map[i][j] == 0x22){
                if(quarter_map[i - 1][j] == 0xaa || quarter_map[i - 1][j] == 0x22) { neighbors[i][j]++; }
                if(i + 1 < 20){
                    if(quarter_map[i + 1][j] == 0xaa || quarter_map[i + 1][j] == 0x22) { neighbors[i][j]++; }
                }
                if(quarter_map[i][j - 1] == 0xaa || quarter_map[i][j - 1] == 0x22) { neighbors[i][j]++; }
                if(j + 1 < 15){
                    if(quarter_map[i][j + 1] == 0xaa || quarter_map[i][j + 1] == 0x22) { neighbors[i][j]++; }
                }
            }
            else{neighbors[i][j] = -1;}
        }
    }

    if(quarter_map[x][y] != 0xff && neighbors[x][y] < 2){
        while(1){
            switch(rand() % 4){
                case 0:
                    if(y - 1 != 0){
                        if(quarter_map[x][y - 1] == 0xff){
                            quarter_map[x][y - 1] = 0xaa;
                        }
                        else{
                            continue;
                        }
                    }
                    else{
                        continue;
                    }
                break;

                case 1:
                    if(y + 1 != 15){
                        if(quarter_map[x][y + 1] == 0xff){
                            quarter_map[x][y + 1] = 0xaa;
                        }
                        else{
                            continue;
                        }
                    }
                    else{
                        continue;
                    }
                break;

                case 2:
                    if(x - 1 != 0){
                        if(quarter_map[x - 1][y] == 0xff){
                            quarter_map[x - 1][y] = 0xaa;
                        }
                        else{
                            continue;
                        }
                    }
                    else{
                        continue;
                    }
                break;

                case 3:
                    if(x + 1 != 20){
                        if(quarter_map[x + 1][y] == 0xff){
                            quarter_map[x + 1][y] = 0xaa;
                        }
                        else{
                            continue;
                        }
                    }
                    else{
                        continue;
                    }
                break;
            }
            break;
        }
    }
    else{
        return;
    }
    check_pos(x, y,quarter_map, neighbors);
}

void show_map(uint8_t** map, int max_x, int max_y, int food_count){
    for(int i = 0; i < max_x; i++){
        for(int j = 0; j < max_y; j++){
            mvaddch(j, i, map[i][j] == 0xff ? '#' : (map[i][j] == 0xaa ? '.' : (map[i][j] == 0x22 ? '*' : ' ')));
        }
        for(int i = 0; i < getmaxx(stdscr); i++){
            mvaddch(35, i, ' ');
        }
        mvprintw(35, 0, "Food remains: %d", food_count);
    }
    refresh();
}

uint8_t** generate_map(){
    srand(time(0));
    uint8_t** quarter_map = (uint8_t**)malloc(20*sizeof(uint8_t*));
    for(int i = 0; i < 20; i++){
        quarter_map[i] = (uint8_t*)malloc(15*sizeof(uint8_t));
    }

    int player_ready = 0;
    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){

            if(i == 0 || j == 0){
                quarter_map[i][j] = 0xff;
                continue;
            }
            
            switch(rand() % 3){
                case 0: // Wall
                    quarter_map[i][j] = 0xff;
                break;

                case 1: //Food
                    quarter_map[i][j] = 0xaa;
                break;

                case 2: //Player
                    if(!player_ready){
                        quarter_map[i][j] = 0x22;
                        player_ready++;
                    }
                    else{
                        j--;
                    }
                break;
            }
        }
    }

    int** neighbors = (int**)malloc(20*sizeof(int*));
    for(int i = 0; i < 20; i++){
        neighbors[i] = (int*)malloc(15*sizeof(int));
    }

    for(int k = 0; k < 2; k++){
        for(int i = 0; i < 20; i++){
            for(int j = 0; j < 15; j++){
                check_pos(i, j, quarter_map, neighbors);
            }
        }
    }
    return quarter_map;
}

uint8_t** unpack_map(uint8_t** quarter_map, int player_count){
    uint8_t** map = (uint8_t**)malloc(40*sizeof(uint8_t*));
    for(int i = 0; i < 40; i++){
        map[i] = (uint8_t*)malloc(30*sizeof(uint8_t));
    }


    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){
            map[i][j] = quarter_map[i][j];
            map[39 - i][j] = quarter_map[i][j];
            map[i][29 - j] = quarter_map[i][j];
            map[39 - i][29 - j] = quarter_map[i][j];
        }
    }



    for(int i = 0; i < 20; i++){
        free(quarter_map[i]);
    }
    free(quarter_map);

    int p = 0;
    for(int j = 0; j < 30; j++){
        for(int i = 0; i < 40; i++){
            if(map[i][j] == 0x22){
                p++ ;
                if(p > player_count){ map[i][j] = 0xaa; }
            }
        }
    }
    return map;
}

int start_server(){
    players_sockets[0] = socket(AF_INET, SOCK_STREAM, 0);
    if(players_sockets[0] == -1){
        printf("Error socket creation\n");
        return 0;
    }
    players_addr[0].sin_family = AF_INET;
    players_addr[0].sin_addr.s_addr = inet_addr("0.0.0.0");
    players_addr[0].sin_port = htons(port);
    players_addr_size[0] = sizeof(players_addr[0]);
    if(bind(players_sockets[0], (struct sockaddr*)&players_addr, sizeof(players_addr)) != 0){
        printf("Error socket bind\n");
        return 0;
    }

    return 1;
}

void init_player(int number, player* pacman, enum direction dir, uint8_t** q_map){
    //Инициализируем направление игрока симметрично
    switch(number){
        case 0:
            pacman->direction = dir;
            break;
        case 1:
            if(dir == right || dir == left){
                pacman->direction = (dir + 2) % 4;
            }
            else{
                pacman->direction = dir;
            }
            break;
        case 2:
            if(dir == up || dir == down){
                pacman->direction = (dir + 2) % 4; 
            }
            else{
                pacman->direction = dir;
            }
            break;
        case 3:
            pacman->direction = (dir + 2) % 4;
            break;
    }

    int x0, y0;
    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){
            if(q_map[i][j] == 0x22){
                x0 = i;
                y0 = j;
            }
        }
    }

    pacman->x = (number == 0 || number == 2) ? x0 : (39 - x0);
    pacman->y = (number == 0 || number == 1) ? y0 : (29 - y0); 

    strcpy(pacman->player_name, player_name[number]);
    pacman->player_name_len = strlen(player_name[number]);
    ////////////////////////////////////////////////////////////////
    printf("Player #%d %s with coords x: %d y: %d and direction %d\n", number, pacman->player_name, pacman->x, pacman->y, pacman->direction);
    //////////////////////////////////////////////////////////////////

}

void init_game(uint8_t** q_map){
    srand(time(0));

    //Находим координаты первого игрока
    enum direction dir;
    int first_x;
    int first_y;
    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){
            if(q_map[i][j] == 0x22){
                first_x = i;
                first_y = j;
            }
        }
    }

    //Генерируем направление
    while(1){
        dir = rand() % 4;
        if(dir == 0){
            if(q_map[first_x][first_y - 1] != 0xff){ break; }
        }
        if(dir == 1){
            if(q_map[first_x + 1][first_y] != 0xff){ break; }
        }
        if(dir == 2){
            if(q_map[first_x][first_y + 1] != 0xff){ break; }
        }
        if(dir == 3){
            if(q_map[first_x - 1][first_y] != 0xff){ break; }
        }
    }

    //Инициализируем игроков
    for(int i = 0; i < player_count; i++){
        init_player(i, &pacman[i], dir, q_map);
    }
}

int server_wait(uint8_t** q_map){
    if(listen(players_sockets[0], player_count - 1) != 0){
        printf("Error socket listen\n");
        return 0;
    }

    for(int i = 1; i < player_count; i++){
        printf("Waiting for players: %d/%d\n", i, player_count);
        players_sockets[i] = accept(players_sockets[0], (struct sockaddr*)&players_addr[i], &players_addr_size[i]);
        if(players_sockets[i] >= 0){
            //Принимаем имя
            packet name_pack;
            recv(players_sockets[i], &name_pack, sizeof(packet), 0);
            if(name_pack.magic != MAGIC || name_pack.ptype != 0x01){
                i--;
                printf("Connection error\n");
                continue;
            }
            player_name[i] = (uint8_t*)malloc(name_pack.datasize + 1);
            recv(players_sockets[i], player_name[i], name_pack.datasize, 0);
            printf("Player %s connected\n", player_name[i]);

            //Отправляем карту
            packet map_pack;
            map_pack.magic = MAGIC;
            map_pack.ptype = 0x10;
            map_pack.datasize = 20*15;
            sendto(players_sockets[i], &map_pack, sizeof(packet), 0, (struct sockaddr*)&players_addr[i], players_addr_size[i]);
            for(int k = 0; k < 15; k++){
                for(int j = 0; j < 20; j++){
                    sendto(players_sockets[i], &q_map[j][k], 1, 0, (struct sockaddr*)&players_addr[i], players_addr_size[i]);
                }
            }
        }
        else{
            printf("Connection error\n");
        }
    }

    init_game(q_map);

    for(int i = 1; i < player_count; i++){
        packet ready_pack;
        recv(players_sockets[i], &ready_pack, sizeof(packet), 0);
        if(ready_pack.ptype != 0x02 || ready_pack.magic != MAGIC){
            i--;
            continue;
        }
    }

    //Отправка всей инфы об игре и игроках



}

int start_client(){
    players_sockets[0] = socket(AF_INET, SOCK_STREAM, 0);
    if(players_sockets[0] == -1){
        printf("Error socket creation\n");
        return 0;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    players_addr_size[0] = sizeof(server_addr);
    return 1;
}

int connect_client(uint8_t** q_map){
    if(connect(players_sockets[0], (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
        printf("Error socket connection\n");
        return 0;
    }
    else{
        printf("Connected\n");
    }

    //Оправляем имя
    packet name_pack;
    name_pack.ptype = 0x01;
    name_pack.magic = MAGIC;
    name_pack.datasize = player_name_len[0];
    sendto(players_sockets[0], &name_pack, sizeof(name_pack), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    sendto(players_sockets[0], player_name[0], player_name_len[0], 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    packet map_pack;
    recv(players_sockets[0], &map_pack, sizeof(packet), 0);
    if(map_pack.ptype == 0x10 && map_pack.magic == MAGIC){
        for(int j = 0; j < 15; j++){
            for(int i = 0; i < 20; i++){
                recv(players_sockets[0], &q_map[i][j], 1, 0);
            }
        }
    }
    else{
        printf("Error getting map\n");
        return 0;
    }

    //Отправляем готовность
    packet ready_pack;
    ready_pack.datasize = 0;
    ready_pack.magic = MAGIC;
    ready_pack.ptype = 0x02;
    sendto(players_sockets[0], &ready_pack, sizeof(packet), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    //Получаем тот самый большой пакет с инфой об игре и игроках




}

void end_game(){
    usleep(300000);
    for(int i = 0; i < 40; i++){
        free(map[i]);
    }
    free(map);
    
    for(int i = 0; i < player_count; i++){
        free(player_name[i]);
    }

    endwin();
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv){
    if(!parse_args(argc, argv)){ return -1; }
    uint8_t** q_map;
    if(is_serv){
        q_map = generate_map();
        if(!start_server() || !server_wait(q_map)){ return -1; }

    }

    else{
        if(!start_client()){ return -1; }
        q_map = (uint8_t**)malloc(20*sizeof(uint8_t*));
        for(int i = 0; i < 20; i++){
            q_map[i] = (uint8_t*)malloc(15*sizeof(uint8_t));
        }

        if(!connect_client(q_map)){ return -1; }
    }

    map = unpack_map(q_map, 3);

    initscr();
    curs_set(0);
    noecho();

    show_map(map, 40, 30, 0);
    getch();

    endwin();

    return 0;
}