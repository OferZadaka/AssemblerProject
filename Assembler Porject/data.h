/*
============================================================================
Name        : Assembler.c
Author      : Ofer Zadaka, 302632385
============================================================================
*/

#ifndef MEMORY_BLOCK
#define MEMORY_BLOCK

/*Bits*/
#define THREE_BITS		 3
#define FOUR_BITS		 4
#define TWELVE_BITS	 12


/* Machine instruction structure */

typedef struct /* 15 bits */
{
	unsigned int are : THREE_BITS; /* ABSOLUTE = 00, RELOCATABLE = 01, EXTERNAL = 02*/

	union /* 12 bits */
	{
		/* Commands */
		struct
		{
			int dest : FOUR_BITS;	/* Address Destination */
			int src : FOUR_BITS;		/* Addressing a source oparend */
			int opcode : FOUR_BITS;	 /*commandInfo ID */
		} commandInfBits;

		/* Registers */
		struct
		{
			int destBits : THREE_BITS;
			int srcBits : THREE_BITS;

		} registersBits;

		/*Struct*/
		struct
		{
			int field : TWELVE_BITS; /*00000001+are(00) or 00000010+are(00)*/
		}structFieldBits;

		struct
		{
			int value : TWELVE_BITS;
		}structAddressBits;

		/* Other operands */
		int value : TWELVE_BITS;

	} valueBits;

} memory_block;

typedef enum { EXTERNAL  = 1, RELOCATABLE , ABSOLUTE = 4 } areType;

/* Adds the value of memory block to the memoryArr */
void addWordToMemory(int *memoryArr, int *memoryCounter, memory_block memory);
/*Returns an error if the  label values were not declared*/
/*if no error this method is updating the value address of the label.  */
bool addAdressOperand(operandInfo *op, int Number_Line);
/* Adds line into the memoryArr, and increment the memoryCounter. */
/* Don't do anything if the line is error or if it's not a command line. returns an error if found while adding the line to the memory */
bool addLineToMemory(int *memoryArr, int *memoryCounter, instructionLine *line, int Number_Line);
/* Returns a memory instruction structure which represents the command in a line. */
memory_block getCommandMemory_block(instructionLine line);
/* Returns a memory instruction structure which represents the operand in a line.*/
memory_block getOpMemory_block(operandInfo op, bool isDest);
/*performs the second scan on the assembly source files and It converts all the lines into the memory_block. */
int secondFileRead(int *memoryArr, instructionLine *linesArr, int Number_Line, int IC, int DC);

int decToBinary(int n);

#endif
