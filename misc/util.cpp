#include <iostream>
#include <fstream>
#include "cpg/utf/UTF.hpp"
#include "util.h"

#include <vector>

using namespace std;

vector<string> readLines(string filename) {
    ifstream infile(filename);
    vector<string> v;
    string line;
    bool readedNewline;
    if (infile.peek() != EOF)
        while (true) {
            readedNewline = getline(infile, line).good();
            v.push_back(line);
            if (!readedNewline) break;
        }
    return v;
}


string trimLeft(const string &str) {
    const auto strBegin = str.find_first_not_of(" \t");
    return str.substr(strBegin, str.length() - strBegin);
}


string trimRight(const string &str) {
    const auto strEnd = str.find_last_not_of(" \t\r");
    return str.substr(0, strEnd + 1);
}

string trim(const string &str) {
    return trimLeft(trimRight(str));
}

string checkNoTabs(const string &line) {
    for (char c: line) {
        if (c == '\t') throw runtime_error("must not have literal tabs, use \\t");
    }
    return line;
}

/* trimRight for all lines
 * remove last empty line if exists
 * */
vector<string> clear(const vector<string> &lines) {
    vector<string> v;
    for (auto &line: lines)
        v.push_back(checkNoTabs(trimRight(line)));
    if (v.size() > 0 && v.back().empty())
        v.pop_back();
    return v;
}

string unslash(const string &line) {
    string result;
    int k = 0;
    while (k < line.length()) {
        char c = line[k];
        if (c == '\\') {
            char c1 = line[k + 1];
            switch (c1) {
                case 't':
                    result += '\t';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 'n':
                    result += '\n';
                    break;
                default :
                    throw runtime_error("bad slashed char");
            }
            k++;
        } else
            result += c;
        k++;
    }
    return result;
}

string unslashTest(string line, bool last) {
    line = unslash(line);
    for (int i = 0; i < line.length(); i++) {
        char c = line[i];
        if (c == '\r' || c == '\n') {
            if (c == '\r') {
                if (i < line.length() - 2)
                    throw runtime_error("\\r must only be at end or before (1) \\n");
                else if (i < line.length() - 1 && line[i + 1] != '\n')
                    throw runtime_error("\\r must only be at end or before (2)\\n");
            } else {
                if (i < line.length() - 1)
                    throw runtime_error("\\n must only be at end");
            }
        } else {
            if (!last && i == line.length() - 1)
                throw runtime_error("last line must not ends with \r or \n");
        }
    }
    return line;
}

vector<string> unslashTest(const vector<string> &lines) {
    vector<string> v;
    for (int i = 0; i < lines.size(); i++)
        v.push_back(unslashTest(lines[i], i == lines.size() - 1));
    return v;
}

string unslashExpect(string line, bool last) {
    line = unslash(line);
    for (int i = 0; i < line.length(); i++) {
        char c = line[i];
        if (c == '\n' || c == '\r') {
            if (i < line.length() - 1)
                throw runtime_error("\\n must only be at end");
        } else {
            if (!last && i == line.length() - 1)
                throw runtime_error("last line must not ends with \r or \n");
        }
    }
    return line;
}

vector<string> unslashExpect(const vector<string> &lines) {
    vector<string> v;
    for (int i = 0; i < lines.size(); i++)
        v.push_back(unslashExpect(lines[i], i == lines.size() - 1));
    return v;
}

vector<string> removeLFExpect(const vector<string> &lines) {
    vector<string> v;
    bool addEmpty = lines.size() > 0 && lines.back().back() == '\n';
    for (int i = 0; i < lines.size(); i++) {
        string newLine = lines[i];
        if (!newLine.empty() && newLine[newLine.length() - 1] == '\n')
            newLine.pop_back();
        v.push_back(newLine);
    }
    if (addEmpty)
        v.push_back("");
    return v;
}

vector<u32string> toUTF32(const vector<string> &lines) {
    vector<u32string> v;
    UTF utf;
    for (auto &line: lines) {
        v.push_back(utf.u8to32(line));
    }
    return v;
}

