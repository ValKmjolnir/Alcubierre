#include <raylib.h>

#include "ui/text_wrap.hpp"

namespace {

std::string greedy_wrap(const std::string& paragraph, int fontsize, int max_width) {
    std::string result;
    std::string line;
    std::string word;

    for (size_t i = 0; i <= paragraph.size(); i++) {
        if (i < paragraph.size() && paragraph[i] != ' ') {
            word += paragraph[i];
            continue;
        }

        if (!word.empty()) {
            const std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && MeasureText(candidate.c_str(), fontsize) > max_width) {
                result += line + "\n";
                line = word;
            } else {
                line = candidate;
            }
            word.clear();
        }
    }

    result += line;
    return result;
}

bool ends_with_single_word(const std::string& wrapped) {
    const size_t last_break = wrapped.rfind('\n');
    if (last_break == std::string::npos) {
        return false;
    }
    return wrapped.find(' ', last_break + 1) == std::string::npos;
}

std::string wrap_paragraph(const std::string& paragraph, int fontsize, int max_width) {
    const std::string wrapped = greedy_wrap(paragraph, fontsize, max_width);
    if (!ends_with_single_word(wrapped)) {
        return wrapped;
    }

    const std::string last_word = wrapped.substr(wrapped.rfind('\n') + 1);
    const int narrower = max_width - MeasureText(last_word.c_str(), fontsize) - MeasureText(" ", fontsize);
    if (narrower <= 0) {
        return wrapped;
    }

    const std::string retry = greedy_wrap(paragraph, fontsize, narrower);
    if (retry.find('\n') != std::string::npos && !ends_with_single_word(retry)) {
        return retry;
    }
    return wrapped;
}

}

std::string wrap_text(const std::string& text, int fontsize, int max_width) {
    std::string result;
    size_t start = 0;

    while (true) {
        const size_t end = text.find('\n', start);
        const size_t length = (end == std::string::npos) ? std::string::npos : end - start;
        if (!result.empty()) {
            result += '\n';
        }
        result += wrap_paragraph(text.substr(start, length), fontsize, max_width);
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }

    return result;
}
