// ================== การวาดหน้าจอ ==================

// ---------- ตัวช่วยข้อความ ----------
int advWidth(const char* s, const GFXfont* f) {
  if (!f) return strlen(s) * 6;
  int w = 0;
  for (const char* p = s; *p; p++) {
    uint8_t c = (uint8_t)*p;
    if (c < f->first || c > f->last) continue;
    w += f->glyph[c - f->first].xAdvance;
  }
  return w;
}

// หมายเหตุ: ฟอนต์ปกติ (nullptr) y = ขอบบน, ฟอนต์ Free* y = เส้นฐาน
void printAt(const char* s, int x, int y, const GFXfont* f) {
  display.setFont(f);
  display.setCursor(x, y);
  display.print(s);
}
void printCenter(const char* s, int y, const GFXfont* f) { printAt(s, 64 - advWidth(s, f) / 2, y, f); }
void printRight(const char* s, int xr, int y, const GFXfont* f) { printAt(s, xr - advWidth(s, f), y, f); }

// ---------- ไอคอน (วาดด้วยเส้น) ----------
void iconWifi(int x, int y) {
  display.fillRect(x, y + 5, 2, 2, WHITE);
  display.fillRect(x + 3, y + 3, 2, 4, WHITE);
  display.fillRect(x + 6, y + 1, 2, 6, WHITE);
}
void iconHourglass(int x, int y) {
  display.drawFastHLine(x, y, 7, WHITE);
  display.drawFastHLine(x, y + 7, 7, WHITE);
  display.drawLine(x + 1, y + 1, x + 5, y + 6, WHITE);
  display.drawLine(x + 5, y + 1, x + 1, y + 6, WHITE);
}
void iconStopwatch(int x, int y) {
  display.drawCircle(x + 3, y + 4, 3, WHITE);
  display.drawFastVLine(x + 3, y + 2, 3, WHITE);
  display.drawPixel(x + 3, y, WHITE);
}
void iconThermoSmall(int x, int y) {
  display.drawRoundRect(x + 1, y, 3, 7, 1, WHITE);
  display.fillCircle(x + 2, y + 7, 2, WHITE);
}
void iconDropSmall(int x, int y) {
  display.fillTriangle(x + 2, y, x, y + 4, x + 4, y + 4, WHITE);
  display.fillCircle(x + 2, y + 5, 2, WHITE);
}
void iconThermoBig(int x, int y) {
  display.drawRoundRect(x + 3, y, 6, 16, 3, WHITE);
  display.fillCircle(x + 6, y + 18, 5, WHITE);
  display.fillRect(x + 5, y + 6, 2, 12, WHITE);
}
void iconDropBig(int x, int y) {
  display.fillTriangle(x + 6, y, x + 1, y + 11, x + 11, y + 11, WHITE);
  display.fillCircle(x + 6, y + 13, 5, WHITE);
  display.drawFastVLine(x + 4, y + 11, 4, BLACK);
}
void degree(int x, int y, int r) { display.drawCircle(x + r, y + r, r, WHITE); }

// ---------- แถบบน ----------
void drawStatusBar(const char* left) {
  printAt(left, 0, 1, nullptr);
  int x = 128;
  if (portalOn) { x -= 12; printAt("AP", x, 1, nullptr); x -= 3; }
  if (cfg.wifiOn && cfg.ssid.length()) {
    x -= 8;
    bool c = WiFi.status() == WL_CONNECTED;
    if (c || (millis() / 500) % 2) iconWifi(x, 1);
    x -= 4;
  }
  if (tmrState == TM_RUN || tmrState == TM_PAUSE) { x -= 7; iconHourglass(x, 1); x -= 4; }
  if (swRunning) { x -= 7; iconStopwatch(x, 1); x -= 4; }
  for (int i = 0; i < 128; i += 2) display.drawPixel(i, 10, WHITE);
}

