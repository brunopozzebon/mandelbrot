#include <GLFW/glfw3.h>
#include "vec2.h"
#include <pthread.h>

GLFWwindow *window;
const int size = 800;
float buffer[size][size];
int projection = 4; // 10 x 10

int divisionThreads = size / projection;

struct threadsLimits {
    int iMin, iMax, jMin, jMax;
};

int halfSize = size / 2.0f;
int maxInteractions = 50;
float zoom = 1.0f;
float deltaX = -1.5f;
float deltaY = -1.0f;
float canRead;

float moveIncrement = 0.1f;

pthread_mutex_t lock;
pthread_mutex_t bufferLock;

pthread_cond_t conditional;

pthread_barrier_t our_barrier;

vec2 complexSquared(vec2 *c) {
    return vec2(
            c->x() * c->x() - c->y() * c->y(),
            2.0 * c->x() * c->y()
    );
}

/*
 * Actually, the concept of mandelbrot distance probably doesn´t exist,
 * the set of mandelbrot is just a boolean data, the number is inside the mandelbrot set or is outside.
 * But, we insert this "distance" to see the borders of mandelbrot with a gradient.
 * With this, the complex numbers that are closest to the mandelbrot set, will appear with a light color
 */

int getMandelbrotDistance(vec2 complexNumber) {
    vec2 complexNumberCopy = complexNumber.copy();

    int n;

    for (n = 0; n < maxInteractions; n++) {
        complexNumberCopy = complexSquared(&complexNumberCopy) + complexNumber;

        // The mathematicians discovered that if a module of a number is bigger that 2, the number
        // is outside the mandelbrot set
        if (complexNumberCopy.length() > 2.0) {
            break;
        }
    }
    return n;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (!canRead) {
        return;
    }
    if ((key == GLFW_KEY_KP_8 && action == GLFW_PRESS) ||
        (key == GLFW_KEY_8 && action == GLFW_PRESS)||
        (key == GLFW_KEY_UP && action == GLFW_PRESS) ||
        (key == GLFW_KEY_W && action == GLFW_PRESS)) {
        deltaY += moveIncrement;
    } else if ((key == GLFW_KEY_KP_2 && action == GLFW_PRESS) ||
               (key == GLFW_KEY_2 && action == GLFW_PRESS)||
               (key == GLFW_KEY_DOWN && action == GLFW_PRESS) ||
               (key == GLFW_KEY_S && action == GLFW_PRESS)) {
        deltaY -= moveIncrement;
    } else if ((key == GLFW_KEY_KP_6 && action == GLFW_PRESS) ||
               (key == GLFW_KEY_6 && action == GLFW_PRESS)||
               (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) ||
               (key == GLFW_KEY_D && action == GLFW_PRESS)) {
        deltaX += moveIncrement;
    } else if ((key == GLFW_KEY_KP_4 && action == GLFW_PRESS) ||
               (key == GLFW_KEY_4 && action == GLFW_PRESS)||
               (key == GLFW_KEY_LEFT && action == GLFW_PRESS) ||
               (key == GLFW_KEY_A && action == GLFW_PRESS)) {
        deltaX -= moveIncrement;
    }

    pthread_mutex_lock(&lock);
    canRead = false;
    pthread_cond_broadcast(&conditional);
    pthread_mutex_unlock(&lock);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (!canRead) {
        return;
    }
    if (yoffset > 0) {
        zoom *= 1.1f;
        moveIncrement *= 1.1f;
        maxInteractions -= 10.0f;
    } else {
        zoom /= 1.1f;
        moveIncrement /= 1.1f;
        maxInteractions += 10.0f;
    }

    pthread_mutex_lock(&lock);
    canRead = false;
    pthread_cond_broadcast(&conditional);
    pthread_mutex_unlock(&lock);
}

