//PES1201801482 - YASH GAWANKAR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////

#define TXT_SIZE 4010
#define PAT_SIZE 1010

//Parsed regex pattern is stored as a structure
typedef struct regex_t
{
  int type;
  union
  {
    char ch;
    char *char_class;
  };
}regex;

//Global parameters for ease of access
//Parsed pattern is stored in this array of structures
regex parsed[PAT_SIZE];
//The charcter class in each array points to the specific
//char class in the following local buffer
char char_class[PAT_SIZE];
//No. of character matched - To find ending index
int match_length = 0;

/////////////////////////////////////////////////////////////////////////////

//Defining all functions used
static int match_pattern(regex* pattern,char* text);
static void parse_pattern(char * pattern);
static int match_one_char(regex pattern, char c);
static int match_star(regex p, regex* pattern,char* text,int greedy_flag);
static int match_plus(regex p, regex* pattern, char* text, int greedy_flag);
static int match_question(regex p, regex* pattern, char* text);
static int match_util(regex* pattern, char* text);
int match(char* pattern, char* text);
static int matchdigit(char c);
static int matchalpha(char c);
static int matchwhitespace(char c);
static int matchalphanum(char c);
static int matchrange(char c, char* str);
static int matchdot(char c);
static int matchmetachar(char c, char cls);
static int matchcharclass(char c,char *str);

//Type of each regex element in the pattern is an enum for ease of use
enum { SENTINEL, DOT, STARTSWITH, ENDSWITH, QUESTIONMARK,
      STAR, PLUS, CHAR, CHAR_CLASS,
      INV_CHAR_CLASS, DIGIT, NOT_DIGIT,
      ALPHA, NOT_ALPHA, WHITESPACE, NOT_WHITESPACE};

/////////////////////////////////////////////////////////////////////////////

//Parse each character/flag in the regex pattern and store as structure
static void parse_pattern(char * pattern)
{
  char ch;
  int i = 0;
  int j = 0;
  int index = 1;

  while(pattern[i]!='\0')
  {
    ch = pattern[i];
    switch (ch)
   {
     // Meta-characters - set 1
     case '^': {    parsed[j].type = STARTSWITH;           } break;
     case '$': {    parsed[j].type = ENDSWITH;             } break;
     case '.': {    parsed[j].type = DOT;             } break;
     case '*': {    parsed[j].type = STAR;            } break;
     case '+': {    parsed[j].type = PLUS;            } break;
     case '?': {    parsed[j].type = QUESTIONMARK;    } break;
     // Meta-characters - set 2
     case '\\':
     {
       switch (pattern[++i]) {
         case 'd': {    parsed[j].type = DIGIT;            } break;
         case 'D': {    parsed[j].type = NOT_DIGIT;        } break;
         case 'w': {    parsed[j].type = ALPHA;            } break;
         case 'W': {    parsed[j].type = NOT_ALPHA;        } break;
         case 's': {    parsed[j].type = WHITESPACE;       } break;
         case 'S': {    parsed[j].type = NOT_WHITESPACE;   } break;
         default : //Escaped characters
         {
           parsed[j].type = CHAR;
           parsed[j].ch = pattern[i];
         } break;
       }
     } break;
     // Character Class
     case '[':
     {
       int start = index;
       if (pattern[i+1] == '^')
       {
         parsed[j].type = INV_CHAR_CLASS;
         i += 1;
       }
       else
       {
         parsed[j].type = CHAR_CLASS;
       }

       while ( (pattern[++i] != ']') && (pattern[i] != '\0') )
       {
         char_class[index++] = pattern[i];
       }
       char_class[index++] = '\0';
       parsed[j].char_class = &char_class[start];
     } break;

     default:
     {
       parsed[j].type = CHAR;
       parsed[j].ch = ch;
     } break;
    }
    i++; j++;
  }
  parsed[j].type = SENTINEL;
}

//Debug function to check if parsing has been done right
static void print_parsed()
{
  int i = 0;
  while(parsed[i].type != SENTINEL)
  {
    printf("%d\n",parsed[i].type);
    i++;
  }
}

/////////////////////////////////////////////////////////////////////////////

static int matchdigit(char c)
{
  return ((c >= '0') && (c <= '9'));
}
static int matchalpha(char c)
{
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}
static int matchwhitespace(char c)
{
  return ((c == ' ') || (c == '\t') || (c == '\n'));
}
static int matchalphanum(char c)
{
  return ((c == '_') || matchalpha(c) || matchdigit(c));
}
static int matchrange(char c, char* str)
{
  return( (str[0] != '\0')
           && (str[0] != '-')
           && (str[1] == '-')
           && (str[2] != '\0')
           && ((c >= str[0]) && (c <= str[2]))
         );
}
static int matchdot(char c)
{
  return (c!= '\n');
}
static int matchmetachar(char c, char cls)
{
  switch (cls)
  {
    case 'd': return  matchdigit(c);
    case 'D': return !matchdigit(c);
    case 'w': return  matchalphanum(c);
    case 'W': return !matchalphanum(c);
    case 's': return  matchwhitespace(c);
    case 'S': return !matchwhitespace(c);
    default:  return (c == cls);
  }
}

static int matchcharclass(char c,char *str)
{
  int i = 0;
  do {
    if(matchrange(c,&str[i]))
    {
      return 1;
    }
    if(str[i]=='\\')
    {
      i++;
      if(matchmetachar(c,str[i]))
      {
        return 1;
      }
    }
    if(str[i] == c)
    {
      return 1;
    }
    i++;
  } while(str[i] != '\0');
}