string join(const vector<string> &v) {
    string result;
    for (auto &line: v) {
        result += line;
    }
    return result;
}

string addEols(string str, int nEols, bool rn = false) {
    char c = str.back();
    if (nEols > 0) {
        if (c == '\n' || c == '\r')
            throw runtime_error("last line here must not ends with \r or \n");
        string eolStr;
        if (rn)
            eolStr = "\r\n";
        else
            eolStr = "\n";
        for (int i = 0; i < nEols; i++)
            str += eolStr;
    } else if (nEols < 0) {
        if (nEols != -1)
            throw runtime_error("can't be < -1");
        if (c != '\n')
            throw runtime_error("last line here must ends with \n");
        str.pop_back();
    }
    return str;
}

vector<int> getSizesStr(string filename) {
    auto v = readLines(filename);
    vector<int> result;
    int size = 0;
    for (auto &line: v) {
        size += line.size() + 1;
        result.push_back(size);
    }
    return result;
}

string makeString(string filename, int nEols, bool rn) {
    ifstream infile(filename);
    std::string result((std::istreambuf_iterator<char>(infile)),
                       std::istreambuf_iterator<char>());
    infile.close();
    if (nEols > 0)
        result = addEols(result, nEols, rn);
    return result;
}

string makeStringN(string filename, int n) {
    ifstream infile(filename);
    std::string str((std::istreambuf_iterator<char>(infile)),
                    std::istreambuf_iterator<char>());
    infile.close();
    string result;
    for (int i = 0; i < n; i++)
        result += str;
    return result;
}

vector<int> getSizes(string filename) {
    auto v = unslashTest(clear(readLines(filename)));
    vector<int> result;
    int size = 0;
    for (auto &line: v) {
        size += line.size();
        result.push_back(size);
    }
    return result;
}

vector<string> addEols(vector<string> v, int nEols, bool rn) {
    string lastLine = v.back();
    lastLine = addEols(lastLine, nEols, rn);
    v.pop_back();
    v.push_back(lastLine);
    return v;
}

/*
 * if start>0, without starting lines
 * if start<0, without leading lines
 * if lastEol>0 - add \n at end of file, check if \n is not at end
 * */
string makeContent(string filename, int nEols, bool rn) {
    auto v = unslashTest(clear(readLines(filename)));
    v = addEols(v, nEols, rn);
    string content = join(v);
    if (v.empty())
        throw runtime_error("empty test sample");
    return content;
}

vector<u32string> makeExpect(string filename) {
    auto v = toUTF32(removeLFExpect(unslashExpect(clear(readLines(filename)))));
    if (v.empty())
        throw runtime_error("empty test expect");
    return v;
}

//for test two or more empty lines one after another
vector<int> multiply(vector<int>& lineLens, int factor) {
    vector<int> result;
    for (int i=0; i<lineLens.size(); i++)
        for (int j=0; j<factor; j++)
            result.push_back(lineLens[i]);
    return result;
}

string genSampleLineBreaks(vector<int> lineLens, int lineBreaksKind) {
    string s;
    for (int i = 0; i < lineLens.size(); i++) {
        for (int j = 0; j < lineLens[i]; j++)
            s += 'a';
        bool addLineBreak;
        if (lineLens.back() == 0)
            addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
        else
            addLineBreak = i < lineLens.size() - 1;
        if (addLineBreak)
        switch (lineBreaksKind) {
            case 0:
                s += '\n';
                break;
            case 1:
                s += '\r';
                break;
            case 2:
                s += "\r\n";
                break;
            default:
                throw logic_error("genSample: lineBreaksKind out of range");
        }
    }
    return s;
}

/* generates with smart line breaks, with BOM and \r\n breaks
   generate with 1,2,3,4 code byte len, fill
   see https://r12a.github.io/app-conversion/ */
