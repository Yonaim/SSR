g++ -std=c++11 ./srcs/*.c ./srcs/*.cpp \
-I./include -I./glfw-3.3.8/include \
-L./glfw-3.3.8 -lglfw3 \
-framework OpenGL -framework Appkit -framework IOKit \
-o ssr
./ssr