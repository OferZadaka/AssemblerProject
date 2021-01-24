/*
============================================================================
Name        : Assembler.c
Author      : Ofer Zadaka, 302632385
============================================================================
*/
#include "output.h"

extern SymbolTables array_labels[];
extern int count_label;
instructionLine *array_entry[MAX_COUNT_LABEL];
extern int count_entryLabels;
extern int array_data[];


/* Creates a file for writing (mode="w") or reading.  The function gets the name of the file, its format (.ob, .ext or .ent) and returns a pointer to it. */
FILE *openFile(char *name, char *format, const char *mode)
{
	FILE *file;
	char *mallocStr = (char *)malloc(strlen(name) + strlen(format) + 1), *fileName = mallocStr;
	sprintf(fileName, "%s%s", name, format);

	file = fopen(fileName, mode);
	free(mallocStr);

	return file;
}

/* Creates the .obj file for writing addresses of the assembled lines */
void OpenObjectFile(char *name, int IC, int DC, int *memoryArr)
{
	int i;
	FILE *file;
	int ICcount, DCcount;
	file = openFile(name, FileOdject, "w");

	/*counts how many IC there is*/
	for (i = 0; i < IC ;i++) {
		ICcount = i;
	}
	/*counts how many DC there is*/
	for (i = 0; i < DC ;i++) {
		DCcount = i;
	}
	/* Print IC and DC */
	fprintf(file,"%d", ICcount);
	fprintf(file, "\t\t");
	fprintf(file,"%d", DCcount);

	/* Print all of memoryArr */
	for (i = 0; i < IC + DC; i++)
	{
		fprintf(file, "\n");
		fprintf(file, "0%d\t\t", BEGIN_OFFSET + i);
		fprintf(file, "%05o\t\t", bitExtracted(memoryArr[i], 16, 1));
	}

	fclose(file);
}

/* Creates the .ent file for writing addresses of the .entry labels */
void OpenEntryFile(char *name)
{
	int i;
	FILE *file;

	/* Don't create the entries file if count_entryLabels = 0 */
	if (count_entryLabels == 0)
	{
		return;
	}

	file = openFile(name, FileEntry, "w");

	for (i = 0; i < count_entryLabels; i++)
	{
		fprintf(file, "%s\t\t", array_entry[i]->lineStringNext);
		fprintf(file, "%d", getLabel(array_entry[i]->lineStringNext)->address);

		if (i != count_entryLabels - 1)
		{
			fprintf(file, "\n");
		}
	}

	fclose(file);
}

/* Creates the .ext file for writing addresses of the .extern labels*/
void OpenExternalFile(char *name, instructionLine *linesArr, int lines)
{
	int i;
	SymbolTables *label;
	bool firstPrint = TRUE; /* This bool meant to prevent the creation of the file if there aren't any externs */
	FILE *file = NULL;

	for (i = 0; i < lines; i++)
	{
		/* Check if the 1st operand is extern label, and prints it. */
		if (linesArr[i].cmd && linesArr[i].cmd->operandsInputNum >= 2 && linesArr[i].op1.type == LABEL)
		{
			label = getLabel(linesArr[i].op1.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least one extern */
					file = openFile(name, FileExtern, "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintf(file, "0%d", linesArr[i].op1.address);
				firstPrint = FALSE;
			}
		}

		/* Check if the 2nd operand is extern label, and prints it. */
		if (linesArr[i].cmd && linesArr[i].cmd->operandsInputNum >= 1 && linesArr[i].op2.type == LABEL)
		{
			label = getLabel(linesArr[i].op2.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, FileExtern, "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintf(file, "0%d", linesArr[i].op2.address);
				firstPrint = FALSE;
			}
		}
	}

	if (file)
	{
		fclose(file);
	}
}

/* free all the malloc  and Clear all the globals data */
void clearData(instructionLine *linesArr, int Number_line, int dataCount)
{
	int i;
	/* Clean global labels */
	for (i = 0; i < count_label; i++)
	{
		array_labels[i].address = 0;
		array_labels[i].isData = 0;
		array_labels[i].isExtern = 0;
	}
	count_label = 0;

	/* Clean global .entry */
	for (i = 0; i < count_entryLabels; i++)
	{
		array_entry[i] = NULL;
	}
	count_entryLabels = 0;

	/* Clean global data */
	for (i = 0; i < dataCount; i++)
	{
		array_data[i] = 0;
	}

	/* Free malloc*/
	for (i = 0; i < Number_line; i++)
	{
		free(linesArr[i].line_string);
	}
}
  /*finding the number of lines in the file*/
	int findNumLines(FILE *file)
	{
		int numLine = 0;
		char ch;
		/*Checks if the file was not opened*/
		if (file == NULL)
    {
        return 0;
    }
		/*going through the file*/
		for(ch = getc(file); ch != EOF; ch = getc(file)){
			if (ch == '\n')

				numLine = numLine + 1;
		}
		rewind(file);

	 return numLine;
	}
	/*Extracts only the selected bits of the int array*/
	int bitExtracted(int number, int k, int p)
  {
    return (((1 << k) - 1) & (number >> (p - 1)));
  }
