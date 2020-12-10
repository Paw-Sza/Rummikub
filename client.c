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
#include <arpa/inet.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_ttf.h>
#define MATRIX 15
#define BUF_SIZE 1024
#define NUM_THREADS 5
#define resolution 600
double tile = resolution * 0.757 / 15;
int turn = 0;
char check_error[50];
int check = 0;
struct block
{
    int value;
    int color;
    int unused;
};
struct block matrix[MATRIX][MATRIX];
struct block matrix_saved[MATRIX][MATRIX];
struct block hand[80];
struct block hand_saved[80];
void print_matrix_console(struct block matrix[][MATRIX])
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
    *y = (int)yd + 3;
    *x = (int)xd + 3;
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
void render_button(char *source, int row, int col, SDL_Rect viewport, SDL_Renderer *ren)
{
    int x, y;
    SDL_Texture *tex;
    SDL_Surface *bmp;
    viewport.w = resolution * 0.757 / 15 * 3;
    viewport.h = resolution * 0.757 / 15;
    matrix_coords_to_x_y(row, col, &x, &y);
    viewport.x = x;
    viewport.y = y;
    SDL_RenderSetViewport(ren, &viewport);
    bmp = SDL_LoadBMP(source);
    tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(bmp);
    viewport.w = resolution * 0.757 / 15;
    viewport.h = resolution * 0.757 / 15;
}
void render_text(char *text, TTF_Font *gFont, double size, int row, int col, SDL_Rect viewport, SDL_Renderer *ren)
{
    SDL_Color textColor = {0, 0, 0};
    int x, y;
    SDL_Texture *tex;
    SDL_Surface *bmp;
    viewport.w = size;
    viewport.h = resolution * 0.757 / 15;
    matrix_coords_to_x_y(row, col, &x, &y);
    viewport.x = x;
    viewport.y = y;
    SDL_RenderSetViewport(ren, &viewport);
    bmp = TTF_RenderText_Blended(gFont, text, textColor);
    tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(bmp);
    viewport.w = resolution * 0.757 / 15;
    viewport.h = resolution * 0.757 / 15;
}
int get_block_ID_from_hand(struct block array[80], int rowIn, int colIn)
{
    int row = 16;
    int col = 0;
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
void switch_in_matrix(struct block matrix[][MATRIX], int rowA, int colA, int rowB, int colB)
{
    struct block b;
    b = matrix[rowA][colA];
    matrix[rowA][colA] = matrix[rowB][colB];
    matrix[rowB][colB] = b;
}
void get_from_matrix_to_hand(struct block matrix[][MATRIX], struct block hand[80], int rowA, int colA)
{
    struct block b;
    b = matrix[rowA][colA];
    if (b.unused != 0)
    {
        matrix[rowA][colA].value = 0;
        matrix[rowA][colA].color = 0;
        matrix[rowA][colA].value = 0;
        put_in_hand(hand, b);
    }
}
int is_valid_ip(char *ip)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
    return result != 0;
}
int is_valid_port(char *port)
{
    int result = htons(atoi(port));
    return result != 0;
} //struktura zawierająca dane, które zostanły przekazane do wątku
struct thread_data_t
{
    int connection_socket_descriptor;
};

