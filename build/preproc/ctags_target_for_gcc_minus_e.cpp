# 1 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino"
/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/

# 8 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 2
# 9 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 2

# 11 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 2
# 12 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 2

# 14 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 2
# 15 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 2

OLED_Display display;
ESP8266Timer ITimer;


int32_t lastScanMillis = -5000;
int len;

uint8_t state = 0;

bool btnSingle = 0, btnLong = 0;
Switch btn(12);

struct WifiEntry
{
    int rssi;
    int channel;
    char ssid[64];
    char bssid[64];
} wifiEntries[64], lastSelected = {0};

int cmpfunc(const void *a, const void *b)
{
    return (((WifiEntry *)b)->rssi - ((WifiEntry *)a)->rssi);
}

void btnCallbackSingle(void *s)
{
    btnSingle = 1;
}

void btnCallbackLong(void *s)
{
    btnLong = 1;
}

void timer2ISR()
{
    btn.poll();
}

void setup()
{
    Serial.begin(74880);
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 59 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "Soldered-WiFi-Scanner-Meter-Kit.ino" "." "59" "." "15" "\", \"aSM\", @progbits, 1 #"))) = (
# 59 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino"
                  "\nESP8266 WiFi scan example"
# 59 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino" 3
                  ); &__pstr__[0];}))
# 59 "/Users/nitkonitkic/Documents/Code/Soldered-WiFi-Scanner-Meter-Kit/Soldered-WiFi-Scanner-Meter-Kit.ino"
                  ))));

    display.begin();

    // Set WiFi to station mode
    WiFi.mode(WIFI_STA);

    // Disconnect from an AP if it was previously connected
    WiFi.disconnect();
    delay(100);

    ITimer.attachInterruptInterval(1000, timer2ISR);
    btn.setSingleClickCallback(&btnCallbackSingle, (void *)"0");
    btn.setLongPressCallback(&btnCallbackLong, (void *)"0");
}

void scan()
{
    if (millis() - lastScanMillis > 5000)
    {
        WiFi.scanNetworks(true);
        Serial.print("\nScan start ... ");
        lastScanMillis = millis();
    }

    int scanResult = WiFi.scanComplete();
    if (scanResult <= 0)
        return;

    len = scanResult;
    for (int8_t i = 0; i < len && i < 64; i++)
    {
        wifiEntries[i].rssi = WiFi.RSSI(i);
        wifiEntries[i].channel = WiFi.channel(i);
        strcpy(wifiEntries[i].ssid, WiFi.SSID(i).c_str());
        strcpy(wifiEntries[i].bssid, WiFi.BSSIDstr(i).c_str());

        if (strcmp(wifiEntries[i].bssid, lastSelected.bssid) == 0 &&
            strcmp(wifiEntries[i].ssid, lastSelected.ssid) == 0)
            lastSelected.rssi = wifiEntries[i].rssi, lastSelected.channel = wifiEntries[i].channel;
    }
    WiFi.scanDelete();

    // Sort scan results
    qsort(wifiEntries, len, sizeof(WifiEntry), cmpfunc);
}

int findIndex()
{
    for (int8_t i = 0; i < len && i < 64; ++i)
    {
        if (strcmp(wifiEntries[i].bssid, lastSelected.bssid) == 0)
            return i;
    }
    return 0;
}

void drawMenu()
{
    if (len == 0)
    {
        display.clearDisplay();

        display.setTextSize(1);
        display.setCursor(47, 57);
        display.setTextColor(1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/);

        display.drawBitmap(0, -4, splash1_data, 128, 64, 1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/);
        display.println("NO WIFI");
        display.setTextWrap(false);

        display.display();
    }
    else if (len > 0)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(0 /*|< Draw 'off' pixels*/ /*|< Draw 'off' pixels*/, 1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/);
        display.setCursor(2, 0);
        display.setTextWrap(false);

        display.drawRect(0, 0, 2, 8, 1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/);
        display.println("rssi|ch| ssid                             ");

        display.setTextColor(1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/);

        if (lastSelected.rssi == 0)
            lastSelected = wifiEntries[0];

        int idx = findIndex();
        if (btnSingle && state == 0)
        {
            idx = (idx + 1) % min(len, 64);

            Serial.println(idx);
            Serial.println(min(len, 64));
            Serial.println();

            lastSelected = wifiEntries[idx];
            btnSingle = 0;
        }

        int lo, hi;
        if (len <= 7)
            lo = 0, hi = len;
        else if (idx < 3)
            lo = 0, hi = min(idx + 7, len);
        else if (idx > len - 4)
            lo = len - 7, hi = len;
        else
            lo = idx - 3, hi = idx + 4;

        for (int8_t i = lo; i < hi; ++i)
        {
            char buf[128];
            sprintf(buf, "%4d|%2d| %s\n", wifiEntries[i].rssi, wifiEntries[i].channel, wifiEntries[i].ssid);

            if (idx == i)
                buf[0] = '>';

            display.setCursor(2, display.getCursorY());
            display.print(buf);
        }

        display.display();
    }
}

void drawPolarLine(OLED_Display &display, int x, int y, float phi, int from, int to, float a = 1.0f, float b = 1.0f)
{
    int x1 = x + (int)(cos(phi) * a * (float)from);
    int y1 = y + (int)(sin(phi) * b * (float)from);
    int x2 = x + (int)(cos(phi) * a * (float)to);
    int y2 = y + (int)(sin(phi) * b * (float)to);

    display.drawLine(x1, y1, x2, y2, 1 /*|< Draw 'on' pixels*/);
}

void drawDetail(bool analog)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/);

    display.setCursor(2, 47);
    display.print("SSID: ");
    display.println(lastSelected.ssid);

    display.setCursor(2, display.getCursorY() + 1);

    display.print("Strength: ");
    if (lastSelected.rssi == 0 || lastSelected.rssi == -1)
        display.println("No signal");
    else if (lastSelected.rssi > -70)
        display.println("Excellent");
    else if (lastSelected.rssi > -80)
        display.println("Good");
    else if (lastSelected.rssi > -90)
        display.println("Fair");
    else
        display.println("Poor or No signal");

    if (analog)
    {
        for (int i = 0; i <= 10; ++i)
            drawPolarLine(display, 64, 40, 3.1415926535897932384626433832795 * i / 10 - 3.1415926535897932384626433832795, (i % 2 == 0 ? 22 : 27), 30, 1.2, 1.0);
        display.setCursor(5, 36);
        display.print("-90");
        display.setCursor(107, 37);
        display.print("-50");
        display.setCursor(55, 0);
        display.print("-70");
        display.setCursor(19, 11);
        display.print("-80");
        display.setCursor(90, 11);
        display.print("-60");

        drawPolarLine(display, 64, 40,
                      3.1415926535897932384626433832795 * map(lastSelected.rssi, -90, -50, 0, 100) / 100 - 3.1415926535897932384626433832795,
                      0, 28, 1.2, 1.0);
    }
    else
    {
        display.setFont(&Roboto_20);
        display.setCursor(9, 36);
        display.setTextSize(2);
        display.print(lastSelected.rssi);
        display.setTextSize(1);
        display.print("dBm");
    }

    display.setFont();
    display.display();
}

void loop()
{
    scan();

    if (btnLong && len)
        state = !state, btnLong = 0;

    if (btnSingle && len && state == 1)
        state = 2, btnSingle = 0;
    if (btnSingle && len && state == 2)
        state = 1, btnSingle = 0;

    if (state == 0)
        drawMenu();
    else if (state == 1)
        drawDetail(false);
    else if (state == 2)
        drawDetail(true);

    delay(10);
}
