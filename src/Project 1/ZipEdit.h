#ifndef ZIPEDIT_H
#define ZIPEDIT_H

#include <M5Core2.h>

class ZipEdit {
public:
    ZipEdit();
    void init(String initialZip);
    void setZip(String newZip);
    String getZip();
    void display();
    void handleTouch();
    bool isSavePressed();
    
private:
    String zip; // 5-digit zip code
    int digits[5];
    // Simple button structure
    struct Button {
        int x, y, w, h;
    };
    Button upButtons[5];
    Button downButtons[5];
    Button saveButton;
};

#endif
