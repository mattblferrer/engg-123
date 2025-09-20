/**
 * main.cpp
 * 
 * BERNARDO, Jonathan
 * FERRER, Matt
 * 
 * ENGG 123.01 - J1
 */

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

/**
 * takes an input fixed length k hex string and validates - returns 
 * the string if valid, empty string otherwise
 */
string validate_hex(int k, string input)
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
 * converts a hexadecimal string to a 32-bit integer corresponding to
 * the binary representation of the hexadecimal digits
 */
unsigned int convert_hex(const string& input)
{
  unsigned int converted = 0;
  int s = input.length();
  for (int i = 0; i < s; i++)
  {
    if (input[i] > '9')  // for letters A-F (10-15)
    {
      converted |= (input[i] - 'A' + 10) << ((s - i - 1) * 4);
    }
    else  // for numbers 0-9
    {
      converted |= (input[i] - '0') << ((s - i - 1) * 4);
    }
  }
  return converted;
}

void parse_instruction(unsigned int instruction)
{
  // constants to define instruction types
  const int ADDSUB = 0x33;
  const int ADDI = 0x13;
  const int LD = 0x03;
  const int SD = 0x23;
  const int ADDFUNCT7 = 0x00;
  const int SUBFUNCT7 = 0x20;
  const int FUNCT3A = 0x00;  // for ADD, SUB, ADDI
  const int FUNCT3B = 0x03;  // for LD, SD

  // calculate fields of instruction using bitmasks
  int funct7 = (instruction & (0x7F << 25)) >> 25;
  int rs2 = (instruction & (0x1F << 20)) >> 20;
  int rs1 = (instruction & (0x1F << 15)) >> 15;
  int funct3 = (instruction & (0x7 << 12)) >> 12;
  int rd = (instruction & (0x1F << 7)) >> 7;
  int opcode = (instruction & 0x7F);
  int immediate = (int)instruction >> 20;

  // immediate bits for SD instruction
  int imm11_5 = funct7;
  int imm4_0 = rd;
  int immSD = (imm11_5 << 5) | imm4_0;
  if (immSD & (1 << 11))  // sign-extend if negative
  {
    immSD |= 0xFFFFF000;
  }

  // parse if the instruction is valid and supported
  if (opcode == ADDSUB)
  {
    if (rd == 0)
    {
      cout << "Invalid register access.\n";
      return;
    }
    if (funct7 == ADDFUNCT7 && funct3 == FUNCT3A)  // ADD
    {
      cout << "add x" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
    }
    else if (funct7 == SUBFUNCT7 && funct3 == FUNCT3A)  // SUB
    {
      cout << "sub x" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
    }
    else 
    {
      cout << "Cannot be decoded. opcode: " << opcode << ", funct7: " 
        << funct7 << ", funct3: " << funct3 << "\n";
    }
  }
  else if (opcode == ADDI && funct3 == FUNCT3A)  // ADDI
  {
    if (rd == 0)
    {
      cout << "Invalid register access.\n";
      return;
    }
    cout << "addi x" << rd << ", x" << rs1 
      << ", " << immediate << "\n";
  }
  else if (opcode == LD && funct3 == FUNCT3B)  // LD
  {
    if (rd == 0)
    {
      cout << "Invalid register access.\n";
      return;
    }
    cout << "ld x" << rd << ", " << immediate << "(x" << rs1 << ")\n";
  }
  else if (opcode == SD && funct3 == FUNCT3B)  // SD
  {
    if (rs2 == 0)
    {
      cout << "Invalid register access.\n";
      return;
    }
    cout << "sd x" << rs2 << ", " 
      << immSD << "(x" << rs1 << ")\n";
  }
  else  // invalid or unsupported instruction
  {
    cout << "Cannot be decoded. opcode: " << opcode << ", funct7: " 
      << funct7 << ", funct3: " << funct3 << "\n";
    return;
  }
}

void printHelpMenu()
{
  const string help_message = "\n"
    "RISC-V Simulator\n"
    "Type \"help\" for more information.\n"
    "Type \"loaddata [address] [filename]\" to obtain lines of "
    "hexadecimal strings from a text file specified by filename and "
    "store the values to simulated RISC-V memory starting at the "
    "location specified by address.\n"
    "Type \"showdata [address] [N]\" to display contents of the "
    "simulated RISC-V data memory in hexadecimal, starting at the "
    "location specified by address and displaying N words.\n"
    "Type \"loadcode [address] [filename]\" to obtain lines of "
    "hexadecimal strings from a text file specified by filename and "
    "store the values to simulated RISC-V memory starting at the "
    "location specified by address.\n"
    "Type \"showcode [address] [N]\" to display contents of the "
    "simulated RISC-V data memory in hexadecimal, starting at the "
    "location specified by address and displaying N words.\n"
    "Type \"exec [address]\" to simulation execution of code "
    "starting at the specified address.\n"
    "Type \"exit\" to exit the program.\n";
  cout << help_message;
}

int main()
{
  bool exitTyped = false;
  string input;
  string command;
  string address;
  string filename;
  string extra;
  stringstream ss;
  int N = 0;
  

  // register and memory declarations
  long long* reg = new long long[32]; // 32 64-bit registers
  const int mem_size = 1024 * 64; // 64 KB memory
  uint8_t* mem = new uint8_t[mem_size];

  // initialize registers and memory to zero
  for (int i = 0; i < 32; i++)
  {
    reg[i] = 0;
  }
  for (int i = 0; i < mem_size; i++)
  {
    mem[i] = 0;
  }


  // starting program 
  cout << "RISC-V Simulator\nType \"help\" for more information.\n";

  // program loop, will only exit when command exit is typed
  while (!exitTyped)
  {
    ss.clear();
    cout << "> ";
    getline(cin, input);
    ss.str(input);
    ss >> command >> address;
    if (!(ss.str().empty()))
    {
      if (!(ss >> N)) ss >> filename; // if argument is int or string
      else ss >> filename;
    }

    // check if input has more arguments
    if (ss >> extra)
    {
      cout << "Invalid command.\n";
    }
    else if (command == "help" && address.empty())
    {
      printHelpMenu();
    }
    else if (command == "loaddata")
    {

    }
    else if (command == "showdata")
    {

    }
    else if (command == "loadcode")
    {

    }
    else if (command == "showcode")
    {

    }
    else if (command == "exec")
    {

    }
    else if (command == "exit" && address.empty())
    {
      exitTyped = true;
    }
    else
    {
      cout << "Invalid command.\n";
    }
  }

  return 0;
}
