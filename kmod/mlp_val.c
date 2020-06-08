#include <string.h>
#include <time.h>

#include "c_mlp.h"

#define DATA_FILE "./wpred_combined0.csv"
/* #define DATA_FILE "./wpred_idle.csv" */
#define NR_FEAT     15

#define m2d(x, i, j) (x)->values[i * (x)->ncol + j]
#define m1d(x, i) (x)->values[i]
#define _ReLU(x) (x > 0 ?  x : 0)


struct matrix {
    int nrow;
    int ncol;
    dtype *values;
};

int matmul(struct matrix *X, struct matrix *Y, struct matrix *Z) 
{
    int i, j, k;
    for(i = 0; i < X->nrow; i++)
        for(j = 0; j < Y->ncol; j++)
            for(k = 0; k < X->ncol; k++)
            {
                m2d(Z, i, j) = m2d(Z, i, j) + (m2d(X, i, k) * m2d(Y, k, j));
            }
    return 0;
}

int matadd(struct matrix *X, struct matrix *Y, struct matrix *Z)
{
    int i;
    for (i = 0; i < X->nrow * X->ncol; i++) {
        Z->values[i] = X->values[i] + Y->values[i];
    }
}

void print_matrix(struct matrix *X)
{
    int i, j;

    for(i=0; i<X->nrow; i++)
    {
        printf("\n\t");
        for(j=0; j<X->ncol;j++)
        {
            printf("%f\t", m2d(X, i, j));
        }
    }
    printf("\n");
}


void ReLU(struct matrix *X)
{
    int i;
    for (i = 0; i < X->nrow * X->ncol; i++) {
        X->values[i] = _ReLU(X->values[i]);
    }
}

float forward_pass(struct matrix *input){
    float output;
    dtype o1[10] = {0};
    dtype o2[10] = {0};

    struct matrix W1 = {16, 10, w1};
    struct matrix out1 = {1, 10, o1};
    struct matrix B1 = {1, 10, b1};
    struct matrix W2 = {10, 1, w2};
    struct matrix out2 = {1, 1, o2};
    struct matrix B2 = {1, 1, b2};

    matmul(input, &W1, &out1);

    matadd(&out1, &B1, &out1);

    ReLU(&out1);

    matmul(&out1, &W2, &out2);

    matadd(&out2, &B2, &out2);

    output = m1d(&out2, 0);

    /* printf("output: %f\n", output); */
    /* return output > 0.5 ? 1 : 0; */
    return output;
}


int main()
{
    dtype mval[NR_FEAT];
    struct matrix input = {1, NR_FEAT, mval};
    char line[300];
    int correct = 0, total = 0;
    int py_correct = 0, discrep = 0;

    FILE* f = fopen(DATA_FILE, "r");
    fgets(line, 300, f); // Header
    while (fgets(line, 300, f))
    {
        int i;
        char *token, *string, *tofree;

        tofree = string = strdup(line);
        for (i = 0; i < NR_FEAT; i++) {
            token = strsep(&string, ",");
            float num = strtof(token, NULL);
            input.values[i] = num;
            /* printf("%f ", num); */
        }
        int label = atoi(strsep(&string, ","));
        int py_pred = atoi(strsep(&string, ","));
        /* float py_output = strtof(strsep(&string, ","), NULL); */
        free(tofree);

        float output = forward_pass(&input);
        int prediction = output > 0.5 ? 1: 0; 
        total++;
        if (prediction == label)
            correct++;
        if (prediction != py_pred) {
            discrep++;
            /* printf("%d %d %d %f %f\n", label, py_pred, prediction, output, py_output); */
            printf("%d %d %d %f\n", label, py_pred, prediction, output);
        }
        if (py_pred == label)
            py_correct++;
    }

    printf("%d corrects out of %d. accuracy: %f\n", correct, total, (float)correct / total);
    printf("%d py_corrects, %d discreps\n", py_correct, discrep);

    /* printf("prediction: %d\n", prediction); */
    /* printf("execution time: %f\n", time_taken); */

    return 0;
}
