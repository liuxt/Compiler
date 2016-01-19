int g_a;
void f(){
	g_a = 3;
}
void f_p(){
	write(g_a);
	write("\n");
}
int MAIN(){
	int g_a;
	int l_a;
	f();
	l_a = 4;
	write(l_a);
	write("\n");
	g_a = l_a;
	write(g_a);
	write("\n");
	f_p();
	return 0;

}
