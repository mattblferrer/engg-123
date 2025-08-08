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
string validate_hex(int k)
{
  string input;
  cin >> input;
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
 * converts a hexadecimal string to a boolean array corresponding to
 * the binary representation of the hexadecimal digits
 */
bool* convert_hex(const string& input)
{
  bool* converted = new bool[input.length() * 4];
  for (int i = 0; i < input.length() * 4; i++)
  {
    if (input[i / 4] > '9')  // for letters A-F (10-15)
    {
      converted[i] = (input[i / 4] - 'A' + 10) & (1 << (3 - (i % 4)));
    }
    else  // for numbers 0-9
    {
      converted[i] = (input[i / 4] - '0') & (1 << (3 - (i % 4)));
    }
  }
  return converted;
}

int main()
{
  
  return 0;
}
