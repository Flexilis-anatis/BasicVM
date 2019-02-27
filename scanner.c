#include "scanner.h"
#include "vec/vector.h"
#include <string.h>
#include <stdio.h> // fprintf(stderr, ...)

Source init_source(const char *source) {
    Source s;
    s.end = s.start = s.source = source;
    s.private.peek = false;
    return s;
}

static Token token(TokID type, Source *source) {
    Token tok;
    tok.id = type;
    tok.lex.start = source->start;
    tok.lex.end = source->end;
    return tok;
}

#define INRANGE(x,l,h) ((x) >= l && (x) <= h)
#define ISIDENTCHAR(x) (INRANGE((x), 'a', 'z') || \
                        INRANGE((x), 'A', 'Z') || \
                        INRANGE((x), '0', '9') || \
                        (x) == '_')
#define ISDIGIT(x) (INRANGE((x),'0','9') || (x) == '.')

Token parse_number(Source *source) {
    while (ISDIGIT(*source->end))
        ++source->end;

    return token(TOK_NUMBER, source);
}

static Token match_kwd(const char *string, size_t length, Source *source, TokID tok) {
    if (source->start+length+1 == source->end &&
        memcmp(source->start+1, string, length) == 0)
        return token(tok, source);
    return token(TOK_IDENT, source);
}

static Token parse_ident(Source *source) {
    while(ISIDENTCHAR(*source->end))
        ++source->end;

    switch (*source->start) {
        case 'a':
            return match_kwd("nd", 2, source, TOK_AND);
        case 'c':
            if (source->end - source->start == 5) {
                if (*(source->start+2) == 'a')
                    return match_kwd("lass", 4, source, TOK_CLASS);
                return token(TOK_IDENT, source);
            }
            return match_kwd("losure", 6, source, TOK_CLOSURE);
        case 'e':
            return match_kwd("lse", 3, source, TOK_ELSE);
        case 'f':
            if (*(source->start+1) == 'a')
                return match_kwd("alse", 4, source, TOK_FALSE);
            return match_kwd("unction", 7, source, TOK_FUNCTION);
        case 'i':
            if (*(source->start+1) == 's')
                return match_kwd("s", 1, source, TOK_IS);
            return match_kwd("f", 1, source, TOK_IF);
        case 'n':
            if (*(source->start+1) == 'i')
                return match_kwd("il", 2, source, TOK_NIL);
            else if (*(source->start+1) == 'e')
                return match_kwd("ew", 2, source, TOK_NEW);
            return match_kwd("ot", 2, source, TOK_NOT);
        case 'o':
            return match_kwd("r", 1, source, TOK_OR);
        case 'p':
            return match_kwd("rint", 4, source, TOK_PRINT);
        case 'r':
            return match_kwd("eturn", 5, source, TOK_RETURN);
        case 't':
            return match_kwd("rue", 3, source, TOK_TRUE);
        case 'w':
            return match_kwd("hile", 4, source, TOK_WHILE);
    }
    return token(TOK_IDENT, source);
}

// More macro abuse! :D
#define CASE(char, type) case char: return token(TOK_##type, source)
static Token parse_other(Source *source) {
    switch(*source->end++) {
        CASE('(', LPAREN);
        CASE(')', RPAREN);
        CASE('+', ADD);
        CASE('-', SUB);
        CASE('*', MUL);
        CASE('/', DIV);
        CASE('%', MOD);
        CASE(';', SEMICOLON);
        CASE(',', COMMA);
        CASE('{', LBRACE);
        CASE('}', RBRACE);
        CASE('.', DOT);
        case '<':
            if (*source->end == '=') {
                ++source->end;
                return token(TOK_LTE, source);
            }
            return token(TOK_LT, source);
        case '>':
            if (*source->end == '=') {
                ++source->end;
                return token(TOK_GTE, source);
            }
            return token(TOK_GT, source);
        case '=':
            if (*source->end == '=') {
                ++source->end;
                return token(TOK_EQU, source);
            }
            return token(TOK_ASSIGN, source);
        default:
            return parse_ident(source);
    }
}
#undef CASE

static Token parse_string(Source *source) {
    ++source->start;
    char last = '\0';
    do
        last = *source->end++;
    while (*source->end != '"' || last == '\\');
    
    Token t = token(TOK_STRING, source);
    ++source->end;
    t.lex.start = delexify(t.lex);
    t.lex.end = t.lex.start+(source->end-source->start);
    return t;
}

static void parse_line_comment(Source *source) {
    while (*++source->end != '\n' && *source->end != '\0');
}

static void parse_multiline_comment(Source *source) {
    do {
        while (*++source->end != '*')
            if (*source->end == '\0')
                goto end;
    } while (*++source->end != '/' && *source->end != '\0');
end:
    if (*source->end == '\0') {
        fprintf(stderr, "Unterminated multi-line comment\n");
        exit(17); // random, I'll admit
    }
    ++source->end;
}

static Token get_next_token(Source *source) {
    source->start = source->end;

    // Skip over whitespace. Not recording lines for now
    while (*source->end == ' ' || *source->end == '\t' || *source->end == '\n')
        source->start = ++source->end;

    // See if we're at the end
    if (*source->end == '\0')
        return token(TOK_EOF, source);

    if (*source->end == '"') {
        return parse_string(source);
    } else if (ISDIGIT(*source->end)) {
        return parse_number(source);
    } else if (*source->end == '/') {
        if (*(source->end+1) == '/')
            parse_line_comment(source);
        else if (*(source->end+1) == '*')
            parse_multiline_comment(source);
        return get_next_token(source);
    }
    return parse_other(source);
}

// This is where the peeking happens
Token next_token(Source *source) {
    if (source->private.peek) {
        consume(source);
        return source->private.last;
    }
    return source->private.last = get_next_token(source);
}

Token peek_token(Source *source) {
    Token token = next_token(source);
    source->private.peek = true;
    return token;
}

void consume(Source *source) {
    source->private.peek = false;
}

bool match_token(Source *source, TokID id) {
    if (peek_token(source).id != id)
        return false;
    consume(source);
    return true;
}

char *delexify(Lex lex) {
    size_t len = lex.end-lex.start;
    char *string = malloc(len+1);
    size_t current = 0;
    for (size_t i = 0; i < len; ++i, ++current) {
        char c = lex.start[i];
        if (c == '\\') {
            switch (lex.start[++i]) {
                case '\\':
                    string[current] = '\\';
                    break;
                case 'n':
                    string[current] = '\n';
                    break;
                case '"':
                    string[current] = '"';
                    break;
                default:
                    string[current] = '\\';
                    string[++current] = c;
                    break;
            }
        } else {
            string[current] = c;
        }
    }
    string[current] = '\0';
    return string;
}