void calculateBuffer(int iMin, int iMax, int jMin, int jMax, float **localBuffer) {

    for (int i = iMin; i < iMax; i++) {
        for (int j = jMin; j < jMax; j++) {
            double positionX = ((j * zoom / halfSize) + deltaX);
            double positionY = ((i * zoom / halfSize) + deltaY);

            vec2 complexNumber = vec2(
                    positionX,
                    positionY
            );

            int n = getMandelbrotDistance(complexNumber);
            float colorIntensity = ((n * 2.0f) + 40.0f) / 255.0f;

            if (colorIntensity > 1.0f) {
                colorIntensity = 1.0f;
            } else if (colorIntensity < 0.0f) {
                colorIntensity = 0.0f;
            }

            if (n == maxInteractions) {
                localBuffer[j - jMin][i - iMin] = 0.0f;
            } else {
                localBuffer[j - jMin][i - iMin] = colorIntensity;
            }
        }
    }
}

void *calculateWorker(void *voidArgs) {
    threadsLimits *args = (threadsLimits *) voidArgs;

    while (!glfwWindowShouldClose(window)) {
        float **localBuffer;
        localBuffer = (float **) calloc(args->jMax - args->jMin, sizeof(int *));

        for (int i = 0; i < args->jMax - args->jMin; i++) {
            localBuffer[i] = (float *) calloc(args->iMax - args->iMin, sizeof(int)); // allocate row array
        }

        calculateBuffer(args->iMin, args->iMax, args->jMin, args->jMax, localBuffer);

        pthread_mutex_lock(&bufferLock);
        for (int i = args->iMin; i < args->iMax; i++) {
            for (int j = args->jMin; j < args->jMax; j++) {
                buffer[j][i] = localBuffer[j - args->jMin][i - args->iMin];
            }
        }
        pthread_mutex_unlock(&bufferLock);

        pthread_barrier_wait(&our_barrier);

        pthread_mutex_lock(&lock);
        canRead = true;
        pthread_cond_wait(&conditional, &lock);
        pthread_mutex_unlock(&lock);

    }
}

void *mainThreadRoutine(void *window) {
    int projectionX = 0;
    int projectionY = 0;
    int count = 0;
    pthread_t mainThreadBufferId[projection][projection];
    threadsLimits threadsLimits[projection][projection];

    projectionX = 0;
    for (int i = 0; i < size; i += divisionThreads) {
        projectionY = 0;
        for (int j = 0; j < size; j += divisionThreads) {

            threadsLimits[projectionX][projectionY].iMin = i;
            threadsLimits[projectionX][projectionY].iMax = i + divisionThreads;
            threadsLimits[projectionX][projectionY].jMin = j;
            threadsLimits[projectionX][projectionY].jMax = j + divisionThreads;

            pthread_create(&mainThreadBufferId[projectionX][projectionY], NULL, &calculateWorker,
                           &threadsLimits[projectionX][projectionY]);
            count++;
            projectionY++;
        }
        projectionX++;
    }

    for (int i = 0; i < projection; i++) {
        for (int j = 0; j < projection; j++) {
            pthread_join(mainThreadBufferId[i][j], NULL);
        }
    }

    return nullptr;
}

int main(void) {


    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(size, size, "Processamento de alto desempenho", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glPointSize(1);

    glOrtho(-halfSize,
            halfSize,
            -halfSize,
            halfSize, 0.0f, 1.0f);

    pthread_t mainThreadId;
    pthread_create(&mainThreadId, NULL, &mainThreadRoutine, (void *) window);

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&bufferLock, NULL);
    pthread_barrier_init(&our_barrier, NULL, 16);
    pthread_cond_init(&conditional, NULL);

    while (!glfwWindowShouldClose(window)) {

        glBegin(GL_POINTS);

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                float greenIntensity = buffer[j][i];
                glColor3f(
                        0.0f, greenIntensity, 0.0f);
                glVertex3f(j - halfSize, i - halfSize, 0.0f);
            }
        }

        glEnd();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}


