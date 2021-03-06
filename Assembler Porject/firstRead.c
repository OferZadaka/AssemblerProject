/*
============================================================================
Name        : Assembler.c
Author      : Ofer Zadaka, 302632385
============================================================================
*/

#include "parsing.h"

extern SymbolTables array_labels[MAX_COUNT_LINES];
extern int count_label;
instructionLine *array_entry[MAX_COUNT_LINES];
extern int count_entryLabels;
extern int array_data[MEMORY_SIZE];
extern commandType cmdTab[];

/*First reading the file and parsing it. Returns count of errors that were found */
int firstReadFile(FILE *file, instructionLine *linesArr, int Number_line, int *IC, int *DC)
{
	char lineStringNext[MAX_LINE_LENGTH];
	int errorsFound = 0;
	Number_line = 0;
	count_entryLabels = 0;
	while (!feof(file))
	{
		if (fgets(lineStringNext, MAX_LINE_LENGTH, file))
		{
			/* checking the count of line */
			if (Number_line >= MAX_COUNT_LABEL)
			{
				printf("ERROR. Max lines number is %d in file.\n", MAX_COUNT_LABEL);
				return ++errorsFound;
			}

			/* Parsing a line */
			parsingLine(&linesArr[Number_line], lineStringNext, Number_line +1, IC, DC);

			if (linesArr[Number_line].isError) /*if found error in the line*/
			{
				errorsFound++;
			}

			/* Check size overflow */
			if (*IC + *DC > MEMORY_SIZE)
			{
				/* Memory overflow (array_data). Stop reading the file. */
				printf("ERROR. At line %d: Memory overflow. Stoping to read the file.\n", Number_line + 1);
				return ++errorsFound;
			}
			Number_line = Number_line + 1;
		}
		else if (!feof(file))
		{
			/* Line is too long */
			printf("ERROR. At line %d:\tLine is long. Max line length is %d.\n", Number_line + 1, MAX_LINE_LENGTH);
			errorsFound++;
			++Number_line;
		}
	}

	return errorsFound;
}

/* Find the label and adds it to the label list. */
/* Returns a pointer to the next char after the label, or NULL is there isn't a legal label. */
char *findLabel(instructionLine *line, int IC)
{
	char *labelEnd = strchr(line->lineStringNext, ':');
	SymbolTables label = { "" };
	label.address = BEGIN_OFFSET + IC;

	if (!labelEnd)
	{
		return NULL; /*there isn't a legal label*/
	}
	*labelEnd = '\0';

	/* Check if the ':'*/
	while (*labelEnd)
	{
		if (!isspace(*labelEnd++)) /*if str contains only white spaces*/
		{
			*labelEnd = ':'; /* Fix the change in line->lineStringNext */
			return NULL;
		}
	}

	line->label = addLabelToArr(label, line);

	return labelEnd + 1; /* +1 to make it point at the next char after the \0 */

}

/* Adds the label to the array_labels. Returns a pointer to the label in the array if no found errors */
SymbolTables *addLabelToArr(SymbolTables label, instructionLine *line)
{
/* Check if label is legal */
	if (!(checkLegalLabel(line->lineStringNext, line->Number_Line, TRUE)))
	{
		line->isError = TRUE;
		return NULL;
	}

	/* Check if if the label exists */
	if (getLabel(line->lineStringNext))
	{
		printf("ERROR. At line %d:\tLabel previously declared.\n", line->Number_Line);
		line->isError = TRUE;
		return NULL;
	}

	/* Add the name to the label */
	strcpy(label.name, line->lineStringNext);
	/* Add the label to array_labels and to the instructionLine */
	if (count_label < MAX_COUNT_LINES)
	{
		array_labels[count_label] = label;
		return &array_labels[count_label++];
	}
	/* the names are more than allowed */
	printf("ERROR. At line %d:\tToo many labels - max is %d.\n", line->Number_Line, MAX_COUNT_LINES);
	line->isError = TRUE;
	return NULL;
}


