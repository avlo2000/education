#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

typedef struct complex
{
    double real;
    double imag;
} complex_t;

complex_t complex_add(const complex_t* a, const complex_t* b)
{
    complex_t c;
    c.real = a->real + b->real;
    c.imag = a->imag + b->imag;
    return c;
}

complex_t complex_sub(const complex_t* a, const complex_t* b)
{
    complex_t c;
    c.real = a->real - b->real;
    c.imag = a->imag - b->imag;
    return c;
}

complex_t complex_mul(const complex_t* a, const complex_t* b)
{
    complex_t c;
    c.real = a->real * b->real - a->imag * b->imag;
    c.imag = a->real * b->imag + a->imag * b->real;
    return c;
}

complex_t complex_unit(double phase)
{
    complex_t c;
    c.real = cos(phase);
    c.imag = sin(phase);
    return c;
}

complex_t complex_conj(const complex_t* a)
{
    complex_t b;
    b.real = a->real;
    b.imag = -a->imag;
    return b;
}

void complex_print(const complex_t* a) {
    printf("%f+%fi  ", a->real, a->imag);
}

complex_t* fft(complex_t* a, int n)
{
    if(n == 1)
    {
        return a;
    }
    complex_t* even = malloc(sizeof(complex_t) * n / 2);
    complex_t* odd = malloc(sizeof(complex_t) * (n + 1) / 2);
    for(int i = 0; 2 * i < n; i++) {
        even[i] = a[i * 2];
        odd[i] = a[i * 2 + 1];
    }
    complex_t* fft_even  = fft(even, n / 2);
    complex_t* fft_odd  = fft(odd, (n + 1) / 2);
    complex_t* res = malloc(sizeof(complex_t) * n);
    for(int i = 0; 2 * i < n; i++)
    {
        double phs = -2.0 * PI * i / n;
        complex_t unit = complex_unit(phs);
        complex_t rotated_odd = complex_mul(&fft_odd[i], &unit);
        res[i] = complex_add(&fft_even[i], &rotated_odd);
        res[i + n / 2] = complex_sub(&fft_even[i], &rotated_odd);
    }
    free(fft_even);
    free(fft_odd);
    return res;
}

complex_t* ifft(complex_t* a, int n)
{
    for(int i = 0; i < n; ++i)
        a[i] = complex_conj(&a[i]);
    complex_t* a_fft = fft(a, n);
    for(int i = 0; i < n; ++i) {
        a_fft[i] = complex_conj(&a_fft[i]);
        a_fft[i].real /= n;
        a_fft[i].imag /= n;
    }
    return a_fft;
}

int main()
{
    printf("HELLO FFT\n");
    int n = 5;
    complex_t* x = malloc(n * sizeof(complex_t));
    for(int i = 0; i < n; ++i)
    {
        x[i].real = i;
        x[i].imag = 0.0;
    }
    complex_t* fft_x = fft(x, n);
    for(int i = 0; i < n; ++i)
    {
        printf("%f+%fi  ", fft_x[i].real, fft_x[i].imag);
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    complex_t* ifft_x = ifft(fft_x, n);
    for(int i = 0; i < n; ++i)
    {
        printf("%f+%fi  ", ifft_x[i].real, ifft_x[i].imag);
        printf("\n");
    }
    printf("\n");
    return 0;
}