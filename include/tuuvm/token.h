#ifndef TUUVM_TOKEN_H
#define TUUVM_TOKEN_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_token_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t kind;
    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t value;
} tuuvm_token_t;

typedef enum tuuvm_tokenKind_e
{
    TUUVM_TOKEN_KIND_CHARACTER,
    TUUVM_TOKEN_KIND_FLOAT,
    TUUVM_TOKEN_KIND_IDENTIFIER,
    TUUVM_TOKEN_KIND_INTEGER,
    TUUVM_TOKEN_KIND_KEYWORD,
    TUUVM_TOKEN_KIND_MULTI_KEYWORD,
    TUUVM_TOKEN_KIND_OPERATOR,
    TUUVM_TOKEN_KIND_STRING,
    TUUVM_TOKEN_KIND_SYMBOL,

    TUUVM_TOKEN_KIND_LPARENT,
    TUUVM_TOKEN_KIND_RPARENT,
    TUUVM_TOKEN_KIND_LBRACKET,
    TUUVM_TOKEN_KIND_RBRACKET,
    TUUVM_TOKEN_KIND_LCBRACKET,
    TUUVM_TOKEN_KIND_RCBRACKET,

    TUUVM_TOKEN_KIND_LESS_THAN,
    TUUVM_TOKEN_KIND_GREATER_THAN,
    TUUVM_TOKEN_KIND_STAR,

    TUUVM_TOKEN_KIND_COLON,
    TUUVM_TOKEN_KIND_COLON_COLON,
    TUUVM_TOKEN_KIND_BAR,

    TUUVM_TOKEN_KIND_ASSIGNMENT,
    TUUVM_TOKEN_KIND_SEMICOLON,
    TUUVM_TOKEN_KIND_COMMA,
    TUUVM_TOKEN_KIND_DOT,
    TUUVM_TOKEN_KIND_ELLIPSIS,

    TUUVM_TOKEN_KIND_QUOTE,
    TUUVM_TOKEN_KIND_QUASI_QUOTE,
    TUUVM_TOKEN_KIND_QUASI_UNQUOTE,
    TUUVM_TOKEN_KIND_SPLICE,

    TUUVM_TOKEN_KIND_BYTE_ARRAY_START,
    TUUVM_TOKEN_KIND_DICTIONARY_START,
    TUUVM_TOKEN_KIND_LITERAL_ARRAY_START,

    TUUVM_TOKEN_KIND_ERROR,
} tuuvm_tokenKind_t;

/**
 * Creates a token.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_token_create(tuuvm_context_t *context, tuuvm_tuple_t kind, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value);

/**
 * Creates a token with the specified kind.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_token_createWithKind(tuuvm_context_t *context, tuuvm_tokenKind_t kind, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value);

/**
 * Gets the kind of a token.
 */
TUUVM_API tuuvm_tokenKind_t tuuvm_token_getKind(tuuvm_tuple_t token);

/**
 * Gets the source position of a token.
 */
TUUVM_API tuuvm_tuple_t tuuvm_token_getSourcePosition(tuuvm_tuple_t token);

/**
 * Gets the value of a token.
 */
TUUVM_API tuuvm_tuple_t tuuvm_token_getValue(tuuvm_tuple_t token);

#endif //TUUVM_TOKEN_H
