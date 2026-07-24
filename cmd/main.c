#include "pch.h"
#include "sdk.h"

int
main(int argc, char** argv)
{
    (void)argc;
    (void)argv[0];

    if (argc < 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        return 1;
    }

    const char* path = argv[1];

    String_View source;
    Errno err = io_read_file(path, &source);
    if (err != 0) {
        VTableError_File(TYPE_ERROR_FILE_NOT_FOUND, "could not read source file", path);
        return 1;
    }

    Tokenizer tokenizer = tokenizer_make(path, source);
    Tokens tokens = tokenizer_tokenize(&tokenizer);

    AST* program = parser_ast_from_tokens(tokens);
    /* ast_dump(program, 0); */ /* descomente pra ver a AST no stdout */

    FILE* fp = fopen("a.c", "wb");
    if (fp == NULL) {
        fprintf(stderr, "could not open file: %s\n", strerror(errno));
        free(tokens.data);
        free((void*)source.data);
        return 1;
    }

    cg_ast_to_c(fp, program);
    fclose(fp);

    free(tokens.data);
    free((void*)source.data);

    return 0;
}