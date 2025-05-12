#include <ncurses.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <limits>

// Color definitions
#define COLOR_COMMENT 1
#define COLOR_KEYWORD 2
#define COLOR_STRING 3
#define COLOR_NUMBER 4
#define COLOR_PREPROC 5
#define COLOR_DEFAULT 6

// Editor configuration
struct Editor {
    std::vector<std::string> lines;
    int cursorX = 0;
    int cursorY = 0;
    int screenX = 0;
    int screenY = 0;
    std::string filename;
    bool modified = false;
    int maxLines = 0;
    int maxCols = 0;
};

// C++ keywords for syntax highlighting
const std::vector<std::string> keywords = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
    "class", "compl", "concept", "const", "consteval", "constexpr", "const_cast",
    "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
    "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
    "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
    "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
    "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast",
    "struct", "switch", "template", "this", "thread_local", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void",
    "volatile", "wchar_t", "while", "xor", "xor_eq"
};

// Initialize ncurses and colors
void initEditor() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    
    // Initialize color pairs
    init_pair(COLOR_COMMENT, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_KEYWORD, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_STRING, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_NUMBER, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_PREPROC, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
}

// Check if a word is a C++ keyword
bool isKeyword(const std::string& word) {
    return std::binary_search(keywords.begin(), keywords.end(), word);
}

// Syntax highlighting for a line
void highlightLine(const std::string& line, int y) {
    int len = line.length();
    int i = 0;
    
    while (i < len) {
        // Skip whitespace
        if (isspace(line[i])) {
            move(y, i);
            addch(line[i]);
            i++;
            continue;
        }
        
        // Check for preprocessor directives
        if (i == 0 && line[i] == '#') {
            attron(COLOR_PAIR(COLOR_PREPROC));
            while (i < len) {
                move(y, i);
                addch(line[i]);
                i++;
            }
            attroff(COLOR_PAIR(COLOR_PREPROC));
            break;
        }
        
        // Check for comments
        if (i + 1 < len && line[i] == '/' && line[i+1] == '/') {
            attron(COLOR_PAIR(COLOR_COMMENT));
            while (i < len) {
                move(y, i);
                addch(line[i]);
                i++;
            }
            attroff(COLOR_PAIR(COLOR_COMMENT));
            break;
        }
        
        // Check for strings
        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            attron(COLOR_PAIR(COLOR_STRING));
            move(y, i);
            addch(line[i]);
            i++;
            
            while (i < len && line[i] != quote) {
                move(y, i);
                addch(line[i]);
                i++;
                // Handle escape sequences
                if (line[i-1] == '\\' && i < len) {
                    move(y, i);
                    addch(line[i]);
                    i++;
                }
            }
            
            if (i < len) {
                move(y, i);
                addch(line[i]);
                i++;
            }
            attroff(COLOR_PAIR(COLOR_STRING));
            continue;
        }
        
        // Check for numbers
        if (isdigit(line[i])) {
            attron(COLOR_PAIR(COLOR_NUMBER));
            while (i < len && (isdigit(line[i]) || line[i] == '.' || tolower(line[i]) == 'e' || 
                              line[i] == '+' || line[i] == '-' || line[i] == 'x')) {
                move(y, i);
                addch(line[i]);
                i++;
            }
            attroff(COLOR_PAIR(COLOR_NUMBER));
            continue;
        }
        
        // Check for keywords
        if (isalpha(line[i]) || line[i] == '_') {
            int start = i;
            while (i < len && (isalnum(line[i]) || line[i] == '_')) {
                i++;
            }
            std::string word = line.substr(start, i - start);
            
            if (isKeyword(word)) {
                attron(COLOR_PAIR(COLOR_KEYWORD));
                for (int j = start; j < i; j++) {
                    move(y, j);
                    addch(line[j]);
                }
                attroff(COLOR_PAIR(COLOR_KEYWORD));
            } else {
                attron(COLOR_PAIR(COLOR_DEFAULT));
                for (int j = start; j < i; j++) {
                    move(y, j);
                    addch(line[j]);
                }
                attroff(COLOR_PAIR(COLOR_DEFAULT));
            }
            continue;
        }
        
        // Default case
        attron(COLOR_PAIR(COLOR_DEFAULT));
        move(y, i);
        addch(line[i]);
        attroff(COLOR_PAIR(COLOR_DEFAULT));
        i++;
    }
}