static int match_one_char(regex pattern, char c)
{
  switch (pattern.type)
  {
    case DOT:            return matchdot(c);
    case CHAR_CLASS:     return  matchcharclass(c,pattern.char_class);
    case INV_CHAR_CLASS: return !matchcharclass(c,pattern.char_class);
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalphanum(c);
    case NOT_ALPHA:      return !matchalphanum(c);
    case WHITESPACE:     return  matchwhitespace(c);
    case NOT_WHITESPACE: return !matchwhitespace(c);
    default:             return  (pattern.ch == c);
  }
}

/////////////////////////////////////////////////////////////////////////////

// * matches 0 or more characters
// Support for non greediness with succeeding ?
static int match_star(regex p, regex* pattern,char* text,int greedy_flag)
{
  int temp = match_length;
  char* prepoint = text;
  while ((text[0] != '\0') && match_one_char(p, *text))
  {
    if(!greedy_flag)
    {
      int temp1 = match_length;
      if( match_pattern(pattern, text) )
      {
        match_length = temp1;
        break;
      }
    }
    text++;
    match_length++;
  }
  while (text >= prepoint)
  {
    if (match_pattern(pattern, text))
      return 1;
    text--;
    match_length--;
  }

  match_length = temp;
  return 0;
}

// + matches 1 or more characters
// Support for non greediness with succeeding ?
static int match_plus(regex p, regex* pattern, char* text, int greedy_flag)
{
  char* prepoint = text;
  while ((text[0] != '\0') && match_one_char(p, *text))
  {
    text++;
    match_length++;
    if(!greedy_flag)
    {
      int temp1 = match_length;
      if( match_pattern(pattern, text) )
      {
        match_length = temp1;
        break;
      }
    }
  }
  while (text > prepoint)
  {
    if (match_pattern(pattern, text--))
      return 1;
    match_length--;
  }
  return 0;
}

//Matches 0 or 1 charqcter
static int match_question(regex p, regex* pattern, char* text)
{
  if (p.type == SENTINEL)
    return 1;

  int rt=0;
  int prev = match_length;
  if (!match_one_char(p,*text) && match_pattern(pattern, text))
    rt = 1;

  if (*text && match_one_char(p, *text))
  {
    match_length = prev;
    if (match_pattern(pattern, &text[1]))
    {
      match_length++;
      return 1;
      match_length = prev;
    }
    else if(match_pattern(pattern, text))
      return 1;
  }
  return rt;
}

/////////////////////////////////////////////////////////////////////////////

//Used to match entire patterns following *,+,? and/or ^
static int match_pattern(regex* pattern, char* text)
{
  int pre = match_length;
  int rt;
  do
  {
    if ((pattern[0].type == SENTINEL) ||(pattern[1].type == QUESTIONMARK))
    {
      rt = match_question(pattern[0], &pattern[2], text);
      if(!rt)
        match_length = pre;

      return rt;
    }
    else if (pattern[1].type == STAR)
    {
      if (pattern[2].type == QUESTIONMARK)
        rt = match_star(pattern[0], &pattern[3], text,0);
      else
        rt = match_star(pattern[0], &pattern[2], text,1);
      if(!rt)
        match_length = pre;

      return rt;
    }
    else if (pattern[1].type == PLUS)
    {
      if(pattern[2].type == QUESTIONMARK)
        rt = match_plus(pattern[0], &pattern[3], text,0);
      else
        rt = match_plus(pattern[0], &pattern[2], text,1);
      if(!rt)
        match_length = pre;

      return rt;
    }
    else if ((pattern[0].type == ENDSWITH) && pattern[1].type == SENTINEL)
    {
      return (text[0] == '\0');
    }

   match_length++;
  } while ((text[0] != '\0') && match_one_char(*pattern++, *text++));
  match_length = pre;
  return 0;
}

//Utility match function which iterates through the pattern and text
//Finds the match and returns the starting index if match found
//-1 otherwise
static int match_util(regex* pattern, char* text)
{
  match_length = 0;
  if(pattern[0].type == STARTSWITH)
  {
    return ((match_pattern(&pattern[1], text)) ? 0 : -1);
  }
  int i = -1;
  do
  {
    i++;
    if(match_pattern(pattern,&text[i]))
    {
      if(text[i]=='\0')
        return -1;
      return i;
    }
  } while (text[i] != '\0');
  return -1;
}

//Simulation of python's re.match function
//First parses the pattern, then iteratively checks for match
int match(char* pattern, char* text)
{
  parse_pattern(pattern);
  return match_util(parsed, text);
}

/////////////////////////////////////////////////////////////////////////////

int main()
{
  char text[TXT_SIZE];
  char pattern[PAT_SIZE];
  int no_of_patterns, l;
  fgets(text,TXT_SIZE,stdin);
  l = strlen(text);
  if(text[l-1] == '\n')
    text[l-1] = '\0';
  int start_index = -1;
  scanf("%d\n",&no_of_patterns);
  while (no_of_patterns--)
  {
    fgets(pattern,PAT_SIZE,stdin);
    l = strlen(pattern);
    if(pattern[l-1] == '\n')
      pattern[l-1] = '\0';

    start_index = match(pattern,text);
    if(start_index == -1)
      printf("0\n");
    else
    {
      if(match_length == 0)
        match_length = 1;
        printf("1 %d %d\n",start_index,start_index + match_length - 1);
    }
  }
  return 0;
}
