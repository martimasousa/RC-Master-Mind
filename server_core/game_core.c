#include "game_core.h"
// Processes commands received via UDP or TCP

void process_command_udp(int udp_fd, struct sockaddr_in *client_addr, const char *command) {
    char type[COMMAND_LEN + 1 + 1];
    sscanf(command, "%s", type);

    if (!strcmp(SNG_CMD, type)) {
        process_sng_command(udp_fd, client_addr, command);
        return;

    } else if (!strcmp(TRY_CMD, type)) {
        process_try_command(udp_fd, client_addr, command);
        return;

    } else if (!strcmp(QUT_CMD, type)) {
        process_qut_command(udp_fd, client_addr, command);
        return;

    } else if (!strcmp(DBG_CMD, type)) {
        process_dbg_command(udp_fd, client_addr, command);
        return;
    } else {
        process_udp_uknown_command(udp_fd, client_addr);
        return;
    }
}

void process_command_tcp(int client_fd, const char *command) {
    char type[COMMAND_LEN + 1];
    sscanf(command, "%s\n", type);

    if (!strcmp(STR_CMD, type)) {
        process_str_command(client_fd, command);
        return;

    } else if (!strcmp(SSB_CMD, type)) {
        process_ssb_command(client_fd, command);
        return;

    } else {
        process_tcp_uknown_command(client_fd);
        return;
    }
}


// Specific handlers for known UDP commands

void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS + 1];
    char PLID[PLID_DIGITS + 1];
    GameTry *game_solution = malloc(sizeof(GameTry));
    char *response;

    if (validate_start_command(command, PLID, time, SERVER_SIDE) == ERROR) {
        response = "RSG ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    // If the player had an ongoing game but the request was made after the game
    // max time, end the other game and start the new one
    if (has_ongoing_game(PLID)) {
        if (!has_exceeded_time(PLID)) {
            response = "RSG NOK\n";
            send_udp_response(udp_fd, response, client_addr);
            return;
        }
        end_game(PLID, END_TIMEOUT, NOT_NEEDED);
    }
    

    if (start_game(PLID, time, PLAY_MODE, game_solution) == OK) response = "RSG OK\n";
    else response = "RSG NOK\n";

    send_udp_response(udp_fd, response, client_addr);
}

