#include "EphemerisDisplay.h"
#include <esp-iot-utils.h>

EphemerisDisplay::EphemerisDisplay(
    GxEPD2_3C<GxEPD2_420c_GDEY042Z98, GxEPD2_420c_GDEY042Z98::HEIGHT> &display,
    U8G2_FOR_ADAFRUIT_GFX &u8g2)
    : _display(display), _u8g2(u8g2) {}

// Center text helper
int getCenteredX(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const char *text, int x,
                 int w) {
  int textWidth = u8g2Fonts.getUTF8Width(text);
  return x + (w - textWidth) / 2;
}

void EphemerisDisplay::init() {
  _display.init();
  _display.setRotation(1);
  _display.setFullWindow();
}

void EphemerisDisplay::clear() { _display.clearScreen(); }

void EphemerisDisplay::setData(const DateData &date, const SunData &sun,
                               const SeasonData &season) {
  _date = date;
  _sun = sun;
  _season = season;
}

void EphemerisDisplay::update(bool fullRefresh) {
  _u8g2.begin(_display);
  _display.setRotation(1);
  if (fullRefresh) {
    _display.setFullWindow();
  } else {
    _display.setPartialWindow(0, 0, _display.width(), _display.height());
  }

  _display.firstPage();
  do {
    _display.fillScreen(GxEPD_WHITE);
    drawLayout();
    drawDate(_date);
    drawSunInfo(_sun);
    drawSeason(_season);
  } while (_display.nextPage());
}

void EphemerisDisplay::drawLayout() {
  int w = _display.width();
  int h = _display.height();
  int margin = 10;

  _display.drawRect(margin, margin, w - 2 * margin, h - 2 * margin,
                    GxEPD_BLACK);
  _display.drawRect(margin + 2, margin + 2, w - 2 * margin - 4,
                    h - 2 * margin - 4, GxEPD_RED);
}

void EphemerisDisplay::drawError(const char *message) {
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
    const char *sub = "Check WiFi / URL";
    int w2 = _u8g2.getUTF8Width(sub);
    _u8g2.setCursor((_display.width() - w2) / 2, y + 25);
    _u8g2.print(sub);

  } while (_display.nextPage());
}

void EphemerisDisplay::drawDate(const DateData &date) {
  int margin = 10;
  int16_t centerX = _display.width() / 2;
  int marginBottom = 25;
  int dateBottomY = _display.height() - margin - marginBottom;

  _u8g2.setFont(u8g2_font_helvB18_tf);
  int h_font18 = _u8g2.getFontAscent() - _u8g2.getFontDescent();

  _u8g2.setFont(u8g2_font_logisoso50_tn);
  int h_chiffre = _u8g2.getFontAscent() - _u8g2.getFontDescent();

  _u8g2.setFont(u8g2_font_helvR12_tf);
  int h_info = _u8g2.getFontAscent() - _u8g2.getFontDescent();

  int gap = 8;
  int dateBlockH = h_font18 + gap + h_chiffre + gap + h_font18 + gap + h_info;
  int dateStartY = dateBottomY - dateBlockH;
  int currentY = dateStartY;

  // Jour
  _u8g2.setForegroundColor(GxEPD_BLACK);
  _u8g2.setBackgroundColor(GxEPD_WHITE);
  _u8g2.setFont(u8g2_font_helvB18_tf);
  int w = _u8g2.getUTF8Width(date.dayName);
  _u8g2.setCursor(centerX - w / 2, currentY + h_font18);
  _u8g2.print(date.dayName);
  currentY += h_font18 + gap;

  // Chiffre (rouge)
  _u8g2.setFont(u8g2_font_logisoso50_tn);
  _u8g2.setForegroundColor(GxEPD_RED);
  w = _u8g2.getUTF8Width(date.dayNumber);
  _u8g2.setCursor(centerX - w / 2, currentY + h_chiffre);
  _u8g2.print(date.dayNumber);
  _u8g2.setForegroundColor(GxEPD_BLACK);
  currentY += h_chiffre + gap;

  // Mois
  _u8g2.setFont(u8g2_font_helvB18_tf);
  w = _u8g2.getUTF8Width(date.monthName);
  _u8g2.setCursor(centerX - w / 2, currentY + h_font18);
  _u8g2.print(date.monthName);
  currentY += h_font18 + gap;

  // Ligne info
  _u8g2.setFont(u8g2_font_helvR12_tf);
  char infoStr[60];
  snprintf(infoStr, sizeof(infoStr), "%s - S%d - %d/%d", date.year,
           date.weekNumber, date.dayOfYear, date.totalDays);
  w = _u8g2.getUTF8Width(infoStr);
  _u8g2.setCursor(centerX - w / 2, currentY + h_info);
  _u8g2.print(infoStr);
}

