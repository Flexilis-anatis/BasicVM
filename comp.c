#include "comp.h"
#include <stdio.h>
#include <string.h>

/*
if (x)
    print "hey";
else
    print "aw";

0 LOAD x
2 JMP 5
4 PUSH "hey"
6 PUTS


*/

#define EXPR(prec) expression(chunk, source, prec)
#define ERROR(msg, code) \
    do {                      \
        fprintf(stderr, msg); \
        exit(code);           \
    } while (0)
#define REQUIRE(tok, msg, code) \
    do {                               \
        if (!match_token(source, tok)) \
            ERROR(msg, code);          \
    } while (0);

static void expression(Chunk *chunk, Source *source, Prec prec);
static void statement(Chunk *chunk, Source *source);

// Get the index of a constant it. Add it if it's not found
static size_t jump_ind(Chunk *chunk, size_t jump_size) {
    size_t size = vector_size(chunk->jumps);
    for (size_t i = 0; i < size; ++i) {
        if (chunk->jumps[i] == jump_size)
            return i;
    }
    vector_push_back(chunk->jumps, jump_size);
    return size;
}

// Emit a constant. Makes sure it's not a repeat
static void emit_constant(Chunk *chunk, Value constant) {
    // Check to see if it's a repeat constant
    for (size_t i = 0; i < vector_size(chunk->data); ++i) {
        if (same_value(chunk->data[i], constant)) {
            emit_number(chunk, i);
            return;
        }
    }
    // If it's not a dup, add it to the data section AND emit it
    emit_value(chunk, constant);
}

static void emit_ident(Chunk *chunk, Lex ident) {
    // Check to see if it's a repeat ident
    for (size_t i = 0; i < vector_size(chunk->idents); ++i) {
        Lex data = chunk->idents[i];
        if (data.end - data.start == ident.end - ident.start &&
            memcmp(data.start, ident.start, data.end - data.start) == 0) {
            printf("Found: %lu\n", i);
            emit_number(chunk, i);
            return;
        }
    }
    vector_push_back(chunk->idents, ident);
    emit_number(chunk, vector_size(chunk->idents)-1);
}

static void parse_atom(Chunk *chunk, Source *source) {
    if (peek_token(source).id == TOK_NUMBER) {
        Token numtok = next_token(source);
        double number = strtod(numtok.lex.start, NULL);

        emit_byte(chunk, OP_PUSH);
        emit_constant(chunk, double_val(number));
    } else if (peek_token(source).id == TOK_STRING) {
        emit_byte(chunk, OP_PUSH);
        emit_constant(chunk, string_val((char *)next_token(source).lex.start));
    } else if (peek_token(source).id == TOK_IDENT) {
        emit_byte(chunk, OP_LOAD);
        emit_ident(chunk, next_token(source).lex);
    }
}

static void parse_paren(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_LPAREN)) {
        EXPR(PREC_NONE);
        REQUIRE(TOK_RPAREN, "No closing parenthesis in expression", -1);
    } else {
        EXPR(PREC_PAREN+1);
    }
}

static void parse_negate(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_SUB)) {
        EXPR(PREC_NEGATE);
        emit_byte(chunk, OP_NEG);
    } else {
        EXPR(PREC_NEGATE+1);
    }
}

static void parse_multiplication(Chunk *chunk, Source *source) {
    EXPR(PREC_MUL+1);
    uint8_t op;
    switch (peek_token(source).id) {
        case TOK_MOD:
            op = OP_MOD;
            break;
        case TOK_DIV:
            op = OP_DIV;
            break;
        case TOK_MUL:
            op = OP_MUL;
            break;
        default:
            return;
    }
    consume(source);

    EXPR(PREC_MUL+1);
    emit_byte(chunk, op);
    EXPR(PREC_NONE);
}

static void parse_addition(Chunk *chunk, Source *source) {
    EXPR(PREC_ADD+1);
    uint8_t op;
    switch (peek_token(source).id) {
        case TOK_ADD:
            op = OP_ADD;
            break;
        case TOK_SUB:
            op = OP_SUB;
            break;
        default:
            return;
    }
    consume(source);

    EXPR(PREC_ADD+1);
    emit_byte(chunk, op);
    EXPR(PREC_NONE);
}

#define EXPR_ENDER(id) ((id) == TOK_SEMICOLON || (id) == TOK_RPAREN || (id) == TOK_COMMA)
static void parse_assignment(Chunk *chunk, Source *source) {
    // Is there an identifier?
    if (peek_token(source).id == TOK_IDENT) {
        // Save it
        Token var = next_token(source);
        // Is there an equals sign?
        if (match_token(source, TOK_ASSIGN)) {
            // Is the next token a number or string?
            bool is_number = peek_token(source).id == TOK_NUMBER;
            if (is_number || peek_token(source).id == TOK_STRING) {
                Token val = next_token(source);
                Value value = is_number ? double_val(strtod(val.lex.start, NULL)) : 
                                          string_val((char *)val.lex.start);
                size_t index = write_value(chunk, value);

                // If it's just a number and a expr-ender, you can load it directly
                if (EXPR_ENDER(peek_token(source).id)) {
                    emit_byte(chunk, OP_CONST_STORE);
                    emit_ident(chunk, var.lex);
                    emit_number(chunk, index);
                }
                // Otherwise you have to evaluate it
                else {
                    emit_byte(chunk, OP_PUSH);
                    emit_number(chunk, index);
                    EXPR(PREC_NONE);
                    emit_byte(chunk, OP_STORE);
                    emit_ident(chunk, var.lex);
                }
            } else {
                EXPR(PREC_NONE);
                emit_byte(chunk, OP_STORE);
            }
        } else {
            emit_byte(chunk, OP_LOAD);
            emit_ident(chunk, var.lex);
            EXPR(PREC_ASSIGN+1);
        }
    } else {
        EXPR(PREC_ASSIGN+1);
    }
}

