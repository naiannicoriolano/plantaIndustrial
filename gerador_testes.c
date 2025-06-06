#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    if (timestamp == -1) {
       printf("Data invalida. Tente novamente.\n");
    }
    return timestamp;
}

time_t gerar_timestamp_aleatorio(struct tm *inicial_tm_ptr, struct tm *final_tm_ptr) {
    time_t timestamp_inicial, timestamp_final;

    struct tm t_inicial = *inicial_tm_ptr;
    struct tm t_final = *final_tm_ptr;

    timestamp_inicial = mktime(&t_inicial);
    if (timestamp_inicial == -1) {
        printf("Data inicial invalida.\n");
        return -1;
    }

    timestamp_final = mktime(&t_final);
    if (timestamp_final == -1) {
        printf("Data final invalida.\n");
        return -1;
    }

    if (timestamp_final < timestamp_inicial) {
        printf("Data final e anterior a data inicial.\n");
        return -1;
    }
    if (timestamp_final == timestamp_inicial) {
        return timestamp_inicial; 
    }

    time_t diff = timestamp_final - timestamp_inicial;
    if (diff == 0) return timestamp_inicial;

    time_t random_offset = rand() % (diff + 1);
    time_t timestamp_aleatorio = timestamp_inicial + random_offset;

    return timestamp_aleatorio;
}

int main(int argc, char *argv[]) {
    if (argc < 15) { 
        printf("Uso: %s <arquivo_saida> <ano_i> <mes_i> <dia_i> <h_i> <m_i> <s_i> <ano_f> <mes_f> <dia_f> <h_f> <m_f> <s_f> <ID_SENSOR_1> <TIPO_DADO_1> [<ID_SENSOR_2> <TIPO_DADO_2> ...]\n", argv[0]);
        printf("Tipos de dados suportados: int, float, bool, string\n");
        printf("Exemplo: %s dados_brutos.txt 2024 1 1 0 0 0 2024 1 1 23 59 59 TEMP float PRES int\n", argv[0]);
        return 1;
    }

    char *output_filename = argv[1];

    struct tm tm_inicio = {0};
    tm_inicio.tm_year = atoi(argv[2]) - 1900;
    tm_inicio.tm_mon = atoi(argv[3]) - 1;
    tm_inicio.tm_mday = atoi(argv[4]);
    tm_inicio.tm_hour = atoi(argv[5]);
    tm_inicio.tm_min = atoi(argv[6]);
    tm_inicio.tm_sec = atoi(argv[7]);
    tm_inicio.tm_isdst = -1;

    struct tm tm_fim = {0};
    tm_fim.tm_year = atoi(argv[8]) - 1900;
    tm_fim.tm_mon = atoi(argv[9]) - 1;
    tm_fim.tm_mday = atoi(argv[10]);
    tm_fim.tm_hour = atoi(argv[11]);
    tm_fim.tm_min = atoi(argv[12]);
    tm_fim.tm_sec = atoi(argv[13]);
    tm_fim.tm_isdst = -1;
    
    time_t ts_check_inicio = mktime(&tm_inicio);
    if (ts_check_inicio == -1) {
        printf("Data de inicio invalida.\n");
        return 1;
    }
    time_t ts_check_fim = mktime(&tm_fim);
     if (ts_check_fim == -1) {
        printf("Data de fim invalida.\n");
        return 1;
    }
    if (ts_check_fim < ts_check_inicio) {
        printf("Erro: Timestamp de fim e menor que timestamp de inicio.\n");
        return 1;
    }

    FILE *outfile = fopen(output_filename, "w");
    if (!outfile) {
        perror("Erro ao abrir arquivo de saida");
        return 1;
    }

    srand(time(NULL));

    int num_leituras_por_sensor = 2000;
    int arg_sensores_inicio = 14;

    if ((argc - arg_sensores_inicio) % 2 != 0) {
        printf("Numero incorreto de argumentos para sensores e tipos.\n");
        fclose(outfile);
        return 1;
    }

    for (int i = arg_sensores_inicio; i < argc; i += 2) {
        char *sensor_id = argv[i];
        char *sensor_type = argv[i+1];

        for (int j = 0; j < num_leituras_por_sensor; j++) {
            time_t random_ts = gerar_timestamp_aleatorio(&tm_inicio, &tm_fim);
            if (random_ts == -1) {
                 fprintf(stderr, "Erro critico ao gerar timestamp aleatorio. Saindo.\n");
                 fclose(outfile);
                 return 1;
            }

            if (strcmp(sensor_type, "int") == 0) {
                fprintf(outfile, "%ld %s %d\n", (long)random_ts, sensor_id, rand() % 1000);
            } else if (strcmp(sensor_type, "float") == 0) {
                fprintf(outfile, "%ld %s %.2f\n", (long)random_ts, sensor_id, (float)rand() / RAND_MAX * 100.0f);
            } else if (strcmp(sensor_type, "bool") == 0) {
                fprintf(outfile, "%ld %s %d\n", (long)random_ts, sensor_id, rand() % 2);
            } else if (strcmp(sensor_type, "string") == 0) {
                fprintf(outfile, "%ld %s MSG_%d\n", (long)random_ts, sensor_id, rand() % 100);
            } else {
                printf("Tipo de sensor desconhecido: %s para sensor %s. Pulando.\n", sensor_type, sensor_id);
            }
        }
    }

    fclose(outfile);
    printf("Arquivo de teste '%s' gerado com sucesso.\n", output_filename);

    return 0;
}