/**
 * main.cpp
 * 
 * BERNARDO, Jonathan
 * FERRER, Matt
 * 
 * Project 2 - Interactive Basic RISC-V Simulator
 * ENGG 123.01 - J1
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

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
  , int k = 8)
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
    hexString = validateHex(k, hexString);
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
 * converts a hexadecimal string to a 32-bit integer corresponding to
 * the binary representation of the hexadecimal digits
 */
unsigned int convertHex(const string& input)
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

/**
 * parses and executes a RISC-V instruction
 */
void parseInstruction(unsigned int instruction, long long* &reg, 
  unsigned char* &mem, const int mem_size)
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
      reg[rd] = reg[rs1] + reg[rs2];
    }
    else if (funct7 == SUBFUNCT7 && funct3 == FUNCT3A)  // SUB
    {
      cout << "sub x" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
      reg[rd] = reg[rs1] - reg[rs2];
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
    reg[rd] = reg[rs1] + immediate;
  }
  else if (opcode == LD && funct3 == FUNCT3B)  // LD
  {
    if (rd == 0)
    {
      cout << "Invalid register access.\n";
      return;
    }
    long long addr = reg[rs1] + immediate; // effective address
    if (addr < 0 || addr + 7 >= mem_size) // valid memory access
    {
      cout << "Invalid memory access.\n";
      return;
    }
    cout << "ld x" << rd << ", " << immediate << "(x" << rs1 << ")\n";
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
      cout << "Invalid register access.\n";
      return;
    }
    long long addr = reg[rs1] + immSD; // effective address
    if (addr < 0 || addr + 7 >= mem_size) // valid memory access
    {
      cout << "Invalid memory access.\n";
      return;
    }
    cout << "sd x" << rs2 << ", " 
      << immSD << "(x" << rs1 << ")\n";
    long long value = reg[rs2]; // value to store
    for (int i = 0; i < 8; i++) // store 8 bytes
    {
      mem[addr + i] = (value >> (i * 8)) & 0xFF;
    }
  }
  else  // invalid or unsupported instruction
  {
    cout << "Cannot be decoded. opcode: " << opcode << ", funct7: " 
      << funct7 << ", funct3: " << funct3 << "\n";
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
void loadMemory(string filename, string addr, unsigned char* &mem,
  const int word_size, const int mem_size)
{
  string* hexData = new string[0];
  int counter = 0;
  if (!importHexFromFile(filename, hexData, counter))
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
      int mem_index = convertHex(addr) + i * word_size + j;
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
void showMemory(string addr, int N, unsigned char* &mem,
  const int word_size, const int mem_size)
{
  int start = convertHex(addr);
  if (start < 0 || start >= mem_size)
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
    int mem_index = start + i * word_size;
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
    string address;
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
    else if (command == "help" && address.empty())
    {
      printHelpMenu();
      continue;
    }
    else if (command == "exit" && address.empty())
    {
      exitTyped = true;
      continue;
    }

    // commands with address argument: validate address
    if (validateHex(8, address).empty())
    {
      cout << "Invalid address.\n";
      continue;
    }
    if (command == "exec")
    {
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
