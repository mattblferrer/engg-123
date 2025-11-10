/**
 * main.cpp
 * 
 * BERNARDO, Jonathan
 * FERRER, Matt
 * 
 * Project 3 - RISC-V Pipeline Simulator using C++
 * ENGG 123.01 - J1
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Instruction
{
  string code;
  int arg1, arg2, arg3;
};

/**
 * takes an input fixed length k hex string and validates - returns 
 * the string if valid, empty string otherwise
 */
string validateHex(int k, string input)
{
  if (input.length() != k)
  {
    return "";
  }
  for (int i = 0; i < k; i++)
  {
    if (!isxdigit(input[i]))  // check if character is valid hex
    {
      return "";
    }
  }
  for (int i = 0; i < k; i++)
  {
    input[i] = toupper(input[i]);  // convert to uppercase
  }
  return input;
}

/**
 * returns true if hexadecimal strings of fixed length k are obtained
 * from the filename
 * returns false otherwise
 */
bool importHexFromFile(string filename, string* &hexData, int &counter
  , int k)
{
  ifstream myfile;
  stringstream ss;
  string line;
  vector<string> data;

  // check if filename is valid
  myfile.open(filename);
  if (!(myfile.is_open())) return false;

  // get lines of data from file
  while (getline(myfile, line))
  {
    ss.str(line);
    string hexString;
    if (!(ss >> hexString)) break;
    hexString = validateHex(min(k, (int)hexString.length()), hexString);
    if (hexString == "") break;
    data.push_back(hexString);
    counter++;
    ss.clear();
  }
  myfile.close();

  // convert data as vector to array
  if (data.size() == 0) return false;
  hexData = new string[data.size()];
  for (int i = 0; i < data.size(); i++)
  {
    hexData[i] = data[i];
  }
  return true;
}

/**
 * converts a hexadecimal string to a 64-bit integer corresponding to
 * the binary representation of the hexadecimal digits
 */
unsigned long long convertHex(const string& input)
{
  unsigned long long converted = 0;
  int s = input.length();
  for (int i = 0; i < s; i++)
  {
    if (input[i] > '9')  // for letters A-F (10-15)
    {
      converted |= (unsigned long long)(input[i] - 'A' + 10) << ((s - i - 1) * 4);
    }
    else  // for numbers 0-9
    {
      converted |= (unsigned long long)(input[i] - '0') << ((s - i - 1) * 4);
    }
  }
  return converted;
}

/**
 * parses and executes a RISC-V instruction
 */
void parseInstruction(unsigned int instruction, long long* &reg, 
  unsigned char* &mem, const int mem_size, int &pc)
{
  // constants to define instruction types
  const int ADDSUB = 0x33;
  const int ADDISLLI = 0x13;
  const int LD = 0x03;
  const int SD = 0x23;
  const int BEQBLT = 0x63;
  const int ADDSLLIFUNCT7 = 0x00; 
  const int SUBFUNCT7 = 0x20;
  const int FUNCT3A = 0x00;  // for ADD, SUB, ADDI, BEQ
  const int FUNCT3B = 0x03;  // for LD, SD
  const int FUNCT3C = 0x04; // for BLT
  const int FUNCT3D = 0x01; // for SLLI

  // calculate fields of instruction using bitmasks
  int funct7 = (instruction & (0x7F << 25)) >> 25;  // 7 bits
  int rs2 = (instruction & (0x1F << 20)) >> 20;  // 5 bits
  int rs1 = (instruction & (0x1F << 15)) >> 15;  // 5 bits
  int funct3 = (instruction & (0x7 << 12)) >> 12;  // 3 bits
  int rd = (instruction & (0x1F << 7)) >> 7;  // 5 bits
  int opcode = (instruction & 0x7F);  // 7 bits
  int immediate = (int)instruction >> 20;

  // immediate bits for SD, BEQ, and BLT instructions
  int imm11_5 = funct7;
  int imm4_0 = rd;
  int immSDandB = (imm11_5 << 5) | imm4_0;
  if (immSDandB & (1 << 11))  // sign-extend if negative
  {
    immSDandB |= 0xFFFFF000;
    immSDandB--;
  }

  // parse if the instruction is valid and supported
  if (opcode == ADDSUB)
  {
    if (rd == 0)
    {
      return;
    }
    if (funct7 == ADDSLLIFUNCT7 && funct3 == FUNCT3A)  // ADD
    {
      reg[rd] = reg[rs1] + reg[rs2];
    }
    else if (funct7 == SUBFUNCT7 && funct3 == FUNCT3A)  // SUB
    {
      reg[rd] = reg[rs1] - reg[rs2];
    }
  }
  else if (opcode == ADDISLLI && funct3 == FUNCT3A)  // ADDI
  {
    if (rd == 0)
    {
      return;
    }
    reg[rd] = reg[rs1] + immediate;
  }
  else if (opcode == LD && funct3 == FUNCT3B)  // LD
  {
    if (rd == 0)
    {
      return;
    }
    long long addr = reg[rs1] + immediate; // effective address
    if (addr < 0 || addr + 7 >= mem_size) // valid memory access
    {
      return;
    }
    long long value = 0;
    for (int i = 0; i < 8; i++) // load 8 bytes
    {
      value |= ((long long)mem[addr + i]) << (i * 8);
    }
    reg[rd] = value;
  }
  else if (opcode == SD && funct3 == FUNCT3B)  // SD
  {
    if (rs2 == 0)
    {
      return;
    }
    long long addr = reg[rs1] + immSDandB; // effective address
    if (addr < 0 || addr + 7 >= mem_size) // valid memory access
    {
      return;
    }
    long long value = reg[rs2]; // value to store
    for (int i = 0; i < 8; i++) // store 8 bytes
    {
      mem[addr + i] = (value >> (i * 8)) & 0xFF;
    }
  }
  else if (opcode == BEQBLT && funct3 == FUNCT3A) // BEQ
  {
    if (reg[rs1] == reg[rs2]) 
    {
      pc += immSDandB;
      return;
    }
  }
  else if (opcode == BEQBLT && funct3 == FUNCT3C) // BLT
  {
    if (reg[rs1] < reg[rs2])
    {
      pc += immSDandB;
      return;
    }
  }
  else if (opcode == ADDISLLI && funct7 == ADDSLLIFUNCT7 
    && funct3 == FUNCT3D) // SLLI
  {
    // shift ammount for slli
    int shamt = rs2;
    if (rd == 0)
    {
      return;
    }
    reg[rd] = reg[rs1] << shamt;
  }
  else  // invalid or unsupported instruction
  {
    return;
  }
}

