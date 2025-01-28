#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <ncurses.h>
#include <algorithm>

namespace fs = std::filesystem;
std::vector<std::string> executeCommand(const std::string& command) {
    std::vector<std::string> output;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return output;

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output.emplace_back(buffer);
    }
    pclose(pipe);

    for (std::string& line : output) {
        line.erase(line.find_last_not_of("\n") + 1);
    }

    return output;
}

void executeGitCommand(const std::string& command, const std::string& filename) {
    std::string fullCommand = command + " \"" + filename + "\"";
    int result = std::system(fullCommand.c_str());
    if (result != 0) {
        printw("Failed to execute command on '%s'.\n", filename.c_str());
    } else {
        printw("Command executed successfully on '%s'.\n", filename.c_str());
    }
}

void drawUI(WINDOW* listWin, const std::vector<std::string>& files, const std::vector<std::string>& assumeUnchangedFiles, size_t current_selection, size_t current_tab, size_t scroll_offset) {
    werase(stdscr);
    werase(listWin);
    attron(COLOR_PAIR(3));
    mvprintw(1, 2, "Git Assume-Unchanged Manager");

    mvprintw(2, 2, "---------------------------------------------------");
    mvprintw(3, 2, "Instructions:");
    mvprintw(4, 4, "- UP/DOWN: Navigate the list");
    mvprintw(5, 4, "- ENTER: Toggle assume-unchanged");
    mvprintw(6, 4, "- TAB: Switch between add/remove tabs");
    mvprintw(7, 4, "- Q: Quit the application");
    attroff(COLOR_PAIR(3));

    wattron(listWin, COLOR_PAIR(1));
    box(listWin, 0, 0);
    std::string tabText = current_tab == 0 ? "Available to Assume Unchanged" : "Currently Assumed Unchanged";
    mvwprintw(listWin, 0, 2, " %s ", tabText.c_str());

    const std::vector<std::string>& activeList = current_tab == 0 ? files : assumeUnchangedFiles;
    size_t list_size = activeList.size();

    size_t visible_end = std::min(scroll_offset + getmaxy(listWin) - 2, list_size);

    for (size_t i = scroll_offset, row = 1; i < visible_end; ++i, ++row) {
        if (i == current_selection) {
            wattron(listWin, A_REVERSE);
            mvwprintw(listWin, row, 2, "%-3lu %s", i + 1, activeList[i].c_str());
            wattroff(listWin, A_REVERSE);
        } else {
            mvwprintw(listWin, row, 2, "%-3lu %s", i + 1, activeList[i].c_str());
        }
    }

    wattroff(listWin, COLOR_PAIR(1));
    wnoutrefresh(stdscr);
    wnoutrefresh(listWin);
    doupdate();
}

int main() {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLACK); 
    init_pair(2, COLOR_BLACK, COLOR_CYAN); 
    init_pair(3, COLOR_GREEN, COLOR_BLACK); 
    bkgd(COLOR_PAIR(1));

    cbreak();
    noecho();
    keypad(stdscr, TRUE);


    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        if (fs::is_regular_file(entry.status())) {
            std::string relativePath = fs::relative(entry.path(), fs::current_path()).string();
            files.push_back(relativePath);
        }
    }

    std::vector<std::string> assumeUnchangedFiles = executeCommand("git ls-files -v | grep '^h' | awk '{print $2}'");

    files.erase(std::remove_if(files.begin(), files.end(), [&](const std::string& file) {
        return std::find(assumeUnchangedFiles.begin(), assumeUnchangedFiles.end(), file) != assumeUnchangedFiles.end();
    }), files.end());

    if (files.empty() && assumeUnchangedFiles.empty()) {
        printw("No files found in the current directory or assume-unchanged list.\n");
        printw("Press any key to exit.\n");
        getch();
        endwin();
        return 0;
    }

    size_t current_selection = 0;
    size_t current_tab = 0; 
    size_t scroll_offset = 0;


    int listWinHeight = LINES - 12;
    int listWinWidth = COLS - 4;
    WINDOW* listWin = newwin(listWinHeight, listWinWidth, 10, 2);
    wbkgd(listWin, COLOR_PAIR(1));

    while (true) {
        drawUI(listWin, files, assumeUnchangedFiles, current_selection, current_tab, scroll_offset);

        int ch = getch();
        if (ch == 'q') {
            break;
        } else if (ch == KEY_UP) {
            if (current_selection > 0) {
                --current_selection;
                if (current_selection < scroll_offset) {
                    --scroll_offset;
                }
            }
        } else if (ch == KEY_DOWN) {
            const std::vector<std::string>& activeList = current_tab == 0 ? files : assumeUnchangedFiles;
            if (current_selection < activeList.size() - 1) {
                ++current_selection;
                if (current_selection >= scroll_offset + listWinHeight - 2) {
                    ++scroll_offset;
                }
            }
        } else if (ch == 9) { // TAB key
            current_tab = 1 - current_tab;
            current_selection = 0;
            scroll_offset = 0;
        } else if (ch == 10) { // ENTER key
            std::vector<std::string>& activeList = current_tab == 0 ? files : assumeUnchangedFiles;
            if (!activeList.empty()) {
                if (current_tab == 0) { 
                    executeGitCommand("git update-index --assume-unchanged", activeList[current_selection]);
                    assumeUnchangedFiles.push_back(activeList[current_selection]);
                    files.erase(files.begin() + current_selection);
                } else {
                    executeGitCommand("git update-index --no-assume-unchanged", activeList[current_selection]);
                    files.push_back(activeList[current_selection]);
                    assumeUnchangedFiles.erase(assumeUnchangedFiles.begin() + current_selection);
                }
                if (current_selection >= activeList.size() && current_selection > 0) {
                    --current_selection;
                }
            }
        }
    }

    delwin(listWin);
    endwin();

    return 0;
}

