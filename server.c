#include "header.h"

#define MAGIC 0xabcdfe01

extern player pacman[4];
extern uint8_t* player_name[4];
extern uint32_t player_name_len[4];
extern uint8_t** map;
extern int player_count;
extern int port;
extern int players_sockets[4];
extern struct sockaddr_in players_addr[4];
extern int players_addr_size[4];

//Запуск сервера
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
            if(name_pack.magic == MAGIC && name_pack.ptype == 0x01){
                player_name[i] = (uint8_t*)malloc(name_pack.datasize + 1);
                recv(players_sockets[i], player_name[i], name_pack.datasize, 0);
            }
            else{
                i--;
                continue;
            }
            printf("Player %s connected\n", player_name[i]);

            //Отправляем карту
            packet map_pack;
            map_pack.magic = MAGIC;
            map_pack.ptype = 0x10;
            map_pack.datasize = 20*15;
            for(int j = 0; j < 15; j++){
                for(int i = 0; i < 20; i++){
                    sendto(players_sockets[i], &q_map[i][j], 1, 0, (struct sockaddr*)&players_addr[i], players_addr_size[i]);
                }
            }
        }
        else{
            printf("Error connection\n");
        }
    }

    //Ждем готовности и начинаем игру
    
}
//Функции сервера

//Запуск функций сервера