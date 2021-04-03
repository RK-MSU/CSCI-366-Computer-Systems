
#include <ctype.h>
#include <stdbool.h>
#include "game_data.h"
#include "game.h"



int get_opponent(int player) {
    return (player + 1) % 2;
}

bool players_ready(struct game *game) {
    if (game->players[0].ready_to_play != true || game->players[1].ready_to_play != true) return false;
    return true;
}

bool other_player_ready(struct game *game, int player) {
    int opponent = get_opponent(player);
    return game->players[opponent].ready_to_play;
}


int get_char_ship_type_len(char ship) {
    int len = 0;

    if(is_char_valid_ship(ship) == false) {
        return -1;
    }

    ship = tolower(ship);

    switch (ship) {
        case 'c':
            len = 5;
            break;
        case 'b':
            len = 4;
            break;
        case 'd':
            len = 3;
            break;
        case 's':
            len = 3;
            break;
        case 'p':
            len = 2;
            break;
    }

    return len;
}


bool is_ship_horizontal(char ship) {
    return (isupper(ship)) ? true : false;
}

void init_ships_data(struct ships_data * ship_data) {
    ship_data->carrier = 0;
    ship_data->battleship = 0;
    ship_data->destroyer = 0;
    ship_data->submarine = 0;
    ship_data->patrol_boat = 0;
}


void mark_ship_seen(char ship, struct ships_data * data) {
    ship = tolower(ship);
    switch (ship) {
        case 'c':
            data->carrier = 1;
            break;
        case 'b':
            data->battleship = 1;
            break;
        case 'd':
            data->destroyer = 1;
            break;
        case 's':
            data->submarine = 1;
            break;
        case 'p':
            data->patrol_boat = 1;
            break;
    }
}

bool seen_all_ships(struct ships_data *data) {
    bool seen_all = false;

    if(data->battleship == 1 && data->carrier == 1 && data->destroyer == 1 && data->patrol_boat == 1 && data->submarine == 1) {
        seen_all = true;
    }

    return seen_all;
}

bool has_seen_char_ship(char ship, struct ships_data *data) {
    bool seen = false;
    ship = tolower(ship);
    switch (ship) {
        case 'c':
            if(data->carrier == 1) {
                seen = true;
            }
            break;
        case 'b':
            if(data->battleship == 1) {
                seen = true;
            }
            break;
        case 'd':
            if(data->destroyer == 1) {
                seen = true;
            }
            break;
        case 's':
            if(data->submarine == 1) {
                seen = true;
            }
            break;
        case 'p':
            if(data->patrol_boat == 1) {
                seen = true;
            }
            break;
    }
    return seen;
}





bool is_char_valid_ship(char ship) {
    ship = tolower(ship);
    bool is_valid = false;

    switch (ship) {
        case 'c':
            is_valid = true;
            break;
        case 'b':
            is_valid = true;
            break;
        case 'd':
            is_valid = true;
            break;
        case 's':
            is_valid = true;
            break;
        case 'p':
            is_valid = true;
            break;
    }
    return is_valid;
}

bool is_char_valid_coordinate(char coordinate) {
    for(int i = 0; i < BOARD_DIMENSION; i++) {
        char temp = i + '0';
        if(coordinate == temp) {
            return true;
        }
    }
    return false;
}


bool is_valid_coordinate(int x, int y) {
    bool is_valid = true;
    if(x < 0 || BOARD_DIMENSION <= x || y < 0 || BOARD_DIMENSION <= y) {
        is_valid = false;
    }
    return is_valid;
}