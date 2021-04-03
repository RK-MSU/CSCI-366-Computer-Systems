//
// Created by carson on 5/20/20.
//

#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>
#include "server.h"
#include "char_buff.h"
#include "game.h"
#include "game_data.h"
#include "repl.h"
#include "pthread.h"
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h>    //inet_addr
#include<unistd.h>    //write

static game_server *SERVER;

void init_server() {
    if (SERVER == NULL) {
        SERVER = calloc(1, sizeof(struct game_server));
    } else {
        printf("Server already started");
    }
}

int handle_client_connect(int player) {
    // STEP 8 - This is the big one: you will need to re-implement the REPL code from
    // the repl.c file, but with a twist: you need to make sure that a player only
    // fires when the game is initialized and it is there turn.  They can broadcast
    // a message whenever, but they can't just shoot unless it is their turn.
    //
    // The commands will obviously not take a player argument, as with the system level
    // REPL, but they will be similar: load, fire, etc.
    //
    // You must broadcast informative messages after each shot (HIT! or MISS!) and let
    // the player print out their current board state at any time.
    //
    // This function will end up looking a lot like repl_execute_command, except you will
    // be working against network sockets rather than standard out, and you will need
    // to coordinate turns via the game::status field.


    // we need some buffers: one for output/input, a char[] for raw input, and an int for the read_size of the input
    // output_buffer - used to send output messages
    char_buff *output_buffer = cb_create(DEFAULT_BUFFER_SIZE);
    // input_buffer - used to send output messages
    char_buff *input_buffer = cb_create(DEFAULT_BUFFER_SIZE);
    // raw_input_data
    char raw_input_data[DEFAULT_BUFFER_SIZE];
    // temp message - used for formatting, then appending to a buffer
    char temp_message[DEFAULT_BUFFER_SIZE];
    // integer used to signify the size of the input
    int input_read_size;


    // bool to indicate that the player has exited the game
    bool exit_game = false;


    // the user just joined, lets send out the welcome message and input prompt
    sprintf(temp_message, "\n--- Welcome to BattleBit ---\nYou are Player: %d\nbattleBit (? for help) > ", player);
    cb_append(output_buffer, temp_message);
    // send/write message
    cb_write(SERVER->player_sockets[player], output_buffer);

    while((input_read_size = recv(SERVER->player_sockets[player], raw_input_data, DEFAULT_BUFFER_SIZE, 0)) > 0) {

        // reset output and input buffers buffer
        cb_reset(output_buffer);
        cb_reset(input_buffer);

        cb_append(output_buffer, "\n");

        // is the read_size appropriate?
        if(input_read_size < 1) { // nope
            cb_append(output_buffer, "No command given, try again!\n");
            cb_write(SERVER->player_sockets[player], output_buffer);
            continue; // skip loop
        }

        // Where does the raw input end? Add termination
        raw_input_data[input_read_size-2] = '\0';

        // set raw input to input_buffer
        cb_append(input_buffer, raw_input_data);

        // get user input command... similar to repl.c->repl_execute_command()
        char* command = cb_tokenize(input_buffer, " \n");

        if(!command) { // command empty?
            cb_append(output_buffer, "No command given, try again!\n");
            cb_write(SERVER->player_sockets[player], output_buffer);
            continue; // skip loop
        }

        char* arg1 = cb_next_token(input_buffer);
        char* arg2 = cb_next_token(input_buffer);


        /*
        // debugging...
        sprintf(temp_message, "User Input: %s\n", input_buffer->buffer);
        cb_append(output_buffer, temp_message);

        sprintf(temp_message, "Input Read Size: %d\n", input_read_size);
        cb_append(output_buffer, temp_message);

        sprintf(temp_message, "Input Command: %s\n", command);
        cb_append(output_buffer, temp_message);

        sprintf(temp_message, "Arg1: %s\n", arg1);
        cb_append(output_buffer, temp_message);

        sprintf(temp_message, "Arg2: %s\n", arg2);
        cb_append(output_buffer, temp_message);

        cb_write(SERVER->player_sockets[player], output_buffer);
        continue;
         */


        // process the command
        if(strcmp(command, "?") == 0) { // show help
            cb_append(output_buffer, "? - show help\n");
            cb_append(output_buffer, "load <string> - load a ship layout file for the given player\n");
            cb_append(output_buffer, "show - shows the board for the given player\n");
            cb_append(output_buffer, "fire [0-7] [0-7] - fire at the given position\n");
            cb_append(output_buffer, "say <string> - Send the string to all players as part of a chat\n");
            cb_append(output_buffer, "exit - quit the game\n");

        } else if (strcmp(command, "load") == 0) { // load player board
            if(game_load_board(game_get_current(), player, arg1) != -1) { // board loaded successfully
                cb_append(output_buffer, "Game board loaded successfully!\n");
            } else { // show invalid board load message
                sprintf(temp_message, "Invalid load game board input: %s\nPlease try again.\n", arg1);
                cb_append(output_buffer, temp_message);
            }

        } else if (strcmp(command, "show") == 0) { // show player battleship game details
            repl_print_board(game_get_current(), player, output_buffer);

        } else if (strcmp(command, "fire") == 0) { // fire a shot!
            // get x y coordinates from input args
            int x = atoi(arg1);
            int y = atoi(arg2);

            if(is_valid_coordinate(x,y) == true) { // valid coordinates
                sprintf(temp_message, "Player %i fired at %i %i\n", player + 1, x, y);
                if (game_fire(game_get_current(), player, x, y)) {
                    cb_append(output_buffer,"  HIT!!!\n");
                } else {
                    cb_append(output_buffer,"  Miss\n");
                }
            } else {
                // invalid coordinates, send error message
                sprintf(temp_message, "Invalid coordinate: %i %i\n", x, y);
                cb_append(output_buffer, temp_message);
            }
        } else if (strcmp(command, "say") == 0) { // send a message!

            char_buff *player_msg = cb_create(DEFAULT_BUFFER_SIZE);

            if(arg1 != NULL) {
                sprintf(temp_message, "%s", arg1);
                cb_append(player_msg, temp_message);

                if(arg2 != NULL) {
                    sprintf(temp_message, " %s", arg2);
                    cb_append(player_msg, temp_message);

                    char *next_arg;
                    while((next_arg = cb_next_token(input_buffer)) != NULL) {
                        sprintf(temp_message, " %s", next_arg);
                        cb_append(player_msg, temp_message);
                    }
                }

                prepare_player_message(player_msg, player);

                if(message_other_player(player_msg, player)) {
                    cb_append(output_buffer, "Message Sent!\n");
                } else {
                    cb_append(output_buffer, "Other player not connected, try again later!\n");
                }

            } else {
                // no message details...
                cb_append(output_buffer, "Message is empty, cannot send blank messages.\n");
            }

            cb_free(player_msg);

        } else if (strcmp(command, "exit") == 0) { // exit the game
            cb_append(output_buffer, "Goodbye!\n\n");
            exit_game = true;
        } else { // show some error message for incorrect input
            sprintf(temp_message, "Unknown Command: %s\n", command);
            cb_append(output_buffer, temp_message);
        }

        if(exit_game == false) {
            cb_append(output_buffer, "\nbattleBit (? for help) > ");
        }

        // write out message results to player
        cb_write(SERVER->player_sockets[player], output_buffer);

        // exit the game?
        if(exit_game == true) {
            close(SERVER->player_sockets[player]);
            SERVER->player_sockets[player] = NULL;
            break;
        }

    } // END OF - fetch user input loop

    cb_free(output_buffer);
    cb_free(input_buffer);

    return 0;
}