void EphemerisDisplay::drawSunInfo(const SunData &sun) {
  int margin = 10;
  int16_t centerX = _display.width() / 2;
  int sunBaseY = margin + 70;
  int arcRadius = 55;
  // int sunBottomLimit = sunBaseY + 30; // Unused variable warning fix

  // Arc de cercle
  for (int angle = 180; angle <= 360; angle += 2) {
    float rad = angle * PI / 180.0;
    int x = centerX + arcRadius * cos(rad);
    int y = sunBaseY + arcRadius * sin(rad);
    _display.drawPixel(x, y, GxEPD_BLACK);
    _display.drawPixel(x, y - 1, GxEPD_BLACK);
  }

  // Soleil rouge
  int sunY = sunBaseY - 25;
  int sunR = 8;
  _display.fillCircle(centerX, sunY, sunR, GxEPD_RED);

  // Rayons
  for (int a = 0; a < 360; a += 45) {
    float r1 = sunR + 3;
    float r2 = sunR + 7;
    float rad = a * PI / 180.0;
    _display.drawLine(centerX + r1 * cos(rad), sunY + r1 * sin(rad),
                      centerX + r2 * cos(rad), sunY + r2 * sin(rad), GxEPD_RED);
  }

  // Textes lever/coucher
  _u8g2.setFont(u8g2_font_helvB12_tf);
  int textOffsetY = 20;
  int wSunrise = _u8g2.getUTF8Width(sun.sunrise.c_str());
  int xSunrise = centerX - arcRadius - wSunrise / 2;
  if (xSunrise < margin + 5)
    xSunrise = margin + 5;
  _u8g2.setCursor(xSunrise, sunBaseY + textOffsetY);
  _u8g2.print(sun.sunrise);

  int wSunset = _u8g2.getUTF8Width(sun.sunset.c_str());
  int xSunset = centerX + arcRadius - wSunset / 2;
  if (xSunset + wSunset > _display.width() - margin - 5)
    xSunset = _display.width() - margin - 5 - wSunset;
  _u8g2.setCursor(xSunset, sunBaseY + textOffsetY);
  _u8g2.print(sun.sunset);

  // Daily variation bar
  _u8g2.setFont(u8g2_font_helvR10_tf);
  int wChange = _u8g2.getUTF8Width(sun.dailyChange.c_str());
  int barPadding = 10;
  int barW = wChange + barPadding * 2;
  if (barW < 60)
    barW = 60;
  int barH = 24;
  int barX = centerX - barW / 2;
  int barY = sunBaseY + 5;
  _display.fillRect(barX, barY, barW, barH, GxEPD_RED);

  _u8g2.setForegroundColor(GxEPD_WHITE);
  _u8g2.setBackgroundColor(GxEPD_RED);
  int yTextVal = barY + (barH - 10) / 2 + 10; // Approx centering
  _u8g2.setCursor(centerX - wChange / 2, yTextVal);
  _u8g2.print(sun.dailyChange);
  _u8g2.setForegroundColor(GxEPD_BLACK);
  _u8g2.setBackgroundColor(GxEPD_WHITE);
}

