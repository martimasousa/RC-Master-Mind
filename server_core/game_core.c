#include "game_core.h"
#include "aux_game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"
#include <sys/stat.h>



void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS + 1];
    char PLID[PLID_DIGITS + 1];
    GameTry *game_solution = malloc(sizeof(GameTry));
    char *response;

    sscanf(command, "SNG %s %s\n", PLID, time);
    // TODO: Verify arguments. In case they are wrongly formed return "RSG ERR\n"

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

    int *check_try_counter = malloc(2 * sizeof(int));
    check_try_counter[0] = 0;
    check_try_counter[1] = 0;


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
        end_game(PLID, END_TIMEOUT);
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
        end_game(PLID, END_FAIL);
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    int* player_try_res = make_try(PLID, player_try);

    size_t response_len = COMMAND_LEN + 1 + RESPONSE_LEN + TRIAL_MAX_LEN*3*2 + 2;
    char response[response_len];
    sprintf(response, "RTR OK %c %d %d\n", nt, player_try_res[0], player_try_res[1]);

    if (has_won(player_try_res)) {
        end_game(PLID, END_WIN);
    }

    send_udp_response(udp_fd, response, client_addr);
}

void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char PLID[PLID_DIGITS + 1];
    
    sscanf(command, "QUT %s\n", PLID);
    // TODO: Verify arguments. In case they are wrongly formed return "RQT ERR\n"

    /* If there is an ongoing game, respond with "RQT NOK" */
    if (!has_ongoing_game(PLID)) {
        char *response = "RQT NOK\n";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    /* Get the solution and send the message */
    char *response = build_end_game_response(PLID, "RQT", "OK");

    end_game(PLID, END_QUIT);
    send_udp_response(udp_fd, response, client_addr);
}

void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS + 1];
    char PLID[PLID_DIGITS + 1];
    GameTry *game_solution = malloc(sizeof(GameTry));
    char *response;

    // Informação não está a ser bem guardada na estrutura
    sscanf(command, "DBG  %s %s %c %c %c %c\n", PLID, time,
                                              &game_solution->colours[0],
                                              &game_solution->colours[1],
                                              &game_solution->colours[2],
                                              &game_solution->colours[3]);

    // TODO: Verify arguments. In case they are wrongly formed return "RSG ERR\n"


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
    char response[BUFFER_SIZE];
    char *filename; // Caminho do arquivo de jogo

    // Verifica o comando recebido (espera "STR <PLID>")
    sscanf(command, "STR %s", PLID);

    if (!has_ongoing_game(PLID)) {
        filename = malloc(BUFFER_SIZE);
        FindLastGame(PLID, filename);
    } else filename = get_game_folder_path(PLID);

    // Abre o arquivo
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        snprintf(response, sizeof(response), "RST NOK\n");
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return;
    }

    int file_data_size = get_data_size(file);

    char filedata[BUFFER_SIZE];
    int total_size = 0;

    int i = 0;
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        i++;
        size_t len = strlen(line);
        memcpy(filedata + total_size, line, len);
        total_size += len;
    }

    char teste[BUFFER_SIZE * 10];
    sprintf(teste, "RST ACT Teste %d %s", file_data_size, filedata);

    int n = 0;
    int total = 0;
    while (total < strlen(teste))
    {
        n = write(client_fd, teste, strlen(teste));
        total += n;
    }
}

void process_ssb_command(int client_fd, const char *command) {
    // TODO: Check for command errors

    // Get scores list
    SCORELIST *files = malloc(sizeof(SCORELIST));
    if (FindTopScores(files) <= 0) {
        char res_init[20] = "RSS EMPTY\n";
        if (tcp_write(client_fd, res_init)) {
            return;
        }
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

        sprintf(line, "%s %s %d\n", PLID, colcode, notries);

        Fsize += strlen(line);
        Fdata = realloc(Fdata, Fsize);
        strcat(Fdata, line); // Append line to the end of Fdata
    }

    // Write Fname and Fsize
    char res_init[20];
    sprintf(res_init, "RSS OK Fname %ld ", Fsize);
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


void process_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {
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
        printf("Error: %s\n", type);
        return;
    }
}


void handle_TCP_messages(int client_fd, const char *command) { 
    char type[3];
    sscanf(command, "%s", type);

    if (!strcmp(STR_CMD, type)) {
        process_str_command(client_fd, command);
        return;

    } else if (!strcmp(SSB_CMD, type)) {
        process_ssb_command(client_fd, command);
        return;

    } else {
        printf("Error: %s\n", command);
        return;
    }
}