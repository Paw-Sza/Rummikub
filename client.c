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
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_ttf.h>
#define MATRIX 15
#define BUF_SIZE 1024
#define NUM_THREADS 5
#define resolution 700
enum KeyPressSurfaces
{
    KEY_PRESS_SURFACE_DEFAULT,
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL
};
struct block
{
    int value;
    int color;
    int unused;
};
struct block matrix[MATRIX][MATRIX];
struct block hand[80];
void x_y_to_matrix_coords(int *row, int *col, int x, int y)
{
    double x_ratio = resolution * 0.757 / 15;
    double y_ratio = resolution * 0.757 / 15;
    double xd = floor(x / x_ratio);
    double yd = floor(y / y_ratio);
    *row = (int)yd;
    *col = (int)xd;
}
void matrix_coords_to_x_y(int row, int col, int *x, int *y)
{
    double x_ratio = resolution * 0.757 / 15;
    double y_ratio = resolution * 0.757 / 15;
    double xd = floor(col * x_ratio);
    double yd = floor(row * y_ratio);
    *y = (int)yd+3;
    *x = (int)xd+3;
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
SDL_Surface *block_to_bmp(struct block b)
{
    char combined[20];
    char *colorch;

    int combined_int = atoi(combined);
    SDL_Surface *bmp;
    switch (b.color)
    {
    case 1:
        colorch = "CZ";
        break;
    case 2:
        colorch = "C";
        break;
    case 3:
        colorch = "P";
        break;
    case 4:
        colorch = "N";
        break;
    default:
        break;
    }
    sprintf(combined, "blocks/%d%s.bmp", b.value, colorch);
    bmp = SDL_LoadBMP(combined);
    //printf("%s\n",combined);
    return bmp;
}
void print_matrix(struct block matrix[][MATRIX], SDL_Rect viewport, SDL_Renderer *ren)
{
    int x, y;
    SDL_Texture *tex;
    SDL_Surface *bmp;
    for (int i = 0; i < MATRIX; i++)
    {
        for (int j = 0; j < MATRIX; j++)
        {
            if (matrix[i][j].value != 0)
            {
                matrix_coords_to_x_y(i, j, &x, &y);
                viewport.x = x;
                viewport.y = y;
                SDL_RenderSetViewport(ren, &viewport);
                bmp = block_to_bmp(matrix[i][j]);
                tex = SDL_CreateTextureFromSurface(ren, bmp);
                SDL_RenderCopy(ren, tex, NULL, NULL);
                SDL_DestroyTexture(tex);
                SDL_FreeSurface(bmp);
            }
        }
    }
}
void print_hand(struct block array[80], SDL_Rect viewport, SDL_Renderer *ren)
{
    int row = 16;
    int col = 0;
    int x, y;
    SDL_Texture *tex;
    SDL_Surface *bmp;
    for (int i = 0; i < 80; i++)
    {
        if (col >= 15)
        {
            col = 0;
            row++;
        }
        if (array[i].value != 0)
        {
            matrix_coords_to_x_y(row, col, &x, &y);
            viewport.x = x;
            viewport.y = y;
            SDL_RenderSetViewport(ren, &viewport);
            bmp = block_to_bmp(array[i]);
            tex = SDL_CreateTextureFromSurface(ren, bmp);
            SDL_RenderCopy(ren, tex, NULL, NULL);
            SDL_DestroyTexture(tex);
            SDL_FreeSurface(bmp);
            col++;
        }
    }
}
int get_block_ID_from_hand(struct block array[80], int rowIn, int colIn)
{
    int row = 16;
    int col = 0;
    printf("RC :%d %d\n");
    int x, y;
    for (int i = 0; i < 80; i++)
    {
        if (col >= 15)
        {
            col = 0;
            row++;
        }
        if (array[i].value != 0)
            if (col == colIn && row == rowIn)
            {
                return i;
                printf("ID %d\n", i);
            }
        col++;
    }
    return 999;
}
void switch_in_matrix(struct block matrix[][MATRIX],int rowA, int colA, int rowB, int colB){
    struct block b;
    b=matrix[rowA][colA];
    matrix[rowA][colA]=matrix[rowB][colB];
    matrix[rowB][colB]=b;
}
//struktura zawierająca dane, które zostanły przekazane do wątku
struct thread_data_t
{
    int connection_socket_descriptor;
};

//wskaźnik na funkcję opisującą zachowanie wątku
void *ThreadBehavior(void *t_data)
{
    //The window we'll be rendering to
    SDL_Window *window = NULL;

    //The surface contained by the window
    SDL_Surface *screenSurface = NULL;

    //Initialize SDL

    struct thread_data_t *th_data = (struct thread_data_t *)t_data;
    pthread_detach(pthread_self());
    char buf[2000];
    char conn[10] = "Connected\n";
    struct block b;
    int first = 1;
    int ID1;
    int ID2;
    int row;
    int col;
    int x, y;
    int res = resolution;
    int isHolding = 0;
    int isMoving = 0;
    int rowHold;
    int colHold;
    SDL_Window *w = SDL_CreateWindow("Hello world!", 100, 100, res, res, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface *bmp;
    SDL_Texture *tex;
    SDL_Event e;
    int sw = 0;
    int quit = 0;
    bmp = SDL_LoadBMP("back.bmp");
    tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_Rect Viewport_main;
    Viewport_main.x = 0;
    Viewport_main.y = 0;
    Viewport_main.w = resolution * 0.757 / 15 -3;
    Viewport_main.h = resolution * 0.757 / 15 -3;
    SDL_Rect Viewport_back;
    Viewport_back.x = 0;
    Viewport_back.y = 0;
    Viewport_back.w = resolution;
    Viewport_back.h = resolution;
    while (th_data->connection_socket_descriptor)
    {
        sw = 0;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_KEYDOWN)
            {
                //Select surfaces based on key press
                switch (e.key.keysym.sym)
                {
                case SDLK_p:
                    sw = 1;
                    break;

                case SDLK_h:
                    sw = 3;
                    break;

                case SDLK_c:
                    sw = 4;
                    break;

                case SDLK_b:
                    sw = 5;
                    break;
                case SDLK_w:
                    bmp = block_to_bmp(hand[0]);
                    break;

                case SDLK_q:
                    exit(0);
                    break;

                default:
                    break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                SDL_GetMouseState(&x, &y);
                printf("%d %d\n", x, y);
                x_y_to_matrix_coords(&row, &col, x, y);
                if (row >= 16 && row < 19 && col <= 14 && isHolding == 0)
                {
                    printf("Holding\n");
                    printf("%d %d\n\n", row, col);
                    rowHold = row;
                    colHold = col;
                    isHolding = 1;
                }
                else if (row >= 16 && row < 19 && col <= 14 && isHolding == 1)
                {
                    printf("Release\n");
                    printf("%d %d\n\n", row, col);
                    isHolding = 0;
                    ID1 = get_block_ID_from_hand(hand, rowHold, colHold);
                    ID2 = get_block_ID_from_hand(hand, row, col);
                    if (ID1 != 999 && ID2 != 999)
                        switch_in_hand(hand, ID1, ID2);
                    rowHold = 0;
                    colHold = 0;
                }
                else if (row <= 14 && col <= 14 && isHolding == 1)
                {
                    printf("Release\n");
                    printf("%d %d\n\n", row, col);
                    isHolding = 0;
                    ID1 = get_block_ID_from_hand(hand, rowHold, colHold);
                    if (ID1 != 999)
                    {
                        b = get_from_hand(hand, ID1);
                        place_on_matrix(matrix, b, row, col);
                    }
                    rowHold = 0;
                    colHold = 0;
                }
                else if (row <= 14 && col <= 14 && isMoving == 0)
                {
                    printf("Holding\n");
                    printf("%d %d\n\n", row, col);
                    rowHold = row;
                    colHold = col;
                    isMoving = 1;
                }
                else if (row <= 14 && col <= 14 && isMoving == 1)
                {
                    printf("Release\n");
                    printf("%d %d\n\n", row, col);
                    isMoving = 0;
                    switch_in_matrix(matrix,row,col,rowHold,colHold);
                    rowHold = 0;
                    colHold = 0;
                }

                //                printf("%d %d\n", row, col);
                //matrix_coords_to_x_y(row, col, &x, &y);
                              
            }
        }
        SDL_RenderClear(ren);
        tex = SDL_CreateTextureFromSurface(ren, bmp);

        bzero(buf, 2000);
        //User requests quit

        SDL_RenderSetViewport(ren, &Viewport_back);
        //SDL_FreeSurface(bmp);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_DestroyTexture(tex);
        print_hand(hand, Viewport_main, ren);
        print_matrix(matrix, Viewport_main, ren);
        //SDL_FreeSurface(bmp);
        int n = 0;
        //printf("%d\n", sw);
        //while ((buf[n++] = getchar()) != '\n')
        ;
        if (sw == 1)
            sw=1;
        else if (sw == 3)
            print_hand(hand, Viewport_main, ren);
        else if (sw == 5)
            send(th_data->connection_socket_descriptor, "b", 1, MSG_DONTWAIT);
        if (sw == 10)
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
        else if (buf[0] == 'm')
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
        else if (buf[0] == 's')
        {
            send(th_data->connection_socket_descriptor, buf, 1, MSG_DONTWAIT);
            matrix_to_buffer(matrix, buf);
        }
        else if (sw == 4)
        {
            send(th_data->connection_socket_descriptor, "c", 1, MSG_DONTWAIT);
            matrix_to_buffer(matrix, buf);
        }
        send(th_data->connection_socket_descriptor, buf, 2000, MSG_DONTWAIT);
        SDL_RenderPresent(ren);
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
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
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
    printf("X\n");
    handleConnection(connection_socket_descriptor);
    printf("X2\n");
    while (1)
    {
        printf("X3\n");
    }

    return 0;
}
