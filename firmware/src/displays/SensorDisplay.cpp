#include "SensorDisplay.h"

SensorDisplay::SensorDisplay(
    GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &display,
    U8G2_FOR_ADAFRUIT_GFX &u8g2)
    : _display(display), _u8g2(u8g2) {
  for (int i = 0; i < 8; i++) {
    _data[i] = {"", "--", ""};
  }
}

void SensorDisplay::init() {
  _display.init(115200);
  _display.clearScreen();
}

void SensorDisplay::clear() { _display.clearScreen(); }

void SensorDisplay::setData(SensorData row1_col1, SensorData row1_col2,
                            SensorData row2_col1, SensorData row2_col2,
                            SensorData row3_col1, SensorData row3_col2,
                            SensorData row4_col1, SensorData row4_col2) {
  _data[0] = row1_col1;
  _data[1] = row1_col2;
  _data[2] = row2_col1;
  _data[3] = row2_col2;
  _data[4] = row3_col1;
  _data[5] = row3_col2;
  _data[6] = row4_col1;
  _data[7] = row4_col2;
}

void SensorDisplay::update(bool fullRefresh) {
  _u8g2.begin(_display);
  _display.setRotation(1);

  if (fullRefresh) {
    _display.setFullWindow();
  } else {
    _display.setPartialWindow(0, 0, _display.width(), _display.height());
  }

  _u8g2.setFontMode(1);
  _u8g2.setForegroundColor(GxEPD_BLACK);
  _u8g2.setBackgroundColor(GxEPD_WHITE);

  int fullW = _display.width();
  int fullH = _display.height();
  int margin = 10;

  int contentX = margin + 4;
  int contentY = margin + 4;
  int contentW = fullW - 2 * contentX;
  int contentH = fullH - 2 * contentY;

  int rowHeight = contentH / 4;
  int colWidth = contentW / 2;

  if (_style == 1) {
    drawCircles();
    return;
  }

  _display.firstPage();
  do {
    // global background
    _display.fillScreen(GxEPD_WHITE);

    for (int i = 0; i < 4; i++) {
      _display.drawRect(margin + i, margin + i, fullW - 2 * (margin + i),
                        fullH - 2 * (margin + i), GxEPD_BLACK);
    }

    // Draw 8 cells (4 rows x 2 columns)
    drawCell(0, 0, _data[0].label, _data[0].value, _data[0].unit, false);
    drawCell(1, 0, _data[1].label, _data[1].value, _data[1].unit, false);

    drawCell(0, 1, _data[2].label, _data[2].value, _data[2].unit, false);
    drawCell(1, 1, _data[3].label, _data[3].value, _data[3].unit, false);

    drawCell(0, 2, _data[4].label, _data[4].value, _data[4].unit, true);
    drawCell(1, 2, _data[5].label, _data[5].value, _data[5].unit, true);

    drawCell(0, 3, _data[6].label, _data[6].value, _data[6].unit, true);
    drawCell(1, 3, _data[7].label, _data[7].value, _data[7].unit, true);

    // Draw Last Update Timestamp
    if (!_lastUpdateTime.isEmpty()) {
      _u8g2.setFont(u8g2_font_helvB10_tf);
      String msg = "MAJ: " + _lastUpdateTime;
      int bW = _u8g2.getUTF8Width(msg.c_str());
      int x = (fullW - bW) / 2;
      int y = fullH - 8;

      // Draw white background for the text
      _display.fillRect(x - 4, y - 12, bW + 8, 15, GxEPD_WHITE);
      _display.drawRect(x - 4, y - 12, bW + 8, 15, GxEPD_BLACK);

      _u8g2.setForegroundColor(GxEPD_BLACK);
      _u8g2.setBackgroundColor(GxEPD_WHITE);
      _u8g2.setCursor(x, y);
      _u8g2.print(msg);
    }
  } while (_display.nextPage());
}