/**
 * prints the help menu with available commands and their usage
 */
void printHelpMenu()
{
  const string help_message = "\n"
    "RISC-V Simulator\n\n"
    "Type \"help\" for more information.\n\n"
    "Type \"loaddata [address] [filename]\" to obtain lines of "
    "hexadecimal strings from a text file specified by filename and "
    "store the values to simulated RISC-V memory starting at the "
    "location specified by address.\n\n"
    "Type \"showdata [address] [N]\" to display contents of the "
    "simulated RISC-V data memory in hexadecimal, starting at the "
    "location specified by address and displaying N words.\n"
    "Type \"loadcode [address] [filename]\" to obtain lines of "
    "hexadecimal strings from a text file specified by filename and "
    "store the values to simulated RISC-V memory starting at the "
    "location specified by address.\n\n"
    "Type \"showcode [address] [N]\" to display contents of the "
    "simulated RISC-V data memory in hexadecimal, starting at the "
    "location specified by address and displaying N words.\n"
    "Type \"exec [address]\" to simulation execution of code "
    "starting at the specified address.\n\n"
    "Type \"exit\" to exit the program.\n";
  cout << help_message;
}

/**
 * store hexadecimal strings from a file to simulated RISC-V memory
 */
void loadMemory(string filename, int addr, 
  unsigned char* &mem, const int word_size, const int mem_size)
{
  string* hexData = new string[0];
  int counter = 0;
  if (!importHexFromFile(filename, hexData, counter, word_size * 2))
  {
    cout << "Unable to import valid hex data from "
      << filename << ".\n";
    delete[] hexData;
    return;
  }
  for (int i = 0; i < counter; i++)
  {
    unsigned long long value = convertHex(hexData[i]);
    for (int j = 0; j < word_size; j++) // store word_size bytes
    {
      int mem_index = addr + i * word_size + j;
      if (mem_index < 0 || mem_index >= mem_size) 
      {
        cout << "Memory write out of bounds at address " << 
          mem_index << ".\n";
        delete[] hexData;
        return;
      }
      mem[mem_index] = (value >> (j * 8)) & 0xFF;
    }
  }
  cout << "Data loaded to memory starting at address " 
    << addr << ".\n";
  delete[] hexData;
}

/**
 * shows the contents of simulated RISC-V memory in hexadecimal
 */
