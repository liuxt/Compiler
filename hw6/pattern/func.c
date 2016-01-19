int k;


void try() {
    if (k < 10) {
        write(k);
        write("\n");
        k = k + 1;
        try();
    }
}

int MAIN() {
    k = 0;
    try();
    return 0;
}