// Render the editor content
void renderEditor(Editor& editor) {
    clear();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    editor.maxLines = maxY - 2;
    editor.maxCols = maxX;
    
    // Display file content with syntax highlighting
    int displayLines = std::min((int)editor.lines.size() - editor.screenY, editor.maxLines);
    for (int i = 0; i < displayLines; i++) {
        int lineNum = editor.screenY + i;
        if (lineNum < editor.lines.size()) {
            highlightLine(editor.lines[lineNum], i);
        }
    }
    
    // Status bar with exit options
    attron(A_REVERSE);
    mvprintw(maxY - 2, 0, "F2:Save F5:Run F10/Ctrl+X:Exit");
    std::string status = " " + editor.filename + " - " + std::to_string(editor.cursorY + 1) + 
                         "," + std::to_string(editor.cursorX + 1);
    if (editor.modified) {
        status += " [Modified]";
    }
    mvprintw(maxY - 2, 23, "%s", status.c_str());
    attroff(A_REVERSE);
    
    // Help bar
    mvprintw(maxY - 1, 0, "AOI Editor - Competitive Programming Editor | C++ Syntax Highlighting");
    
    // Position cursor
    move(editor.cursorY - editor.screenY, editor.cursorX - editor.screenX);
    refresh();
}

// Load file into editor
void loadFile(Editor& editor, const std::string& filename) {
    editor.filename = filename;
    editor.lines.clear();
    
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            editor.lines.push_back(line);
        }
        file.close();
    } else {
        // New file - start with one empty line
        editor.lines.push_back("");
    }
    editor.modified = false;
}

// Save editor content to file
void saveFile(Editor& editor) {
    std::ofstream file(editor.filename);
    if (file.is_open()) {
        for (const auto& line : editor.lines) {
            file << line << "\n";
        }
        file.close();
        editor.modified = false;
        
        // Show save confirmation
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        attron(A_REVERSE);
        mvprintw(maxY - 1, 0, "File saved successfully!");
        attroff(A_REVERSE);
        refresh();
        napms(1000);
    }
}

