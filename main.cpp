#include <iostream>
#include "logic/LineView.h"
#include "logic/LineDocument.h"
#include "misc/util.h"

using namespace std;

int main() {
    string content = makeString("../main.cpp");
    vl::LineDocument ldoc(content);
    vl::LineView lv(&ldoc);
    lv.setScreenLineLen(40);
    lv.setScreenLineCount(10);
    lv.fillDeque();
    lv.recalcLines();
    for (int i=0; i<lv.size(); i++)
        wcout << lv[i] << endl;
    return 0;
}
