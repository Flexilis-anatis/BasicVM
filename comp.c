#include "comp.h"
#include "func.h"
#include "closure.h"
#include <stdio.h>
#include <string.h>

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

static void parse_call(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_LPAREN)) {
        size_t arity = 0;
        if (peek_token(source).id != TOK_RPAREN) {
            do {
                EXPR(PREC_NONE);
                ++arity;
            } while (match_token(source, TOK_COMMA));
        }
        REQUIRE(TOK_RPAREN, "expected right parenthesis in function call", 2);

        emit_byte(chunk, OP_CALL);
        emit_number(chunk, arity);
        parse_call(chunk, source);
    }
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
        parse_call(chunk, source);
        EXPR(PREC_NONE);
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
            EXPR(PREC_MUL+1);
            return;
    }
    consume(source);

    EXPR(PREC_MUL+1);
    emit_byte(chunk, op);
    EXPR(PREC_NONE);
}

static void parse_addition_no_lhs(Chunk *chunk, Source *source) {
    uint8_t op;
    switch (peek_token(source).id) {
        case TOK_ADD:
            op = OP_ADD;
            break;
        case TOK_SUB:
            op = OP_SUB;
            break;
        default:
            EXPR(PREC_ADD+1);
            return;
    }
    consume(source);

    EXPR(PREC_ADD+1);
    emit_byte(chunk, op);
    EXPR(PREC_ADD+1);
}

static void parse_addition(Chunk *chunk, Source *source) {
    EXPR(PREC_ADD+1);
    parse_addition_no_lhs(chunk, source);
}

static void parse_logic_no_lhs(Chunk *chunk, Source *source) {
    uint8_t op;
    switch (peek_token(source).id) {
        case TOK_LT:
            op = OP_LT;
            break;
        default:
            parse_addition_no_lhs(chunk, source);
            return;
    }
    consume(source);

    EXPR(PREC_LOGIC+1);
    emit_byte(chunk, op);
    EXPR(PREC_NONE);
 }

