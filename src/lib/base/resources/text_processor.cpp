#if 0
#include <cassert>

#include "resources/string_tables.hpp"
#include "resources/text_processor.hpp"

namespace MoonGlare {

TextProcessor::TextProcessor(InterfaceMap &ifaceMap) {
    ifaceMap.GetObject(stringTables);
}

TextProcessor::~TextProcessor() {
}

void TextProcessor::Process(const std::string &input, std::string &out) {
    assert(stringTables);

    if (input.empty()) {
        out.clear();
        return;
    }
    char buffer[4096];

    char *outbuf = buffer;
    size_t capacity = sizeof(buffer);

    const char *inp = input.c_str();

    auto push = [&outbuf, &capacity](char c) {
        if (capacity <= 1)
            return;
        *outbuf = c;
        --capacity;
        ++outbuf;
    };

    auto pushstring = [&outbuf, &capacity](std::string_view in) {
        for (const auto &it : in) {
            if (capacity <= 1)
                return;
            *outbuf = it;
            --capacity;
            ++outbuf;
        }
    };

    while (capacity > 1) {
        char c = *inp;
        ++inp;
        switch (c) {
        case '\0':
            push('\0');
            out = buffer;
            return;
        case '$':
            if (inp[0] == '{') {
                const char *end = strchr(inp + 1, '}');
                if (end) {
                    ++inp; // skip { char
                    //std::string id(inp, end - inp);
                    const char *dot = strchr(inp, '.');

                    std::string table;
                    std::string id;
                    if (dot) {
                        table = std::string(inp, dot - inp);
                        id = std::string(dot + 1, end - dot - 1);
                    } else {
                        id = std::string(inp, end - inp);
                    }
                    auto outx = stringTables->GetString(id, table);
                    pushstring(outx);
                    inp = end + 1;
                    continue;
                }
            }
        default:
            push(c);
            continue;
        }
    }
}

} //namespace MoonGlare
#endif