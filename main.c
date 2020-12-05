#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#define MATRIX 15
int bl=10;
int re=10;
int or=10;
int bu=10;
struct block{
        int value;
        int color;
        int unused;
} ;
void matrix_fill(int mat[][MATRIX]){
    for(int i=0; i<MATRIX;i++){
        for(int j=0; j<MATRIX;j++){
            mat[i][j]=0;
        }
    }
}
void matrix_check(int matrix[][MATRIX]){
    for(int i=0; i<MATRIX;i++){
        for(int j=0; j<MATRIX;j++){
            matrix[i][j]=0;
        }
    }
}
void list_fill(struct block array[10], int c){
for(int i=0;i<10;i++){
    array[i].value=i+1;
    array[i].unused=2;
    array[i].color=c;
}
}
struct block randomize_block(struct block array1[10],struct block array2[10],struct block array3[10],struct block array4[10]){
    struct block b;
    int index;
    bool chosen=false;
    double r = rand()%1000;
    r=r/1000;
    while(chosen==false){
    if(r>=0.75 && bl>0){
        index=rand()%10;
        b=array1[index];
        while(b.unused==0){
        index=rand()%10;
        b=array1[index];}
        array1[index].unused=array1[index].unused-1;
        if(array1[index].unused==0)bl=bl-1;
        chosen=true;
    }
    if (r>=0.5 && r<0.75 && re>0){
        index=rand()%10;
        b=array2[index];
        while(b.unused==0){
        index=rand()%10;
        b=array2[index];}
        array2[index].unused=array2[index].unused-1;
        if(array2[index].unused==0)re=re-1;
        chosen=true;

    }
    if (r>=0.25 && r<0.5 && or>0){
        index=rand()%10;
        b=array3[index];
        while(b.unused==0){
        index=rand()%10;
        b=array3[index];}
        array3[index].unused=array3[index].unused-1;
        if(array3[index].unused==0)or=or-1;
        chosen=true;
    }
    if (r<0.25 && bu>0){
        index=rand()%10;
        b=array4[index];
        while(b.unused==0){
        index=rand()%10;
        b=array4[index];}
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
int main()
{
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
for(int i=0; i<80;i++){
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

}

int matrix[MATRIX][MATRIX];
matrix_fill(matrix);

    for(int i=0; i<MATRIX;i++){
        for(int j=0; j<MATRIX;j++){
            printf("[%d] ",matrix[i][j]);
        }
        printf("\n");
    }

    return 0;
}
