#include "EventsDisplay.h"
#include <esp-iot-utils.h>

EventsDisplay::EventsDisplay(
    GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &display,
    U8G2_FOR_ADAFRUIT_GFX &u8g2)
    : _display(display), _u8g2(u8g2) {}

void EventsDisplay::init() {
  _display.init(115200);
  _display.clearScreen();
}

void EventsDisplay::clear() { _display.clearScreen(); }

void EventsDisplay::drawError(const char *message) {
  _u8g2.begin(_display);
  _display.setRotation(1);
  _display.setFullWindow();

  _display.firstPage();
  do {
    _display.fillScreen(GxEPD_WHITE);
    _u8g2.setForegroundColor(GxEPD_BLACK);
    _u8g2.setBackgroundColor(GxEPD_WHITE);
    _u8g2.setFont(u8g2_font_helvB14_tf);

    int w = _u8g2.getUTF8Width(message);
    int x = (_display.width() - w) / 2;
    int y = _display.height() / 2;

    _u8g2.setCursor(x, y);
    _u8g2.print(message);

    _u8g2.setFont(u8g2_font_helvR10_tf);
    const char *sub = (TimeHelper::getLanguage() == "fr")
                          ? "Verifier WiFi / URL"
                          : "Check WiFi / URL";
    int w2 = _u8g2.getUTF8Width(sub);
    _u8g2.setCursor((_display.width() - w2) / 2, y + 25);
    _u8g2.print(sub);

  } while (_display.nextPage());
}

void EventsDisplay::setData(const TrashData &trash,
                            const std::vector<Birthday> &birthdays,
                            int currentDay) {
  _trash = trash;
  _birthdays = birthdays;
  _currentDay = currentDay;
}

void EventsDisplay::update(bool fullRefresh) {
  _u8g2.begin(_display);
  _display.setRotation(1);
  _display.setFullWindow();

  _u8g2.setFontMode(1);
  _u8g2.setForegroundColor(GxEPD_BLACK);
  _u8g2.setBackgroundColor(GxEPD_WHITE);

  int fullW = _display.width();
  int fullH = _display.height();
  int centerX = fullW / 2;
  int topMargin = 20;

  _display.firstPage();
  do {
    _display.fillScreen(GxEPD_WHITE);

    // --- GLOBAL FRAME ---
    uint16_t margin = 10;
    for (int i = 0; i < 4; i++) {
      _display.drawRect(margin + i, margin + i,
                        _display.width() - 2 * (margin + i),
                        _display.height() - 2 * (margin + i), GxEPD_BLACK);
    }

    // --- TRASH ---
    int binW = 50;
    int binH = 70;
    int colLeftX = fullW / 3;
    int colRightX = (fullW * 2) / 3;
    int binY = topMargin + 45;

    // Draw bins
    drawBin(colLeftX, binY, true, _trash.blackToday, _trash.blackDays);
    drawBin(colRightX, binY, false, _trash.yellowToday, _trash.yellowDays);

    // Title
    _u8g2.setForegroundColor(GxEPD_BLACK);
    _u8g2.setBackgroundColor(GxEPD_WHITE);
    _u8g2.setFont(u8g2_font_helvB14_tf);
    const char *title =
        (TimeHelper::getLanguage() == "fr") ? "SORTIR LES POUBELLES" : "TRASH";
    int wTitle = _u8g2.getUTF8Width(title);
    _u8g2.setCursor(centerX - wTitle / 2, topMargin + 30);
    _u8g2.print(title);

    // --- BIRTHDAYS ---
    int bdY = binY + binH + 65;

    // Separator
    _display.drawLine(margin + 20, bdY - 10, fullW - margin - 20, bdY - 10,
                      GxEPD_BLACK);

    _u8g2.setFont(u8g2_font_helvB14_tf);
    const char *bdTitle =
        (TimeHelper::getLanguage() == "fr") ? "ANNIVERSAIRES" : "BIRTHDAYS";
    int wBdTitle = _u8g2.getUTF8Width(bdTitle);
    _u8g2.setCursor(centerX - wBdTitle / 2, bdY + 10);
    _u8g2.print(bdTitle);

    _u8g2.setFont(u8g2_font_helvB14_tf);
    const char *bdSubTitle =
        (TimeHelper::getLanguage() == "fr") ? "DU MOIS" : "THIS MONTH";
    int wBdSubTitle = _u8g2.getUTF8Width(bdSubTitle);
    _u8g2.setCursor(centerX - wBdSubTitle / 2, bdY + 30);
    _u8g2.print(bdSubTitle);

    int currentY = bdY + 60;
    _u8g2.setFont(u8g2_font_helvB12_tf);
    int hLine = 26;

    for (const auto &bd : _birthdays) {
      if (bd.day < _currentDay)
        continue; // Skip past birthdays

      String line = String(bd.day) + " : " + bd.name;
      int wLine = _u8g2.getUTF8Width(line.c_str());

      _u8g2.setCursor(centerX - wLine / 2, currentY);
      _u8g2.print(line);
      currentY += hLine;

      if (currentY > fullH - margin)
        break; // Safety
    }

  } while (_display.nextPage());
}

