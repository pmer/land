#include "lex.h"

#include "os/chrono.h"
#include "os/os.h"
#include "util/int.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"

// Reduce tests via https://en.wikipedia.org/wiki/Setjmp.h?

static void
stmtPrint(Token* tokens, Size size) noexcept {
    // Skip open parenthesis.
    tokens++;
    size--;

    // Only support a single string argument for now.
    StringView argument(tokens[0].data.string.data, tokens[0].data.string.size);

    sout << argument << '\n';
}

static void
stmt(Token* tokens, Size size) noexcept {
    /*
    sout << "stmt(tokens = [\n";
    for (Size i = 0; i < size; i++) {
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
        if (content[0] == 'g' && content == "greet") {
            sout << "Greetings!\n";
        }
        else if (content[0] == 'p' && content == "print") {
            stmtPrint(tokens + 1, size - 1);
        }
        break;
    case T_EOF:
    case T_INT:
    case T_PAREN_O:
    case T_PAREN_C:
    case T_SEMI:
    case T_STRING:
        break;
    }
}

static void
file(StringView path) noexcept {
    String data;
    //Nanoseconds start;

    //start = chronoNow();
    readFile(path, data);
    //serr << "readFile [" << ns_to_s_d(chronoNow() - start) << "s]\n";

    if (data.data == 0) {
        sout << "Cannot read file\n";
        return;
    }

    Vector<Token> tokens;
    Token* token;
    char* src = data.data;
    String buf;

    tokens.reserve(32);
    buf.reserve(256);

    //start = chronoNow();
    while (true) {
        tokens.grow();
        token = &tokens[tokens.size++];

        lexToken(token, &src, &buf);

        if (token->type == T_EOF) {
            break;
        }
        if (token->type == T_SEMI) {
            stmt(tokens.data, tokens.size - 1);
            tokens.size = 0;
            buf.size = 0;
        }
    }

    //serr << "lex+eval [" << ns_to_s_d(chronoNow() - start) << "s]\n";
}

int
main(int argc, char* argv[]) noexcept {
    Flusher f1(serr);
    Flusher f0(sout);

    if (argc == 1) {
        sout << "usage: land INPUT\n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        file(argv[i]);
    }

    return 0;
}
