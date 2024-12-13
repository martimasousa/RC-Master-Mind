#include "game_core.h"

void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS + 1];
    char PLID[PLID_DIGITS + 1];
    GameTry *game_solution = malloc(sizeof(GameTry));
    char *response;

    if (validate_start_command(command, PLID, time, SERVER_SIDE) == ERROR) {
        char *response = "RSG ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    /* If there is an ongoing game, respond with "RSG NOK" */
    if (has_ongoing_game(PLID)) {
        char *response = "RSG NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    if (start_game(PLID, time, PLAY_MODE, game_solution) == OK) response = "RSG OK\n";
    else response = "RSG NOK\n";

    send_udp_response(udp_fd, response, client_addr);
}

void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    GameTry player_try;
    char PLID[PLID_DIGITS + 1], nt;

    if (validate_try_command(command, PLID, &player_try.colours[0], &player_try.colours[1], 
                                            &player_try.colours[2], &player_try.colours[3], 
                                            &nt, SERVER_SIDE)) {
        char *response = "RTR ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    sscanf(command, "TRY %s %c %c %c %c %c\n", PLID, &player_try.colours[0], &player_try.colours[1], 
                                                   &player_try.colours[2], &player_try.colours[3], &nt);

    // TODO: Verify Sintaxe!

    // TODO: Verify INV!


    if (!has_ongoing_game(PLID)) {
        char *response =  "RTR NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    } 
    
    if (has_exceeded_time(PLID)) {
        char *response = build_end_game_response(PLID, "RTR", "ETM");
        end_game(PLID, END_TIMEOUT, NOT_NEEDED);
        send_udp_response(udp_fd, response, client_addr);
        return;
    }
    
    if (is_duplicated(PLID, &player_try)) {
        char *response = "RTR DUP\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    } 
    
    if (has_exceeded_max_turn(nt)) {
        char *response = build_end_game_response(PLID, "RTR", "ENT");
        end_game(PLID, END_FAIL, NOT_NEEDED);
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    int* player_try_res = make_try(PLID, player_try);

    size_t response_len = COMMAND_LEN + 1 + RESPONSE_LEN + TRIAL_MAX_LEN*3*2 + 2;
    char response[response_len];
    sprintf(response, "RTR OK %c %d %d\n", nt, player_try_res[0], player_try_res[1]);

    if (has_won(player_try_res)) {
        end_game(PLID, END_WIN, convert_char_to_int(nt));
    }

    send_udp_response(udp_fd, response, client_addr);
}

void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char PLID[PLID_DIGITS + 1];
    
    if (validate_quit_command(command, PLID, SERVER_SIDE, NONE) == ERROR) {
        char *response = "RQT ERR\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    // if (has_exceeded_time(PLID)) {
    //     char *response = build_end_game_response(PLID, "RQT", "OK");
    //     end_game(PLID, END_TIMEOUT, NOT_NEEDED);
    //     send_udp_response(udp_fd, response, client_addr);
    //     return;
    // }

    /* If there is an ongoing game, respond with "RQT NOK" */
    if (!has_ongoing_game(PLID)) {
        char *response = "RQT NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    /* Get the solution and send the message */
    char *response = build_end_game_response(PLID, "RQT", "OK");

    end_game(PLID, END_QUIT, NOT_NEEDED);
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


    /* If there is an ongoing game, respond with "RSG NOK" */
    if (has_ongoing_game(PLID)) {
        char *response = "RDB NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    if (start_game(PLID, time, DEBUG_MODE, game_solution) == OK) response = "RDB OK\n";
    else response = "RDB NOK\n";

    send_udp_response(udp_fd, response, client_addr);
}

void process_str_command(int client_fd, const char *command) {

    char PLID[PLID_DIGITS + 1];
    char *response;
    char *filepath;

    if (validate_showtrials_command(command, PLID, SERVER_SIDE) == ERROR) {
        response = "RST NOK\n";
        tcp_write(client_fd, response);
        return;
    }

    // if (has_exceeded_time(PLID)) {
    //     char *response = build_end_game_response(PLID, "RTR", "ETM");
    //     end_game(PLID, END_TIMEOUT, NOT_NEEDED);
    //     send_udp_response(udp_fd, response, client_addr);
    //     return;
    // }

    char *PLID_to_send = NULL;
    if (!has_ongoing_game(PLID)) {
        filepath = malloc(sizeof(char) * BUFFER_SIZE);
        FindLastGame(PLID, filepath);
    } else {
        filepath = get_game_folder_path(PLID);
        PLID_to_send = PLID;
    }

    // Send PLID as NULL if game not active (to avoid returning remaining time)
    execute_show_trials(client_fd, filepath, PLID_to_send);
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
            // Error handling
            return;
        }
        return;
    }

    // Create a string (Fdata) containing all the scores returned
    char *Fdata = malloc(1);
    Fdata[0] = '\0';
    size_t Fsize = 1;
    for (int i = 0; i < files->nscores; i++) {
        char line[6 + 1 + 4 + 1 + 1 + 1 + 1]; // PLID + SPACE + CODE + SPACE + N_PLAYS + \n

        char *PLID = files->PLID[i];
        char *colcode = files->colcode[i];
        int notries = files->notries[i];

        sprintf(line, "%d: %s %s %d\n", i+1, PLID, colcode, notries);

        Fsize += strlen(line);
        Fdata = realloc(Fdata, Fsize);
        strcat(Fdata, line); // Append line to the end of Fdata
    }

    // Create filename
    char filename[MAX_FNAME];
    time_t now = time(NULL);
    sprintf(filename, "SCOREBOARD_%ld", now);

    // Write Fname and Fsize
    char res_init[20];
    sprintf(res_init, "RSS OK %s %ld ", filename, Fsize);
    if (tcp_write(client_fd, res_init)) {
        free(Fdata);
        return;
    }

    // Write Fdata
    if (tcp_write(client_fd, Fdata)) {
        free(Fdata);
        return;
    }

    free(Fdata);
    return;
}


void process_command_udp(int udp_fd, struct sockaddr_in *client_addr, const char *command) {
    char type[3];
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
        fprintf(stderr, "Error: %s\n", type);
        return;
    }
}

void process_command_tcp(int client_fd, const char *command) { 
    char type[3];
    sscanf(command, "%s", type);

    if (!strcmp(STR_CMD, type)) {
        process_str_command(client_fd, command);
        return;

    } else if (!strcmp(SSB_CMD, type)) {
        process_ssb_command(client_fd, command);
        return;

    } else {
        fprintf(stderr, "Error: %s\n", command);
        return;
    }
}