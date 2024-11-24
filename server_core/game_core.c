#include "game_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"
#include <sys/stat.h>

void generate_solution(GameTry* game_solution) {
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    int len_colors = sizeof(colors) / sizeof(colors[0]);
    int len_solution = sizeof(game_solution->colours) / sizeof(game_solution->colours[0]);

    srand((unsigned) time(NULL));

    for (int i = 0; i < len_solution; i++) {

        int random_index = rand() % len_colors;
        game_solution->colours[i] = colors[random_index];
    }
}


void check_try(const char *PLID, GameTry player_try, int* res) {

    GameTry game_solution;
    strncpy(game_solution.colours, extract_game_info(PLID, ARG_SOLUTION), sizeof(game_solution.colours));

    int i, j;

    res[0] = 0; // Correct position and color
    res[1] = 0; // Correct color but wrong position

    char wrong_solutions[4];    // Solution colors that were not guessed
    char wrong_tries[4];        // Try colors that were wrong
    int wrongs = 0;             // Number of wrong guesses

    // First step: Check for exact matches (correct color and position)
    for (i = 0; i < 4; i++) {
        if (game_solution.colours[i] == player_try.colours[i]) {
            res[0]++;
        } else {
            wrong_solutions[wrongs] = game_solution.colours[i];
            wrong_tries[wrongs] = player_try.colours[i];
            wrongs++;
        }
    }

    // Second step: Find correct colors in wrong positions
    for (i = 0; i < wrongs; i++) {
        for (j = 0; j < wrongs; j++) {
            if (wrong_tries[i] == wrong_solutions[j]) {
                res[1]++;
                wrong_solutions[j] = 'N'; // No color
                break;
            }
        }
    }
}


void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS];
    char PLID[PLID_DIGITS];

    sscanf(command, "SNG %s %s", PLID, time);
    // TODO: Verify arguments. In case they are wrongly formed return "RSG ERR\n"

    /* If there is an ongoing game, respond with "RSG NOK" */
    if (has_ongoing_game(PLID)) {
        char *response = "RSG NOK";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    /* If not, generate a new solution, create a new file and write the first line infos */
    GameTry *game_solution = malloc(sizeof(GameTry));
    generate_solution(game_solution);

    create_game_log_timestamp(PLID, game_solution, time,'P');

    char *response = "RSG OK";
    send_udp_response(udp_fd, response, client_addr);
}


