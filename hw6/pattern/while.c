void print_space(){
	write(" ");
}
void print_star(){
	write("*");
}

int MAIN(){
	int i,j,space,num;
	num=11;
/*	space = 5;  modify*/
	space = (num-1)/2;
/*	write(space);
	write("\n");*/
	while(space != 0){
		i=space;
		while(i!=0){
			print_space();
			i=i-1;
		}
		i=num-space*2;
		while(i!=0){
			print_star();
			i=i-1;
		}
		i=space;
		while(i!=0){
			print_space();
			i=i-1;
		}
		space =space-1;
		write("\n");
	}
	space = 5;
	j=3;
	while(j!=0){
			i=space;              
			while(i!=0){          
				print_space();
				i=i-1;        
			}                     
			i=num-space*2;       
			while(i!=0){         
				print_star();
				i=i-1;       
			}                    
			i=space;
			while(i!=0){
				print_space();
				i=i-1;
			}
			write("\n");
		j=j-1;
	}

}
