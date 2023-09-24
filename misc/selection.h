//
// Created by Andrzej Borucki on 2022-10-04
//

#ifndef TEXTVIEWER_SELECTION_H
#define TEXTVIEWER_SELECTION_H

#include <cstdint>
#include <QByteArray>
#include "logic/Interface.h"
#include "logic/AbstractView.h"

class Selection {
    vl::Interface *doc = nullptr;
    vl::FilePosition selBegin; //inclusive
    vl::FilePosition selEnd; //exclusive
    vl::FilePosition firstPos;
    vl::FilePosition secondPos;
    std::pair<int, int> selBeginScreen;
    std::pair<int, int> selEndScreen;
public:
    bool charSelected(std::pair<int, int> point, vl::AbstractView *vr);
    void setFirst(std::pair<int,int> pos,  vl::AbstractView *vr);
    void setSecond(std::pair<int,int> pos, vl::AbstractView *vr);
    void compute(vl::AbstractView *vr);
    void setViewLogic(vl::Interface *vl) { this->doc = vl;}
    int selColBeg(int row, vl::AbstractView *vr);
    int selColEnd(int row, vl::AbstractView *vr);
    QByteArray get();
};


#endif //TEXTVIEWER_SELECTION_H