void EphemerisDisplay::drawSeason(const SeasonData &season) {
  int margin = 10;
  int16_t centerX = _display.width() / 2;
  int sunBaseY = margin + 70;
  int sunBottomLimit = sunBaseY + 30;

  // Recalculate layout metrics for season widget
  int marginBottom = 25;
  int dateBot = _display.height() - margin - marginBottom;

  // Font heights duplicated from drawDate for calculation
  _u8g2.setFont(u8g2_font_helvB18_tf);
  int h_font18 = _u8g2.getFontAscent() - _u8g2.getFontDescent();
  _u8g2.setFont(u8g2_font_logisoso50_tn);
  int h_chiffre = _u8g2.getFontAscent() - _u8g2.getFontDescent();
  _u8g2.setFont(u8g2_font_helvR12_tf);
  int h_info = _u8g2.getFontAscent() - _u8g2.getFontDescent();
  int gap = 8;
  int dateBlockH = h_font18 + gap + h_chiffre + gap + h_font18 + gap + h_info;
  int dateStartY = dateBot - dateBlockH;

  int availableGap = dateStartY - sunBottomLimit;
  int widgetCenterY = sunBottomLimit + availableGap / 2;
  int opticalOffset = 7;
  widgetCenterY += opticalOffset;
  if (availableGap < 40)
    widgetCenterY = sunBottomLimit + 30;

  int circleR = 19;
  int spacing = 55;
  int startX = centerX - (3 * spacing) / 2;

  const char *seasonNames[4];
  if (TimeHelper::getLanguage() == "fr") {
    seasonNames[0] = "PRINT.";
    seasonNames[1] = "ETE";
    seasonNames[2] = "AUT.";
    seasonNames[3] = "HIVER";
  } else {
    seasonNames[0] = "SPRING";
    seasonNames[1] = "SUMMER";
    seasonNames[2] = "AUTUMN";
    seasonNames[3] = "WINTER";
  }

  const char *seasonKeys[4] = {"spring", "summer", "fall", "winter"};
  int seasonDays[4] = {season.daysUntilSpring, season.daysUntilSummer,
                       season.daysUntilFall, season.daysUntilWinter};

  for (int i = 0; i < 4; i++) {
    int cx = startX + i * spacing;
    bool isCur = (season.currentSeason == seasonKeys[i]);
    int val = isCur ? (int)season.seasonProgress : seasonDays[i];
    drawSeasonCircle(cx, widgetCenterY, circleR, seasonNames[i], isCur, val,
                     isCur);
  }
}

void EphemerisDisplay::drawSeasonCircle(int x, int y, int r, const char *label,
                                        bool isCurrent, int value,
                                        bool isPercent) {
  _u8g2.setFont(u8g2_font_helvB08_tf);
  int wLabel = _u8g2.getUTF8Width(label);

  // Label
  _u8g2.setForegroundColor(GxEPD_BLACK);
  _u8g2.setCursor(x - wLabel / 2, y - r - 4);
  _u8g2.print(label);

  // circle
  if (isCurrent) {
    _display.drawCircle(x, y, r, GxEPD_RED);
    _display.drawCircle(x, y, r - 1, GxEPD_RED); // more visible
  } else {
    _display.drawCircle(x, y, r, GxEPD_BLACK);
  }

  // Value inside
  char valStr[10];
  if (isPercent)
    snprintf(valStr, sizeof(valStr), "%d%%", value);
  else
    snprintf(valStr, sizeof(valStr), "%dj", value);

  _u8g2.setFont(u8g2_font_helvR10_tf);
  if (isCurrent)
    _u8g2.setForegroundColor(GxEPD_RED);
  else
    _u8g2.setForegroundColor(GxEPD_BLACK);

  int wVal = _u8g2.getUTF8Width(valStr);
  int hVal = _u8g2.getFontAscent() - _u8g2.getFontDescent();
  _u8g2.setCursor(x - wVal / 2, y + hVal / 2 - 2);
  _u8g2.print(valStr);
}