void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    GameTry player_try;
    char PLID[PLID_DIGITS], nt;

    int *check_try_counter = malloc(2 * sizeof(int));
    check_try_counter[0] = 0;
    check_try_counter[1] = 0;


    sscanf(command, "TRY %s %c %c %c %c %c", PLID, &player_try.colours[0], &player_try.colours[1], 
                                                   &player_try.colours[2], &player_try.colours[3], &nt);


    // TODO: Verify Sintaxe!

    if (!has_ongoing_game(PLID)) {
        char *response = "RTR NOK";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    if (!inTime(PLID)) {
        char *response = "RTR ETM";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    if (is_duplicated(PLID, &player_try)) {
        char *response = "RTR NOK";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    if (hasExceededMaxTurn(nt)) {
        GameTry *game_solution = malloc(sizeof(GameTry));
    
        extract_game_colour(PLID, game_solution);

        char response[100];
        snprintf(response, sizeof(response), "RTR ENT %c %c %c %c\n", game_solution->colours[0],
                                                                    game_solution->colours[1],
                                                                    game_solution->colours[2],
                                                                    game_solution->colours[3]);
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    // TODO: Verify the INV response!

    // OK response
    int *player_try_res = malloc(sizeof(int) * 2);
    check_try(PLID, player_try, player_try_res);
    write_try(PLID, player_try, player_try_res);

    char response[50];
    sprintf(response, "RTR OK %d %d %c",player_try_res[0], player_try_res[1], nt); 

    send_udp_response(udp_fd, response, client_addr);

    // TODO: Add function that verifies if game solved
}

void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char PLID[PLID_DIGITS];
    
    sscanf(command, "QUT %s", PLID);
    // TODO: Verify arguments. In case they are wrongly formed return "RQT ERR\n"

    /* If there is an ongoing game, respond with "RQT NOK" */
    if (!has_ongoing_game(PLID)) {
        char *response = "RQT NOK";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    /* Get the solution and send the message */
    GameTry *game_solution = malloc(sizeof(GameTry));
    
    extract_game_colour(PLID, game_solution);

    char response[100];
    snprintf(response, sizeof(response), "RQT OK %c %c %c %c\n", game_solution->colours[0],
                                                                 game_solution->colours[1],
                                                                 game_solution->colours[2],
                                                                 game_solution->colours[3]);

    send_udp_response(udp_fd, response, client_addr);
}

void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {

    char time[TIME_DIGITS];
    char PLID[PLID_DIGITS];
    GameTry *game_solution = malloc(sizeof(GameTry));

    // Informação não está a ser bem guardada na estrutura
    sscanf(command, "DBG  %s %s %c %c %c %c", PLID, time,
                                              &game_solution->colours[0],
                                              &game_solution->colours[1],
                                              &game_solution->colours[2],
                                              &game_solution->colours[3]);

    // TODO: Verify arguments. In case they are wrongly formed return "RSG ERR\n"


    /* If there is an ongoing game, respond with "RSG NOK" */
    if (has_ongoing_game(PLID)) {
        char *response = "RDB NOK";
        send_udp_response(udp_fd, response, client_addr);
        return;
    }

    create_game_log_timestamp(PLID, game_solution, time,'D');

    char *response = "RDB OK";
    send_udp_response(udp_fd, response, client_addr);
}

void process_str_command(int client_fd, const char *command) {

    char PLID[PLID_DIGITS];
    char response[BUFFER_SIZE];
    char *filename; // Caminho do arquivo de jogo
    struct stat file_stat;

    printf("Command: %s\n", command);
    // Verifica o comando recebido (espera "STR <PLID>")
    if (sscanf(command, "STR %s", PLID) != 1) {
        snprintf(response, sizeof(response), "RST NOK\n");
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return;
    }
    printf("1\n");
    // Determine o arquivo associado ao jogador
    filename = get_game_folder_path(PLID);

    // Verifica se o arquivo existe
    if (stat(filename, &file_stat) != 0) {
        printf("Erro STAT!\n");
        snprintf(response, sizeof(response), "RST NOK\n");
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return;
    }
    printf("2\n");

    // Abre o arquivo
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        snprintf(response, sizeof(response), "RST NOK\n");
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return;
    }
    printf("3\n");

    // Ignora a primeira linha
    char line[BUFFER_SIZE];
    if (fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Erro ao ignorar a primeira linha ou arquivo vazio.\n");
        fclose(file);
        snprintf(response, sizeof(response), "RST NOK\n");
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return;
    }
    printf("4\n");

    // Lê o restante do arquivo e calcula o tamanho real
    size_t total_size = 0;
    char* filedata = malloc(file_stat.st_size); // Aloca com base no tamanho original (garantido suficiente)
    if (!filedata) {
        perror("Erro ao alocar memória");
        snprintf(response, sizeof(response), "RST NOK\n");
        send(client_fd, response, strlen(response), 0);
        fclose(file);
        close(client_fd);
        return;
    }
    printf("5\n");

    // Constrói o buffer com as linhas restantes
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        memcpy(filedata + total_size, line, len);
        total_size += len;
    }

    fclose(file);

    // Prepara a resposta com o status "ACT" ou "FIN"
    snprintf(response, sizeof(response), "RST ACT %s %ld ", filename, total_size);

    printf("%s", filedata);

    // // Envia o cabeçalho
    // send(client_fd, response, strlen(response), 0);

    // // Envia o conteúdo do arquivo (após ignorar a primeira linha)
    // send(client_fd, filedata, total_size, 0);

    // // Libera recursos
    // free(filedata);
    // close(client_fd);
    // printf("Resposta enviada ao cliente.\n");
    /*
    char *response = "RST NOK";
    printf("Respondi!\n");
    write(client_fd, response, sizeof(response));
    */
}

void process_ssb_command(int client_fd, const char *command) {
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