static void parse_logic(Chunk *chunk, Source *source) {
    EXPR(PREC_LOGIC+1);
    parse_logic_no_lhs(chunk, source);
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
                emit_ident(chunk, var.lex);
            }
        } else {
            emit_byte(chunk, OP_LOAD);
            emit_ident(chunk, var.lex);
            parse_call(chunk, source);
            parse_logic_no_lhs(chunk, source);
            EXPR(PREC_NONE);
        }
    } else {
        EXPR(PREC_ASSIGN+1);
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

static void parse_closure(Chunk *chunk, Source *source) {
    bool isclosure = false;
    if (match_token(source, TOK_FUNCTION) || (isclosure = match_token(source, TOK_CLOSURE))) {
        Func *func = malloc(sizeof(Func));
        func->chunk = empty_chunk(chunk->consts);
        func->params = NULL;

        // parse param list
        REQUIRE(TOK_LPAREN, "Expected left parenthesis for function declaration", -21);
        if (peek_token(source).id == TOK_IDENT) {
            do {
                Token ident = next_token(source);
                if (ident.id != TOK_IDENT)
                    ERROR("Only identifiers allowed in parameter declarations", -20);
                vector_push_back(func->params, ident.lex);
            } while (match_token(source, TOK_COMMA));
        }
        REQUIRE(TOK_RPAREN, "Expected right parenthesis for function declaration", -22);

        // Body
        if (peek_token(source).id != TOK_LBRACE)
            ERROR("Expected left brace for function declaration", 1);
        parse_block(&func->chunk, source);

        if (isclosure) {
            Closure *closure = malloc(sizeof(Closure));
            closure->func = func;

            Value close_val;
            close_val.p = 0;
            SET_TYPE(close_val, TYPE_CLOSURE);
            SET_VALUE(close_val, closure);

            emit_byte(chunk, OP_BIND);
            emit_constant(chunk, close_val);
            parse_call(chunk, source);
        } else {
            // Set up value pointer
            Value func_val;
            func_val.p = 0;
            SET_TYPE(func_val, TYPE_FUNC);
            SET_VALUE(func_val, func);

            // Add the bytecode! :D
            emit_byte(chunk, OP_PUSH);
            emit_constant(chunk, func_val);
            parse_call(chunk, source);
        }
    } else {
        EXPR(PREC_CLOSURE+1);
    }
}

static void expression(Chunk *chunk, Source *source, Prec prec) {
    switch (prec) {
        case PREC_NONE:
            expression(chunk, source, prec+1);
            break;
        case PREC_ASSIGN:
            parse_assignment(chunk, source);
            break;
        case PREC_CLOSURE:
            parse_closure(chunk, source);
            break;
        case PREC_LOGIC:
            parse_logic(chunk, source);
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

static bool retstmt(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_RETURN)) {
        EXPR(PREC_NONE);
        REQUIRE(TOK_SEMICOLON, "Expected semicolon after return statement", 3);
        emit_byte(chunk, OP_RETURN);
        return true;
    }
    return false;
}

static void emit_max_jump(Chunk *chunk) {
    emit_number(chunk, vector_size(chunk->consts->jumps));
}

static bool ifstmt(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_IF)) {
        REQUIRE(TOK_LPAREN, "expected ( after if statement", -7);
        EXPR(PREC_NONE);
        REQUIRE(TOK_RPAREN, "expected ) after if statement", -7);

        emit_byte(chunk, OP_COND_JMP);

        // Save start of data and length of data
        size_t start = vector_size(chunk->code);
        emit_max_jump(chunk);
        size_t data_len = vector_size(chunk->code)-start;

        // Record the body length
        size_t start_body = vector_size(chunk->code);
        parse_block(chunk, source);
        size_t body_len = vector_size(chunk->code)-start_body;

        bool iselse = match_token(source, TOK_ELSE);
        unsigned long bits = BITS(vector_size(chunk->consts->jumps));
        
        size_t index = jump_ind(chunk, body_len+(iselse?BYTES(bits)+1:0));
        // new scope just cause it's messy
        {
            Chunk tmp = empty_chunk(chunk->consts);
            emit_number(&tmp, index);
            // Zero out old data
            for (size_t i = 0; i < data_len; ++i)
                chunk->code[start+i] = 0;

            // Write in new data
            for (size_t i = vector_size(tmp.code); i > 0; --i) {
                chunk->code[start+i-1] = tmp.code[i-1];
            }

            vector_free(tmp.code);
        }

        if (iselse) {
            emit_byte(chunk, OP_CONST_JMP);

            size_t start = vector_size(chunk->code);
            emit_max_jump(chunk);
            size_t data_len = vector_size(chunk->code)-start;
            
            size_t oldlen = vector_size(chunk->code);
            parse_block(chunk, source);
            size_t size = vector_size(chunk->code)-oldlen;

            size_t index = jump_ind(chunk, size);

            {
                Chunk tmp = empty_chunk(chunk->consts);
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
    return retstmt(chunk, source);
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
    size_t start = vector_size(chunk->code);
    expression(chunk, source, PREC_NONE);
    if (vector_size(chunk->code)-start > 0) {
        REQUIRE(TOK_SEMICOLON, "Expected semicolon after expression", -5);
        emit_byte(chunk, OP_POP_TOP);
    }
}

static void statement(Chunk *chunk, Source *source) {
    if (printstmt(chunk, source))
        return;
    exprstmt(chunk, source);
}

// Probably should make it take a Source object, but hey
Chunk compile(const char *code) {
    Source source = init_source(code);

    Consts *consts = empty_consts();
    Chunk chunk = empty_chunk(consts);
    while (peek_token(&source).id != TOK_EOF)
        statement(&chunk, &source);
    return chunk;
}
