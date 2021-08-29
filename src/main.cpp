#include "os/os.h"
#include "util/int.h"
#include "util/io.h"
#include "util/string.h"
#include "util/string2.h"

// Reduce tests via https://en.wikipedia.org/wiki/Setjmp.h?

enum Type {
    T_EOF,
    T_IDENT,
    T_INT,
    T_SEMI,
    T_STRING,
};

struct IdentData {
    const char* data;
    size_t size;
};

struct IntegerData {
    int32_t value;
};

struct StringData {
    const char* data;
    size_t size;
};

union TokenData {
    struct IdentData ident;
    struct IntegerData integer;
    struct StringData string;
};

struct Token {
    enum Type type;
    union TokenData data;
};

const char* TYPE_NAMES[] = {
    "T_EOF",
    "T_IDENT",
    "T_INT",
    "T_SEMI",
    "T_STRING",
};

static void
printToken(Token* token) noexcept {
    sout << "Token(type=" << TYPE_NAMES[token->type];
    switch (token->type) {
    case T_IDENT:
        sout << ", data=" << StringView(token->data.ident.data, token->data.ident.size);
        break;
    case T_INT:
        sout << ", data=" << token->data.integer.value;
        break;
    case T_STRING:
        sout << ", data=" << StringView(token->data.string.data, token->data.string.size);
        break;
    }
    sout << ")\n";
}

static void
lexIdent(Token* token, char** src, String* buf) noexcept {
    char* data = *src;
    char c;

    do {
        c = *data++;
    } while (c && ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'));
    data--;

    size_t size = data - *src;
    *buf << StringView(*src, size);
    *src = data;

    token->type = T_IDENT;
    token->data.ident.data = buf->data + buf->size - size;
    token->data.ident.size = size;
}

static void
lexInteger(Token* token, char** src) noexcept {
    char* data = *src;
    char c;

    do {
        c = *data++;
    } while (c && '0' <= c && c <= '9');
    data--;

    size_t size = data - *src;

    int32_t value;
    char old = *data;
    *data = 0;
    parseInt0(&value, StringView(*src, size));
    *data = old;

    *src = data;

    token->type = T_INT;
    token->data.integer.value = value;
}

static void
lexString(Token* token, char** src, String* buf) noexcept {
    char* data = *src + 1;
    char c;
    size_t begin = buf->size;

    while (true) {
        c = *data++;
        if (c == 0 || c == '"') {
            break;
        }
        if (c == '\\') {
            switch (*data++) {
            case '"':
                c = '"';
                break;
            case '\\':
                c = '\\';
                break;
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            }
        }
        *buf << c;
    }

    *src = data;

    token->type = T_STRING;
    token->data.string.data = buf->data + begin;
    token->data.string.size = buf->size - begin;

    /*
    sout << "string() = ";
    printToken(token);
    */
}

static void
lex(Token* token, char** src, String* buf) noexcept {
again:
    char c = **src;

    switch (c) {
    case 0:
        token->type = T_EOF;
        break;
    case '\r':
        *src += 2;
        goto again;
    case '\n': case ' ': case '\t':
        *src += 1;
        goto again;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        lexInteger(token, src);
        break;
    case ';':
        *src += 1;
        token->type = T_SEMI;
        break;
    case '"':
        lexString(token, src, buf);
        break;
    default:
        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
            lexIdent(token, src, buf);
        }
        else {
            *src += 1;
            goto again;
        }
    }

    /*
    sout << "lex() = ";
    printToken(token);
    */
}

static void
stmt(Token* tokens, size_t size) noexcept {
    /*
    sout << "stmt(tokens = [\n";
    for (size_t i = 0; i < size; i++) {
        sout << "    ";
        printToken(tokens + i);
    }
    sout << "])\n";

    if (size == 0) {
        return;
    }
    */

    StringView content;

    switch (tokens[0].type) {
    case T_IDENT:
        content.data = tokens[0].data.ident.data;
        content.size = tokens[0].data.ident.size;
        if (content == "greet") {
            sout << "Greetings!\n";
        }
        break;
    case T_INT:
        break;
    case T_STRING:
        break;
    }
}

int
main() noexcept {
    String data;
    readFile("hi.dl", data);

    if (data.data == 0) {
        sout << "No file\n";
        return 1;
    }

    Vector<Token> tokens;
    Token* token;
    char* src = data.data;
    String buf;

    while (true) {
        tokens.grow();
        token = &tokens[tokens.size++];

        lex(token, &src, &buf);

        if (token->type == T_EOF) {
            break;
        }
        if (token->type == T_SEMI) {
            stmt(tokens.data, tokens.size - 1);
            tokens.size = 0;
            buf.size = 0;
        }
    }

    return 0;
}
