#ifndef SYSBVM_TOKEN_H
#define SYSBVM_TOKEN_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_token_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t kind;
    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t value;
} sysbvm_token_t;

typedef enum sysbvm_tokenKind_e
{
    SYSBVM_TOKEN_KIND_CHARACTER,
    SYSBVM_TOKEN_KIND_FLOAT,
    SYSBVM_TOKEN_KIND_IDENTIFIER,
    SYSBVM_TOKEN_KIND_INTEGER,
    SYSBVM_TOKEN_KIND_KEYWORD,
    SYSBVM_TOKEN_KIND_MULTI_KEYWORD,
    SYSBVM_TOKEN_KIND_OPERATOR,
    SYSBVM_TOKEN_KIND_STRING,
    SYSBVM_TOKEN_KIND_SYMBOL,

    SYSBVM_TOKEN_KIND_LPARENT,
    SYSBVM_TOKEN_KIND_RPARENT,
    SYSBVM_TOKEN_KIND_LBRACKET,
    SYSBVM_TOKEN_KIND_RBRACKET,
    SYSBVM_TOKEN_KIND_LCBRACKET,
    SYSBVM_TOKEN_KIND_RCBRACKET,

    SYSBVM_TOKEN_KIND_LESS_THAN,
    SYSBVM_TOKEN_KIND_GREATER_THAN,
    SYSBVM_TOKEN_KIND_STAR,

    SYSBVM_TOKEN_KIND_COLON,
    SYSBVM_TOKEN_KIND_COLON_COLON,
    SYSBVM_TOKEN_KIND_BAR,

    SYSBVM_TOKEN_KIND_ASSIGNMENT,
    SYSBVM_TOKEN_KIND_SEMICOLON,
    SYSBVM_TOKEN_KIND_COMMA,
    SYSBVM_TOKEN_KIND_DOT,
    SYSBVM_TOKEN_KIND_ELLIPSIS,

    SYSBVM_TOKEN_KIND_QUOTE,
    SYSBVM_TOKEN_KIND_QUASI_QUOTE,
    SYSBVM_TOKEN_KIND_QUASI_UNQUOTE,
    SYSBVM_TOKEN_KIND_SPLICE,

    SYSBVM_TOKEN_KIND_BYTE_ARRAY_START,
    SYSBVM_TOKEN_KIND_DICTIONARY_START,
    SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START,

    SYSBVM_TOKEN_KIND_ERROR,
} sysbvm_tokenKind_t;

/**
 * Creates a token.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_token_create(sysbvm_context_t *context, sysbvm_tuple_t kind, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value);

/**
 * Creates a token with the specified kind.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_token_createWithKind(sysbvm_context_t *context, sysbvm_tokenKind_t kind, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value);

/**
 * Gets the kind of a token.
 */
SYSBVM_API sysbvm_tokenKind_t sysbvm_token_getKind(sysbvm_tuple_t token);

/**
 * Gets the source position of a token.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_token_getSourcePosition(sysbvm_tuple_t token);

/**
 * Gets the value of a token.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_token_getValue(sysbvm_tuple_t token);

#endif //SYSBVM_TOKEN_H
