#include "lex.h"

#include "os/c.h"
#include "util/int.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/string2.h"

static
const char* TYPE_NAMES[] = {
    "T_EOF",
    "T_KW",
    "T_IDENT",
    "T_INT",
    "T_PAREN_O",
    "T_PAREN_C",
    "T_SEMI",
    "T_STRING",
};

static
const char* KEYWORD_NAMES[] = {
    "int",
    "return",
};

static void
printToken(Token* token) noexcept {
    sout << "Token(type=" << TYPE_NAMES[token->type];
    switch (token->type) {
    case T_KW:
        sout << ", data=" << KEYWORD_NAMES[token->data.kw.value];
        break;
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
lexSinglelineComment(char** src) noexcept {
    *src = static_cast<char*>(memchr(*src, '\n', SIZE_MAX / 2)) + 1;
}

static void
lexMultilineComment(char** src) noexcept {
    char* end = static_cast<char*>(memmem(*src, 1024 * 1024, "*/", 2));
    assert_(end != 0);
    if (end != 0) {
        *src = end + 2;
    }
    else {
        *src += strlen(*src);
    }
}

static void
lexComment(char** src) noexcept {
    switch ((*src)[1]) {
        case '*':
            lexMultilineComment(src);
            break;
        // Should be
        // case '/':
        default:
            lexSinglelineComment(src);
            break;
    }
}

static void
lexIdent(Token* token, char** src, String* buf) noexcept {
    char* data = *src;
    char c;

    do {
        c = *data++;
    } while (c && ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'));
    data--;

    Size size = data - *src;
    *buf << StringView(*src, size);
    *src = data;

    token->type = T_IDENT;
    token->data.ident.data = buf->data + buf->size - size;
    token->data.ident.size = size;
}

static void
lexKwIdent(Token* token, char** src, String* buf) noexcept {
    lexIdent(token, src, buf);

    struct IdentData ident = token->data.ident;
    const char* val = token->data.ident.data;
    Size size = token->data.ident.size;
    enum Keyword kw;

    switch (size) {
    case 3:
        if (val[0] == 'i' && val[1] == 'n' && val[2] == 't') {
            kw = KW_INT;
            goto isKw;
        }
        return;
    case 6:
        if (val[0] == 'r' && val[1] == 'e' && val[2] == 't' && val[3] == 'u' && val[4] == 'r' && val[5] == 'n') {
            kw = KW_RETURN;
            goto isKw;
        }
        return;
    default:
        return;
    }

isKw:
    token->type = T_KW;
    token->data.kw.value = kw;
}

static void
lexInteger(Token* token, char** src) noexcept {
    char* data = *src;
    char c;

    do {
        c = *data++;
    } while (c && '0' <= c && c <= '9');
    data--;

    Size size = data - *src;

    I32 value;
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
    Size begin = buf->size;

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

void
lexToken(Token* token, char** src, String* buf) noexcept {
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
    case '/':
        lexComment(src);
        goto again;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        lexInteger(token, src);
        break;
    case ';':
        *src += 1;
        token->type = T_SEMI;
        break;
    case '(':
        *src += 1;
        token->type = T_PAREN_O;
        break;
    case ')':
        *src += 1;
        token->type = T_PAREN_C;
        break;
    case '"':
        lexString(token, src, buf);
        break;
    default:
        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
            lexKwIdent(token, src, buf);
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
