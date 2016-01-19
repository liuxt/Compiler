/*************************************************************************
	> File Name: convert.c
	> Author: 
	> Mail: 
	> Created Time: Mon Jan 18 19:47:25 2016
 ************************************************************************/

#include<stdio.h>

float pi=3.1416;

float sqr(float num)
{
      return (num*num);

}

float calarea(int r)
{
      float area;
      area = pi * sqr(r);
      return area;

}

int fl(float num)
{
      int temp;
      temp = num;
      return temp;

}

int main(){
      
      int r;
      float area;
      float rem;
      printf("Enter an Integer :"); 
      scanf("%d", &r);

      area=calarea(r);
      rem=area-fl(area);
    
      printf("%f",area);
      printf(" ");
      printf("%f",rem);
      printf("\n");

      return 0;

}

