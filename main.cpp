#include <iostream>
#include "logic/LineView.h"
#include "logic/LineDocument.h"
#include "misc/util.h"

using namespace std;

int main() {
    string content = makeString("../main.cpp");
    vl::LineDocument ldoc(content);
    vl::LineView lv(&ldoc);
    lv.fillDeque();
    lv.recalcLines();
    return 0;
}
