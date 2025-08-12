/**
 * main.cpp
 * 
 * FERRER, Matt
 * 
 * ENGG 123.01 - J1
*/

#include <cctype>
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

int main()
{
  int* reg = new int[32];
  for (int i = 0; i < 32; i++)  // initialize registers to zero
  {
    reg[i] = 0;
  }
  string input = "";
  while (input.empty())
  {
    cout << "Enter an 8-character hex string (or 'X' to exit): ";
    getline(cin, input);
    if (input == "X")
    {
      cout << "Exiting program." << endl;
      return 0;
    }

    input = validate_hex(8, input);
    if (input.empty())
    {
      cout << "Invalid input. Please try again." << endl;
      continue;
    }
    unsigned int instruction = convert_hex(input);
    
  }
  delete[] reg;
  return 0;
}
