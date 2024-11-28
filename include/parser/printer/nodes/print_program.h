#ifndef PRINT_PROGRAM_H
#define PRINT_PROGRAM_H

#include "../../../ast/definitions.h"
#include "../visited.h"

void print_program(const AstNode *node, int depth, VisitedNodes *visited);

#endif // PRINT_PROGRAM_H