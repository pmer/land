#ifndef LEX_H_
#define LEX_H_

#include "util/int.h"
#include "util/string.h"

enum Type {
    T_EOF,
    T_IDENT,
    T_INT,
    T_PAREN_O,
    T_PAREN_C,
    T_SEMI,
    T_STRING,
};

// 8/16 bytes
struct IdentData {
    const char* data;
    Size size;
};

// 4 bytes
struct IntegerData {
    I32 value;
};

// 8/16 bytes
struct StringData {
    const char* data;
    Size size;
};

// 8/16 bytes
union TokenData {
    struct IdentData ident;
    struct IntegerData integer;
    struct StringData string;
};

// 12/20 bytes
struct Token {
    enum Type type;
    union TokenData data;
};

void
lexToken(Token* token, char** src, String* buf) noexcept;

#endif
