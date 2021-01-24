/*
============================================================================
Name        : Assembler.c
Author      : Ofer Zadaka, 302632385
============================================================================
*/

#ifndef OUTPUT_WRITER_H
#define OUTPUT_WRITER_H

#include "checkData.h"
#include "output.h"
#include <stdlib.h>

#define FileEntry	 ".ent"
#define FileExtern	 ".ext"
#define FileOdject	 ".ob"


/* Creates a file for writing (mode="w").  The function gets the name of the file, its format (.ob, .ext or .ent) and returns a pointer to it. */
FILE *openFile(char *name, char *format, const char *mode);
/* Creates the .obj file for writing addresses of the assembled lines */
void OpenObjectFile(char *name, int IC, int DC, int *memory_block);
/* Creates the .ent file for writing addresses of the .entry labels */
void OpenEntryFile(char *name);
/* Creates the .ext file for writing addresses of the .extern labels*/
void OpenExternalFile(char *name, instructionLine *linesArr, int Number_line);
/* free all the malloc  and Clear all the globals data */
void clearData(instructionLine *linesArr, int Number_line, int dataCount);
/*counts the number of lines in the file*/
int findNumLines(FILE *file);
/*Extracts only the selected bits of the int array*/
int bitExtracted(int number, int k, int p);

#endif

#pragma once
