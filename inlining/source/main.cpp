//__attribute__((always_inline)) 
static float mult_float(float num1, int num2) {
    return num1 * num2;
}

//__attribute__((always_inline)) 
static int mult_fixed(int num1, int num2) {
    return num1 * num2;
}

int main(void) {
    float f1=2;
    int k1=3;
    float r1 = mult_float(f1, k1);
}
