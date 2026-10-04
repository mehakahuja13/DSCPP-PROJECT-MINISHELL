#include "CommandCorrector.h"
#include <cctype>

// All commands MiniShell understands.
static const char* const VALID_COMMANDS[] = {
    "pwd", "ls", "cd", "mkdir", "touch", "write",
    "cat", "rm", "menu", "help", "exit", "quit"
};
static const int NUM_COMMANDS = sizeof(VALID_COMMANDS) / sizeof(VALID_COMMANDS[0]);

std::string CommandCorrector::toLower(const std::string& s) const {
    std::string r = s;
    for (size_t i = 0; i < r.size(); i++) {
        r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    }
    return r;
}

// dp[i][j] = edit distance between first i chars of a and first j chars of b.
int CommandCorrector::distance(const std::string& a, const std::string& b) const {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());
    if (n > MAX_LEN || m > MAX_LEN) return MAX_LEN + 1;   // too long: treat as "far"

    int dp[MAX_LEN + 1][MAX_LEN + 1];

    for (int i = 0; i <= n; i++) dp[i][0] = i;   // delete all i characters
    for (int j = 0; j <= m; j++) dp[0][j] = j;   // insert all j characters

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            int best = dp[i - 1][j - 1] + cost;                         // replace / match
            if (dp[i - 1][j] + 1 < best) best = dp[i - 1][j] + 1;       // delete
            if (dp[i][j - 1] + 1 < best) best = dp[i][j - 1] + 1;       // insert
            // Swapping two neighbouring letters ("cta" -> "cat") counts as ONE edit,
            // since that is the most common typing mistake.
            if (i > 1 && j > 1 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1]) {
                if (dp[i - 2][j - 2] + 1 < best) best = dp[i - 2][j - 2] + 1;
            }
            dp[i][j] = best;
        }
    }
    return dp[n][m];
}

bool CommandCorrector::isValid(const std::string& cmd) const {
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (cmd == VALID_COMMANDS[i]) return true;
    }
    return false;
}

bool CommandCorrector::suggest(const std::string& cmd, std::string& best) const {
    std::string word = toLower(cmd);

    int bestDist = 1000;
    int bestIdx  = -1;
    for (int i = 0; i < NUM_COMMANDS; i++) {
        std::string candidate = VALID_COMMANDS[i];

        // Allowed mistakes depend on the REAL command's length:
        // short commands (ls, cd, cat) allow 1, longer ones (mkdir, touch) allow 2.
        int limit = (candidate.size() <= 3) ? 1 : 2;

        int d = distance(word, candidate);
        if (d <= limit && d < bestDist) {
            bestDist = d;
            bestIdx  = i;
        }
    }

    if (bestIdx == -1) return false;
    best = VALID_COMMANDS[bestIdx];
    return true;
}
