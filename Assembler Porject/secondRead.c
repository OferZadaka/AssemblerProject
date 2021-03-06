/*
============================================================================
Name        : Assembler.c
Author      : Ofer Zadaka, 302632385
============================================================================
*/
#include "checkData.h"
#include "data.h"

extern SymbolTables array_labels[];
extern int count_label;
instructionLine *array_entry[MAX_COUNT_LABEL];
extern int count_entryLabels;
extern int array_data[];

/*performs the second scan on the assembly source files and It converts all the lines into the memory_block. */
int secondFileRead(int *memoryArr, instructionLine *linesArr, int Number_Line, int IC, int DC)
{
	int errorsFound = 0, memoryCounter = 0, i;
	int error_entry = 0; /* using to check illegal entries */
	SymbolTables *label;
	unsigned int mask = ~0; /* use to create an int of "MEMORY_DATA"*/
	mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_DATA);


	/* Updates the addresses in array_labels */
	for (i = 0; i < count_label; i++)
	{
		if (array_labels[i].isData) /*label with isData flag*/
			array_labels[i].address += IC;
	}
	/* Check if there are illegal entries */
	for (i = 0; i < count_entryLabels; i++)
	{
		label = getLabel(array_entry[i]->lineStringNext);
		if (label)
		{
			if (label->isExtern)
			{
				printf("ERROR. At line %d:\tThe parameter for .entry can't be an external label.\n", array_entry[i]->Number_Line);
				error_entry++;
			}
		}
		else
		{
			printf("ERROR. At line %d:\tNo such label as \"%s\".\n", array_entry[i]->Number_Line, array_entry[i]->lineStringNext);
			error_entry++;
		}
	}

	errorsFound += error_entry; /*if there is an illegal entry line in array_entry than error_entry will be greater than zero*/

	/* Add  all lines to the memoryArr */
	for (i = 0; i < IC+DC; i++)
	{
		if (!addLineToMemory(memoryArr, &memoryCounter, &linesArr[i], Number_Line))
		{
			errorsFound++;/*if found error while adding the line to the memory */
		}
	}
	/* Add each int from array_data to the end of memoryArr */
	for (i = 0; i < DC; i++)
	{
		if (memoryCounter < MEMORY_SIZE){
			/*only use the first "MEMORY_DATA" bits */
			memoryArr[(memoryCounter)++] = mask & array_data[i];
		}
	}
	return errorsFound;
}
/* Adds the value of memory block to the memoryArr */
void addWordToMemory(int *memoryArr, int *memoryCounter, memory_block memory)
{
	if (*memoryCounter < MEMORY_SIZE)
	{	/* Create an int of "MEMORY_DATA" times '1', and all the rest are '0' */
		unsigned int mask = ~0;
		mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_DATA);
		/* Add in memory block and increment memoryCounter */
		memoryArr[(*memoryCounter)++] = (mask & ((memory.valueBits.value << 3) + memory.are));/* use the first "MEMORY_DATA" bits */
	}
}


/*Returns an error if the  label values were not declared*/
/*if no error this method is updating the value of the it to be the address of the label.  */
bool addaddressOperand(operandInfo *op, int Number_Line)
{
	if ((op->type == LABEL) || (op->type == HASH))
	{
		SymbolTables *label = getLabel(op->str);
		/* Check if op.str is illegal or not exists*/
		if (op->str[0] != '*') { /*checks if it is not a hash */
		if (label == NULL)
		{
				if (checkLegalLabel(op->str, Number_Line, TRUE))
				{
					printf("ERROR. At line %d:\tNo such label as \"%s\".\n", Number_Line, op->str);
				}
				return FALSE;
		}
		op->value = label->address;
	}
	}

	return TRUE;
}

/* Returns a memory instruction structure which represents the operand in a line.*/
memory_block getOpMemory_block(operandInfo op, bool isDest)
{
	memory_block memory = { 0 };

	/* Check if type of operand is a register or not */
	if (op.type == REGISTER)
	{
		memory.are = (areType)ABSOLUTE;

		if (isDest) /*if Destination*/
		{
			memory.valueBits.registersBits.destBits = op.value;
		}
		else /*source operand*/
		{
			memory.valueBits.registersBits.srcBits = op.value;
		}
	}
	/* Check if type of operand is a HASH*/
	 else if (op.type == HASH)
	 {
 		memory.are = (areType)ABSOLUTE;

 		if (isDest) /*if Destination*/
 		{
 			memory.valueBits.registersBits.destBits = op.value;
 		}
 		else /*source operand*/
 		{
 			memory.valueBits.registersBits.srcBits = op.value;
 		}
 	}
	else
	{
		SymbolTables *label = getLabel(op.str);

		/* Check if isExtern*/
		if ((op.type == LABEL && label && label->isExtern))
		{
			memory.are = EXTERNAL;
			memory.valueBits.value = op.value;
		}
		else

			memory.are = (op.type == NUMBER) ? (areType)ABSOLUTE : (areType)RELOCATABLE;


		memory.valueBits.value = op.value;
	}
	return memory;
}
/* Returns a memory instruction structure which represents the command in a line. */
memory_block getCommandMemory_block(instructionLine line)
{
	memory_block memory = { 0 };

	/* Update all the bits in commandInfBits */
	memory.are = (areType)ABSOLUTE;
	memory.valueBits.commandInfBits.dest = ((line.op2.type != ERROR) ? ((int)line.op2.type) : 0);
	memory.valueBits.commandInfBits.src = ((line.op1.type != ERROR) ? ((int)line.op1.type) : 0);
	memory.valueBits.commandInfBits.opcode = line.cmd->opcode;
	return memory;
}

