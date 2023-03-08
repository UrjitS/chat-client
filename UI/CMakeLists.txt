cmake_minimum_required(VERSION 3.22)
project(chat_client C)

set(CMAKE_C_STANDARD 11)

add_executable(chat_client
        chat_ui.c)
target_link_libraries(chat_client ncurses)
