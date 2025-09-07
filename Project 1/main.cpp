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
  int immediate = (instruction & 0xFFF << 20) >> 20;

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
    if (funct7 == SUBFUNCT7 && funct3 == FUNCT3A)  // SUB
    {
      cout << "sub x" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
    }
  }
  else if (opcode == ADDI && funct3 == FUNCT3A)  // ADDI
  {
    if (rd == 0 && immediate < -2048 && immediate > 2047)
    {
      cout << "Invalid register access.\n";
      return;
    }
    cout << "addi x" << rd << ", x" << rs1 
      << ", " << immediate << "\n";
  }
  else if (opcode == LD && funct3 == FUNCT3B)  // LD
  {
    if (rd == 0 && immediate < -2048 && immediate > 2047)
    {
      cout << "Invalid register access.\n";
      return;
    }
    cout << "ld x" << rd << ", " << immediate << "(x" << rs1 << ")\n";
  }
  else if (opcode == SD && funct3 == FUNCT3B)  // SD
  {
    if (immediate < -2048 && immediate > 2047)
    {
      cout << "Invalid register access.\n";
      return;
    }
    cout << "sd x" << rs2 << ", " 
      << immediate << "(x" << rs1 << ")\n";
  }
  else  // invalid or unsupported instruction
  {
    cout << "Cannot be decoded. opcode: " << opcode << ", funct7: " 
      << funct7 << ", funct3: " << funct3 << "\n";
    return;
  }
}

int main()
{
  string input = "";
  while (input.empty())
  {
    cout << "Enter an 8-character hex string (or 'X' to exit): ";
    getline(cin, input);
    if (input == "X")
    {
      cout << "Exiting program.\n";
      return 0;
    }

    input = validate_hex(8, input);
    if (input.empty())
    {
      cout << "Invalid input. Please try again.\n";
      continue;
    }
    unsigned int instruction = convert_hex(input);
    parse_instruction(instruction);
    input = "";  // reset input for next iteration
  }
  return 0;
}