/* Parses a .data directive. */
void DircDataParsing(instructionLine *line, int *IC, int *DC)
{
	char *operand_str = line->lineStringNext, *end = line->lineStringNext;
	int operandValue;
	bool foundComma;

	/* Make the label a data label (is there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = BEGIN_OFFSET + *DC;
	}

	/* Check if there are params */
	if (isWhiteSpaces(line->lineStringNext))
	{
		/* No parameters */
		printf("ERROR. At line %d:\tMissing argument in directive.\n", line->Number_Line);
		line->isError = TRUE;
		return;
	}

	/* Find all the params and add them to array_data */
	for (;;)
	{
		/* Get next param or break if there isn't */
		if (isWhiteSpaces(line->lineStringNext))
		{
			break;
		}
		operand_str = foundFirstOp(line->lineStringNext, &end, &foundComma);

		/* Add the param to array_data */
		if (CheckLegalNumber(operand_str, line->Number_Line, &operandValue, MAXDATANUM, MINDATANUM))
		{

				/* Check if there is enough space in array_data for the data */
				if (*DC + *IC < MEMORY_SIZE)
				{
					array_data[(*DC)++] = operandValue;
				}
				else
				{
					/* Not enough memory */
					line->isError = TRUE;
					return;
				}
		}
		else
		{
			/* Illegal number */
			line->isError = TRUE;
			return;
		}

		/* Change the line to start after the parameter */
		line->lineStringNext = end;
	}

	if (foundComma)
	{
		printf("ERROR. At line %d:\tDo not write a comma after the last parameter.\n", line->Number_Line);
		line->isError = TRUE;
		return;
	}
}

/* Parses a .string directive. */
void DircStringParsing(instructionLine *line, int *IC, int *DC)
{
	/* Make the label a data label (is there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = BEGIN_OFFSET + *DC;
	}

	SKIPNONSPACE(&line->lineStringNext);

	if (CheckLegalString(&line->lineStringNext, line->Number_Line))
	{
		if (!addStringToData(line->lineStringNext, IC, DC, line->Number_Line))
		{
			/* Not enough memory */
			line->isError = TRUE;
			return;
		}
	}
	else
	{
		/* Illegal string */
		line->isError = TRUE;
		return;
	}
}

/* Parsing  .entry/ .extern directives. */
void DircEntryExternParsing(instructionLine *line)
{
	SymbolTables label = { "0" }, *labelPointer;
	/* If there is a label in the line, remove it from labelArr */
	if (line->label)
	{
		count_label--;
		printf("[Warning] At line %d: The assembler ignored the label before the directive.\n", line->Number_Line);
	}
	SKIPNONSPACE(&line->lineStringNext);
	if(!strcmp(line->cmd_str, "entry"))/* Add the label to the entry labels list */
	{
		if (checkLegalLabel(line->lineStringNext, line->Number_Line, TRUE))
		{
			if (count_entryLabels < MAX_COUNT_LINES)
			{
				array_entry[count_entryLabels++] = line;
			}
		}
	}
	else
	{
		labelPointer = addLabelToArr(label, line);
		/* Make the label an extern label */
		if (!line->isError)
		{
			labelPointer->address = 0;
			labelPointer->isExtern = TRUE;
		}
	}
}


