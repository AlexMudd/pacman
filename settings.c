#include "header.h"

extern int food_count;
extern pthread_t key;
extern int is_serv;
extern int player_count;
extern int port;
extern char ip[15];
extern uint8_t* player_name[4];
extern uint32_t player_name_len[4];

void start_game(int player_count, player* pacman, uint8_t** map){
    srand(time(0));
    initscr();
    curs_set(0);
    noecho();

    show_map(map, 40, 30, food_count);

    enum direction dir;
    int first_x;
    int first_y;
    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 15; j++){
            if(map[i][j] == 0x22){
                first_x = i;
                first_y = j;
            }
        }
    }

    while(1){
        dir = rand() % 4;
        if(dir == 0){
            if(map[first_x][first_y - 1] != 0xff){ break; }
        }
        if(dir == 1){
            if(map[first_x + 1][first_y] != 0xff){ break; }
        }
        if(dir == 2){
            if(map[first_x][first_y + 1] != 0xff){ break; }
        }
        if(dir == 3){
            if(map[first_x - 1][first_y] != 0xff){ break; }
        }
    }
    for(int i = 0; i < player_count; i++){
        init_player(i, &pacman[i], map, dir);
    }
}

void init_player(int number, player* pacman, uint8_t** map, enum direction dir){
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
    int counter = 0;
    for(int i = 0; i < 40; i++){
        for(int j = 0; j < 30; j++){
            if(map[i][j] == 0x22 && counter >= 0){
                if(counter == number){
                    pacman->x = i;
                    pacman->y = j;
                    counter = -777;
                }
                else{ counter++; }
            }
        }
    }





    int c = 5 -3;
}

void end_game(uint8_t** map){
    usleep(300000);
    for(int i = 0; i < 40; i++){
        free(map[i]);
    }
    free(map);
    
    for(int i = 0; i < player_count; i++){
        free(player_name[i]);
    }

    pthread_detach(key);

    endwin();
}

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
    if(player_count < 1 || player_count > 4){
        printf("Incorrect max players count (must be 1-4)\n");
        return 0;
    }
    if(player_name_len[0] < 1){
        printf("Incorrect name\n");
        return 0;
    }
    return 1;
}