// Compile and run the current file
void compileAndRun(Editor& editor) {
    saveFile(editor);
    
    endwin();
    
    pid_t pid = fork();
    if (pid == 0) {
        std::string compileCmd = "g++ " + editor.filename + " -o a.out && ./a.out";
        execl("/bin/sh", "sh", "-c", compileCmd.c_str(), NULL);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        std::cout << "\nPress Enter to return to editor...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    initEditor();
}

// Handle cursor movement
void moveCursor(Editor& editor, int key) {
    switch (key) {
        case KEY_UP:
            if (editor.cursorY > 0) {
                editor.cursorY--;
                if (editor.cursorY < editor.screenY) {
                    editor.screenY--;
                }
                if (editor.cursorX > editor.lines[editor.cursorY].length()) {
                    editor.cursorX = editor.lines[editor.cursorY].length();
                }
            }
            break;
        case KEY_DOWN:
            if (editor.cursorY < editor.lines.size() - 1) {
                editor.cursorY++;
                if (editor.cursorY >= editor.screenY + editor.maxLines) {
                    editor.screenY++;
                }
                if (editor.cursorX > editor.lines[editor.cursorY].length()) {
                    editor.cursorX = editor.lines[editor.cursorY].length();
                }
            }
            break;
        case KEY_LEFT:
            if (editor.cursorX > 0) {
                editor.cursorX--;
                if (editor.cursorX < editor.screenX) {
                    editor.screenX--;
                }
            } else if (editor.cursorY > 0) {
                editor.cursorY--;
                editor.cursorX = editor.lines[editor.cursorY].length();
                if (editor.cursorX >= editor.maxCols) {
                    editor.screenX = editor.cursorX - editor.maxCols + 1;
                }
            }
            break;
        case KEY_RIGHT:
            if (editor.cursorX < editor.lines[editor.cursorY].length()) {
                editor.cursorX++;
                if (editor.cursorX >= editor.screenX + editor.maxCols) {
                    editor.screenX++;
                }
            } else if (editor.cursorY < editor.lines.size() - 1) {
                editor.cursorY++;
                editor.cursorX = 0;
                editor.screenX = 0;
                if (editor.cursorY >= editor.screenY + editor.maxLines) {
                    editor.screenY++;
                }
            }
            break;
        case KEY_HOME:
            editor.cursorX = 0;
            editor.screenX = 0;
            break;
        case KEY_END:
            editor.cursorX = editor.lines[editor.cursorY].length();
            if (editor.cursorX >= editor.maxCols) {
                editor.screenX = editor.cursorX - editor.maxCols + 1;
            }
            break;
    }
}

// Handle text input and editing
void handleInput(Editor& editor, int key) {
    switch (key) {
        case KEY_BACKSPACE:
        case 127:
            if (editor.cursorX > 0) {
                editor.lines[editor.cursorY].erase(editor.cursorX - 1, 1);
                editor.cursorX--;
                editor.modified = true;
            } else if (editor.cursorY > 0) {
                std::string prevLine = editor.lines[editor.cursorY - 1];
                editor.cursorX = prevLine.length();
                editor.lines[editor.cursorY - 1] += editor.lines[editor.cursorY];
                editor.lines.erase(editor.lines.begin() + editor.cursorY);
                editor.cursorY--;
                editor.modified = true;
            }
            break;
        case KEY_DC:
            if (editor.cursorX < editor.lines[editor.cursorY].length()) {
                editor.lines[editor.cursorY].erase(editor.cursorX, 1);
                editor.modified = true;
            } else if (editor.cursorY < editor.lines.size() - 1) {
                editor.lines[editor.cursorY] += editor.lines[editor.cursorY + 1];
                editor.lines.erase(editor.lines.begin() + editor.cursorY + 1);
                editor.modified = true;
            }
            break;
        case KEY_ENTER:
        case 10:
            {
                std::string left = editor.lines[editor.cursorY].substr(0, editor.cursorX);
                std::string right = editor.lines[editor.cursorY].substr(editor.cursorX);
                editor.lines[editor.cursorY] = left;
                editor.lines.insert(editor.lines.begin() + editor.cursorY + 1, right);
                editor.cursorY++;
                editor.cursorX = 0;
                editor.screenX = 0;
                editor.modified = true;
            }
            break;
        default:
            if (key >= 32 && key <= 126) {
                editor.lines[editor.cursorY].insert(editor.cursorX, 1, (char)key);
                editor.cursorX++;
                if (editor.cursorX >= editor.screenX + editor.maxCols) {
                    editor.screenX++;
                }
                editor.modified = true;
            }
            break;
    }
}

// Main editor loop
void runEditor(Editor& editor) {
    int key;
    while (true) {
        key = getch();
        
        // Exit conditions (F10 or Ctrl+X)
        if (key == KEY_F(10) || key == 24) {
            if (editor.modified) {
                // Prompt to save before exit if needed
                // (Implementation left as exercise)
            }
            break;
        }
        
        switch (key) {
            case KEY_F(2):
                saveFile(editor);
                break;
            case KEY_F(5):
                compileAndRun(editor);
                break;
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
                moveCursor(editor, key);
                break;
            default:
                handleInput(editor, key);
                break;
        }
        renderEditor(editor);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.cpp>" << std::endl;
        return 1;
    }
    
    Editor editor;
    initEditor();
    loadFile(editor, argv[1]);
    
    renderEditor(editor);
    runEditor(editor);
    
    endwin();
    return 0;
}