/* Updates the type and value of operand. */
void parsingOpInfo(operandInfo *operand, int Number_Line)
{
	int value = 0;

	if (isWhiteSpaces(operand->str))
	{
		printf("ERROR. At line %d:\tInvalid characters.\n", Number_Line);
		operand->type = ERROR;
		return;
	}
	/* Check if the type is NUMBER */
	if (*operand->str == '#')
	{
		operand->str++; /* Remove the '#' */

		/* Check if the number is legal. Don't use a white space after the '#' */
		if (isspace(*operand->str))
		{
			printf("ERROR. At line %d:\tDon't use a white space after the '#'.\n", Number_Line);
			operand->type = ERROR;
		}
		else
		{
			operand->type = CheckLegalNumber(operand->str, Number_Line, &value, DIRECTMAX, DIRECTMIN) ? NUMBER : ERROR;

		}
	}
	/* Check if the type is  hashREGISTER */
	else if (checkHashRegister(operand->str, &value)) {
		operand->type = HASH;
	}
	/* Check if the type is REGISTER */
	else if (checkRegister(operand->str, &value))
	{
		operand->type = REGISTER;
	}
	/* Check if the type is LABEL */
	else if (checkLegalLabel(operand->str, Number_Line, FALSE))
	{
		operand->type = LABEL;
	}

	/* The type is ERROR */
	else
	{
		printf("ERROR. At line %d:\tInvalid characters \"%s\".\n", Number_Line, operand->str);
		operand->type = ERROR;
		value = -1;
	}

	operand->value = value;

}

/* Parsing the operands*/
void parsingOpCmd(instructionLine *line, int *IC, int *DC)
{
	char *startNextWord = line->lineStringNext;
	bool foundComma = FALSE;
	int numOfOpsFound = 0;
	line->op1.type = ERROR;
	line->op2.type = ERROR;


	for (;;)
	{
		/* If two operands are registers,  they should occupy in the command structure, only 6 bits (registersBits) */
		if (!((line->op1.type == REGISTER) && (line->op2.type == REGISTER)) || ((line->op1.type == REGISTER) && (line->op2.type == HASH)) || ((line->op1.type == HASH) && (line->op2.type == REGISTER)) || ((line->op1.type == HASH) && (line->op2.type == HASH)))
		{
			(*IC + *DC < MEMORY_SIZE) ?(*IC)++: (line->isError = TRUE);
			if (line->isError == TRUE)
				return;
		}
		/*check that the number of operands is not more than two */
		if (isWhiteSpaces(line->lineStringNext) || numOfOpsFound > 2)
		{
			break;
		}

		/* If there is one operand */
		if (numOfOpsFound == 1)
		{

			line->op1 = line->op2;
			line->op2.type = ERROR;
		}
		/* Parse the opernad*/
		line->op2.str = foundFirstOp(line->lineStringNext, &startNextWord, &foundComma);
		parsingOpInfo(&line->op2, line->Number_Line);

		if (line->op2.type == ERROR)
		{
			line->isError = TRUE;
			return;
		}

		numOfOpsFound++;
		line->lineStringNext = startNextWord;
	}
	  /* Check if there are enough operands */
	if (numOfOpsFound != line->cmd->operandsInputNum)
	{
		/* Missing operand(more/less operands than needed) */
		if (numOfOpsFound <  line->cmd->operandsInputNum)
		{
			printf("ERROR. At line %d:\tMissing operand.\n", line->Number_Line);
		}
		else
		{
			printf("ERROR. At line %d:\tInvalid characters. Too many operands.\n", line->Number_Line);
		}

		line->isError = TRUE;
		return;
	}

	/* Check if there is a comma after the last param */
	if (foundComma)
	{
		printf("ERROR. At line %d:\tInvalid character. Extra comma after the last parameter.\n", line->Number_Line);
		line->isError = TRUE;
		return;
	}
	/* Check if the operands' types are legal */

	if (line->cmd->opcode == 4 && line->op1.value != FALSE)/* command "lea" can only get a label as the first operand */
	{
		printf("ERROR. At line %d:\tInvalid source operand. \"%s\" command must be a label.\n", line->Number_Line, line->cmd->cmd_str);
		line->isError = TRUE;
		return;
	}

	/* check if second operand a number only if the command is "cmp" or "prn"*/
	if (line->op2.type == NUMBER && line->cmd->opcode != 1 && line->cmd->opcode != 12)
	{
		printf("ERROR. At line %d:\tInvalid operand. Destination operand for \"%s\" command can't be a number.\n", line->Number_Line, line->cmd->cmd_str);
		line->isError = TRUE;
		return;
	}
}

