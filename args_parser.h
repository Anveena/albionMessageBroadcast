//
// Created by tyran on 2024/8/26.
//

#ifndef TRANSPORT_ARGS_PARSER_H
#define TRANSPORT_ARGS_PARSER_H

int parsePort(int argc, char *argv[]);

int parseProcessName(int argc, char *argv[], int *pNameIndex);

int parseAddresses(int argc, char *argv[], int *filterIndex);

#endif //TRANSPORT_ARGS_PARSER_H