void server_broadcast(char_buff *msg) {
    // send message to all players

    // who are we sending to? player-1 and player-2... obviously.
    int player1 = 0;
    int player2 = 1;

    // output message to each player
    if(SERVER->player_sockets[player1] != NULL) {
        cb_write(SERVER->player_sockets[player1], msg);
    }
    if(SERVER->player_sockets[player2] != NULL) {
        cb_write(SERVER->player_sockets[player2], msg);
    }
}


void prepare_player_message(char_buff *msg, int player) {
    char msg_div[] = "\n-------------------------\n";
    char message_details[DEFAULT_BUFFER_SIZE];
    sprintf(message_details, "\n\n-------------------------\nMessage from Player: %d\n-------------------------\n%s\n-------------------------\n", player, msg->buffer);
    cb_reset(msg);
    cb_append(msg, message_details);

}

bool message_other_player(char_buff *msg, int player) {
    int opponent = get_opponent(player);
    if(SERVER->player_sockets[opponent] != NULL) {
        cb_append(msg, "\nbattleBit (? for help) > ");
        cb_write(SERVER->player_sockets[opponent], msg);
        return true;
    }
    return false;
}


int run_server() {
    // STEP 7 - implement the server code to put this on the network.
    // Here you will need to initalize a server socket and wait for incoming connections.
    //
    // When a connection occurs, store the corresponding new client socket in the SERVER.player_sockets array
    // as the corresponding player position.
    //
    // You will then create a thread running handle_client_connect, passing the player number out
    // so they can interact with the server asynchronously


    // Create a socket file descriptor
    // (domain, type, protocol)
    //
    // - AF_INET: Internet IP Protocol
    // - SOCK_STREAM: String type
    // - IPPROTO_TCP: Use TCP
    int server_socket_fd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);

    // error detection (C sucks...)
    if (server_socket_fd == -1) {
        printf("Could not create socket\n");
    }

    // set the socket option
    // IMPORTANT for re-using the same address (lets say if the server crashes)
    int yes = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // create struct for representing the internet address
    // Note: created on the stack (remember what will happen in life time in C)
    // we will need to update some settings in the following step inorder to make a connection
    struct sockaddr_in server;

    // fill out the socket information
    server.sin_family = AF_INET;
    // bind the socket on all available interfaces
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9876);


    // now we are ready to attempt binding
    // Note: Again with the cast (struct sockaddr *)
    int request = 0;
    if (bind(server_socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        // something went wrong and binding failed...
        puts("Bind failed");
    } else {
        // binding was a success!
        puts("Bind worked!");

        // But, the server is not ready, we need to start listening
        // let us also defined the number of connection allowed to be made
        // at once... for us it will be 3
        listen(server_socket_fd, 3);
        // now we are listening on the network

        //Accept an incoming connection
        puts("Waiting for incoming connections...");

        // The server is running, and we are now waiting for incoming connection...
        // When a connection is made, we will be creating/using a new file descriptor.

        // create the new file descriptor for the incoming connection.
        struct sockaddr_in client;
        socklen_t size_from_connect;
        int client_socket_fd; // everything is an number...
        int request_count = 0; // will be used to identify connecting players

        // begin searching for client connections
       while ((client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client, &size_from_connect)) > 0) {
            // ... while accepting/connecting to client sockets
            // set the server player socket by the new client socket file descriptor
            // pass client connection off to a thread... single thread severs are boring... and useless
            // update request count
            SERVER->player_sockets[request_count] = client_socket_fd;
            pthread_create(&SERVER->player_threads[request_count], NULL, handle_client_connect, request_count);
            request_count++;

        }
    }
}

int server_start() {
    // STEP 6 - using a pthread, run the run_server() function asynchronously, so you can still
    // interact with the game via the command line REPL

    // initialize the server
    init_server();
    // run the server (on a new thread)
    pthread_create(&SERVER->server_thread, NULL, run_server, NULL);
}
