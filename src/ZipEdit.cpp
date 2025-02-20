#include "ZipEdit.h"

ZipEdit::ZipEdit() {
    zip = "00000";
    for (int i = 0; i < 5; i++) {
        digits[i] = 0;
    }
}

void ZipEdit::init(String initialZip) {
    setZip(initialZip);
    int startX = 20;
    int digitWidth = 50;
    int gap = 10;
    // Set up up and down buttons for each digit
    for (int i = 0; i < 5; i++) {
        int x = startX + i * (digitWidth + gap);
        upButtons[i] = { x, 40, digitWidth, 30 };
        downButtons[i] = { x, 120, digitWidth, 30 };
    }
    // Center the Save button at the bottom
    int saveX = (M5.Lcd.width() - 100) / 2;
    saveButton = { saveX, M5.Lcd.height() - 60, 100, 40 };
}

void ZipEdit::setZip(String newZip) {
    if (newZip.length() == 5) {
        zip = newZip;
        for (int i = 0; i < 5; i++) {
            digits[i] = zip.charAt(i) - '0';
        }
    }
}

String ZipEdit::getZip() {
    String newZip = "";
    for (int i = 0; i < 5; i++) {
        newZip += String(digits[i]);
    }
    return newZip;
}

void ZipEdit::display() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.print("Edit Zip Code");
    
    for (int i = 0; i < 5; i++) {
        // Draw the up button
        M5.Lcd.drawRect(upButtons[i].x, upButtons[i].y, upButtons[i].w, upButtons[i].h, TFT_WHITE);
        M5.Lcd.setCursor(upButtons[i].x + 15, upButtons[i].y + 5);
        M5.Lcd.print("^");
        
        // Display the digit
        M5.Lcd.setCursor(upButtons[i].x + 20, 80);
        M5.Lcd.print(digits[i]);
        
        // Draw the down button
        M5.Lcd.drawRect(downButtons[i].x, downButtons[i].y, downButtons[i].w, downButtons[i].h, TFT_WHITE);
        M5.Lcd.setCursor(downButtons[i].x + 15, downButtons[i].y + 5);
        M5.Lcd.print("v");
    }
    
    // Draw the Save button
    M5.Lcd.drawRect(saveButton.x, saveButton.y, saveButton.w, saveButton.h, TFT_WHITE);
    M5.Lcd.setCursor(saveButton.x + 10, saveButton.y + 10);
    M5.Lcd.print("Save");
}

bool pointInRect(int px, int py, int rx, int ry, int rw, int rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

void ZipEdit::handleTouch() {
    if (M5.Touch.ispressed()) {
        auto pos = M5.Touch.getPressPoint();
        int tx = pos.x;
        int ty = pos.y;
        // Check each up and down button for each digit
        for (int i = 0; i < 5; i++) {
            if (pointInRect(tx, ty, upButtons[i].x, upButtons[i].y, upButtons[i].w, upButtons[i].h)) {
                digits[i] = (digits[i] + 1) % 10;
                delay(300);
                display();
                return;
            }
            if (pointInRect(tx, ty, downButtons[i].x, downButtons[i].y, downButtons[i].w, downButtons[i].h)) {
                digits[i] = (digits[i] + 9) % 10;
                delay(300);
                display();
                return;
            }
        }
    }
}

bool ZipEdit::isSavePressed() {
    if (M5.Touch.ispressed()) {
        auto pos = M5.Touch.getPressPoint();
        int tx = pos.x;
        int ty = pos.y;
        if (pointInRect(tx, ty, saveButton.x, saveButton.y, saveButton.w, saveButton.h)) {
            delay(300);
            return true;
        }
    }
    return false;
}
