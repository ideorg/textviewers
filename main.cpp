#include <iostream>
#include "logic/LineView.h"
#include "logic/LineDocument.h"
#include "misc/util.h"
#include "UTF/UTF.hpp"

using namespace std;

int main() {
    string content = makeString("../main.cpp");
    vl::LineDocument ldoc(content);
    vl::LineView lv(&ldoc);
    lv.setScreenLineLen(40);
    lv.setScreenLineCount(10);
    lv.fillDeque();
    lv.recalcLines();
    UTF utf;
    for (int i=0; i<lv.size(); i++)
        cout << utf.u32to8(lv[i]) << endl;
    return 0;
}
