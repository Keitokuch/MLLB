#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/fpu/api.h>

#include "c_mlp.h"

#define NR_FEAT     15

#define m2d(x, i, j)    ((x)->values[i * (x)->ncol + j])
#define m1d(x, i)       ((x)->values[i])
#define _ReLU(x)        (x > 0 ?  x : 0)
#define ftox(f)         (*(unsigned *)&((float){f}))

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JC");
MODULE_DESCRIPTION("MLP in kernel");
MODULE_VERSION("0.02");

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
    return 0;
}

void ReLU(struct matrix *X)
{
    int i;
    for (i = 0; i < X->nrow * X->ncol; i++) {
        X->values[i] = _ReLU(X->values[i]);
    }
}

int forward_pass(struct matrix *input){
    float output;
    dtype o1[10] = {0};
    dtype o2[10] = {0};

    struct matrix W1 = {NR_FEAT, 10, w1};
    struct matrix out1 = {1, 10, o1};
    struct matrix B1 = {1, 10, b1};
    struct matrix W2 = {10, 1, w2};
    struct matrix out2 = {1, 1, o2};
    struct matrix B2 = {1, 1, b2};
    /* printk("pk test: %08x", ftox(m1d(&B2, 0))); */

    matmul(input, &W1, &out1);

    matadd(&out1, &B1, &out1);

    ReLU(&out1);

    matmul(&out1, &W2, &out2);

    matadd(&out2, &B2, &out2);

    output = m1d(&out2, 0);

    /* printf("output: %f\n", output); */
    printk("forward_pass output: %08x", ftox(output));
    return output > 0.5 ? 1 : 0;
    /* return output; */
}

static int jc_main(void) {
    dtype mval[NR_FEAT] = {
        1,0,0,0,1,0,0,0,0,0.008,0.009000000000000001,0,0.0,0,0
    };
    struct matrix input = {1, NR_FEAT, mval};
    int output;

    output = forward_pass(&input);
    printk("pred: %d", output);

    return 0;
}

static int __init jc_kmod_init(void) {
    printk(KERN_INFO "Hello JC");
    kernel_fpu_begin();
    jc_main();
    kernel_fpu_end();
    return 0;
}

static void __exit jc_kmod_exit(void) {
    printk(KERN_INFO "Bye JC");
}

module_init(jc_kmod_init);
module_exit(jc_kmod_exit);