void drawTitle(const char* t, const char* right) {
  display.fillRoundRect(0, 0, 128, 11, 2, WHITE);
  display.setTextColor(BLACK);
  printAt(t, 4, 2, nullptr);
  if (right) printRight(right, 124, 2, nullptr);
  display.setTextColor(WHITE);
}

void drawProgress(int y, float frac) {
  if (frac < 0) frac = 0;
  if (frac > 1) frac = 1;
  display.drawRoundRect(14, y, 100, 5, 2, WHITE);
  int w = (int)(96 * frac);
  if (w > 0) display.fillRect(16, y + 1, w, 3, WHITE);
}

// ---------- หน้า: นาฬิกา ----------
void drawClockPage() {
  char buf[24];
  snprintf(buf, sizeof buf, "%s %02d %s", DOW[tnow.dayOfTheWeek()], tnow.day(), MON[tnow.month() - 1]);
  drawStatusBar(buf);

  bool pm;
  int h = dispHour(&pm);
  char hh[4], mm[4];
  snprintf(hh, sizeof hh, "%02d", h);
  snprintf(mm, sizeof mm, "%02d", tnow.minute());

  const GFXfont* F = &FreeSansBold18pt7b;
  int wH = advWidth(hh, F), wM = advWidth(mm, F), gap = 10;
  int total = wH + gap + wM;
  int x0 = (128 - total) / 2 - (cfg.h12 ? 7 : 0);
  int base = 40;
  printAt(hh, x0, base, F);
  printAt(mm, x0 + wH + gap, base, F);
  if (millis() - secStartMs < 500) {                 // โคลอนกะพริบตามวินาที
    int cx = x0 + wH + gap / 2 - 2;
    display.fillRoundRect(cx, base - 19, 4, 4, 1, WHITE);
    display.fillRoundRect(cx, base - 8, 4, 4, 1, WHITE);
  }
  if (cfg.h12) printAt(pm ? "PM" : "AM", x0 + total + 3, 16, nullptr);

  drawProgress(45, tnow.second() / 59.0);            // แถบวินาที

  // แถวล่าง: อุณหภูมิ / ความชื้น
  iconThermoSmall(0, 54);
  if (isnan(roomTemp)) strcpy(buf, "--.-"); else snprintf(buf, sizeof buf, "%.1f", roomTemp);
  printAt(buf, 8, 56, nullptr);
  int x = 8 + advWidth(buf, nullptr) + 1;
  degree(x, 56, 1);
  printAt("C", x + 4, 56, nullptr);

  if (isnan(roomHum)) strcpy(buf, "--%"); else snprintf(buf, sizeof buf, "%.0f%%", roomHum);
  int w = advWidth(buf, nullptr);
  printRight(buf, 128, 56, nullptr);
  iconDropSmall(128 - w - 7, 54);

  snprintf(buf, sizeof buf, "%04d", tnow.year());
  printCenter(buf, 56, nullptr);
}

// ---------- หน้า: อุณหภูมิ / ความชื้น ----------
const char* comfortLabel() {
  if (isnan(roomTemp)) return "";
  if (roomTemp >= 30) return "HOT";
  if (roomTemp <= 18) return "COLD";
  if (roomHum >= 70) return "HUMID";
  if (roomHum <= 30) return "DRY";
  return "COMFY";
}

