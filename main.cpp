#include <iostream>

using Byte = unsigned char;
using Word = unsigned short;

struct Memory
{
	static constexpr unsigned int MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void Initialise() {
		for (unsigned int i = 0; i < MAX_MEM; i++) {
			Data[i] = 0;
		}
	}

	//Read 1 byte
	Byte operator[](unsigned int Address) const {
		return Data[Address];
	}

	//Write 1 byte
	Byte& operator[](unsigned int Address) {
		return Data[Address];
	}
};

struct CPU {

	Word PC; // Program Counter
	Byte SP; //Stack Pointer

	Byte A, X, Y; //Registers

	//Processor Status Registers
	Byte C : 1;
	Byte Z : 1;
	Byte I : 1;
	Byte D : 1;
	Byte B : 1;
	Byte V : 1;
	Byte N : 1;

	void Reset(Memory& memory) 
	{
		PC = 0xFFFC;
		SP = 0x0100;
		C,Z,I,D,B,V,N = 0;
		A = X = Y = 0;
		memory.Initialise();
	}

	Byte FetchByte(unsigned int& Cycles, Memory& memory) {
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;
	}

	Word FetchWord(unsigned int& Cycles, Memory& memory)
	{
		Word Data = memory[PC];
		PC++;

		Data |= (memory[PC] << 8);
		PC++;

		Cycles-=2;

		return Data;
	}

	Byte ReadByte(unsigned int& Cycles, Byte Address,Memory& memory) {
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	//Opcodes
	static constexpr Byte
		INS_LDA_IM = 0xA9,
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_JSR = 0x20;

	void LDASetStatus() {
		Z = (A == 0);
		N = (A & 0b10000000) > 0;
	}

	void Execute(unsigned int Cycles,Memory& memory)
	{
		while (Cycles > 0) {
			Byte Ins = FetchByte(Cycles, memory);
			switch (Ins)
			{
			case INS_LDA_IM:
			{
				Byte Value = FetchByte(Cycles, memory);
				A = Value;
				LDASetStatus();
			} break;
			case INS_LDA_ZP:
			{
				Byte ZeroPageAddr= FetchByte(Cycles, memory);
				A= ReadByte(Cycles,ZeroPageAddr, memory);
				LDASetStatus();
			}break;
			case INS_LDA_ZPX:
			{
				Byte ZeroPageAddr = FetchByte(Cycles, memory);
				ZeroPageAddr += X;
				Cycles--;
				A = ReadByte(Cycles, ZeroPageAddr, memory);
				LDASetStatus();
			}break;
			case INS_JSR:
			{
				Word SubroutineAddr = FetchWord(Cycles, memory);
				memory[SP] = PC - 1;
				Cycles--;
				PC = SubroutineAddr;
				Cycles--;
			}break;
			default:
				std::cout << "Instruction " << Ins << " not handled" << std::endl;
				break;
			}
		}
	}
};

int main()
{
	Memory mem;
	CPU cpu;
	cpu.Reset(mem);
	mem[0xFFFC] = CPU::INS_LDA_ZP;
	mem[0xFFFD] = 0x42;
	mem[0X0042] = 0x82;
	cpu.Execute(3,mem);
	return 0;
}