void showMemory(int addr, int N, unsigned char* &mem,
  const int word_size, const int mem_size)
{
  if (addr < 0 || addr >= mem_size)
  {
    cout << "Invalid memory access.\n";
    return;
  }
  if (N <= 0)
  {
    cout << "N must be a positive integer.\n";
    return;
  }
  for (int i = 0; i < N; i++)
  {
    int mem_index = addr + i * word_size;
    if (mem_index < 0 || mem_index + word_size - 1 >= mem_size)
    {
      cout << "Memory read out of bounds at address " 
        << mem_index << ".\n";
      return;
    }
    unsigned long long value = 0;
    for (int j = 0; j < word_size; j++) // load word_size bytes
    {
      value |= ((unsigned long long)mem[mem_index + j]) << (j * 8);
    } 
    cout << hex << uppercase;
    cout.width(word_size * 2);
    cout.fill('0');
    cout << value << "\n";
    cout << dec; // reset to decimal output
  }
}

/**
 * fetches instructions from the address in the memory location whose
 * value is stored in the program counter
 */
unsigned int instructionFetch(const int pc, const int mem_size, 
  unsigned char* &inst_mem)
{
  if (pc < 0 || pc >= mem_size)
  {
    cout << "Invalid memory access.\n";
    return 0;
  }
  if (pc < 0 || pc + 3 >= mem_size)
  {
    cout << "Memory read out of bounds at address " 
      << pc << ".\n";
    return 0;
  }
  unsigned int instruction = 0;
  for (int i = 0; i < 4; i++) // load 4 bytes
  {
    instruction |= ((unsigned int)inst_mem[pc + i]) << (i * 8);
  }
  if (instruction == 0) // halt on instruction of all zeros
  {
    cout << "Halt instruction encountered. Stopping execution.\n";
    return 0;
  }
  return instruction;
}

/**
 * decodes the fetched instruction and reads the necessary registers
 */
Instruction instructionDecode(unsigned int instruction, long long* &reg, 
  unsigned char* &mem, const int mem_size, int &pc)
{
  // initialize instruction
  Instruction inst;
  inst.code = "NOP";
  inst.arg1 = -1;
  inst.arg2 = -1;
  inst.arg3 = -1;

  // constants to define instruction types
  const int ADDSUB = 0x33;
  const int ADDISLLI = 0x13;
  const int LD = 0x03;
  const int SD = 0x23;
  const int BEQBLT = 0x63;
  const int ADDSLLIFUNCT7 = 0x00; 
  const int SUBFUNCT7 = 0x20;
  const int FUNCT3A = 0x00;  // for ADD, SUB, ADDI, BEQ
  const int FUNCT3B = 0x03;  // for LD, SD
  const int FUNCT3C = 0x04; // for BLT
  const int FUNCT3D = 0x01; // for SLLI

  // calculate fields of instruction using bitmasks
  int opcode = (instruction & 0x7F);  // 7 bits
  int immediate = (int)instruction >> 20;
  int funct7 = (instruction & (0x7F << 25)) >> 25;  // 7 bits
  int rs2 = (instruction & (0x1F << 20)) >> 20;  // 5 bits
  int rs1 = (instruction & (0x1F << 15)) >> 15;  // 5 bits
  int funct3 = (instruction & (0x7 << 12)) >> 12;  // 3 bits
  int rd = (instruction & (0x1F << 7)) >> 7;  // 5 bits

  // immediate bits for SD, BEQ, and BLT instructions
  int imm11_5 = funct7;
  int imm4_0 = rd;
  int immSDandB = (imm11_5 << 5) | imm4_0;
  if (immSDandB & (1 << 11))  // sign-extend if negative
  {
    immSDandB |= 0xFFFFF000;
    immSDandB--;
  }

  // parse if the instruction is valid and supported
  if (opcode == ADDSUB)
  {
    if (funct7 == ADDSLLIFUNCT7 && funct3 == FUNCT3A)  // ADD
    {
      inst.code = "ADD";
      inst.arg1 = rd;
      inst.arg2 = rs1;
      inst.arg3 = rs2;
    }
    else if (funct7 == SUBFUNCT7 && funct3 == FUNCT3A)  // SUB
    {
      inst.code = "SUB";
      inst.arg1 = rd;
      inst.arg2 = rs1;
      inst.arg3 = rs2;
    }
  }
  else if (opcode == ADDISLLI && funct3 == FUNCT3A)  // ADDI
  {
    inst.code = "ADDI";
    inst.arg1 = rd;
    inst.arg2 = rs1;
    inst.arg3 = immediate;
  }
  else if (opcode == LD && funct3 == FUNCT3B)  // LD
  {
    inst.code = "LD";
    inst.arg1 = rd;
    inst.arg2 = rs1;
    inst.arg3 = immediate;
  }
  else if (opcode == SD && funct3 == FUNCT3B)  // SD
  {
    inst.code = "SD";
    inst.arg1 = rs1;
    inst.arg2 = rs2;
    inst.arg3 = immSDandB;
  }
  else if (opcode == BEQBLT && funct3 == FUNCT3A) // BEQ
  {
    inst.code = "BEQ";
    inst.arg1 = rs1;
    inst.arg2 = rs2;
    inst.arg3 = immSDandB;
  }
  else if (opcode == BEQBLT && funct3 == FUNCT3C) // BLT
  {
    inst.code = "BLT";
    inst.arg1 = rs1;
    inst.arg2 = rs2;
    inst.arg3 = immSDandB;
  }
  else if (opcode == ADDISLLI && funct7 == ADDSLLIFUNCT7 
    && funct3 == FUNCT3D) // SLLI
  {
    inst.code = "SLLI";
    inst.arg1 = rd;
    inst.arg2 = rs1;
    inst.arg3 = rs2;
  }
  return inst;
}

