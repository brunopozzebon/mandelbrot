# Mandelbrot

![img_1.png](img_1.png)

## Requisitos
Estar em um ambiente linux, de prefêrencia da fámilia Debian/Ubuntu.

## Dependencias
Rodar o seguinte código no seu terminal linux para instalar as bibliotecas necessárias, caso não estejam instaladas.
Esse comando é usado para distros "Debian", caso tiver uma distro baseado em outra familia, pesquisar o instalador de pacotes da sua distro
```shell
    sudo apt-get install build-essential libglu1-mesa-dev freeglut3-dev mesa-common-dev mesa-utils libglfw3-dev libglew-dev cmake
```

### Como executar o programa?

Opção 1) Utilizar uma IDE (Ex: CLion) para execução

Opção 2)

```terminal
cd cmake-build-debug
./madelbrot
```



### Analise das threads:

https://github.com/htop-dev/htop

![img.png](img.png)

### Qual a Melhor quantidade de threads? E Pq?

16:



