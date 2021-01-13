#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#define MATRIX 15
#define QUEUE_SIZE 5
int connections[4];
int con = 0;
int turn = 0;
int bl = 10;
int re = 10;
int or = 10;
int bu = 10;
struct block
{
    int value;
    int color;
    int unused;
};

struct block matrix[MATRIX][MATRIX];
struct block matrix_temp[MATRIX][MATRIX];
struct block b;
struct block black[10];
struct block red[10];
struct block orange[10];
struct block blue[10];
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
int row_check(struct block matrix[][MATRIX], int r, int c, int g)
{
    if (c >= MATRIX - 1)
    {
        return 999;
    }
    if (matrix[r][c].value == 0)
    {
        row_check(matrix, r, c + 1, g);
    }
    else if (matrix[r][c].value < matrix[r][c + 1].value && matrix[r][c].color == matrix[r][c + 1].color && abs(matrix[r][c].value - matrix[r][c + 1].value) == 1)
    {
        row_check(matrix, r, c + 1, g + 1);
        g = g + 1;
    }
    else if (matrix[r][c].value > matrix[r][c + 1].value && matrix[r][c].color == matrix[r][c + 1].color && abs(matrix[r][c].value - matrix[r][c + 1].value) == 1)
    {
        g = g + 1;
        row_check(matrix, r, c + 1, g + 1);
    }
    else if (matrix[r][c].value == matrix[r][c + 1].value &&
             matrix[r][c].color != matrix[r][c + 1].color &&
             matrix[r][c].value == matrix[r][c + 2].value &&
             matrix[r][c].color != matrix[r][c + 2].color &&
             matrix[r][c].value == matrix[r][c + 3].value &&
             matrix[r][c].color != matrix[r][c + 3].color &&
             matrix[r][c + 4].value == 0)
    {

        row_check(matrix, r, c + 4, g + 3);
    }
    else if (matrix[r][c].value == matrix[r][c + 1].value &&
             matrix[r][c].color != matrix[r][c + 1].color &&
             matrix[r][c].value == matrix[r][c + 2].value &&
             matrix[r][c].color != matrix[r][c + 2].color &&
             matrix[r][c + 3].value == 0)
    {
        row_check(matrix, r, c + 3, g + 2);
    }
    else if (g >= 2 && matrix[r][c + 1].value == 0)
    {
        row_check(matrix, r, c + 1, 0);
    }
    else
    {
        return c + 1;
    }
}
int matrix_check(struct block matrix2[][MATRIX], int sock)
{
    int out;
    char buf[20];
    int good = 1;
    for (int i = 0; i < MATRIX; i++)
    {
        out = row_check(matrix2, i, 0, 0);
        if (out != 999)
        {
            good = 0;
            sprintf(buf, "@Blad w R %d K %d!", i, out);
            write(sock, buf, 20);
            return out;
        }
    }
    if (good == 1)
    {
        write(sock, "?", 20);
        return 999;
    }
}
void list_fill(struct block array[10], int c)
{
    for (int i = 0; i < 10; i++)
    {
        array[i].value = i + 1;
        array[i].unused = 2;
        array[i].color = c;
    }
}
struct block randomize_block(struct block array1[10], struct block array2[10], struct block array3[10], struct block array4[10])
{
    struct block b;
    int index;
    bool chosen = false;
    double r = rand() % 1000;
    r = r / 1000;
    while (chosen == false)
    {
        if (r >= 0.75 && bl > 0)
        {
            index = rand() % 10;
            b = array1[index];
            while (b.unused == 0)
            {
                index = rand() % 10;
                b = array1[index];
            }
            array1[index].unused = array1[index].unused - 1;
            if (array1[index].unused == 0)
                bl = bl - 1;
            chosen = true;
        }
        if (r >= 0.5 && r < 0.75 && re > 0)
        {
            index = rand() % 10;
            b = array2[index];
            while (b.unused == 0)
            {
                index = rand() % 10;
                b = array2[index];
            }
            array2[index].unused = array2[index].unused - 1;
            if (array2[index].unused == 0)
                re = re - 1;
            chosen = true;
        }
        if (r >= 0.25 && r < 0.5 && or > 0)
        {
            index = rand() % 10;
            b = array3[index];
            while (b.unused == 0)
            {
                index = rand() % 10;
                b = array3[index];
            }
            array3[index].unused = array3[index].unused - 1;
            if (array3[index].unused == 0)
                or = or -1;
            chosen = true;
        }
        if (r < 0.25 && bu > 0)
        {
            index = rand() % 10;
            b = array4[index];
            while (b.unused == 0)
            {
                index = rand() % 10;
                b = array4[index];
            }
            array4[index].unused = array4[index].unused - 1;
            if (array4[index].unused == 0)
                bu = bu - 1;
            chosen = true;
        }
        r = rand() % 1000;
        r = r / 1000;
    }
    b.unused = b.unused - 1;
    return b;
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
void send_block(int sock)
{
    printf("Wyslano klocek ");
    struct block b = randomize_block(black, red, orange, blue);
    printf("%d%d\n", b.value, b.color);
    char buf[6] = "";
    sprintf(buf, "-%d|%d_", b.value, b.color);

    write(sock, buf, 6);
}
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//struktura zawierająca dane, które zostaną przekazane do wątku

struct thread_data_t
{
    int connection_socket_descriptor;
};

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t *)t_data;
    int ID;
    int check;
    char buf[2000];
    char buf2[100];
    char matrixbuf[2000];
    int recv_matrix = 0;
    pthread_mutex_unlock(&lock);
    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            if (turn != i && connections[i] != 0)
            {
                write(connections[i], "#0$\n", 5);
            }
            else
                write(connections[turn], "#1$\n", 5);
        }
        if (turn == con)
            turn = 0;
        bzero(matrixbuf, 2000);
        bzero(buf, 2000);
        bzero(buf2, 100);
        recv(th_data->connection_socket_descriptor, buf2, 1, MSG_DONTWAIT);
        pthread_mutex_lock(&lock);
        for (int i = 0; i < 4; i++)
        {
            if (connections[i] == th_data->connection_socket_descriptor)
            {
                ID = i;
            }
        }
        if (turn == ID)
        {

            if (buf2[0] == 'b')
            {
                recv_matrix = 0;
                bzero(buf, 2000);
                send_block(th_data->connection_socket_descriptor);
                write(th_data->connection_socket_descriptor, "\n", 1);
            }
            if (buf2[0] == 's' || buf2[0] == 't')
            {
                recv_matrix = 1;
                bzero(matrixbuf, 2000);
                while (recv_matrix == 1)
                {
                    recv(th_data->connection_socket_descriptor, buf, 1, 0);
                    if (buf[0] == '+')
                        recv_matrix = 1;
                    if (recv_matrix == 1)
                    {
                        if (buf[0] == '=')
                        {
                            buffer_to_matrix(matrix_temp, matrixbuf);
                            //memcpy(matrix, matrix_temp, sizeof(matrix_temp));
                            bzero(buf, 2000);
                            matrix_to_buffer(matrix_temp, buf);
                            for (int i = 0; i < 4; i++)
                            {
                                if (th_data->connection_socket_descriptor != connections[i] && connections[i] != 0)
                                {
                                    write(connections[i], buf, 2000);
                                    printf("\nSending matrix from %d  to %d \n", th_data->connection_socket_descriptor, connections[i]);
                                    write(th_data->connection_socket_descriptor, "\n", 1);
                                }
                            }

                            recv_matrix = 0;
                            print_matrix(matrix_temp);
                        }
                        else
                        {
                            strncat(matrixbuf, &buf[0], 1);
                        }
                    }
                }
                check = matrix_check(matrix_temp, th_data->connection_socket_descriptor);
                if (buf2[0] == 't' && check == 999)
                {
                    memcpy(matrix, matrix_temp, sizeof(matrix_temp));
                    turn++;
                }
            }
        }
        pthread_mutex_unlock(&lock);
    }
    free(th_data);
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor)
{
    printf("Nowe polaczenie \n");
    for (int j = 0; j < 14; j++)
        send_block(connection_socket_descriptor);
    //wynik funkcji tworzącej wątek
    int create_result = 0;
    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku

    struct thread_data_t *t_data = (struct thread_data_t *)malloc(sizeof(struct thread_data_t));
    t_data->connection_socket_descriptor = connection_socket_descriptor;

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result)
    {
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    srand(time(0));
    list_fill(black, 1);
    list_fill(red, 2);
    list_fill(orange, 3);
    list_fill(blue, 4);
    setlinebuf(stdout);
    fflush(stdout);
    int server_socket_descriptor;
    int connection_socket_descriptor;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;

    //inicjalizacja gniazda serwera

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr));
    server_address.sin_port = htons(atoi(argv[2]));

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
        exit(1);
    }
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
    if (bind_result < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }
    while (1)
    {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
        if (connection_socket_descriptor < 0)
        {
            fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
            exit(1);
        }
        if (con >= 4)
        {
            write(connection_socket_descriptor, "SERVER IS FULL\n", 30);
        }
        else
        {
            connections[con] = connection_socket_descriptor;
            con++;

            handleConnection(connection_socket_descriptor);
        }
    }

    return (0);
}