void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    GameTry player_try;
    char PLID[PLID_DIGITS + 1], nt;
    char *response;

    if (validate_try_command(command, PLID, &player_try.colours[0], &player_try.colours[1], 
                                            &player_try.colours[2], &player_try.colours[3], 
                                            &nt, SERVER_SIDE)) {
        response = "RTR ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    response = check_inv_status(PLID, nt, player_try);
    if (response != NULL) {
        send_udp_response(udp_fd, response, client_addr);
        return;
    }


    if (!has_ongoing_game(PLID)) {
        response =  "RTR NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }
    
    if (has_exceeded_time(PLID)) {
        response = build_end_game_response(PLID, "RTR", "ETM");
        end_game(PLID, END_TIMEOUT, NOT_NEEDED);
        send_udp_response(udp_fd, response, client_addr);
        return;
    }
    
    if (is_duplicated(PLID, &player_try)) {
        response = "RTR DUP\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    } 
    

    int* player_try_res = make_try(PLID, player_try);
    response = generate_try_result_message(player_try_res, nt);

    if (has_won(player_try_res)) {
        end_game(PLID, END_WIN, convert_char_to_int(nt));
    } else if (has_reached_max_turn(nt)) {
            response = build_end_game_response(PLID, "RTR", "ENT");
            end_game(PLID, END_FAIL, NOT_NEEDED);
            send_udp_response(udp_fd, response, client_addr);
            return;
    }

    send_udp_response(udp_fd, response, client_addr);
}

void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char PLID[PLID_DIGITS + 1];
    int end_type;
    char *response;
    
    if (validate_quit_command(command, PLID, SERVER_SIDE, NONE) == ERROR) {
        response = "RQT ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    /* If there is not an ongoing game, respond with "RQT NOK" */
    if (!has_ongoing_game(PLID)) {
        response = "RQT NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    // If the player had an ongoing game but the request was made after the game
    // max time, end the other game and send RQT NOK
    if (has_exceeded_time(PLID)) {
        end_type = END_TIMEOUT;
        response = "RQT NOK\n";
    } else {
        end_type = END_QUIT;
        response = build_end_game_response(PLID, "RQT", "OK");
    }

    end_game(PLID, end_type, NOT_NEEDED);

    send_udp_response(udp_fd, response, client_addr);
}

void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS + 1];
    char PLID[PLID_DIGITS + 1];
    GameTry *game_solution = malloc(sizeof(GameTry));
    char *response;

    if (validate_debug_command(command, PLID, time,  
                              &game_solution->colours[0],
                              &game_solution->colours[1],
                              &game_solution->colours[2],
                              &game_solution->colours[3], SERVER_SIDE)) {

        response = "RDB ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    // If the player had an ongoing game but the request was made after the game
    // max time, end the other game and send RDB NOK
    if (has_ongoing_game(PLID)) {
        if (!has_exceeded_time(PLID)) {
            response = "RDB NOK\n";
            send_udp_response(udp_fd, response, client_addr);
            return;
        }
        end_game(PLID, END_TIMEOUT, NOT_NEEDED);
    }

    if (start_game(PLID, time, DEBUG_MODE, game_solution) == OK) response = "RDB OK\n";
    else response = "RDB NOK\n";

    send_udp_response(udp_fd, response, client_addr);
}

void process_udp_uknown_command(int udp_fd, struct sockaddr_in *client_addr) {

    char *response = "ERR\n";
    send_udp_response(udp_fd, response, client_addr);
    
    return;
}


// Specific handlers for known TCP commands

void process_str_command(int client_fd, const char *command) {

    char PLID[PLID_DIGITS + 1];
    char *response;
    char *filepath;

    if (validate_showtrials_command(command, PLID, SERVER_SIDE) == ERROR) {
        response = "RST NOK\n";
        tcp_write(client_fd, response);
        return;
    }

    // If the player had an ongoing game that exceeded the maximum allowed time, 
    // terminate the game and send the trial results from the ended game.
    int game_ended = TRUE;
    if (has_ongoing_game(PLID)) {
        if (has_exceeded_time(PLID)) {
            end_game(PLID, END_TIMEOUT, NOT_NEEDED);
        } else {
            game_ended = FALSE;
            filepath = get_game_folder_path(PLID);
        }
    }
    
    if (game_ended) {
        filepath = malloc(sizeof(char) * BUFFER_SIZE);
        FindLastGame(PLID, filepath);
    }
    
    execute_show_trials(client_fd, filepath, PLID);
}

void process_ssb_command(int client_fd, const char *command) {
    
    char *response;

    if (validate_scoreboard_command(command, SERVER_SIDE) == ERROR) {
        response = "RSS ERR\n";
        tcp_write(client_fd, response);
        return;
    }

    SCORELIST *files = malloc(sizeof(SCORELIST));
    if (FindTopScores(files) <= 0) {
        response = "RSS EMPTY\n";
        if (tcp_write(client_fd, response)) {
            fprintf(stderr, "Error: Error while writing to TCP socket.\n");
            return;
        }
        return;
    }

    response = build_scoreboard_response(files);
    if (response == NULL) return;

    if(tcp_write(client_fd, response)) {
        fprintf(stderr, "Error: Error while writing to TCP socket.\n");
    }

    free(response);
    return;
}

void process_tcp_uknown_command(int client_fd) {

    char *response = "ERR\n";
    if(tcp_write(client_fd, response)) {
        fprintf(stderr, "Error: Error while writing to TCP socket.\n");
    }

    return;
}