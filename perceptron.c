#include "perceptron.h"
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/random.h>

typedef struct {
    char* key;
    int value;
} Item;

struct Perceptron {
    struct mutex mutex;
    Item** vector;
    int vectorSize;
    int bias;
    int learningRate;
};


static Item* perceptron_search(Perceptron* self, const char* input)
{
    int i = 0;
    Item* item = NULL;

    for (i = 0; i < self->vectorSize; i++) {
        if (self->vector[i]->key != NULL) {
            if (strcmp(input, self->vector[i]->key) == 0) {
                item = self->vector[i];
                break;
            }
        }
    }

    return item;
}

static int perceptron_add(Perceptron* self, const char* input)
{
    Item* item = NULL;
    int retval = -EFAULT;

    item = kmalloc(sizeof(*item), GFP_KERNEL);
    if (item != NULL) {
        void* newVector = NULL;

        item->key = kstrdup(input, GFP_KERNEL);
        item->value = get_random_int() / 10000000;

        self->vectorSize++;
        newVector = krealloc(self->vector, sizeof(*self->vector) * self->vectorSize, GFP_KERNEL);
        if (newVector != NULL) {
            self->vector = newVector;
            self->vector[self->vectorSize - 1] = item;
            retval = 0;
        }
        else {
            kfree(self->vector), self->vector = NULL;
            retval = -ENOMEM;
        }
    }
    return retval;
}

Perceptron* perceptron_new(int bias, int learningRate)
{
    Perceptron* self = NULL;

    self = kmalloc(sizeof(*self), GFP_KERNEL);
    if (self != NULL) {
        mutex_init(&self->mutex);
        self->vector = NULL;
        self->vectorSize = 0;
        self->bias = bias;
        self->learningRate = learningRate;
    }

    return self;
}

void perceptron_delete(Perceptron* self)
{
    if (self != NULL) {
        int i = 0;

        if (self->vector != NULL) {
            for (i = 0; i < self->vectorSize; i++) {
                kfree(self->vector[i]->key), self->vector[i]->key = NULL;
                kfree(self->vector[i]), self->vector[i] = NULL;
            }
            kfree(self->vector), self->vector = NULL;
        }
        kfree(self);
        mutex_destroy(&self->mutex);
    }
}

int perceptron_learn(Perceptron* self, const char* input, int output)
{
    Item* item = NULL;
    int retval = -EFAULT;

    mutex_lock(&self->mutex);

    item = perceptron_search(self, input);
    if (item == NULL) {
        retval = perceptron_add(self, input);
        if (retval == 0) {
            item = perceptron_search(self, input);
        }
    }

    if (item != NULL) {
        int outcome = 0;

        mutex_unlock(&self->mutex);
        outcome = perceptron_test(self, input);
        mutex_lock(&self->mutex);
        item->value += self->learningRate * (outcome - output);
        self->bias += self->bias + (outcome - output);
        retval = 0;
    }

    mutex_unlock(&self->mutex);
    return retval;
}

int perceptron_test(Perceptron* self, const char* input)
{
    int output = 0;
    Item* item = NULL;

    mutex_lock(&self->mutex);

    item = perceptron_search(self, input);
    if (item != NULL) {
        output = (item->value > 0);
    }

    mutex_unlock(&self->mutex);

    return output;
}
