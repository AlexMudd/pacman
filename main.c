#include "header.h"
#include "globals.h"

int main(int argc, char** argv){
    if(!parse_args(argc, argv)){ return 0; }

    if(is_serv){
        uint8_t** q_map = generate_map();
        if(!start_server()){
            clean_map(q_map, 20);
            free(player_name[0]);
            return 0;
        }

        //Сервер ждет всех, отправляет инфу


        map = unpack_map(generate_map(), player_count);
    }



    start_game(player_count, pacman, map);
    pthread_create(&key, 0, get_key, 0);
    food_count = count(map);
    game();

    end_game(map);
    return 0;
}