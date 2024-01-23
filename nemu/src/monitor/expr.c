#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

extern uint32_t look_up_symtab(char *sym, bool *success);

enum
{
	NOTYPE = 256,
	EQ,
	NEQ,
	NUM,
	HEX,
	AND,
	OR,
	NOT,
	REG,
	SYMB,
	NEG,
	DEREF

	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"[a-zA-Z_][0-9a-zA-Z_]+", SYMB},
	{"0[xX][0-9a-fA-F]+", HEX},
	{"[0-9]+", NUM},
	{"\\+", '+'},
	{"-", '-'},
	{"\\*", '*'},
	{"\\/", '/'},
	{"\\(", '('},
	{"\\)", ')'},
	{"==", EQ},
	{"!=", NEQ},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", NOT}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type)
				{
				    case NUM:
				        memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
				        memcpy(tokens[nr_token].str, substr_start, substr_len);
				        tokens[nr_token].type = rules[i].token_type;
					    nr_token++;
				        break;
				        
				    case HEX:
				        memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
				        memcpy(tokens[nr_token].str, substr_start, substr_len);
				        tokens[nr_token].type = rules[i].token_type;
					    nr_token++;
				        break;
				    
				    case SYMB:
				        memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
				        memcpy(tokens[nr_token].str, substr_start, substr_len);
				        tokens[nr_token].type = rules[i].token_type;
					    nr_token++;
				        break;
				        
				    case NOTYPE:
				        break;
				    
				    default:
					    tokens[nr_token].type = rules[i].token_type;
					    nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

bool check_parantheses(int s, int e, bool *success)
{
    int pair = 0, i;
    for (i = s; i <= e; i++)
    {
        if (tokens[i].type == '(') pair++;
        if (tokens[i].type == ')') pair--;
        if (pair < 0)
        {
            *success = false;
            return false;
        }
    }
    if (pair != 0)
    {
        *success = false;
        return false;
    }
    
    
    if (tokens[s].type != '(' || tokens[e].type != ')')
        return false;
    
    for (i = s; i <= e; i++)
    {
        if (tokens[i].type == '(') pair++;
        if (tokens[i].type == ')') pair--;
        if (pair == 0) break;
    }
    if (i != e) return false;
    return true;

}

int dominant_operator(int s, int e)
{
    /* level:
    ||: 0;
    &&: 1;
    ==, !=: 2;
    +, -: 3;
    *, /: 4;
    !, NEG, DEREF: 5
    */
    int level = 6;
    int op = 0;
    for (int i = s; i <= e; i++)
    {
        switch (tokens[i].type)
        {
            case OR:
                if (level >= 0)
                {
                    level = 0;
                    op = i;
                }
            break;
            
            case AND:
                if (level >= 1)
                {
                    level = 1;
                    op = i;
                }
            break;
            
            case EQ:
            case NEQ:
                if (level >= 2)
                {
                    level = 2;
                    op = i;
                }
            break;
            
            case '+':
            case '-':
                if (level >= 3)
                {
                    level = 3;
                    op = i;
                }
            break;
        
            case '*':
            case '/':
                if (level >= 4)
                {
                    level = 4;
                    op = i;
                }
            break;
            
            case NOT:
            case NEG:
            case DEREF:
                if (level > 5)
                {
                    level = 5;
                    op = i;
                }
            break;
        
            case '(':
                while (i <= e && tokens[i].type != ')')
                    i++;
        }
    }
    return op;
}

uint32_t eval(int s, int e, bool *success)
{
    // Pre-processing
    if (s > e)
    {
        *success = false;
        return 0;
    }
    
    // <decimal-number>, <hexadecimal-number>, <symbol>
    else if (s == e)
    {
        if (tokens[s].type == NUM)
            return atoi(tokens[s].str);
        if (tokens[s].type == HEX)
        {
            int res = 0, i = 2;
            while (tokens[s].str[i] != '\0')
            {
                char c = tokens[s].str[i];
                res = res * 16;
                if (c >= '0' && c <= '9') res += c - '0';
                if (c >= 'a' && c <= 'f') res += c - 'a' + 10;
                if (c >= 'A' && c <= 'F') res += c - 'A' + 10;
                i++;
            }
            return res;
        }
        if (tokens[s].type == SYMB)
        {
            return look_up_symtab(tokens[s].str, success);
        }
        *success = false;
        return 0;
    }
    
    // case: (<expr>)
    else if (check_parantheses(s, e, success) == true)
    {
        return eval(s + 1, e - 1, success);
    }
    else if (*success == false)
    {
        return 0;
    }
    
    // case: <expr> + - * / == != && || ! <expr>
    else
    {
        int op = dominant_operator(s, e);
        if (tokens[op].type == NOT)
        {
            return !eval(op + 1, e, success);
        }
        if (tokens[op].type == NEG)
        {
            return -eval(op + 1, e, success);
        }
        if (tokens[op].type == DEREF)
        {
            return vaddr_read(eval(op + 1, e, success), 0x0, 4);
        }
        uint32_t val1 = eval(s, op - 1, success), val2 = eval(op + 1, e, success);
        switch (tokens[op].type)
        {
            case '+': return val1 + val2;
            case '-': return val1 - val2;
            case '*': return val1 * val2;
            case '/': return val1 / val2;
            case EQ: return val1 == val2;
            case NEQ: return val1 != val2;
            case AND: return val1 && val2;
            case OR: return val1 || val2;
            default:
                assert(0);
        }
    }
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
	
	for (int i = 0; i < nr_token; i++)
	{
	    if (tokens[i].type == '-' && (i == 0 || (tokens[i-1].type != NUM && tokens[i-1].type != HEX && tokens[i-1].type != SYMB && tokens[i-1].type != ')')))
	        tokens[i].type = NEG;
	        
	    if (tokens[i].type == '*' && (i == 0 || (tokens[i-1].type != NUM && tokens[i-1].type != HEX && tokens[i-1].type != SYMB && tokens[i-1].type != ')')))
	        tokens[i].type = DEREF;
	}
	
	*success = true;
	return eval(0, nr_token - 1, success);
}
