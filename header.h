#pragma once
#ifndef HEADER_H
#define HEADER_H

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

uint8_t** generate_map();
void show_map(uint8_t** map, int max_x, int max_y, int food_count);
uint8_t** unpack_map(uint8_t** quarter_map, int player_count);
int count(uint8_t** map);
void clean_map(uint8_t** map, int max_x);
void start_game(int player_count, player* pacman, uint8_t** map);
void end_game(uint8_t** map);
int parse_args(int argc, char** argv);
void init_player(int number, player* pacman, uint8_t** map, enum direction dir);
void* get_key(void* p);
void game();
int start_server();
int server_wait(uint8_t** q_map);





#endif