string genSampleUnicode(vector<int> lineLens, int utf8len) {
    char32_t c[5];
    c[0] = 0xFEFF; //BOM
    c[1] = 'a';
    c[2] = 0x105;
    c[3] = 0x4E2D;
    c[4] = 0x10348;
    string u8[5];
    if (utf8len < 1 || utf8len > 4) throw logic_error("genSampleUnicode: utf8len out of range");
    UTF utf;
    char buf[5];
    for (int i = 0; i <= 4; i++) {
        int len = utf.one32to8(c[i], buf);
        if (i == 0)
            assert(len == 3);
        else
            assert(len == i);
        buf[len] = 0;
        u8[i] = buf;
    }
    string s;
    for (int i = 0; i < lineLens.size(); i++) {
        int len = lineLens[i];
        int residual = len % utf8len;
        int n = len / utf8len;
        assert(n * utf8len + residual == len);
        for (int j = 0; j < n; j++)
            s += u8[utf8len];
        if (residual > 0)
            s += u8[residual];
        bool addLineBreak;
        if (lineLens.back() == 0)
            addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
        else
            addLineBreak = i < lineLens.size() - 1;
        if (addLineBreak)
            s += "\r\n";
    }
    return s;
}

deque<LBInfo> getLineBreaks(vector<int> lineLens, int lineBreaksKind) {
    int lenBreak = lineBreaksKind == 2 ? 2 : 1;
    deque<LBInfo> result;
    int offset = 0;
    for (int i = 0; i < lineLens.size(); i++) {
        bool addLineBreak;
        if (lineLens.back() == 0)
            addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
        else
            addLineBreak = i < lineLens.size() - 1;
        LBInfo lbi;
        if (addLineBreak) {
            lbi = {offset, lineLens[i], lenBreak};
            offset += lineLens[i] + lenBreak;
        } else {
            lbi = {offset, lineLens[i], 0};
            offset += lineLens[i];
        }
        result.push_back(lbi);
    }
    return result;
}

vector<int64_t> computeBreakPoints(int64_t offset, int64_t end, int maxLineLen) {
    int64_t first = offset / maxLineLen;
    int64_t last = end / maxLineLen;
    vector<int64_t> result;
    for (int64_t i = first; i <= last; i++) {
        int64_t n = i * maxLineLen;
        if (n > offset && n < end)
            result.push_back(n);
    }
    return result;
}

deque<LBInfo> getMaxLineBreaks(deque<LBInfo> lineBreaks, int maxLineLen, int utf8len) {
    deque<LBInfo> result;
    int offset = 0;
    for (auto lb: lineBreaks) {
        vector<int64_t> breakPoints = computeBreakPoints(offset, offset + lb.len, maxLineLen);
        auto divided = divideUnicodeToBreaks(lb, breakPoints, utf8len);
        assert(divided.size()>=1);
        if (divided.size() >= 2) {
            LBInfo lbi;
            lbi.offset = divided[0].offset;
            lbi.len = divided[0].len + divided[1].len;
            lbi.breaks = divided[1].breaks;
            divided.pop_front();
            divided.pop_front();
            divided.push_front(lbi);
        }
        for (auto &lbi: divided)
            result.push_back(lbi);
        offset += lb.len + lb.breaks;
    }
    return result;
}

int alignSample(int len, int max, int utf8len) {
    int residual = len % utf8len;
    if (residual > 0)
        len += utf8len - residual;
    if (len > max)
        len = max;
    return len;
}


deque<LBInfo> divideUnicodeToBreaks(LBInfo line, vector<int64_t> breaks, int utf8len) {
    std::deque<LBInfo> result;
    int remain = line.len;
    LBInfo lbi;
    lbi.offset = line.offset;
    for (int b: breaks) {
        int aligned = alignSample(b - lbi.offset, remain, utf8len);
        assert(aligned <= remain);
        if (aligned == remain) break;
        lbi.len = aligned;
        lbi.breaks = 0;
        result.push_back(lbi);
        remain -= lbi.len;
        lbi.offset += lbi.len;
    }
    lbi.len = remain;
    lbi.breaks = line.breaks;
    result.push_back(lbi);
    return result;
}
