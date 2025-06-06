#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_ID_LEN 50
#define MAX_VAL_LEN 100
#define MAX_LINE_LEN 200

typedef struct
{
    time_t timestamp;
    char id_sensor[MAX_ID_LEN];
    char valor[MAX_VAL_LEN];
} Leitura;

time_t converter_para_timestamp(int dia, int mes, int ano, int hora, int min, int seg)
{
    struct tm t;
    t.tm_year = ano - 1900;
    t.tm_mon = mes - 1;
    t.tm_mday = dia;
    t.tm_hour = hora;
    t.tm_min = min;
    t.tm_sec = seg;
    t.tm_isdst = -1;
    time_t timestamp = mktime(&t);
    if (timestamp == -1)
    {
        printf("Data invalida. Tente novamente.\n");
    }
    return timestamp;
}

int busca_binaria_proxima(Leitura leituras[], int num_leituras, time_t target_ts)
{
    if (num_leituras == 0)
        return -1;
    if (num_leituras == 1)
        return 0;

    int baixo = 0, alto = num_leituras - 1;
    int idx_mais_proximo = -1;
    long diff_minima = LONG_MAX;

    if (target_ts <= leituras[0].timestamp)
        return 0;
    if (target_ts >= leituras[num_leituras - 1].timestamp)
        return num_leituras - 1;

    while (baixo <= alto)
    {
        int meio = baixo + (alto - baixo) / 2;
        long diff_atual = leituras[meio].timestamp - target_ts;
        if (diff_atual < 0)
            diff_atual = -diff_atual;

        if (diff_atual < diff_minima)
        {
            diff_minima = diff_atual;
            idx_mais_proximo = meio;
        }
        else if (diff_atual == diff_minima)
        {
            if (leituras[meio].timestamp < leituras[idx_mais_proximo].timestamp)
            {
                idx_mais_proximo = meio;
            }
        }

        if (leituras[meio].timestamp < target_ts)
        {
            baixo = meio + 1;
        }
        else if (leituras[meio].timestamp > target_ts)
        {
            alto = meio - 1;
        }
        else
        {
            return meio;
        }
    }

    if (alto >= 0)
    { 
        long diff = target_ts - leituras[alto].timestamp;
        if (diff < 0)
            diff = -diff;
        if (diff < diff_minima)
        {
            diff_minima = diff;
            idx_mais_proximo = alto;
        }
        else if (diff == diff_minima && leituras[alto].timestamp < leituras[idx_mais_proximo].timestamp)
        {
            idx_mais_proximo = alto;
        }
    }

    if (baixo < num_leituras)
    { 
        long diff = leituras[baixo].timestamp - target_ts;
        if (diff < 0)
            diff = -diff;
        if (diff < diff_minima)
        {
            idx_mais_proximo = baixo;
        }
        else if (diff == diff_minima && leituras[baixo].timestamp < leituras[idx_mais_proximo].timestamp)
        {
            idx_mais_proximo = baixo;
        }
    }
    return idx_mais_proximo;
}

int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        printf("Uso: %s <NOME_SENSOR> <ano> <mes> <dia> <hora> <min> <seg>\n", argv[0]);
        printf("Exemplo: %s TEMP 2024 01 15 10 30 00\n", argv[0]);
        return 1;
    }

    char *sensor_nome = argv[1];
    int ano = atoi(argv[2]);
    int mes = atoi(argv[3]);
    int dia = atoi(argv[4]);
    int hora = atoi(argv[5]);
    int min = atoi(argv[6]);
    int seg = atoi(argv[7]);

    time_t target_ts = converter_para_timestamp(dia, mes, ano, hora, min, seg);
    if (target_ts == -1)
    {
        printf("Data e hora fornecidas sao invalidas.\n");
        return 1;
    }

    char filename[MAX_ID_LEN + 5];
    sprintf(filename, "%s.txt", sensor_nome);

    FILE *sensor_file = fopen(filename, "r");
    if (!sensor_file)
    {
        printf("Erro ao abrir arquivo do sensor: %s\n", filename);
        perror("Detalhe do erro");
        return 1;
    }

    int num_leituras = 0;
    char line_buffer[MAX_LINE_LEN];
    while (fgets(line_buffer, sizeof(line_buffer), sensor_file) != NULL)
    {
        num_leituras++;
    }
    rewind(sensor_file); 

    if (num_leituras == 0)
    {
        printf("Nenhuma leitura encontrada no arquivo do sensor %s.\n", sensor_nome);
        fclose(sensor_file);
        return 1;
    }

    Leitura *leituras = malloc(num_leituras * sizeof(Leitura));
    if (!leituras)
    {
        perror("Falha ao alocar memoria para leituras");
        fclose(sensor_file);
        return 1;
    }

    int i = 0;
    long ts_long_temp; 
    while (i < num_leituras && fscanf(sensor_file, "%ld %49s %99[^\n]",
                                      &ts_long_temp,
                                      leituras[i].id_sensor,
                                      leituras[i].valor) == 3)
    {
        leituras[i].timestamp = (time_t)ts_long_temp;
        i++;
    }
    fclose(sensor_file);
    num_leituras = i; 

    if (num_leituras == 0)
    { 
        printf("Nenhuma leitura valida encontrada no arquivo do sensor %s apos parse.\n", sensor_nome);
        free(leituras);
        return 1;
    }

    int closest_idx = busca_binaria_proxima(leituras, num_leituras, target_ts);

    if (closest_idx != -1)
    {
        printf("Leitura mais proxima encontrada:\n");
        printf("Timestamp: %ld\n", (long)leituras[closest_idx].timestamp);
        char time_str[30];
        struct tm *tm_info = localtime(&leituras[closest_idx].timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        printf("Data/Hora: %s\n", time_str);
        printf("ID Sensor: %s\n", leituras[closest_idx].id_sensor);
        printf("Valor: %s\n", leituras[closest_idx].valor);
    }
    else
    {
        printf("Nao foi possivel encontrar uma leitura proxima para o sensor %s.\n", sensor_nome);
    }

    free(leituras);
    return 0;
}