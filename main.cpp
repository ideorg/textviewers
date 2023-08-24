#include <iostream>
#include "logic/LineView.h"
#include "logic/LineDocument.h"
#include "misc/util.h"

using namespace std;

int main() {
    string content = makeString("../main.cpp");
    LineDocument ldoc(content);
    LineView lv(&ldoc);
    lv.fillDeque();
    return 0;
}
