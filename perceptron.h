#ifndef H_NJ_PERCEPTRON_20150830
#define H_NJ_PERCEPTRON_20150830

typedef struct Perceptron Perceptron;

Perceptron* perceptron_new(int bias, int learningRate);
void perceptron_delete(Perceptron* self);
int perceptron_learn(Perceptron* self, const char* input, int output);
int perceptron_test(Perceptron* self, const char* input);

#endif /* H_NJ_PERCEPTRON_20150830 */