void drawClimatePage() {
  char buf[16];
  bool pm;
  int h = dispHour(&pm);
  if (cfg.h12) snprintf(buf, sizeof buf, "%d:%02d %s", h, tnow.minute(), pm ? "PM" : "AM");
  else snprintf(buf, sizeof buf, "%02d:%02d", h, tnow.minute());
  drawStatusBar(buf);

  const GFXfont* F = &FreeSansBold12pt7b;

  iconThermoBig(2, 14);
  if (isnan(roomTemp)) strcpy(buf, "--.-"); else snprintf(buf, sizeof buf, "%.1f", roomTemp);
  printAt(buf, 20, 35, F);
  int x = 20 + advWidth(buf, F) + 1;
  degree(x, 18, 2);
  printAt("C", x + 7, 18, nullptr);

  iconDropBig(2, 40);
  if (isnan(roomHum)) strcpy(buf, "--"); else snprintf(buf, sizeof buf, "%.0f", roomHum);
  printAt(buf, 20, 61, F);
  printAt("%", 20 + advWidth(buf, F) + 2, 47, nullptr);

  if (!isnan(tMax)) {
    snprintf(buf, sizeof buf, "H%.1f", tMax);
    printRight(buf, 128, 15, nullptr);
    snprintf(buf, sizeof buf, "L%.1f", tMin);
    printRight(buf, 128, 25, nullptr);
  }

  const char* c = comfortLabel();
  if (*c) {
    int w = advWidth(c, nullptr) + 8;
    display.fillRoundRect(128 - w, 47, w, 12, 4, WHITE);
    display.setTextColor(BLACK);
    printAt(c, 128 - w + 4, 49, nullptr);
    display.setTextColor(WHITE);
  }
}

// ---------- หน้า: จับเวลา ----------
void drawBigDuration(unsigned long sec, int base, const char* suffix) {
  char buf[16];
  const GFXfont* F;
  if (sec >= 3600) {
    snprintf(buf, sizeof buf, "%d:%02d:%02d", (int)(sec / 3600), (int)(sec / 60 % 60), (int)(sec % 60));
    F = &FreeSansBold12pt7b;
  } else {
    snprintf(buf, sizeof buf, "%02d:%02d", (int)(sec / 60), (int)(sec % 60));
    F = &FreeSansBold18pt7b;
  }
  const GFXfont* S = &FreeSans9pt7b;
  int w = advWidth(buf, F) + (suffix ? advWidth(suffix, S) : 0);
  int x = (128 - w) / 2;
  printAt(buf, x, base, F);
  if (suffix) printAt(suffix, x + advWidth(buf, F), base, S);
}

void drawStopwatchPage() {
  drawStatusBar("STOPWATCH");
  unsigned long ms = swElapsed();
  char cs[5];
  snprintf(cs, sizeof cs, ".%02d", (int)(ms % 1000 / 10));
  drawBigDuration(ms / 1000, 40, cs);

  const char* hint;
  if (swRunning) hint = "hold: pause";
  else if (ms > 0) hint = "hold: go  3s: reset";
  else hint = "hold hand: start";
  printCenter(hint, 54, nullptr);

  // ไอคอนสถานะ
  if (swRunning) display.fillTriangle(2, 46, 2, 52, 7, 49, WHITE);
  else if (ms > 0) { display.fillRect(2, 46, 2, 7, WHITE); display.fillRect(5, 46, 2, 7, WHITE); }
}

// ---------- หน้า: นับถอยหลัง ----------
void drawTimerPage() {
  drawStatusBar("TIMER");
  unsigned long left = timerLeftMs();
  unsigned long s = (left + 999) / 1000;
  drawBigDuration(s, 38, nullptr);

  if (tmrState == TM_EDIT) {
    if ((millis() / 400) % 2) display.drawRoundRect(4, 12, 120, 32, 6, WHITE);
  } else if (tmrState == TM_RUN || tmrState == TM_PAUSE) {
    drawProgress(44, left / (tmrTotal * 1000.0));
  }

  const char* hint = "hold hand: set";
  if (tmrState == TM_EDIT) hint = "wave: +  hold: start";
  else if (tmrState == TM_RUN) hint = "hold: pause";
  else if (tmrState == TM_PAUSE) hint = "hold: go  3s: reset";
  printCenter(hint, 54, nullptr);
}

void drawAlarm() {
  display.invertDisplay((millis() / 400) % 2);
  printCenter("TIME UP!", 32, &FreeSansBold12pt7b);
  printCenter("wave to stop", 46, nullptr);
}