void SensorDisplay::drawCell(int col, int row, String label, String val,
                             String unit, bool inverted) {
  int fullW = _display.width();
  int fullH = _display.height();
  int margin = 10;
  int contentX = margin + 4;
  int contentY = margin + 4;
  int contentW = fullW - 2 * contentX;
  int contentH = fullH - 2 * contentY;
  int rowH = contentH / 4;
  int colW = contentW / 2;

  int absX = contentX + col * colW;
  int absY = contentY + row * rowH;

  // background
  if (inverted) {
    _display.fillRect(absX, absY, colW, rowH, GxEPD_BLACK);
    _u8g2.setForegroundColor(GxEPD_WHITE);
    _u8g2.setBackgroundColor(GxEPD_BLACK);
  } else {
    _display.fillRect(absX, absY, colW, rowH, GxEPD_WHITE);
    _u8g2.setForegroundColor(GxEPD_BLACK);
    _u8g2.setBackgroundColor(GxEPD_WHITE);
  }

  int leftMargin = 15;

  // Label
  _u8g2.setFont(u8g2_font_helvR10_tf);
  _u8g2.setCursor(absX + leftMargin, absY + 20);
  _u8g2.print(label);

  // value
  _u8g2.setFont(u8g2_font_logisoso28_tn);
  int valH = _u8g2.getFontAscent();
  _u8g2.setCursor(absX + leftMargin, absY + 35 + valH);
  _u8g2.print(val);

  // unit
  int valW = _u8g2.getUTF8Width(val.c_str());
  _u8g2.setFont(u8g2_font_helvR10_tf);
  _u8g2.setCursor(absX + leftMargin + valW + 4, absY + 35 + valH);
  _u8g2.print(unit);
}

void SensorDisplay::drawCircles() {
  _display.setRotation(
      1); // Keep rotation setting but adapt to reported dimensions
  _display.firstPage();
  do {
    _display.fillScreen(GxEPD_WHITE);

    int colW = 150; // 300 / 2
    int rowH = 133; // 400 / 3

    _u8g2.setFontMode(1);
    _u8g2.setForegroundColor(GxEPD_BLACK);
    _u8g2.setBackgroundColor(GxEPD_WHITE);

    for (int i = 0; i < 6; i++) {
      int row = i / 2;
      int col = i % 2;

      // Cell Center
      int cx = col * colW + colW / 2;
      int cy = row * rowH + rowH / 2;

      // Radius 60 fits 150px cell.
      // 1. Outer Thick Ring (Radius 60)
      _display.drawCircle(cx, cy, 60, GxEPD_BLACK);
      _display.drawCircle(cx, cy, 59, GxEPD_BLACK);

      // 2. Inner Thin Ring (Radius 55) - Creates a 4px "gap"
      _display.drawCircle(cx, cy, 55, GxEPD_BLACK);

      // 1. Label (Top) - Reduced to B08
      _u8g2.setFont(u8g2_font_helvB08_tf);
      int wLabel = _u8g2.getUTF8Width(_data[i].label.c_str());
      _u8g2.setCursor(cx - wLabel / 2, cy - 22);
      _u8g2.print(_data[i].label);

      // 2. Value (Middle) - Keep Large
      _u8g2.setFont(u8g2_font_helvB24_tf);
      int wValue = _u8g2.getUTF8Width(_data[i].value.c_str());
      int hValue = _u8g2.getFontAscent(); // ~24
      _u8g2.setCursor(cx - wValue / 2, cy + 10);
      _u8g2.print(_data[i].value);

      // 3. Unit (Bottom) - Reduced to R08
      _u8g2.setFont(u8g2_font_helvR08_tf);
      int wUnit = _u8g2.getUTF8Width(_data[i].unit.c_str());
      _u8g2.setCursor(cx - wUnit / 2, cy + 32);
      _u8g2.print(_data[i].unit);
    }
  } while (_display.nextPage());
}