//wskaźnik na funkcję opisującą zachowanie wątku
void *ThreadBehavior(void *t_data)
{
    //The window we'll be rendering to
    struct thread_data_t *th_data = (struct thread_data_t *)t_data;
    pthread_detach(pthread_self());
    char buf[2000];
    struct block b;
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
    int hasMoved = 0;
    SDL_Window *w = SDL_CreateWindow("Rummikub", 100, 100, res, res, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface *bmp;
    SDL_Texture *tex;
    SDL_Event e;
    TTF_Font *gFont = TTF_OpenFont("arial.ttf", 100);
    bmp = SDL_LoadBMP("back.bmp");
    tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_Rect Viewport_main;
    Viewport_main.x = 0;
    Viewport_main.y = 0;
    Viewport_main.w = resolution * 0.757 / 15 - 3;
    Viewport_main.h = resolution * 0.757 / 15 - 3;
    SDL_Rect Viewport_back;
    Viewport_back.x = 0;
    Viewport_back.y = 0;
    Viewport_back.w = resolution;
    Viewport_back.h = resolution;
    send(th_data->connection_socket_descriptor, "H\n", 2, 0);
    memcpy(matrix, matrix_saved, sizeof(matrix));
    while (th_data->connection_socket_descriptor)
    {
        while (SDL_PollEvent(&e) != 0)
        {

            if (e.type == SDL_KEYDOWN)
            {
                //Select surfaces based on key press
                switch (e.key.keysym.sym)
                {
                case SDLK_END:
                    exit(0);
                    break;

                default:
                    break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && turn == 1)
            {
                SDL_GetMouseState(&x, &y);
                printf("%d %d\n", x, y);
                x_y_to_matrix_coords(&row, &col, x, y);
                //if click on hand not holding
                if (row >= 16 && row < 19 && col <= 14 && isHolding == 0 && isMoving == 0)
                {
                    rowHold = row;
                    colHold = col;
                    isHolding = 1;
                }
                //if click on hand holding
                else if (row >= 16 && row < 19 && col <= 14 && isHolding == 1 && isMoving == 0)
                {
                    isHolding = 0;
                    ID1 = get_block_ID_from_hand(hand, rowHold, colHold);
                    ID2 = get_block_ID_from_hand(hand, row, col);
                    if (ID1 != 999 && ID2 != 999)
                        switch_in_hand(hand, ID1, ID2);
                    rowHold = 0;
                    colHold = 0;
                } //click on check
                  //click on End Turn
                else if (row == 3 && col <= 19 && col >= 16)
                {
                    if (hasMoved == 0)
                        send(th_data->connection_socket_descriptor, "b", 1, 0);
                    send(th_data->connection_socket_descriptor, "t", 1, 0);
                    bzero(buf, 2000);
                    matrix_to_buffer(matrix, buf);
                    send(th_data->connection_socket_descriptor, buf, 2000, 0);

                } //click on Undo
                else if (row == 5 && col <= 19 && col >= 16)
                {
                    memcpy(matrix, matrix_saved, sizeof(matrix));
                    memcpy(hand, hand_saved, sizeof(hand));
                    hasMoved = 0;
                }
                //if click on matrix holding
                else if (row <= 14 && col <= 14 && isHolding == 1 && isMoving == 0)
                {
                    isHolding = 0;
                    hasMoved = 1;
                    ID1 = get_block_ID_from_hand(hand, rowHold, colHold);
                    if (ID1 != 999 && matrix[row][col].value == 0)
                    {
                        b = get_from_hand(hand, ID1);
                        place_on_matrix(matrix, b, row, col);
                        matrix[row][col].unused = 1;
                    }
                    rowHold = 0;
                    colHold = 0;
                    printf("Sending Matrix/n");
                    send(th_data->connection_socket_descriptor, "s", 1, 0);
                    bzero(buf, 2000);
                    matrix_to_buffer(matrix, buf);
                    send(th_data->connection_socket_descriptor, buf, 2000, 0);
                    printf("Sent Matrix/n");
                }
                //if click on matrix not holding or moving
                else if (row <= 14 && col <= 14 && isMoving == 0 && isHolding == 0)
                {
                    printf("Moving %d %d \n", matrix[row][col].value, matrix[row][col].color);
                    rowHold = row;
                    colHold = col;
                    isMoving = 1;
                }
                //if click on matrix not holding and moving
                else if (row <= 14 && col <= 14 && isMoving == 1 && isHolding == 0)
                {
                    isMoving = 0;
                    hasMoved = 1;
                    switch_in_matrix(matrix, row, col, rowHold, colHold);
                    rowHold = 0;
                    colHold = 0;
                    printf("Sending Matrix/n");
                    send(th_data->connection_socket_descriptor, "s", 1, 0);
                    bzero(buf, 2000);
                    matrix_to_buffer(matrix, buf);
                    send(th_data->connection_socket_descriptor, buf, 2000, 0);
                    printf("Sent Matrix/n");
                    //if click on matrix moving
                }
                else if (row >= 16 && row < 19 && col <= 14 && isMoving == 1 && isHolding == 0)
                {
                    printf("Trying ot put in hand %d %d , %d %d %d\n", rowHold, colHold, matrix[rowHold][colHold].value, matrix[rowHold][colHold].color, matrix[rowHold][colHold].unused);
                    isMoving = 0;
                    get_from_matrix_to_hand(matrix, hand, rowHold, colHold);
                    rowHold = 0;
                    colHold = 0;
                    print_matrix_console(matrix);
                    printf("Sending Matrix/n");
                    send(th_data->connection_socket_descriptor, "s", 1, 0);
                    bzero(buf, 2000);
                    matrix_to_buffer(matrix, buf);
                    send(th_data->connection_socket_descriptor, buf, 2000, 0);
                    printf("Sent Matrix/n");
                }
            }
        }
        SDL_RenderClear(ren);
        //render background
        tex = SDL_CreateTextureFromSurface(ren, bmp);
        bzero(buf, 2000);
        SDL_RenderSetViewport(ren, &Viewport_back);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_DestroyTexture(tex);
        //render hand
        print_hand(hand, Viewport_main, ren);
        //render matrix
        print_matrix(matrix, Viewport_main, ren);
        //render buttons & texts
        render_button("buttons/blank.bmp", 3, 16, Viewport_main, ren);
        render_text("End Turn", gFont, tile * 3, 3, 16, Viewport_main, ren);
        render_button("buttons/blank.bmp", 5, 16, Viewport_main, ren);
        render_text("Undo", gFont, tile * 3, 5, 16, Viewport_main, ren);
        render_text(check_error, gFont, tile * 4, 16, 15, Viewport_main, ren);
        if (isHolding == 1 || isMoving == 1)
            render_text("Trzymasz klocek", gFont, tile * 4, 14, 15, Viewport_main, ren);
        if (turn == 0)
        {
            hasMoved = 0;
            render_text("Tura przeciwnika", gFont, tile * 4, 15, 15, Viewport_main, ren);
        }
        if (turn == 1)
        {
            render_text("Twoja trua", gFont, tile * 3, 15, 15, Viewport_main, ren);
        }
        if (hasMoved == 0)
        {
            render_text("Nie wykonano ruchu", gFont, (tile * 5) - 10, 1, 15, Viewport_main, ren);
        }
        if (hasMoved == 1)
        {
            render_text("Wykonano ruch", gFont, tile * 4, 1, 15, Viewport_main, ren);
        }
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
    int recv_turn = 0;
    int recv_check = 0;
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
            switch (buf[0])
            {
            case '+':
                recv_matrix = 1;
                bzero(buf, 2000);
                bzero(matrixbuf, 2000);
                break;
            case '-':
                recv_block = 1;
                bzero(buf, 2000);
                break;
            case '=':
                recv_matrix = 0;
                buffer_to_matrix(matrix, matrixbuf);
                memcpy(matrix_saved, matrix, sizeof(matrix));
                bzero(buf, 2000);
                bzero(matrixbuf, 2000);
                break;
            case '_':
                strncat(blockbuf, &buf[0], 1);
                b = buffer_to_block(blockbuf);
                put_in_hand(hand, b);
                memcpy(hand_saved, hand, sizeof(hand));
                recv_block = 0;
                bzero(buf, 2000);
                bzero(blockbuf, 6);
                break;
            case '#':
                recv_turn = 1;
                bzero(buf, 2000);
                break;
            case '$':
                recv_turn = 0;
                bzero(buf, 2000);
                break;
            case '@':
                recv_check = 1;
                strcpy(check_error, "");
                bzero(buf, 2000);
                break;
            case '?':
                check = 1;
                strcpy(check_error, "Ulozenie poprawne");
                bzero(buf, 2000);
                break;
            case '!':
                recv_check = 0;
                bzero(buf, 2000);
                break;
            default:
                if (recv_matrix == 1)
                {
                    strncat(matrixbuf, &buf[0], 1);
                }
                else if (recv_block == 1)
                {
                    strncat(blockbuf, &buf[0], 1);
                }
                else if (recv_turn == 1)
                {
                    char temp[1];
                    temp[0] = buf[0];
                    turn = atoi(temp);
                    if (turn == 0)
                        memcpy(hand_saved, hand, sizeof(hand));
                }
                else if (recv_check == 1)
                {

                    check = 0;
                    strncat(check_error, &buf[0], 1);
                }
                {
                    //printf("%s", buf);
                }
                break;
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
    int start = 0;
    int row, col, x, y, editing;

    TTF_Font *gFont = TTF_OpenFont("arial.ttf", 100);
    char IP[50] = "";
    char PORT[50] = "";
    char ERROR[100] = "";
    strcpy(IP, argv[1]);
    strcpy(PORT, argv[2]);
    SDL_Window *gWindow = SDL_CreateWindow("Rummikub-setup", 100, 100, resolution * 5 / 7, resolution * 5 / 7, SDL_WINDOW_SHOWN);

    //The window renderer
    SDL_Renderer *gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_Texture *gTexture;
    SDL_Surface *background = SDL_LoadBMP("back2.bmp");
    SDL_Event event;
    SDL_Rect Viewport_main;
    Viewport_main.x = 0;
    Viewport_main.y = 0;
    Viewport_main.w = resolution * 0.757 / 15;
    Viewport_main.h = resolution * 0.757 / 15;
    SDL_Rect Viewport_back;
    Viewport_back.x = 0;
    Viewport_back.y = 0;
    Viewport_back.w = 500;
    Viewport_back.h = 500;
    SDL_StartTextInput();
    while (start == 0)
    {
        SDL_RenderSetViewport(gRenderer, &Viewport_back);
        gTexture = SDL_CreateTextureFromSurface(gRenderer, background);
        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
        SDL_DestroyTexture(gTexture);
        render_button("buttons/blank.bmp", 8, 9, Viewport_main, gRenderer);
        render_text("Polacz", gFont, tile * 3, 8, 9, Viewport_main, gRenderer);
        render_button("buttons/blank.bmp", 3, 3, Viewport_main, gRenderer);
        render_text("IP:", gFont, tile, 3, 0, Viewport_main, gRenderer);
        render_text(IP, gFont, tile * 3, 3, 3, Viewport_main, gRenderer);
        render_text("Port:", gFont, tile * 2, 6, 0, Viewport_main, gRenderer);
        render_button("buttons/blank.bmp", 6, 3, Viewport_main, gRenderer);
        render_text(PORT, gFont, tile * 3, 6, 3, Viewport_main, gRenderer);
        render_text(ERROR, gFont, tile * 14, 11, 0, Viewport_main, gRenderer);
        render_text("Witaj w Rummikub!", gFont, tile * 5, 0, 5, Viewport_main, gRenderer);
        render_text("Kliknij na IP lub port i wpisz wartosc", gFont, tile * 10, 1, 3, Viewport_main, gRenderer);
        render_text("Sterowanie:", gFont, tile * 4, 2, 5, Viewport_main, gRenderer);
        render_text("Backspace - wyzerowanie", gFont, tile * 6, 3, 7, Viewport_main, gRenderer);
        render_text("END - zamkniecie programu", gFont, tile * 6, 4, 7, Viewport_main, gRenderer);

        SDL_DestroyTexture(gTexture);
        SDL_RenderPresent(gRenderer);
        SDL_RenderClear(gRenderer);
        if (SDL_PollEvent(&event))
        {

            switch (event.type)
            {

            case SDL_TEXTINPUT:
                /* Add new text onto the end of our text */
                if (editing == 1)
                    strcat(IP, event.text.text);
                if (editing == 2)
                    strcat(PORT, event.text.text);
                break;
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&x, &y);
                printf("%d %d\n", x, y);
                x_y_to_matrix_coords(&row, &col, x, y);
                printf("%d %d\n", row, col);
                if (row == 3 && col >= 3 && col <= 6)
                {
                    editing = 1;
                }
                else if (row == 6 && col >= 3 && col <= 6)
                {
                    editing = 2;
                }
                else if (row == 8 && col >= 9 && col <= 11)
                {
                    if (is_valid_ip(IP) != 1)
                    {
                        sprintf(ERROR, "Adres IP jest w niepoprawnym formacie");
                    }
                    else if (is_valid_port(PORT) != 1)
                    {
                        sprintf(ERROR, "Port jest w niepoprawnym formacie");
                    }
                    else
                    {

                        strcpy(ERROR, "");
                        server_host_entity = gethostbyname(IP);
                        if (!server_host_entity)
                        {
                            sprintf(ERROR, "Nie mozna uzyskac adresu IP serwera");
                        }

                        connection_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
                        if (connection_socket_descriptor < 0)
                        {
                            sprintf(ERROR, "Blad przy probie utworzenia gniazda");
                        }

                        memset(&server_address, 0, sizeof(struct sockaddr));
                        server_address.sin_family = AF_INET;
                        memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
                        server_address.sin_port = htons(atoi(PORT));

                        connect_result = connect(connection_socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
                        if (connect_result < 0)
                        {
                            sprintf(ERROR, "Blad przy probie polaczenia z serwerem (%s:%d)", IP, atoi(PORT));
                        }
                        else if (connect_result >= 0)
                            start = 1;
                    }
                }
                break;
            }
            if (event.key.keysym.sym == SDLK_BACKSPACE)
            {
                if (editing == 1)
                    strcpy(IP, "");
                if (editing == 2)
                    strcpy(PORT, "");
            }
            if (event.key.keysym.sym == SDLK_END)
                exit(0);
        }
    }
    SDL_StartTextInput();
    SDL_DestroyWindow(gWindow);
    handleConnection(connection_socket_descriptor);

    return 0;
}
