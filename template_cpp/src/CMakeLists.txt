# DO NAME THE SYMBOLIC VARIABLE `SOURCES`

include_directories(include)
set(SOURCES src/main.cpp src/hello.c src/io_handler.c src/lcb_app.c src/stubborn_links.c src/perfect_links.c src/best_effort_broadcast.c src/linked_list.c src/uniform_reliable_broadcast.c src/localized_causal_broadcast.c)
#set(CMAKE_BUILD_TYPE Debug)

# DO NOT EDIT THE FOLLOWING LINE
find_package(Threads)
add_executable(da_proc ${SOURCES})
target_link_libraries(da_proc ${CMAKE_THREAD_LIBS_INIT})