static void expression(Chunk *chunk, Source *source, Prec prec) {
    switch (prec) {
        case PREC_NONE:
        case PREC_ASSIGN:
            parse_assignment(chunk, source);
            break;
        case PREC_ADD:
            parse_addition(chunk, source);
            break;
        case PREC_MUL:
            parse_multiplication(chunk, source);
            break;
        case PREC_NEGATE:
            parse_negate(chunk, source);
            break;
        case PREC_PAREN:
            parse_paren(chunk, source);
            break;
        case PREC_ATOM:
            parse_atom(chunk, source);
            break;
    }
}

static size_t parse_block(Chunk *chunk, Source *source) {
    bool multiline = match_token(source, TOK_LBRACE);
    size_t orig_size = vector_size(chunk->code);

    do
        statement(chunk, source);
    while (multiline && !match_token(source, TOK_RBRACE));

    return vector_size(chunk->code) - orig_size;
}

static bool ifstmt(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_IF)) {
        REQUIRE(TOK_LPAREN, "expected ( after if statement", -7);
        EXPR(PREC_NONE);
        REQUIRE(TOK_RPAREN, "expected ) after if statement", -7);

        emit_byte(chunk, OP_COND_JMP);

        // Write the maximum possible number so we have enough space (plus one in case we need an else)
        size_t jumps_len = vector_size(chunk->jumps);
        // Save start of data and length of data
        size_t start = vector_size(chunk->code);
        emit_number(chunk, jumps_len+1);
        size_t data_len = vector_size(chunk->code)-start;

        // Record the body length
        size_t start_body = vector_size(chunk->code);
        parse_block(chunk, source);
        size_t body_len = vector_size(chunk->code)-start_body;

        bool iselse = match_token(source, TOK_ELSE);
        size_t index = jump_ind(chunk, body_len+iselse);

        // If new data was written
        if (vector_size(chunk->jumps) > jumps_len) {
            Chunk tmp = empty_chunk();
            emit_number(&tmp, index);
            // Zero out old data
            for (size_t i = 0; i < data_len; ++i)
                chunk->code[start+i] = 0;

            // Write in new data
            for (size_t i = vector_size(tmp.code); i > 0; --i)
                chunk->code[start+i-1] = tmp.code[i-1];
            
            vector_free(tmp.code);
        }

        if (iselse) {
            emit_byte(chunk, OP_CONST_JMP);
            size_t start = vector_size(chunk->code);
            size_t jumps_len = vector_size(chunk->jumps);
            emit_number(chunk, jumps_len+1);
            size_t data_len = vector_size(chunk->code)-start;
            
            size_t oldlen = vector_size(chunk->code);
            parse_block(chunk, source);
            size_t size = vector_size(chunk->code)-oldlen;

            size_t index = jump_ind(chunk, size);

            if (vector_size(chunk->jumps) > jumps_len) {
                Chunk tmp = empty_chunk();
                emit_number(&tmp, index);

                // Zero out old data
                for (size_t i = 0; i < data_len; ++i)
                    chunk->code[start+i] = 0;

                // Write in new data
                for (size_t i = vector_size(tmp.code); i > 0; --i)
                    chunk->code[start+i-1] = tmp.code[i-1];

                vector_free(tmp.code);
            }
        }

        return true;
    }
    return false;
}

static bool printstmt(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_PRINT)) {
        bool trailing_comma = true;
        do {
            EXPR(PREC_NONE);
            emit_byte(chunk, OP_PRINT);
            
            if (peek_token(source).id == TOK_SEMICOLON) {
                trailing_comma = false;
                break;
            }
        } while (match_token(source, TOK_COMMA) && !(peek_token(source).id == TOK_SEMICOLON));

        REQUIRE(TOK_SEMICOLON, "Expected semicolon after print statement", -5);

        if (!trailing_comma) {
            vector_pop_back(chunk->code);
            emit_byte(chunk, OP_PUTS);
        }
        return true;
    }
    return ifstmt(chunk, source);
}

static void exprstmt(Chunk *chunk, Source *source) {
    expression(chunk, source, PREC_NONE);
    REQUIRE(TOK_SEMICOLON, "Expected semicolon after expression", -5);
    emit_byte(chunk, OP_POP_TOP);
}

static void statement(Chunk *chunk, Source *source) {
    if (printstmt(chunk, source))
        return;
    exprstmt(chunk, source);
}

// Probably should make it take a Source object, but hey
Chunk compile(const char *code) {
    Source source = init_source(code);

    Chunk chunk = empty_chunk();
    while (peek_token(&source).id != TOK_EOF)
        statement(&chunk, &source);
    return chunk;
}
