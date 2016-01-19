#include <stdio.h>
int dim = 2;

int a[2][2][2], b[2][2][2];
int c[2][2][2];

void print()
{
      int i,j,k;
      for (i = 0; i < dim; i = i + 1)
    {
              for (j = 0; j < dim; j = j + 1)
        {
                    for(k = 0; k < dim; k = k + 1)
            {
                      printf("%d",c[i][j][k]);
                          printf(" ");
                    
            }
                printf("\n");
                  
        }
              printf("\n"); 
            
    }

}



void arraymult()
{
      int i,j,k;
      for (i = 0; i < dim; i = i + 1)
    {
              for (j = 0; j < dim; j = j + 1)
        {
                  for (k = 0; k < dim; k = k + 1)
            {
                          c[i][j][k] = a[i][j][k]*b[i][j][k];
                        
            }
                
        }
    }
    
    print();

}


int main()
{
      int i,j,k;
      printf("Enter matrix 1 of dim 2 x 2 x 2: \n");

      for (i = 0; i < dim; i = i + 1)
    {
              for (j = 0; j < dim; j = j + 1)
        {
                    for (k=0; k < dim; k = k + 1)
            {
                      scanf("%d", &a[i][j][k] );
                    
            }
                  
        }
            
    }
        printf("Enter matrix 2 of dim 2 x 2 x 2 : \n");
      for (i = 0; i < dim; i = i + 1)
    {
              for (j = 0; j < dim; j = j + 1)
        {
                    for (k = 0; k < dim; k = k + 1)
            {
                      scanf("%d", &b[i][j][k]);
                    
            }
                  
        }
            
    }
      arraymult();

      return 0;

}


