int get(int a[]){
    return a[3] + a[4];
}
int MAIN(){
    int b[4][5];
    b[3][3] = 1;
    b[3][4] = 2;
    write(get(b[3]));
}
