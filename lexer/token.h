/* Token: tipos e lógica de tokenização (lexer) construída em cima
   do String_View (sv.h). Header-only, no mesmo padrão do io.h. */

#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "sv.h"
#include "da.h"
#include <stdio.h>
#include <ctype.h>

   /* Posição de um ponto no arquivo-fonte: caminho + linha + coluna.
      Existe só pra permitir mensagens de erro no formato
      "arquivo:linha:coluna: mensagem", igual compilador de verdade.
      row/column começam em 1 (convenção de editor), não em 0. */
typedef struct {
    const char* path;
    size_t row;
    size_t column;
} Location;

Location location_make(const char* path, const size_t row, const size_t column);

/* printf/fprintf para Location, no mesmo espírito de SV_Fmt/SV_Arg
   (sv.h): "%s:%zu:%zu" recebe path, row e column, nessa ordem.
   Uso: fprintf(stderr, Loc_Fmt ": could not parse\n", Loc_Arg(loc)); */
#define Loc_Fmt "%s:%zu:%zu:"
#define Loc_Arg(loc_) (loc_).path ? (loc_).path : "?", (loc_).row, (loc_).column


typedef struct
{
    String_View sv;
    Location location;
} Tokenizer;

Tokenizer tokenizer_make(const char* path, const String_View sv);

/*Categirua de token. */
typedef enum {
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_ID,
    TOKEN_STRING,
    TOKEN_EOF,
} Token_Kind;

typedef union {
    String_View symbol; /* usado por TOKEN_LPAREN / TOKEN_RPAREN */
    String_View id;      /* usado por TOKEN_ID */
    String_View string;  /* usado por TOKEN_STRING (sem as aspas) */
} Token_Value;

/*Um token extraído do arquivo - fonte: o que é(kind), o conteúdo
(value) e onde no arquivo ele começa(location, pra mensagens de
    erro do parser que vier depois). */
typedef struct {
    Token_Kind kind;
    Token_Value value;
    Location location;
} Token;

Token token_make(const Token_Kind kind, const Token_Value value, const Location location);

/* Array dinâmico de Token. Tem exatamente os campos que da.h (macro
   APPEND) espera -- data/count/capacity -- então dá pra construir
   uma lista de tokens usando APPEND(&tokens, token) em quem for
   fazer a análise léxica. */
typedef struct {
    Token* data;
    size_t count;
    size_t capacity;
} Tokens;

/* Remove e devolve o primeiro token de "tokens", avançando o
   array (data++, count--) -- não faz cópia/realloc, só desliza a
   janela sobre o buffer já alocado. Se "tokens" estiver vazio,
   devolve um Token TOKEN_EOF sem consumir nada. */
Token tokens_shift(Tokens* tokens);

/* Consome TODO o conteúdo restante de "tokenizer" (tokenizer->sv) e
   devolve a lista completa de tokens reconhecidos. Reconhece:
   '(' e ')' (TOKEN_LPAREN/TOKEN_RPAREN), identificadores
   ([A-Za-z_][A-Za-z0-9_]*, TOKEN_ID), strings entre aspas duplas
   sem suporte a escape (TOKEN_STRING, sem as aspas no value) e
   ignora espaços/tabs/quebras de linha, atualizando
   tokenizer->location (row/column) conforme avança. Se encontrar um
   caractere não reconhecido ou uma string sem fechamento, imprime
   um erro com a localização e encerra o processo (exit(1)). */
Tokens tokenizer_tokenize(Tokenizer* tokenizer);

#ifdef TOKEN_IMPLEMENTATION

Location
location_make(const char* path, const size_t row, const size_t column)
{
    return (Location) {
        .path = path,
            .row = row,
            .column = column,
    };
}

Tokenizer
tokenizer_make(const char* path, const String_View sv)
{
    return (Tokenizer) {
        .sv = sv,
            .location = location_make(path, 1, 1),
    };
}

Token
token_make(const Token_Kind kind, const Token_Value value, const Location location)
{
    return (Token) {
        .kind = kind,
            .value = value,
            .location = location,
    };
}

Token
tokens_shift(Tokens* tokens)
{
    if (tokens->count == 0) {
        return token_make(TOKEN_EOF, (Token_Value) { 0 }, location_make(NULL, 0, 0));
    }

    Token result = tokens->data[0];

    tokens->count = tokens->count - 1;
    tokens->data = tokens->data + 1;

    return result;
}

/* consome "n" bytes do início de tokenizer->sv, atualizando
   tokenizer->location (row/column) byte a byte -- assim '\n' sempre
   é contado certo, mesmo consumindo vários bytes de uma vez (ex.:
   dentro de uma string literal). */
static void
tokenizer_advance(Tokenizer* tokenizer, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (tokenizer->sv.data[i] == '\n') {
            tokenizer->location.row++;
            tokenizer->location.column = 1;
        }
        else {
            tokenizer->location.column++;
        }
    }
    tokenizer->sv.data += n;
    tokenizer->sv.count -= n;
}

static bool
token_is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool
token_is_id_start(char c)
{
    return isalpha((unsigned char)c) || c == '_';
}

static bool
token_is_id_cont(char c)
{
    return isalnum((unsigned char)c) || c == '_';
}

Tokens
tokenizer_tokenize(Tokenizer* tokenizer)
{
    Tokens tokens = { 0 };

    while (tokenizer->sv.count > 0) {
        char c = tokenizer->sv.data[0];

        if (token_is_space(c)) {
            tokenizer_advance(tokenizer, 1);
            continue;
        }

        Location location = tokenizer->location;

        if (c == '(' || c == ')') {
            String_View symbol = sv_make(tokenizer->sv.data, 1);
            Token_Kind kind = (c == '(') ? TOKEN_LPAREN : TOKEN_RPAREN;
            tokenizer_advance(tokenizer, 1);
            APPEND(&tokens, token_make(kind, (Token_Value) { .symbol = symbol }, location));
            continue;
        }

        if (c == '"') {
            tokenizer_advance(tokenizer, 1); /* consome a aspa de abertura */

            const char* start = tokenizer->sv.data;
            size_t len = 0;
            while (tokenizer->sv.count > 0 && tokenizer->sv.data[0] != '"') {
                tokenizer_advance(tokenizer, 1);
                len++;
            }

            if (tokenizer->sv.count == 0) {
                fprintf(stderr, Loc_Fmt " unterminated string literal\n", Loc_Arg(location));
                exit(1);
            }

            String_View string = sv_make(start, len);
            tokenizer_advance(tokenizer, 1); /* consome a aspa de fechamento */

            APPEND(&tokens, token_make(TOKEN_STRING, (Token_Value) { .string = string }, location));
            continue;
        }

        if (token_is_id_start(c)) {
            const char* start = tokenizer->sv.data;
            size_t len = 0;
            while (tokenizer->sv.count > 0 && token_is_id_cont(tokenizer->sv.data[0])) {
                tokenizer_advance(tokenizer, 1);
                len++;
            }

            String_View id = sv_make(start, len);
            APPEND(&tokens, token_make(TOKEN_ID, (Token_Value) { .id = id }, location));
            continue;
        }

        fprintf(stderr, Loc_Fmt " unexpected character '%c'\n", Loc_Arg(location), c);
        exit(1);
    }

    return tokens;
}

#endif /* TOKEN_IMPLEMENTATION */
#endif /* __TOKEN_H__ */