#include "tokenize.h"
#define internal static

internal inline void
EatAllWhiteSpace(struct tokenizer *Tokenizer)
{
    while((Tokenizer->At[0]) &&
          (IsWhiteSpace(Tokenizer->At[0])))
    {
        if(IsEndOfLine(Tokenizer->At[0]))
            ++Tokenizer->Line;

        ++Tokenizer->At;
    }
}

bool RequireToken(struct tokenizer *Tokenizer, enum token_type DesiredType)
{
    struct token Token = GetToken(Tokenizer);
    bool Result = Token.Type == DesiredType;
    return Result;
}

bool TokenEquals(struct token Token, const char *Match)
{
    const char *At = Match;
    for(int Index = 0;
        Index < Token.Length;
        ++Index, ++At)
    {
        if((*At == 0) ||
           (Token.Text[Index] != *At))
            return false;
    }

    bool Result = (*At == 0);
    return Result;
}

struct token
GetToken(struct tokenizer *Tokenizer)
{
    EatAllWhiteSpace(Tokenizer);

    struct token Token = {};
    Token.Length = 1;
    Token.Text = Tokenizer->At;

    char C = Tokenizer->At[0];
    ++Tokenizer->At;

    switch(C)
    {
        case '\0': { Token.Type = Token_EndOfStream; } break;
        case '{': { Token.Type = Token_OpenBrace; } break;
        case '}': { Token.Type = Token_CloseBrace; } break;
        case '+': { Token.Type = Token_Plus; } break;
        case '!': { Token.Type = Token_Negate; } break;
        case '[':
        {
            Token.Text = Tokenizer->At;

            while((Tokenizer->At[0]) &&
                  (Tokenizer->At[0] != ']'))
            {
                ++Tokenizer->At;
            }

            Token.Type = Token_List;
            Token.Length = Tokenizer->At - Token.Text;

            if(Tokenizer->At[0] == ']')
                ++Tokenizer->At;
        } break;
        case ':':
        {
            EatAllWhiteSpace(Tokenizer);
            Token.Text = Tokenizer->At;

            while((Tokenizer->At[0]) &&
                  (!IsEndOfLine(Tokenizer->At[0])))
            {
                if(Tokenizer->At[0] == '\\')
                {
                    ++Tokenizer->At;
                    if(IsEndOfLine(Tokenizer->At[0]))
                        ++Tokenizer->Line;
                }

                ++Tokenizer->At;
            }

            Token.Type = Token_Command;
            Token.Length = Tokenizer->At - Token.Text;
        } break;
        case '#':
        {
            Token.Text = Tokenizer->At;
            while((Tokenizer->At[0]) &&
                  (!IsEndOfLine(Tokenizer->At[0])))
                ++Tokenizer->At;

            Token.Type = Token_Comment;
            Token.Length = Tokenizer->At - Token.Text;
        } break;
        case '-':
        {
            if(Tokenizer->At[0])
            {
                if(Tokenizer->At[0] == '>')
                {
                    ++Tokenizer->At;
                    Token.Type = Token_Passthrough;
                    Token.Length = Tokenizer->At - Token.Text;
                }
                else
                {
                    EatAllWhiteSpace(Tokenizer);
                    Token.Text = Tokenizer->At;
                    if((IsAlpha(Tokenizer->At[0])) ||
                       (IsNumeric(Tokenizer->At[0])))
                    {
                        ++Tokenizer->At;
                        if((Tokenizer->At[0]) &&
                           (Tokenizer->At[0] == 'x' ||
                            Tokenizer->At[0] == 'X'))
                        {
                            ++Tokenizer->At;
                            while((Tokenizer->At[0]) &&
                                  (IsHexadecimal(Tokenizer->At[0])))
                                ++Tokenizer->At;

                            Token.Type = Token_Hex;
                            Token.Length = Tokenizer->At - Token.Text;
                        }
                        else
                        {
                            while((Tokenizer->At[0]) &&
                                  (IsAlpha(Tokenizer->At[0]) ||
                                  (IsNumeric(Tokenizer->At[0]))))
                                ++Tokenizer->At;

                            Token.Type = Token_Literal;
                            Token.Length = Tokenizer->At - Token.Text;
                        }
                    }
                }
            }
        } break;
        default:
        {
            if(IsAlpha(C))
            {
                while((IsAlpha(Tokenizer->At[0])) ||
                      (IsNumeric(Tokenizer->At[0])))
                    ++Tokenizer->At;

                Token.Type = Token_Identifier;
                Token.Length = Tokenizer->At - Token.Text;
            }
            else if(IsNumeric(C))
            {
                if((C == '0') &&
                   (Tokenizer->At[0] == 'x' ||
                    Tokenizer->At[0] == 'X'))
                {
                    ++Tokenizer->At;
                    while((Tokenizer->At[0]) &&
                          (IsHexadecimal(Tokenizer->At[0])))
                        ++Tokenizer->At;

                    Token.Type = Token_Hex;
                    Token.Length = Tokenizer->At - Token.Text;
                }
                else
                {
                    while((Tokenizer->At[0]) &&
                          (IsNumeric(Tokenizer->At[0]) ||
                           IsDot(Tokenizer->At[0])))
                        ++Tokenizer->At;

                    Token.Type = Token_Digit;
                    Token.Length = Tokenizer->At - Token.Text;
                }
            }
            else
            {
                Token.Type = Token_Unknown;
            }
        } break;
    }

    return Token;
}