/**
 * executes instructions 
 */
void instructionExecute(Instruction &inst, long long* &reg, 
  unsigned char* &mem, const int mem_size, int& pc)
{

}

/**
 * memory operands are read and written from/to memory
 */
void memoryAccess()
{

}

/**
 * computed value is written to the register
 */
void writeBack()
{
  
}

/**
 * executes RISC-V instructions starting from the specified address
 * in simulated RISC-V memory
 */
void programLoop(long long* &reg, unsigned char* &inst_mem, 
  unsigned char* &data_mem, const int mem_size, int& pc)
{
  while (true)
  {
    unsigned int instruction = instructionFetch(pc, mem_size, 
      inst_mem);
    if (instruction == 0) // halt on instruction of all zeros
    {
      return;
    }
    Instruction inst = instructionDecode(instruction, reg, data_mem, 
      mem_size, pc);
    instructionExecute(inst, reg, data_mem, mem_size, pc);
    memoryAccess();
    writeBack();
  }
}

int main()
{
  bool exitTyped = false;
  int N = 0;

  // register and memory declarations
  long long* reg = new long long[32]; // 32 64-bit registers
  const int mem_size = 1024 * 64; // 64 KB memory
  unsigned char* data_mem = new unsigned char[mem_size];
  unsigned char* inst_mem = new unsigned char[mem_size];  

  // initialize registers and memory to zero
  for (int i = 0; i < 32; i++)
  {
    reg[i] = 0;
  }
  for (int i = 0; i < mem_size; i++)
  {
    data_mem[i] = 0;
    inst_mem[i] = 0;
  }

  // starting program 
  cout << "RISC-V Simulator\nType \"help\" for more information.\n";

  // program loop, will only exit when command exit is typed
  while (!exitTyped)
  {
    string input;
    string command;
    int address = -1;
    string filename;
    string extra;
    stringstream ss;

    ss.clear();
    cout << "> ";
    getline(cin, input);
    ss.str(input);
    ss >> command >> address;
    if (!(ss.str().empty()))
    {
      ss >> filename;
      // check if filename is valid integer N
      if (isdigit(filename[0]))
      {
        N = stoi(filename);
        filename = "";
      }
      else
      {
        N = 0;
      }
    }

    // commands with no arguments
    if (ss >> extra) // check if input has more arguments
    {
      cout << "Invalid command.\n";
      continue;
    }
    else if (command == "help" && address == -1)
    {
      printHelpMenu();
      continue;
    }
    else if (command == "exit" && address == -1)
    {
      exitTyped = true;
      continue;
    }

    // commands with address argument: validate address
    if ((address < 0) || (address >= mem_size))
    {
      cout << "Invalid address.\n";
      continue;
    }
    if (command == "exec")
    {
      programLoop(reg, inst_mem, data_mem, mem_size, address);
      continue;
    }
    else if (command == "showdata")
    {
      showMemory(address, N, data_mem, 8, mem_size);
      continue;
    }
    
    else if (command == "showcode")
    {
      showMemory(address, N, inst_mem, 4, mem_size);
      continue;
    }
    
    // commands with address, filename arguments: validate filename
    if (filename.empty())
    {
      cout << "Invalid filename.\n";
      continue;
    }
    if (command == "loaddata")
    {
      loadMemory(filename, address, data_mem, 8, mem_size);
      continue;
    }
    else if (command == "loadcode")
    {
      loadMemory(filename, address, inst_mem, 4, mem_size);
      continue;
    }
    
    cout << "Invalid command.\n";
  }

  // delete dynamically allocated memory
  delete[] reg;
  delete[] data_mem;
  delete[] inst_mem;

  return 0;
}
