#include "header.h"

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

int count(uint8_t** map){
    int food_count = 0;
    for(int i = 0; i < 40; i++){
        for(int j = 0; j < 30; j++){
            if(map[i][j] == 0xaa){
                food_count++;
            }
        }
    }
    return food_count;
}

void clean_map(uint8_t** map, int max_x){
    for(int i = 0; i < max_x; i++){
        free(map[i]);
    }
    free(map);
}
