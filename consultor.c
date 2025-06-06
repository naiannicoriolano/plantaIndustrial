#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_ID_LEN 50
#define MAX_VAL_LEN 100
#define MAX_LINE_LEN 200

typedef struct {
    time_t timestamp;
    char id_sensor[MAX_ID_LEN];
    char valor[MAX_VAL_LEN];
} Leitura;

time_t converter_para_timestamp(int dia, int mes, int ano, int hora, int min, int seg) {
    struct tm t;
    t.tm_year = ano - 1900;
    t.tm_mon = mes - 1;
    t.tm_mday = dia;
    t.tm_hour = hora;
    t.tm_min = min;
    t.tm_sec = seg;
    t.tm_isdst = -1;
    time_t timestamp = mktime(&t);
    return timestamp;
}

// Encontra o indice da leitura com timestamp mais proximo usando busca binaria.
int busca_binaria_proxima(Leitura leituras[], int num_leituras, time_t target_ts) {
    if (num_leituras == 0) return -1;

    int baixo = 0, alto = num_leituras - 1;
    int idx_mais_proximo = 0; 

    while (baixo <= alto) {
        int meio = baixo + (alto - baixo) / 2;
        
        long diff_meio = labs(leituras[meio].timestamp - target_ts);
        long diff_proximo = labs(leituras[idx_mais_proximo].timestamp - target_ts);

        if (diff_meio < diff_proximo) {
            idx_mais_proximo = meio;
        } else if (diff_meio == diff_proximo) {
            if (leituras[meio].timestamp < leituras[idx_mais_proximo].timestamp) {
                idx_mais_proximo = meio;
            }
        }
        
        if (leituras[meio].timestamp < target_ts) {
            baixo = meio + 1;
        } else if (leituras[meio].timestamp > target_ts) {
            alto = meio - 1;
        } else {
            return meio; 
        }
    }
    return idx_mais_proximo;
}

// le argumentos, valida, busca e exibe a leitura mais proxima.
int main(int argc, char *argv[]) {
    // Valida o numero de argumentos
    if (argc != 8) {
        printf("O correto seria: %s <NOME_SENSOR> <ano> <mes> <dia> <hora> <min> <seg>\n", argv[0]);
        printf("Exemplo: %s TEMP 2024 6 1 10 30 00\n", argv[0]);
        return 1;
    }

    // Valida se a DATA e HORA fornecidas existem
    struct tm tm_alvo = {0};
    int ano = atoi(argv[2]);
    int mes = atoi(argv[3]);
    int dia = atoi(argv[4]);
    
    tm_alvo.tm_year = ano - 1900;
    tm_alvo.tm_mon = mes - 1;
    tm_alvo.tm_mday = dia;
    tm_alvo.tm_hour = atoi(argv[5]);
    tm_alvo.tm_min = atoi(argv[6]);
    tm_alvo.tm_sec = atoi(argv[7]);
    tm_alvo.tm_isdst = -1;

    time_t target_ts = mktime(&tm_alvo);
    if (target_ts == -1 || tm_alvo.tm_mday != dia || tm_alvo.tm_mon != mes - 1) {
        printf("Erro: Data ou hora de consulta invalida ou nao existente.\n");
        return 1;
    }

    char *sensor_nome = argv[1];
    char filename[MAX_ID_LEN + 5];
    sprintf(filename, "%s.txt", sensor_nome);

    FILE *sensor_file = fopen(filename, "r");
    if (!sensor_file) {
        printf("Erro: Nao foi possivel abrir o arquivo do sensor '%s'.\n", filename);
        perror("Detalhe do erro");
        return 1;
    }

    Leitura *leituras = NULL;
    int num_leituras = 0;
    int capacity = 0;
    char line_buffer[MAX_LINE_LEN];

    while (fgets(line_buffer, sizeof(line_buffer), sensor_file) != NULL) {
        long ts_long_temp;
        char id_temp[MAX_ID_LEN], val_temp[MAX_VAL_LEN];

        if (sscanf(line_buffer, "%ld %49s %99[^\n]", &ts_long_temp, id_temp, val_temp) == 3) {
            if (num_leituras >= capacity) {
                capacity = (capacity == 0) ? 128 : capacity * 2;
                Leitura *temp = realloc(leituras, capacity * sizeof(Leitura));
                if (!temp) {
                    perror("Falha ao alocar memoria");
                    free(leituras);
                    fclose(sensor_file);
                    return 1;
                }
                leituras = temp;
            }
            leituras[num_leituras].timestamp = (time_t)ts_long_temp;
            strcpy(leituras[num_leituras].id_sensor, id_temp);
            strcpy(leituras[num_leituras].valor, val_temp);
            num_leituras++;
        }
    }
    fclose(sensor_file);

    if (num_leituras == 0) {
        printf("Nenhuma leitura valida encontrada no arquivo do sensor %s.\n", sensor_nome);
        free(leituras);
        return 1;
    }

    // Valida se a data da consulta esta dentro do intervalo de dados disponiveis
    time_t min_ts = leituras[0].timestamp;
    time_t max_ts = leituras[num_leituras - 1].timestamp;

    if (target_ts < min_ts || target_ts > max_ts) {
        char min_str[30], max_str[30];
        strftime(min_str, sizeof(min_str), "%Y-%m-%d %H:%M:%S", localtime(&min_ts));
        strftime(max_str, sizeof(max_str), "%Y-%m-%d %H:%M:%S", localtime(&max_ts));
        printf("Erro: A data de consulta esta fora do intervalo de dados disponiveis.\n");
        printf("Intervalo disponivel para o sensor '%s': de %s ate %s.\n", sensor_nome, min_str, max_str);
        free(leituras);
        return 1;
    }

    //  Realiza a busca e exibe o resultado
    int closest_idx = busca_binaria_proxima(leituras, num_leituras, target_ts);

    printf("Leitura mais proxima encontrada:\n");
    printf("Timestamp: %ld\n", (long)leituras[closest_idx].timestamp);

    char time_str[30];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&leituras[closest_idx].timestamp));
    printf("Data/Hora: %s\n", time_str);
    printf("ID Sensor: %s\n", leituras[closest_idx].id_sensor);
    printf("Valor: %s\n", leituras[closest_idx].valor);

    free(leituras);
    return 0;
}