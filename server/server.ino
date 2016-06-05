/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "FastLED.h"

FASTLED_USING_NAMESPACE

//const char* ssid     = "Noisebridge Legacy 2.4 gHz";
const char* ssid     = "HOME-5A9C";
const char* password = "8C349C2F81C00327";

#define BRIGHTNESS          96  

int count_leds(const int* num_leds_arr, const int len) {
  int count = 0;
  for (int i = 0; i < len; i++) {
    count += num_leds_arr[i];
  }
  return count;
}

const int NUM_LEDS_PER_STRIP[8] = {277, 277, 24, 39, 14, 9, 105, 65};
const int NUM_LEDS = count_leds(NUM_LEDS_PER_STRIP, 8);

const int WIDTH = NUM_LEDS;
const int HEIGHT = 1;

WiFiUDP Udp;

//static const int kBufferSize = 65535;  // maximum UDP has to offer.
static const int kBufferSize = 5000;
char packetBuffer[kBufferSize];

static CRGB leds[810];

struct ImageMetaInfo {
  int width;
  int height;
  int range;      // Range of gray-levels. We only handle 255 correctly(=1byte)

  // FlaschenTaschen extensions
  int offset_x;   // display image at this x-offset
  int offset_y;   // .. y-offset
  int layer;      // stacked layer
};

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

//   WiFi.begin(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(1337);

  FastLED.addLeds<WS2812B, 1, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 0), NUM_LEDS_PER_STRIP[0]);
  FastLED.addLeds<WS2812B, 2, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 1), NUM_LEDS_PER_STRIP[1]);
  FastLED.addLeds<WS2812B, 3, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 2), NUM_LEDS_PER_STRIP[2]);
  FastLED.addLeds<WS2812B, 4, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 3), NUM_LEDS_PER_STRIP[3]);
  FastLED.addLeds<WS2812B, 5, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 4), NUM_LEDS_PER_STRIP[4]);
  FastLED.addLeds<WS2812B, 6, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 5), NUM_LEDS_PER_STRIP[5]);
  FastLED.addLeds<WS2812B, 7, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 6), NUM_LEDS_PER_STRIP[6]);
  //FastLED.addLeds<WS2812B, 8, GRB>(leds, count_leds(NUM_LEDS_PER_STRIP, 7), NUM_LEDS_PER_STRIP[7]);

  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
//  bzero(packetBuffer, kBufferSize);
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    return;

    int len = Udp.read(packetBuffer, kBufferSize);
    if (len > 0) {
      packetBuffer[len] = 0;
    }

//     Serial.println(packetBuffer);

    ImageMetaInfo img_info = {0};
    img_info.width = WIDTH;
    img_info.height = HEIGHT;

    const char *pixel_pos = ReadImageData(packetBuffer, packetSize,
                                          &img_info);

//    Serial.print("img_info.width: ");
//    Serial.print(img_info.width);
//    Serial.print("img_info.height: ");
//    Serial.print(img_info.height);
//    Serial.println();

    for (int y = 0; y < img_info.height; ++y) {
      for (int x = 0; x < img_info.width; ++x) {
        const byte red = *pixel_pos++;
        const byte green  = *pixel_pos++;
        const byte blue = *pixel_pos++;

//        Serial.print(red);
//        Serial.print(", ");
//        Serial.print(green);
//        Serial.print(", ");
//        Serial.print(blue);
//        Serial.println();

       leds[x + y * img_info.width] = CRGB(red, green, blue);
      }
    }
  }
  FastLED.show();
}

static const char *skipWhitespace(const char *buffer, const char *end) {
 for (;;) {
   while (buffer < end && isspace(*buffer))
     ++buffer;
   if (buffer >= end)
     return NULL;
   if (*buffer == '#') {
     while (buffer < end && *buffer != '\n') // read to end of line.
       ++buffer;
     continue;  // Back to whitespace eating.
   }
   return buffer;
 }
}

// Read next number. Start reading at *start; modifies the *start pointer
// to point to the character just after the decimal number or NULL if reading
// was not successful.
static int readNextNumber(const char **start, const char *end) {
 const char *start_number = skipWhitespace(*start, end);
 if (start_number == NULL) {
   *start = NULL;
   return 0;
 }
 char *end_number = NULL;
 int result = strtol(start_number, &end_number, 10);
 if (end_number == start_number) {
   *start = NULL;
   return 0;
 }
 *start = end_number;
 return result;
}

const char *ReadImageData(const char *in_buffer, size_t buf_len,
                          struct ImageMetaInfo *info) {
  if (in_buffer[0] != 'P' || in_buffer[1] != '6' ||
      (!isspace(in_buffer[2]) && in_buffer[2] != '#')) {
    return in_buffer;  // raw image. No P6 magic header.
  }
  const char *const end = in_buffer + buf_len;
  const char *parse_buffer = in_buffer + 2;
  const int width = readNextNumber(&parse_buffer, end);
  if (parse_buffer == NULL) return in_buffer;
  const int height = readNextNumber(&parse_buffer, end);
  if (parse_buffer == NULL) return in_buffer;
  const int range = readNextNumber(&parse_buffer, end);
  if (parse_buffer == NULL) return in_buffer;
  if (!isspace(*parse_buffer++)) return in_buffer;   // last char before data
  // Now make sure that the rest of the buffer still makes sense
  const size_t expected_image_data = width * height * 3;
  const size_t actual_data = end - parse_buffer;
  if (actual_data < expected_image_data)
    return in_buffer;   // Uh, not enough data.
  if (actual_data > expected_image_data) {
    // Our extension: at the end of the binary data, we provide an optional
    // offset. We can't store it in the header, as it is fixed in number
    // of fields. But nobody cares what is at the end of the buffer.
    const char *offset_data = parse_buffer + expected_image_data;
    info->offset_x = readNextNumber(&offset_data, end);
    if (offset_data != NULL) {
      info->offset_y = readNextNumber(&offset_data, end);
    }
    if (offset_data != NULL) {
      info->layer = readNextNumber(&offset_data, end);
    }
  }
  info->width = width;
  info->height = height;
  info->range = range;
  return parse_buffer;
}
