# Simple-Regex-Engine-in-C

The following matches required for this assignment are supported by this code:-
1. Matching of individual characters and numbers
2. Ranges: a-z,A-Z,0-9 or anything in between
3. Character classes
4. Macros like +,*,?
5. \w,\d
6. Greedy and non-greedy matching in case of *

Certain Additional matches are also supported since they weren't very difficult to implement, like
1. Matches '.' - Everything except \n
2. Support for greedy and non greedy wrt +
3. Startswith: ^ and Endswith: $
4. Non inclusion matches in character class: [^...]
5. \W,\D,\s,\S

Data Structures used:-
The input pattern is parsed and stored in the form of a structure whose definition is given below:-
typedef struct regex_t
{
  int type;
  union
  {
    char ch;
    char *char_class;
  };
}regex;

Logic used:-
1. The text and no of patterns are taken through input
2. Each pattern that is input, is parsed, converted and stored in the form of the structure, defined above.
3. The match_here function as defined by Sir in the class, has been converted to an iterative version called match_util which iterates through the pattern and text and returns the starting index in case a match is found. -1 otherwise.
4. The match length is in fact a global variable which indicates the no. of characters matched. This is used to calculate the ending index, as follows:
  End index = Start index + match length - 1
5. The parsed pattern and the character class buffers are also global variables for convenience of scope (Although a bad programming practice)
6. The macros and metacharacters are depicted by their types, which are ENUMS for ease of use, easy debugging and readability.

Input:-
From stdin, as follows

text

no_of_patterns

pattern0

pattern1

.

.

.

Constraints:-

Max length of text = 4000

Max length of patterns = 1000

Output:-
As required by the assignment, this code Outputs:-

   0 - if there is no match
   
   1 start_index end_index - if match is found
   
for each pattern input
