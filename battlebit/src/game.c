//
// Created by carson on 5/20/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "game.h"
#include "game_data.h"

// STEP 9 - Synchronization: the GAME structure will be accessed by both players interacting
// asynchronously with the server.  Therefore the data must be protected to avoid race conditions.
// Add the appropriate synchronization needed to ensure a clean battle.

static game * GAME = NULL;

void game_init() {
    if (GAME) {
        free(GAME);
    }
    GAME = malloc(sizeof(game));
    GAME->status = CREATED;
    game_init_player_info(&GAME->players[0]);
    game_init_player_info(&GAME->players[1]);
}

void game_init_player_info(player_info *player_info) {
    player_info->ready_to_play = false;
    player_info->ships = 0;
    player_info->hits = 0;
    player_info->shots = 0;
}

int game_fire(game *game, int player, int x, int y) {
    // Step 5 - This is the crux of the game.  You are going to take a shot from the given player and
    // update all the bit values that store our game state.
    //
    //  - You will need up update the players 'shots' value
    //  - you You will need to see if the shot hits a ship in the opponents ships value.  If so, record a hit in the
    //    current players hits field
    //  - If the shot was a hit, you need to flip the ships value to 0 at that position for the opponents ships field
    //
    //  If the opponents ships value is 0, they have no remaining ships, and you should set the game state to
    //  PLAYER_1_WINS or PLAYER_2_WINS depending on who won.
    
    // get other player (opponent)
    int opponent = get_opponent(player);

    // get bit value of player shot
    unsigned long long int shot_mask = xy_to_bitval(x, y);

    // update player shots using mask
    game->players[player].shots = game->players[player].shots | shot_mask;

    // update game status for next player turn
    game->status = (player == 0) ? PLAYER_1_TURN : PLAYER_0_TURN;

    // check if shot was a hit
    if(game->players[opponent].ships & shot_mask) { // hit was a success
        // update player hits
        game->players[player].hits = game->players[player].hits | shot_mask;
        // remove hit ship location
        game->players[opponent].ships = game->players[opponent].ships ^ shot_mask;

    } else { // miss cannot result in game end (final ship still standing)
        return 0;
    }


    // game over? check if opponent has any ships left
    if(game->players[opponent].ships == 0) { // GAME OVER
        // set game status of winner
        game->status = (player == 0) ? PLAYER_0_WINS : PLAYER_1_WINS;
    } else { // ships remain, change turns
        // set game status to other player's turn
        game->status = (player == 0) ? PLAYER_1_TURN : PLAYER_0_TURN;
    }

    return 1;
}

unsigned long long int xy_to_bitval(int x, int y) {
    // Step 1 - implement this function.  We are taking an x, y position
    // and using bitwise operators, converting that to an unsigned long long
    // with a 1 in the position corresponding to that x, y
    //
    // x:0, y:0 == 0b00000...0001 (the one is in the first position)
    // x:1, y: 0 == 0b00000...10 (the one is in the second position)
    // ....
    // x:0, y: 1 == 0b100000000 (the one is in the eighth position)
    //
    // you will need to use bitwise operators and some math to produce the right
    // value.
    // return 0;

    if(is_valid_coordinate(x,y)) {
        return 1ull << x << y * 8;
    }
    return 0ull;
}

struct game * game_get_current() {
    return GAME;
}

int game_load_board(struct game *game, int player, char * spec) {
    // Step 2 - implement this function.  Here you are taking a C
    // string that represents a layout of ships, then testing
    // to see if it is a valid layout (no off-the-board positions
    // and no overlapping ships)
    //

    // if it is valid, you should write the corresponding unsigned
    // long long value into the Game->players[player].ships data
    // slot and return 1
    //
    // if it is invalid, you should return -1



    // ---------------------------------------------
    // First, we will validate the input given to us
    // 1 - validate player value
    // 2 - validate the board spec
    // ---------------------------------------------


    // were we given an integer?
    if(typename(player) != "int") {
        return -1;
    }
    // validate player value - is the player a correct value?
    // player must either be 0 or 1... we only have 2 players
    if(player > 1 || player < 0) {
        // invalid player value, return -1
        return -1;
    }


    // validate spec (i.e. a user input)
    // ---------------------------------

    if(typename(spec) != "pointer to char" || typename(*spec) != "char") {
        return -1;
    }

    // if spec is NULL or not valid length, return -1
    if(spec == NULL || strlen(spec) != 15) {
        return -1;
    }

    struct player_info temp_player_info;
    game_init_player_info(&temp_player_info);
    struct ships_data player_ships;
    init_ships_data(&player_ships);

    for(int i = 0; i < 15; i = i+3) {
        char ship_type_char = spec[i],
             ship_x = spec[i+1],
             ship_y = spec[i+2];
        if(is_char_valid_ship(ship_type_char) == false) {
            return -1;
        }

        if(has_seen_char_ship(ship_type_char, &player_ships) == true) {
            return -1;
        }

        if(is_char_valid_coordinate(ship_x) == false) {
            return -1;
        }
        if(is_char_valid_coordinate(ship_y) == false) {
            return -1;
        }
        int ship_len = get_char_ship_type_len(ship_type_char),
            x_coordinate = ship_x -'0',
            y_coordinate = ship_y - '0',
            ship_added_success = 1;

        if(is_ship_horizontal(ship_type_char) == true) {
            ship_added_success = add_ship_horizontal(&temp_player_info, x_coordinate, y_coordinate, ship_len);
        } else {
            ship_added_success = add_ship_vertical(&temp_player_info, x_coordinate, y_coordinate, ship_len);
        }

        if(ship_added_success == -1) {
            return -1;
        }

        mark_ship_seen(ship_type_char, &player_ships);
    }


    if(seen_all_ships(&player_ships) == false) {
        return -1;
    }

    game->players[player].ships = temp_player_info.ships;
    game->players[player].ready_to_play = true;

    // both players ready to play?
    if(players_ready(game) == true) {
        game->status = PLAYER_0_TURN; // set game status to player turn
    }

    return 1;
}

int add_ship_horizontal(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively
    if(length == 0) {
        return 1;
    }
    if(is_valid_coordinate(x, y) == false) {
        return -1;
    }
    unsigned long long int mask = xy_to_bitval(x, y);
    if(player->ships & mask) {
        return -1;
    }
    if(add_ship_horizontal(player, x+1, y, length-1) == -1) {
        return -1;
    }
    player->ships = player->ships | mask;
    return 1;
}

int add_ship_vertical(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively
    if(length == 0) {
        return 1;
    }
    if(is_valid_coordinate(x, y) == false) {
        return -1;
    }
    unsigned long long int mask = xy_to_bitval(x, y);
    if(player->ships & mask) {
        return -1;
    }
    if(add_ship_vertical(player, x, y+1, length-1) == -1) {
        return -1;
    }
    player->ships = player->ships | mask;
    return 1;
}