void EventsDisplay::drawBin(int x, int y, bool isBlack, bool isToday,
                            int days) {
  int binW = 50;
  int binH = 70;
  int left = x - binW / 2;
  int top = y;

  // 1. Shape
  if (isBlack) {
    _display.fillRoundRect(left, top, binW, binH, 4, GxEPD_BLACK);
    _display.fillRect(left - 3, top, binW + 6, 8, GxEPD_BLACK);
  } else {
    _display.drawRoundRect(left, top, binW, binH, 4, GxEPD_BLACK);
    _display.drawRoundRect(left + 1, top + 1, binW - 2, binH - 2, 2,
                           GxEPD_BLACK);
    _display.drawRect(left - 3, top, binW + 6, 8, GxEPD_BLACK);
    _display.drawRect(left - 2, top + 1, binW + 4, 6, GxEPD_BLACK);
  }

  // 2. Text
  _u8g2.setFont(u8g2_font_helvB08_tf);
  if (isBlack) {
    _u8g2.setForegroundColor(GxEPD_WHITE);
    _u8g2.setBackgroundColor(GxEPD_BLACK);
  } else {
    _u8g2.setForegroundColor(GxEPD_BLACK);
    _u8g2.setBackgroundColor(GxEPD_WHITE);
  }

  // "IN"
  const char *inText = (TimeHelper::getLanguage() == "fr") ? "DANS" : "IN";
  _u8g2.setCursor(x - _u8g2.getUTF8Width(inText) / 2, top + 18);
  _u8g2.print(inText);

  // Number of days
  _u8g2.setFont(u8g2_font_logisoso24_tn);
  String daysStr = (days < 0) ? "--" : String(days);
  int wNum = _u8g2.getUTF8Width(daysStr.c_str());
  int hNum = _u8g2.getFontAscent();
  int numY = top + 24 + hNum;
  _u8g2.setCursor(x - wNum / 2, numY);
  _u8g2.print(daysStr);

  // "DAY(S)"
  _u8g2.setFont(u8g2_font_helvB08_tf);
  String unit;
  if (TimeHelper::getLanguage() == "fr") {
    unit = (days <= 1 && days >= 0) ? "JOUR" : "JOURS";
  } else {
    unit = (days == 1) ? "DAY" : "DAYS";
  }
  int wUnit = _u8g2.getUTF8Width(unit.c_str());
  _u8g2.setCursor(x - wUnit / 2, numY + 12);
  _u8g2.print(unit);

  // 3. Color label
  _u8g2.setForegroundColor(GxEPD_BLACK);
  _u8g2.setBackgroundColor(GxEPD_WHITE);
  _u8g2.setFont(u8g2_font_helvB10_tf);
  String label;
  if (TimeHelper::getLanguage() == "fr") {
    label = isBlack ? "NOIR" : "JAUNE";
  } else {
    label = isBlack ? "BLACK" : "YELLOW";
  }
  int wLbl = _u8g2.getUTF8Width(label.c_str());
  _u8g2.setCursor(x - wLbl / 2, top + binH + 20);
  _u8g2.print(label);
}
