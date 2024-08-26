cmake_minimum_required(VERSION 3.29)
project(transport C)

set(CMAKE_C_STANDARD 17)

find_package(Threads REQUIRED)
set(PCAP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/pcap/Include")
set(PCAP_LIB_DIR "${CMAKE_SOURCE_DIR}/pcap/Lib/x64")

include_directories(${PCAP_INCLUDE_DIR})
link_directories(${PCAP_LIB_DIR})

add_executable(transport main.c
        message.c
        message.h
        pcap_helper.c
        pcap_helper.h
        broadcasting_room.c
        broadcasting_room.h
        client.c
        client.h
        buffer.c
        buffer.h
        flitter_parser.c
        flitter_parser.h
        args_parser.c
        args_parser.h
        macros.h
)
target_link_libraries(transport wpcap Packet iphlpapi ws2_32 ${CMAKE_THREAD_LIBS_INIT})
add_executable(filterGetter
        args_parser.c
        args_parser.h
        filter_getter.c)
target_link_libraries(filterGetter iphlpapi ws2_32)
add_executable(filterGetterForRemoteAddr
        args_parser.c
        args_parser.h
        filter_getter_for_remote_addr.c
)
