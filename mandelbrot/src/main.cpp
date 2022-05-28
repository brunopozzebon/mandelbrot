#include <GLFW/glfw3.h>
#include "vec2.h"
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <vector>


float width = 800.0f;
float height = 800.0f;
std::vector<pthread_t> thid(20);

float halfWidth = width / 2.0f;
float halfHeight = height/2.0f;
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

void drawImage() {
    glBegin(GL_POINTS);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {

            double positionX = ((j * zoom / halfWidth) + deltaX);
            double positionY = ((i *zoom / halfHeight) + deltaY);
           
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
               glColor3f(0.0f, 0.0f, 0.0f);
                glVertex3f(j- halfWidth, i- halfHeight, 0.0f);
            }else {
                glColor3f(0.0f, colorIntensity, 0.0f);
                glVertex3f(j-halfWidth, i-halfHeight, 0.0f);
            }
          
        }
    }

    glEnd();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS) {
        zoom /= 1.1f;
        maxInteractions += 10.0f;

    } else if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS) {
        zoom *= 1.1f;
        maxInteractions -= 10.0f;
    } else if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS) {
        deltaY += 0.5f;
    } else if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS) {
        deltaY -= 0.5f;
    } else if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS) {
        deltaX += 0.5f;
    }else if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS) {
        deltaX -= 0.5f;
    }
}

void * teste_thread(void *str) {
    std::cout << "Hello World Thread!";
    return 0;
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
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    glPointSize(1);
   
   
   // glMatrixMode(GL_PROJECTION);
 //   glLoadIdentity();

    glOrtho(-halfWidth,
        halfWidth,
        -halfHeight,
        halfHeight, 0.0f, 1.0f);

    for (int threadNumber = 0; threadNumber < sizeof(thid); threadNumber++) {
       
        pthread_create(&thid[threadNumber], NULL, teste_thread, NULL);
    }
   
    while (!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);

       

        drawImage();


   
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}