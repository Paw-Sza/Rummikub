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
struct block hand[80];
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
void print_hand(struct block array[80])
{
    for (int i = 0; i < 80; i++)
    {
        if (array[i].value != 0)
        {
            printf("|%d[%d", i, array[i].value, array[i].color);
            switch (array[i].color)
            {
            case 1:
                printf("|B]");
                break;
            case 2:
                printf("|R]");
                break;
            case 3:
                printf("|O]");
                break;
            case 4:
                printf("|BL]");
                break;
            default:
                printf("|X]");
                break;
            }
            if (i % 10 == 9)
                printf("\n");
        }
    }
}
void put_in_hand(struct block array[80], struct block b)
{
    for (int i = 0; i < 80; i++)
    {
        if (array[i].value == 0)
        {
            array[i] = b;
            break;
        };
    }
}
struct block buffer_to_block(char *buf)
{

    struct block b;
    char temp[1] = "";
    temp[0] = buf[0];
    b.value = atoi(temp);
    if (buf[1] == '0')
    {
        b.value = 10;
    }
    else if (buf[1] == '|')
    {
        temp[0] = buf[2];
        b.color = atoi(temp);
    }
    if (buf[2] == '|')
    {
        temp[0] = buf[3];
        b.color = atoi(temp);
    }
    if (buf[3] == '_' || buf[4] == '_')
    {

        return b;
    }
    else
    {
        b.value = 0;
        b.color = 5;
        return b;
    }
}
void switch_in_hand(struct block array[80], int a, int b)
{
    struct block temp;
    temp = array[a];
    array[a] = array[b];
    array[b] = temp;
}
struct block get_from_hand(struct block array[80], int a)
{
    struct block b;
    b = array[a];
    for (int i = a; i < 79; i++)
    {
        array[i] = array[i + 1];
    }
    return b;
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
void place_on_matrix(struct block matrix[][MATRIX], struct block b, int row, int col)
{
    matrix[row][col] = b;
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
    struct block b;
    int first = 1;
    int ID1;
    int ID2;
    int row;
    int col;
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
        if (buf[0] == 'o')
            print_hand(hand);
        if (buf[0] == 'i')
        {
            printf("Podaj ID do polozenia: \n");
            n = 0;
            bzero(buf, 2000);
            while ((buf[n++] = getchar()) != '\n')
                ;
            ID1 = atoi(buf);
            printf("Podaj rząd i kolumnę: \n");
            n = 0;
            bzero(buf, 2000);
            while ((buf[n++] = getchar()) != '\n')
                ;
            row = atoi(buf);
            n = 0;
            bzero(buf, 2000);
            while ((buf[n++] = getchar()) != '\n')
                ;
            col = atoi(buf);
            b = get_from_hand(hand, ID1);
            //            printf("%d%d\n", b.value, b.color);
            place_on_matrix(matrix, b, row, col);
        }
        if (buf[0] == 'm')
        {
            printf("Podaj ID do zamiany: \n");
            n = 0;
            bzero(buf, 2000);
            while ((buf[n++] = getchar()) != '\n')
                ;
            ID1 = atoi(buf);
            n = 0;
            bzero(buf, 2000);
            while ((buf[n++] = getchar()) != '\n')
                ;
            ID2 = atoi(buf);
            switch_in_hand(hand, ID1, ID2);
        }
        if (buf[0] == 's')
        {
            write(th_data->connection_socket_descriptor, buf, 1);
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
    char blockbuf[6];
    char buf[2000];
    int recv_matrix = 0;
    int recv_block = 0;
    struct block b;
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
                if (buf[0] == '-')
                    recv_block = 1;
                else if (recv_block == 1)
                {
                    if (buf[0] == '_')
                    {
                        strncat(blockbuf, &buf[0], 1);
                        b = buffer_to_block(blockbuf);
                        put_in_hand(hand, b);
                        recv_block = 0;
                        bzero(buf, 2000);
                        bzero(blockbuf, 6);
                    }
                    else
                    {
                        strncat(blockbuf, &buf[0], 1);
                    }
                }
                else
                {
                    printf("%s", buf);
                }
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
