#include "header.h"

extern player pacman[4];
extern uint8_t** map;
extern int food_count;
extern pthread_mutex_t mutex;


void* get_key(void* p){
    int key;
    while(1){
        key = getch();
        pthread_mutex_lock(&mutex);
        switch(key){
            case 'w':
                if(map[pacman[0].x][pacman[0].y - 1] == 0xff){ break; }
                pacman[0].direction = up;
                break;
            case 'd':
                if(map[pacman[0].x + 1][pacman[0].y] == 0xff){ break; }
                pacman[0].direction = right;
                break;
            case 's':
                if(map[pacman[0].x][pacman[0].y + 1] == 0xff){ break; }
                pacman[0].direction = down;
                break;
            case 'a':
                if(map[pacman[0].x - 1][pacman[0].y] == 0xff){ break; }
                pacman[0].direction = left;
                break;          
        }
        pthread_mutex_unlock(&mutex);
    }
}

void game(){
    while(food_count){
        pthread_mutex_lock(&mutex);
        switch(pacman[0].direction){
            case up:
                if(map[pacman[0].x][pacman[0].y - 1] == 0xff || map[pacman[0].x][pacman[0].y - 1] == 0x22){ break; }
                else{
                    if(map[pacman[0].x][pacman[0].y - 1] == 0xaa){
                        food_count--;
                    }
                    map[pacman[0].x][pacman[0].y] = 0;
                    map[pacman[0].x][pacman[0].y - 1] = 0x22;
                    pacman[0].y--;
                    
                }
                break;
            case down:
                if(map[pacman[0].x][pacman[0].y + 1] == 0xff || map[pacman[0].x][pacman[0].y + 1] == 0x22){ break; }
                
                else{
                    if(map[pacman[0].x][pacman[0].y + 1] == 0xaa){
                        food_count--;
                    }
                    map[pacman[0].x][pacman[0].y] = 0;
                    map[pacman[0].x][pacman[0].y + 1] = 0x22;
                    pacman[0].y++;
                }
                break;
            case right:
                if(map[pacman[0].x + 1][pacman[0].y] == 0xff || map[pacman[0].x + 1][pacman[0].y] == 0x22){ break; }
                else{
                    if(map[pacman[0].x + 1][pacman[0].y] == 0xaa){
                        food_count--;
                    }
                    map[pacman[0].x][pacman[0].y] = 0;
                    map[pacman[0].x + 1][pacman[0].y] = 0x22;
                    pacman[0].x++;
                }
                break;
            case left:
                if(map[pacman[0].x - 1][pacman[0].y] == 0xff || map[pacman[0].x - 1][pacman[0].y] == 0x22){ break; }
                else{
                    if(map[pacman[0].x - 1][pacman[0].y] == 0xaa){
                        food_count--;
                    }
                    map[pacman[0].x][pacman[0].y] = 0;
                    map[pacman[0].x - 1][pacman[0].y] = 0x22;
                    pacman[0].x--;
                }
                break;
        }
        pthread_mutex_unlock(&mutex);
        usleep(200000);
        show_map(map, 40, 30, food_count);
    }
}