/* Adds line into the memoryArr, and increment the memoryCounter. */
bool addLineToMemory(int *memoryArr, int *memoryCounter, instructionLine *line, int Number_Line)
{
	bool foundError = FALSE;
	if (!line->isError && line->cmd != NULL)
	{
		/* Update the label operands value */
		if (!addaddressOperand(&line->op1, line->Number_Line) || !addaddressOperand(&line->op2, line->Number_Line))
		{
			line->isError = TRUE;
			foundError = TRUE;
		}

		/* Add the command word to the memory */
		addWordToMemory(memoryArr, memoryCounter, getCommandMemory_block(*line));
		/*Check if there are two operands in this line */
		if ((line->op1.type != ERROR) && (line->op2.type != ERROR))
		{
			/*if (line->op1.type == HASH)
			{
				line->op1.address = BEGIN_OFFSET + *memoryCounter;
				addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op1, TRUE));

				if (((line->op1.value_field == 1) || (line->op1.value_field == 2)) && (line->op1.type != ERROR)) {
					memory_block memory = { 0 };
					line->op1.address_field = BEGIN_OFFSET + *memoryCounter;
					memory.are = (areType)ABSOLUTE;
					memory.valueBits.structFieldBits.field = line->op1.value_field;
					addWordToMemory(memoryArr, memoryCounter, memory);
				}

				if (line->op2.type != HASH)
				{
					line->op2.address = BEGIN_OFFSET + *memoryCounter;
					addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op2, TRUE));
				}
				else
				{
					line->op2.address = BEGIN_OFFSET + *memoryCounter;
					addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op2, TRUE));
					if (((line->op1.value_field == 1) || (line->op1.value_field == 2)) && (line->op1.type != ERROR)) {
						memory_block memory = { 0 };
						line->op2.address_field = BEGIN_OFFSET + *memoryCounter;
						memory.are = (areType)ABSOLUTE;
						memory.valueBits.structFieldBits.field = line->op2.value_field;

						addWordToMemory(memoryArr, memoryCounter, memory);
					}
				}
			}*/
			 if (((line->op1.type == REGISTER) && (line->op2.type == REGISTER)) || ((line->op1.type == REGISTER) && (line->op2.type == HASH)) || ((line->op1.type == HASH) && (line->op2.type == REGISTER)))
			{
				memory_block memory = { 0 };
				memory.are = (areType)ABSOLUTE;
				memory.valueBits.registersBits.destBits = line->op2.value;
				memory.valueBits.registersBits.srcBits = line->op1.value;
				addWordToMemory(memoryArr, memoryCounter, memory);
			}
			else
			{
				line->op1.address = BEGIN_OFFSET + *memoryCounter;
				addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op1, FALSE));
				if (line->op2.type != HASH)
				{
					line->op2.address = BEGIN_OFFSET + *memoryCounter;
					addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op2, TRUE));
				}
				else
				{
					line->op2.address = BEGIN_OFFSET + *memoryCounter;
					addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op2, TRUE));
					if (((line->op1.value_field == 1) || (line->op1.value_field == 2)) && (line->op1.type != ERROR)) {
						memory_block memory = { 0 };
						line->op2.address_field = BEGIN_OFFSET + *memoryCounter;
						memory.are = (areType)ABSOLUTE;
						memory.valueBits.structFieldBits.field = line->op2.value_field;

						addWordToMemory(memoryArr, memoryCounter, memory);
					}
				}
			}
		}
		else
		{
			/*Check if there is a source operand in this line */
			if (line->op1.type != ERROR)
			{
				/* Add the op1 word to the memory */
				line->op1.address = BEGIN_OFFSET + *memoryCounter;
				addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op1, FALSE));
			}

			/*Check if there is a destination operand in this line */
			if (line->op2.type != ERROR)
			{
				if (line->op2.type == HASH)
				{
					line->op2.address = BEGIN_OFFSET + *memoryCounter;
					addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op2, TRUE));
					if (((line->op1.value_field == 1) || (line->op1.value_field == 2)) && (line->op1.type != ERROR)) {
						memory_block memory = { 0 };
						line->op2.address_field = BEGIN_OFFSET + *memoryCounter;
						memory.are = (areType)ABSOLUTE;
						memory.valueBits.structFieldBits.field = line->op2.value_field;
						addWordToMemory(memoryArr, memoryCounter, memory);
					}
				}
				else
				{
					/* Add the op2 word to the memory */
					line->op2.address = BEGIN_OFFSET + *memoryCounter;
					addWordToMemory(memoryArr, memoryCounter, getOpMemory_block(line->op2, TRUE));
				}

			}
		}

	}

	return !foundError;
}
