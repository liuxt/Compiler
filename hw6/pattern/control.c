int MAIN(){
	int t1,t2;
	float f1,f2;
	t1=100;
	t2=-2;
	f1 = 3.3;
	f2 =-3.3;
	if(1){

		if(t1>t2){
			write("Correct!\n");
			if(f1>f2){

				write("Correct!\n");
				if (f1>0.0){

					write("Correct!\n");

				}
				else{
					write("wrong\n");
				}
			}
			else{
				write("wrong\n");
			}
			if(t2<t1){
				write("Correct!\n");
				if(t2<0){
					write("Correct!\n");
				}
				else{
					write("wrong\n");
				}
			}
			else{
				write("wrong\n");
			}
		}
		else{
			write("wrong\n");
		}
	}
	else{
		write("wrong\n");
	}
}