// ---------- เมนู ----------
void drawMenu() {
  char idx[8];
  snprintf(idx, sizeof idx, "%d/%d", menuSel + 1, MI_COUNT);
  drawTitle("SETTINGS", idx);

  static int top = 0;
  if (menuSel < top) top = menuSel;
  if (menuSel > top + 3) top = menuSel - 3;

  for (int r = 0; r < 4; r++) {
    int i = top + r;
    if (i >= MI_COUNT) break;
    int y = 13 + r * 13;
    bool sel = (i == menuSel);
    if (sel) {
      display.fillRoundRect(0, y, 123, 12, 3, WHITE);
      display.setTextColor(BLACK);
    }
    printAt(MENU_LABELS[i], 4, y + 2, nullptr);
    String v = menuValue(i);
    printRight(v.c_str(), 119, y + 2, nullptr);
    display.setTextColor(WHITE);
  }
  // แถบเลื่อน
  int barH = 51 * 4 / MI_COUNT;
  int barY = 13 + (51 - barH) * menuSel / (MI_COUNT - 1);
  for (int y = 13; y < 64; y += 2) display.drawPixel(126, y, WHITE);
  display.fillRect(125, barY, 3, barH, WHITE);
}

// ---------- ตัวแก้ค่า (ตั้งเวลา/วันที่ ฯลฯ) ----------
void drawEditor() {
  const char* titles[] = {"SET TIME", "SET DATE", "TIME ZONE", "DARK LEVEL"};
  drawTitle(titles[editKind], nullptr);

  const GFXfont* F = &FreeSansBold12pt7b;
  char parts[3][10];
  const char* sep = "";
  switch (editKind) {
    case ED_TIME:
      snprintf(parts[0], 10, "%02d", editVal[0]);
      snprintf(parts[1], 10, "%02d", editVal[1]);
      sep = ":";
      break;
    case ED_DATE:
      snprintf(parts[0], 10, "%02d", editVal[0]);
      snprintf(parts[1], 10, "%02d", editVal[1]);
      snprintf(parts[2], 10, "%04d", editVal[2]);
      sep = "/";
      break;
    case ED_TZ:
      snprintf(parts[0], 10, "UTC%+d", editVal[0]);
      break;
    case ED_DARK:
      snprintf(parts[0], 10, "%d", DARK_STEPS[editVal[0]]);
      break;
  }

  int total = 0;
  for (int i = 0; i < editN; i++) total += advWidth(parts[i], F);
  total += (editN - 1) * advWidth(sep, F);
  int x = (128 - total) / 2, y = 36;
  for (int i = 0; i < editN; i++) {
    int w = advWidth(parts[i], F);
    printAt(parts[i], x, y, F);
    if (i == editIdx) display.fillRect(x, y + 3, w - 2, 3, WHITE);
    x += w;
    if (i < editN - 1) { printAt(sep, x, y, F); x += advWidth(sep, F); }
  }

  if (editKind == ED_DARK) {
    char b[24];
    snprintf(b, sizeof b, "light now: %d", light);
    printCenter(b, 44, nullptr);
  }
  printCenter(editIdx < editN - 1 ? "wave: +1   hold: next" : "wave: +1   hold: save", 55, nullptr);
}

// ---------- ข้อมูลระบบ ----------
void drawInfo() {
  drawTitle("INFO", "v" FW_VERSION);
  char b[32];
  unsigned long up = millis() / 60000;
  snprintf(b, sizeof b, "Up: %lud %luh %lum", up / 1440, up / 60 % 24, up % 60);
  printAt(b, 0, 14, nullptr);
  printAt(("WiFi: " + wifiStatusText()).c_str(), 0, 24, nullptr);
  String ip = "-";
  if (WiFi.status() == WL_CONNECTED) ip = WiFi.localIP().toString();
  else if (portalOn) ip = WiFi.softAPIP().toString();
  printAt(("IP: " + ip).c_str(), 0, 34, nullptr);
  if (ntpOk) snprintf(b, sizeof b, "NTP: %lu min ago", (millis() - lastNtpMs) / 60000);
  else strcpy(b, "NTP: not synced");
  printAt(b, 0, 44, nullptr);
  snprintf(b, sizeof b, "RTC temp: %.1fC", rtc.getTemperature());
  printAt(b, 0, 54, nullptr);
}

