#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#define MATRIX 15
int bl=10;
int re=10;
int or=10;
int bu=10;
char buffer[1000];
struct block
{
    int value;
    int color;
    int unused;
};
void matrix_fill(struct block mat[][MATRIX])
{
    for(int i=0; i<MATRIX; i++)
    {
        for(int j=0; j<MATRIX; j++)
        {
            mat[i][j].value=0;
            mat[i][j].color=0;
        }
    }
}

void matrix_check(int matrix[][MATRIX])
{
    for(int i=0; i<MATRIX; i++)
    {
        printf(" Row %d: %d \n", i, row_check(matrix, i, 0, 0));
    }
}


int row_check(struct block matrix[][MATRIX], int r, int c, int g)
{
    if(c>=MATRIX-1)
    {
        return 999;
    }
    if(matrix[r][c].value==0)
    {
        row_check(matrix, r, c+1, g);
    }
    else if(matrix[r][c].value<matrix[r][c+1].value && matrix[r][c].color==matrix[r][c+1].color && abs(matrix[r][c].value-matrix[r][c+1].value)==1)
    {
        row_check(matrix, r, c+1, g+1);
        g=g+1;
    }
    else if(matrix[r][c].value>matrix[r][c+1].value && matrix[r][c].color==matrix[r][c+1].color && abs(matrix[r][c].value-matrix[r][c+1].value)==1)
    {
        g=g+1;
        row_check(matrix, r, c+1, g+1);
    }
    else if(matrix[r][c].value==matrix[r][c+1].value &&
            matrix[r][c].color!=matrix[r][c+1].color &&
            matrix[r][c].value==matrix[r][c+2].value &&
            matrix[r][c].color!=matrix[r][c+2].color&&
            matrix[r][c].value==matrix[r][c+3].value &&
            matrix[r][c].color!=matrix[r][c+3].color)
    {

        row_check(matrix, r, c+4, g+3);

    }
    else if(matrix[r][c].value==matrix[r][c+1].value &&
            matrix[r][c].color!=matrix[r][c+1].color &&
            matrix[r][c].value==matrix[r][c+2].value &&
            matrix[r][c].color!=matrix[r][c+2].color)
    {

        row_check(matrix, r, c+3, g+2);

    }
    else if (g>=2)
    {
        row_check(matrix, r, c+1, 0);
    }
    else if (g<2)
    {
        return c+1;
    }
}
void list_fill(struct block array[10], int c)
{
    for(int i=0; i<10; i++)
    {
        array[i].value=i+1;
        array[i].unused=2;
        array[i].color=c;
    }
}
void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}
struct block randomize_block(struct block array1[10],struct block array2[10],struct block array3[10],struct block array4[10])
{
    struct block b;
    int index;
    bool chosen=false;
    double r = rand()%1000;
    r=r/1000;
    while(chosen==false)
    {
        if(r>=0.75 && bl>0)
        {
            index=rand()%10;
            b=array1[index];
            while(b.unused==0)
            {
                index=rand()%10;
                b=array1[index];
            }
            array1[index].unused=array1[index].unused-1;
            if(array1[index].unused==0)bl=bl-1;
            chosen=true;
        }
        if (r>=0.5 && r<0.75 && re>0)
        {
            index=rand()%10;
            b=array2[index];
            while(b.unused==0)
            {
                index=rand()%10;
                b=array2[index];
            }
            array2[index].unused=array2[index].unused-1;
            if(array2[index].unused==0)re=re-1;
            chosen=true;

        }
        if (r>=0.25 && r<0.5 && or>0)
        {
            index=rand()%10;
            b=array3[index];
            while(b.unused==0)
            {
                index=rand()%10;
                b=array3[index];
            }
            array3[index].unused=array3[index].unused-1;
            if(array3[index].unused==0)or=or-1;
            chosen=true;
        }
        if (r<0.25 && bu>0)
        {
            index=rand()%10;
            b=array4[index];
            while(b.unused==0)
            {
                index=rand()%10;
                b=array4[index];
            }
            array4[index].unused=array4[index].unused-1;
            if(array4[index].unused==0)bu=bu-1;
            chosen=true;

        }
        r = rand()%1000;
        r=r/1000;
    }
    b.unused=b.unused-1;
    return b;
}

void matrix_to_string(struct block matrix[][MATRIX]){
    for(int i=0; i<MATRIX; i++)
    {
        for(int j=0; j<MATRIX; j++)
        {
            sprintf(buffer  + strlen(buffer), ";%d|%d", matrix[i][j].value, matrix[i][j].color);
            //printf("%d|%d;", matrix[i][j].value, matrix[i][j].color);
        }
        sprintf(buffer + strlen(buffer), "*\n");
        //printf(buffer + strlen(buffer), "\n");
    }
//printf("%s",buffer);
matrix_fill(matrix);
int isval=0;
int iscol=0;
int row=0;
int col=-1;
int temp;
char temp2[2] ="";
     for (int i = 0; i < strlen(buffer); i++){
        if(buffer[i]=='\n'){
            col=-1;
            row=row+1;
        }else if(buffer[i]==';'){
            col=col+1;
            isval=1;
            iscol=0;
        } else if (buffer[i]=='|'){
            isval=0;
            iscol=1;
        } else if (isval==1){
            temp2[0]=buffer[i];
            matrix[row][col].value=atoi(temp2);
            if(buffer[i+1]=='0'){
            matrix[row][col].value=10;
            }
        } else if (iscol==1){
            temp2[0]=buffer[i];
            matrix[row][col].color=atoi(temp2);
        }
            //printf("Row: %d Col: %d Char: %c IsVal: %d IsCol: %d\n",row, col,  buffer[i], isval,iscol);
     }

}
void print_matrix(struct block matrix[][MATRIX]){
    for(int i=0; i<MATRIX; i++)
    {
        for(int j=0; j<MATRIX; j++)
        {
            printf("[%d ",matrix[i][j].value);
            switch(matrix[i][j].color)
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
    }}
int main()
{
    srand(time(0));
    int isgood=0;
    struct block b;
    struct block black[10];
    list_fill(black, 1);
    struct block red[10];
    list_fill(red, 2);
    struct block orange[10];
    list_fill(orange, 3);
    struct block blue[10];
    list_fill(blue, 4);
    /*for(int i=0; i<80;i++){
        b=randomize_block(black,red,orange,blue);
        printf("  %d,  Value: %d, Unused: %d, ",i,b.value, b.unused);
        if(b.unused==0)isgood++;
        switch(b.color){
    case 1: printf("Color: black \n");
    break;
    case 2: printf("Color: red \n");
    break;
    case 3: printf("Color: orange \n");
    break;
    case 4: printf("Color: blue \n");
    break;
    default: printf("blad\n");
    break;
        }

    }*/

    struct block matrix[MATRIX][MATRIX];
    matrix_fill(matrix);

    matrix[0][4].value=1;
    matrix[0][5].value=4;
    matrix[0][6].value=5;
    matrix[0][4].color=2;
    matrix[0][5].color=2;
    matrix[0][6].color=2;
//printf("%s",buffer);
    matrix_check(matrix);
    print_matrix(matrix);
    matrix_to_string(matrix);
    printf("\n");
    print_matrix(matrix);


    return 0;
}
