//test1.c
int main(){
    int i;
    int A[20], C[20], D[20];
    for (i = 4; i < 20; i++) {
        A[i] = C[i];
        D[i] = A[i - 4];
    }
    return 0;
}