// ---------- หน้าตั้งค่า WiFi ผ่านมือถือ ----------
void drawPortal() {
  drawTitle("WIFI SETUP", nullptr);
  printAt("Join: " AP_SSID, 0, 14, nullptr);
  printAt("Pass: " AP_PASS, 0, 24, nullptr);
  printAt("Open: 192.168.4.1", 0, 34, nullptr);
  char b[24];
  snprintf(b, sizeof b, "Phones: %d", WiFi.softAPgetStationNum());
  printAt(b, 0, 44, nullptr);
  printCenter("hold: close", 55, nullptr);
}

// ---------- ส่วนซ้อนทับ ----------
void drawHoldBar() {
  long d = holdMs();
  if (d < 200) return;                       // ไม่โชว์ตอนโบกสั้นๆ
  bool chained = holdChained();
  if (!holdDefer && !chained) {              // แถบไปถึง "ตกลง"
    display.fillRect(0, 60, 128, 4, BLACK);
    display.fillRect(0, 61, (int)(128 * min(1.0f, d / (float)LONG_MS)), 3, WHITE);
    return;
  }
  // กล่องรีเซ็ต: ค้างจนแถบเต็ม = รีเซ็ต
  display.fillRoundRect(0, 38, 128, 26, 4, BLACK);
  display.drawRoundRect(0, 38, 128, 26, 4, WHITE);
  if (chained) {
    printCenter("keep holding:", 41, nullptr);
    printCenter("RESET", 50, nullptr);
  } else if (d >= (long)LONG_MS) {
    printAt("let go : RESUME", 6, 41, nullptr);
    printAt("hold   : RESET", 6, 50, nullptr);
  } else {
    printCenter("keep holding...", 45, nullptr);
  }
  display.fillRect(2, 60, (int)(124 * min(1.0f, d / (float)VLONG_MS)), 2, WHITE);
}

void drawToast() {
  if ((long)(toastUntil - millis()) <= 0) return;
  int w = advWidth(toastMsg, nullptr) + 14;
  int x = (128 - w) / 2;
  display.fillRoundRect(x, 23, w, 17, 5, BLACK);
  display.drawRoundRect(x, 23, w, 17, 5, WHITE);
  printAt(toastMsg, x + 7, 28, nullptr);
}

void drawSplash() {
  display.clearDisplay();
  display.drawRoundRect(14, 8, 100, 48, 8, WHITE);
  printCenter("CLOCK", 36, &FreeSansBold12pt7b);
  printCenter("v" FW_VERSION, 42, nullptr);
  display.display();
  delay(900);
}

// ---------- วาดทั้งหมด ----------
void render() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (tmrState == TM_ALARM) {
    drawAlarm();
  } else {
    switch (uiMode) {
      case UI_PAGE:
        switch (page) {
          case PG_CLOCK:     drawClockPage(); break;
          case PG_CLIMATE:   drawClimatePage(); break;
          case PG_STOPWATCH: drawStopwatchPage(); break;
          case PG_TIMER:     drawTimerPage(); break;
        }
        break;
      case UI_MENU:   drawMenu(); break;
      case UI_EDIT:   drawEditor(); break;
      case UI_INFO:   drawInfo(); break;
      case UI_PORTAL: drawPortal(); break;
    }
  }
  drawHoldBar();
  drawToast();
  display.display();
}
