#include <iostream>
#include <string>
#include <array>
#include <WDL/wdltypes.h>
#include <WDL/projectcontext.h>
#include <WDL/lineparse.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

class TerminalFormatter
{
public:
    static TerminalFormatter& getInstance()
    {
        static TerminalFormatter instance;
        return instance;
    }

    std::string grayText(const std::string& text) const
    {
        return m_supportsAnsi ? "\033[90m" + text + "\033[0m" : text;
    }

    std::string colorBracket(char bracket, int depth) const
    {
        if (!m_supportsAnsi) return std::string(1, bracket);

        // VSCode-like colors for brackets
        static const std::array<const char*, 6> colors = {
            "\033[38;2;255;215;0m",   // Gold      #ffd700
            "\033[38;2;218;112;214m", // Orchid    #da70d6
            "\033[38;2;23;159;255m",  // Blue      #179fff
            "\033[38;2;24;228;150m",  // Green     #18e496
            "\033[38;2;255;138;109m", // Coral     #ff8a6d
            "\033[38;2;78;205;196m"   // Turquoise #4ecdc4
        };

        int colorIndex = depth % colors.size();
        return std::string(colors[colorIndex]) + bracket + "\033[0m";
    }

private:
    TerminalFormatter()
    {
        m_supportsAnsi = checkAnsiSupport();
    }

    bool checkAnsiSupport()
    {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return false;

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) return false;
        
        return (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
        const char *term = getenv("TERM");
        return term != nullptr; // most Unix terminals support ANSI
#endif
    }

    bool m_supportsAnsi;
};

void setupConsole()
{
#ifdef _WIN32
    // set console output to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    
    // enable virtual terminal processing for ANSI escape sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    // set stdin/stdout to binary mode
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stdin), _O_BINARY);
#endif
}

void printProjectStructure(ProjectStateContext* ctx, int indent = 0)
{
    LineParser lp;
    TerminalFormatter &formatter = TerminalFormatter::getInstance();

    while (ProjectContext_GetNextLine(ctx, &lp)) {
        const char* token = lp.gettoken_str(0);
        bool isClosingBracket = (token[0] == '>');
        
        // print indentation (closing bracket uses parent's indentation)
        int currentIndent = isClosingBracket ? (indent - 1) : indent;
        for (int i = 0; i < currentIndent; i++) std::cout << formatter.grayText("⇥ ");
        
        // print the line content with colored brackets
        if (token[0] == '<') {
            std::cout << formatter.colorBracket(token[0], indent);
            std::cout << (token + 1); // print rest of the token
        } else if (token[0] == '>') {
            // Use parent level's color for closing bracket
            std::cout << formatter.colorBracket(token[0], indent - 1);
            std::cout << (token + 1); // print rest of the token
        } else {
            std::cout << token;
        }

        for (int i = 1; i < lp.getnumtokens(); i++) {
            std::cout << formatter.grayText("␣") << lp.gettoken_str(i);
        }
        std::cout << std::endl;

        // if this is a block start, recursively print its contents
        if (token[0] == '<') {
            printProjectStructure(ctx, indent + 1);
        }
        // if this is a block end, return to parent
        else if (token[0] == '>') {
            return;
        }
    }
}

int main(int argc, char* argv[])
{
    setupConsole();

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <rpp_file_path>" << std::endl;
        return 1;
    }

    const char *filename = argv[1];
    ProjectStateContext *ctx = ProjectCreateFileRead(filename);
    
    if (!ctx) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::cout << "Parsing RPP file: " << filename << std::endl;
    std::cout << "Structure:" << std::endl;
    printProjectStructure(ctx);

    delete ctx;
    return 0;
}
