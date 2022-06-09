#include <GLFW/glfw3.h>
#include "vec2.h"
#include <pthread.h>

int width = 800;
int height = 800;
float buffer[800][800];
int projection = 4; // 10 x 10
int divisionThreads = width / projection;

struct thread_args{
    int iMin, iMax, jMin, jMax;
};

bool changeScreen = true;

int halfWidth = width / 2.0f;
int halfHeight = height / 2.0f;
int maxInteractions = 50;
float zoom = 1.0f;
float deltaX = -1.5f;
float deltaY = -1.0f;

vec2 complexSquared(vec2* c) {
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        zoom /= 1.1f;
        maxInteractions += 10.0f;
        changeScreen = true;
    } else if ((key == GLFW_KEY_M && action == GLFW_PRESS)) {
        zoom *= 1.1f;
        maxInteractions -= 10.0f;
        changeScreen = true;
    } else if ((key == GLFW_KEY_KP_8 && action == GLFW_PRESS) ||
            (key == GLFW_KEY_8 && action == GLFW_PRESS) ) {
        deltaY += 0.5f;
        changeScreen = true;
    } else if ((key == GLFW_KEY_KP_2 && action == GLFW_PRESS)||
               (key == GLFW_KEY_2 && action == GLFW_PRESS) ) {
        deltaY -= 0.5f;
        changeScreen = true;
    } else if ((key == GLFW_KEY_KP_6 && action == GLFW_PRESS)||
            (key == GLFW_KEY_6 && action == GLFW_PRESS) ) {
        deltaX += 0.5f;
        changeScreen = true;
    } else if ((key == GLFW_KEY_KP_4 && action == GLFW_PRESS)||
              (key == GLFW_KEY_4 && action == GLFW_PRESS) ) {
        deltaX -= 0.5f;
        changeScreen = true;
    }
}

void *calculateWorker(void *voidArgs) {
    thread_args *args = (thread_args*)voidArgs;

    for (int i = args->iMin; i < args->iMax; i++) {
        for (int j = args->jMin; j < args->jMax; j++) {
            double positionX = ((j * zoom / halfWidth) + deltaX);
            double positionY = ((i * zoom / halfHeight) + deltaY);

            vec2 complexNumber = vec2(
                    positionX,
                    positionY
            );

            int n = getMandelbrotDistance(complexNumber);
            float colorIntensity = ((n * 2.0f) + 40.0f) / 255.0f;

            if (colorIntensity > 1.0f) {
                colorIntensity = 1.0f;
            } else if (colorIntensity<0.0f) {
                colorIntensity = 0.0f;
            }

            if (n == maxInteractions) {
                buffer[j][i] = 0.0f;
            } else {
                buffer[j][i] = colorIntensity;
            }
        }
    }

    return nullptr;
}

void *calculateBuffer(void * window) {
    int projectionX = 0;
    int projectionY = 0;
    int count = 0;
    pthread_t mainThreadBufferId[projection][projection];
    thread_args threadArguments[projection][projection];

    printf("divisionThreads %d\n", divisionThreads);

    while(!glfwWindowShouldClose((GLFWwindow *)window)) {
        if (changeScreen == true) {
            projectionX = 0;
            for (int i = 0; i < width; i += divisionThreads) {
                //printf("i %d\n", i);

                projectionY = 0;

                for (int j = 0; j < height; j += divisionThreads) {
                    //printf("j %d\n", j);

                    threadArguments[projectionX][projectionY].iMin = i;
                    threadArguments[projectionX][projectionY].iMax = i + divisionThreads;
                    threadArguments[projectionX][projectionY].jMin = j;
                    threadArguments[projectionX][projectionY].jMax = j + divisionThreads;

                    pthread_create(&mainThreadBufferId[projectionX][projectionY], NULL, &calculateWorker, &threadArguments[projectionX][projectionY]);
                    printf("Criou thread número %d\n", count);
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

            changeScreen = false;
        }
    }

    return nullptr;
}

int main(void){
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(width, height, "Processamento de alto desempenho", NULL, NULL);

    if (!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glPointSize(1);

    glOrtho(-halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight, 0.0f, 1.0f);

    pthread_t mainThreadId;
    pthread_create(&mainThreadId, NULL, &calculateBuffer, (void *)window);

    while (!glfwWindowShouldClose(window)){

        glBegin(GL_POINTS);
        for(int i = 0; i < 800; i++){
            for(int j = 0; j < 800; j++){
                float greenIntensity = buffer[j][i];
                 glColor3f(
                           0.0f,greenIntensity,  0.0f);
                  glVertex3f(j- halfWidth, i- halfHeight, 0.0f);
            }
        }

        glEnd();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}


