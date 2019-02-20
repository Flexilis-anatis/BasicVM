#include "scanner.h"
#include "vec/vector.h"
#include <assert.h>

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

#define ISDIGIT(x) (((x) >= '0' && (x) <= '9') || (x) == '.')

static Token parse_number(Source *source) {
    while (ISDIGIT(*source->end))
        ++source->end;

    return token(TOK_NUMBER, source);
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
        default:
            return token(TOK_EOF, source);
    }
}
#undef CASE

static Token get_next_token(Source *source) {
    source->start = source->end;

    // Skip over whitespace. Not recording lines for now
    while (*source->end == ' ' || *source->end == '\t' || *source->end == '\n')
        source->start = ++source->end;

    // See if we're at the end
    if (*source->end == '\0')
        return token(TOK_EOF, source);

    if (ISDIGIT(*source->end))
        return parse_number(source);

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