/* Adds the string to the array_data. Returns FALSE if there isn't enough space in array_data */
bool addStringToData(char *str, int *IC, int *DC, int Number_Line)
{
	do
	{
		/* Check if there is enough space in array_data for the data */
		(*DC + *IC < MEMORY_SIZE) ? (array_data[(*DC)++] = ((int)*str)) : FALSE;
	} while (*str++);

	return TRUE;
}

/* Parsing a line and if an error is found reporting it and returning isERROR=True in instructionLine*/
void parsingLine(instructionLine *line, char *lineStringNext, int Number_Line, int *IC, int *DC)
{
	char *startNextWord = lineStringNext;
	char *newString = (char *)malloc(strlen(lineStringNext) + 1); /*Returns the same string in a different part of the memory*/
	line->Number_Line = Number_Line;
	line->address = BEGIN_OFFSET + *IC;
	line->line_string = (newString) ? (strcpy(newString, lineStringNext)) : newString;
	line->lineStringNext = line->line_string;
	line->isError = FALSE;
	line->label = NULL;
	line->cmd_str = NULL;
	line->cmd = NULL;

	if (!line->line_string)
	{
		printf("ERROR. Cannot allocate memory.");
		return;
	}

	/* Check if the line is a comment */
	if (*line->lineStringNext == ';')
	{
		return;
	}
	SKIPSPACE(&startNextWord);
	/* Check if the line is a Empty*/
	if (*startNextWord == '\0')
	{
		return ;
	}
	if (*startNextWord == ';')
	{
		/* Illegal comment - ';'*/
		printf("ERROR. At line %d:\tComments must start with ';' at the start of the line.\n", line->Number_Line);
		line->isError = TRUE;
		return;
	}
	/* Find the label in line */
	startNextWord = findLabel(line, *IC); /* NULL if  isn't a legal label. */
	if (line->isError)
	{
		return;
	}

	/* go to the next Word if it isn't NULL */
	if (startNextWord)
	{
		line->lineStringNext = startNextWord;
	}

	/* Find the command string */
	line->cmd_str = Command_string(line->lineStringNext, &startNextWord);
	line->lineStringNext = startNextWord;

	/* if the cmd_str is a directive, parsing it */
	if (checkDirective(line->cmd_str))
	{
		line->cmd_str++; /* Remove the '.' from the command */
		if (!strcmp(line->cmd_str, "data")){
			DircDataParsing(line, IC, DC);}
		else if (!strcmp(line->cmd_str, "string")){
			DircStringParsing(line, IC, DC);
		}
		else if ((!strcmp(line->cmd_str, "extern")) || (!strcmp(line->cmd_str, "entry"))){
			DircEntryExternParsing(line);
}
		else
		{
			printf("ERROR. At line %d:\tUndefined directive as \"%s\".\n", line->Number_Line, line->cmd_str);
			line->isError = TRUE;
		}
	}
	else
	{
		/* Parsing the command in a command line. */
		int cmdId = GetCommandByName(line->cmd_str);
		if (cmdId == -1)
		{
			line->cmd = NULL;
			if (*line->cmd_str == '\0')
			{
				/* enough parameters */
				printf("ERROR. At line %d:\tCan't write a label to an empty line.\n", line->Number_Line);
			}
			else
			{
				/* Illegal command. */
				printf("ERROR. At line %d:\tUndefined instruction as \"%s\".\n", line->Number_Line, line->cmd_str);
			}
			line->isError = TRUE;
			return;
		}
		line->cmd = &cmdTab[cmdId];
		parsingOpCmd(line, IC, DC); /*parsing operands*/
	}
	if (line->isError)
	{
		return;
	}
}
