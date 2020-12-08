#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#define MATRIX 15
#define BUF_SIZE 1024
#define NUM_THREADS 5
struct block
{
    int value;
    int color;
    int unused;
};
struct block matrix[MATRIX][MATRIX];
void print_matrix(struct block matrix[][MATRIX])
{
    for (int i = 0; i < MATRIX; i++)
    {
        for (int j = 0; j < MATRIX; j++)
        {
            printf("[%d", matrix[i][j].value);
            switch (matrix[i][j].color)
            {
            case 1:
                printf(" B]");
                break;
            case 2:
                printf(" R]");
                break;
            case 3:
                printf(" O]");
                break;
            case 4:
                printf(" BL]");
                break;
            default:
                printf(" X]");
                break;
            }
        }
        printf("\n");
    }
}
void buffer_to_matrix(struct block matrix[][MATRIX], char *buffer)
{
    //	printf("%s", buffer);
    int isval = 0;
    int iscol = 0;
    int row = 0;
    int col = -1;
    int temp;
    char temp2[2] = "";
    for (int i = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == '*')
        {
            col = -1;
            row = row + 1;
        }
        else if (buffer[i] == ';')
        {
            col = col + 1;
            isval = 1;
            iscol = 0;
        }
        else if (buffer[i] == '|')
        {
            isval = 0;
            iscol = 1;
        }
        else if (isval == 1)
        {
            temp2[0] = buffer[i];
            matrix[row][col].value = atoi(temp2);
            if (buffer[i + 1] == '0')
            {
                matrix[row][col].value = 10;
                i++;
            }
        }
        else if (iscol == 1)
        {
            temp2[0] = buffer[i];
            matrix[row][col].color = atoi(temp2);
        }
        //printf("Row: %d Col: %d Char: %c IsVal: %d IsCol: %d\n",row, col,  buffer[i], isval,iscol);
    }
}
void matrix_to_buffer(struct block matrix[][MATRIX], char *buf)
{
    char buffer[2000];
    bzero(buffer, 2000);
    buffer[0] = '+';
    for (int i = 0; i < MATRIX; i++)
    {
        for (int j = 0; j < MATRIX; j++)
        {
            sprintf(buffer + strlen(buffer), ";%d|%d", matrix[i][j].value, matrix[i][j].color);
        }
        sprintf(buffer + strlen(buffer), "*");
    }
    sprintf(buffer + strlen(buffer), "=");
    strcpy(buf, buffer);
}
//struktura zawierająca dane, które zostanły przekazane do wątku
struct thread_data_t
{
    int connection_socket_descriptor;
};

//wskaźnik na funkcję opisującą zachowanie wątku
void *ThreadBehavior(void *t_data)
{
    struct thread_data_t *th_data = (struct thread_data_t *)t_data;
    char buf[2000];
    char conn[10] = "Connected\n";
    int first = 1;
    while (th_data->connection_socket_descriptor)
    {
        if (first == 0)
        {
            write(th_data->connection_socket_descriptor, conn, 10);
            first++;
        }
        int n = 0;
        bzero(buf, 2000);
        while ((buf[n++] = getchar()) != '\n')
            ;
        if (buf[0] == 'p')
            print_matrix(matrix);
        if (buf[0] == 's')
        {
            write(th_data->connection_socket_descriptor, buf, 1);
            matrix[1][4].value = 8;
            matrix[1][5].value = 9;
            matrix[1][6].value = 10;
            matrix[1][4].color = 2;
            matrix[1][5].color = 2;
            matrix[1][6].color = 2;
            matrix_to_buffer(matrix, buf);
        }
        if (buf[0] == 'c')
        {
            write(th_data->connection_socket_descriptor, buf, 1);
            matrix_to_buffer(matrix, buf);
        }
        write(th_data->connection_socket_descriptor, buf, 2000);
    }
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z serwerem
void handleConnection(int connection_socket_descriptor)
{
    //wynik funkcji tworzącej wątek
    int create_result = 0;
    char matrixbuf[2000];
    char buf[2000];
    int recv_matrix = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    struct thread_data_t t_data;

    t_data.connection_socket_descriptor = connection_socket_descriptor;
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)&t_data);
    if (create_result)
    {
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }
    while (connection_socket_descriptor)
    {
        while (recv(connection_socket_descriptor, buf, 1, 0))
        {
            if (buf[0] == '+')
                recv_matrix = 1;
            if (recv_matrix == 1)
            {
                if (buf[0] == '=')
                {
                    buffer_to_matrix(matrix, matrixbuf);
                    recv_matrix = 0;
                    bzero(buf, 2000);
                    bzero(matrixbuf, 2000);
                }
                else
                {
                    strncat(matrixbuf, &buf[0], 1);
                }
            }
            else
            {
                printf("%s", buf);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int connection_socket_descriptor;
    int connect_result;
    struct sockaddr_in server_address;
    struct hostent *server_host_entity;

    if (argc != 3)
    {
        fprintf(stderr, "Sposób uĹźycia: %s server_name port_number\n", argv[0]);
        exit(1);
    }

    server_host_entity = gethostbyname(argv[1]);
    if (!server_host_entity)
    {
        fprintf(stderr, "%s: Nie moĹźna uzyskać adresu IP serwera.\n", argv[0]);
        exit(1);
    }

    connection_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
    if (connection_socket_descriptor < 0)
    {
        fprintf(stderr, "%s: Błąd przy probie utworzenia gniazda.\n", argv[0]);
        exit(1);
    }

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
    server_address.sin_port = htons(atoi(argv[2]));

    connect_result = connect(connection_socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
    if (connect_result < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie połączenia z serwerem (%s:%i).\n", argv[0], argv[1], atoi(argv[2]));
        exit(1);
    }

    handleConnection(connection_socket_descriptor);

    return 0;
}
