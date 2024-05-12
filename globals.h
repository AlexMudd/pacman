#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H

//Game info
player pacman[4];
int score[4] = {0, 0, 0, 0};
int food_count;
uint8_t* player_name[4];
uint32_t player_name_len[4];


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
int my_client_socket;
struct sockaddr_in server_addr;

#endif