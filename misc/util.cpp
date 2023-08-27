#include <iostream>
#include <fstream>
#include "UTF/UTF.hpp"
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

vector<u16string> toUTF16(const vector<string> &lines) {
    vector<u16string> v;
    UTF utf;
    for (auto &line: lines) {
        v.push_back(utf.u8to16(line));
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

vector<u16string> makeExpect(string filename) {
    auto v = toUTF16(removeLFExpect(unslashExpect(clear(readLines(filename)))));
    if (v.empty())
        throw runtime_error("empty test expect");
    return v;
}

//lineBreakAtEnd - 1: add line break at end of file 2: smart line break at end of file
string genSampleLineBreaks(vector<int> lineLens, int lineBreaksKind, int lineBreakAtEnd) {
    string s;
    for (int i = 0; i < lineLens.size(); i++) {
        for (int j = 0; j < lineLens[i]; j++)
            s += 'a';
        bool addLineBreak;
        switch (lineBreakAtEnd) {
            case 0:
                addLineBreak = i < lineLens.size() - 1 || lineLens[i] == 0;
                break;
            case 1:
                addLineBreak = true;
                break;
            default:
                if (lineLens.back() == 0)
                    addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
                else
                    addLineBreak = i < lineLens.size() - 1;
                break;
        }
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
