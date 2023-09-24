//
// Created by Andrzej Borucki on 2022-10-04
//

#ifndef TEXTVIEWER_SELECTION_H
#define TEXTVIEWER_SELECTION_H

#include <cstdint>
#include <QByteArray>
#include "logic/IBaseAccess.h"
#include "logic/AbstractView.h"

class Selection {
    vl::IBaseAccess *m_doc = nullptr;
    vl::FilePosition selBegin; //inclusive
    vl::FilePosition selEnd; //exclusive
    vl::FilePosition firstPos;
    vl::FilePosition secondPos;
    std::pair<int, int> selBeginScreen;
    std::pair<int, int> selEndScreen;
public:
    bool charSelected(std::pair<int, int> point, vl::AbstractView *view);
    void setFirst(std::pair<int,int> pos,  vl::AbstractView *view);
    void setSecond(std::pair<int,int> pos, vl::AbstractView *view);
    void compute(vl::AbstractView *view);
    void setDocument(vl::IBaseAccess *doc);
    int selColBeg(int row, vl::AbstractView *view);
    int selColEnd(int row, vl::AbstractView *view);
    QByteArray get();
};


#endif //TEXTVIEWER_SELECTION_H
