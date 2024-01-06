#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avr/io.h"
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define N 400 // number of samples to average
#define DATAPOINTS 4500 // number of datapoints kept track of (75 samples/second)
#define MA_SAMPLE_SIZE 3 // number of datapoints for moving average
// #define STD_SIZE 20 // number of samples used to calculate std
// #define STD_MA_SIZE 100 // number of std weighted MAs to keep track of
// #define BLINK_SLOPE 11 // slope threshold for start of blink
// #define BLINK_DIFF_THRESHOLD 40 // difference needed between 'avgs' and 'std_ma' for it to be considered "mid-blink"
// #define BLINK_COOLDOWN_MAX 30 // ticks after a blink edge where another blink edge can't be recorded
// #define STD_THRESHOLD 10 // threshold for non blinking std values
#define STD_SIZE 20 // number of samples used to calculate std
#define STD_MA_SIZE 200 // number of std weighted MAs to keep track of
#define BLINK_SLOPE 3 // slope threshold for start of blink
#define BLINK_DIFF_THRESHOLD 10 // difference needed between 'avgs' and 'std_ma' for it to be considered "mid-blink"
#define BLINK_COOLDOWN_MAX 30 // ticks after a blink edge where another blink edge can't be recorded
#define STD_THRESHOLD 5 // threshold for non blinking std values
#define INDEX_SUBTRACT(i, d, s) (((i) - (d) + (s)) % (s)) // returns index 'd' in front of 'i' in circular array of size s
#define BLINK_FREQ_THRESHOLD 12 // blinks per min
#define BLINK_DUR_THRESHOLD 500 // duration in ms

// ######################## Bluetooth setup #######################
#define FACTORYRESET_ENABLE 1
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

short sample[95] = {2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2840,2850,2850,2840,2820,2810,2800,2790,2790,2770,2760,2770,2760,2750,2760,2760,2750,2760,2750,2760,2760,2760,2750,2750,2770,2770,2750,2760,2760,2770,2780,2780,2790,2790,2790,2800,2800,2810,2810,2810,2810,2810,2810,2830,2810,2820,2820,2820,2830,2840,2830,2830,2830,2840,2840,2840,2840};

short data[MA_SAMPLE_SIZE]; // circular array of IR measurements
short avgs[DATAPOINTS]; // circular array moving averages of measurements
float stds[STD_MA_SIZE]; // circular array moving averages of measurements
float std_ma[STD_MA_SIZE];
char blinks[DATAPOINTS]; // circular array of blink state (2 = blink edge (for frequency), 1 = blinking (for duration), 0 = no blink)
int n; // index for data arrays
int b; // boolean for blink
int c = 0; // counter for sampling
int s = 0; // sum for averaging of samples
short blink_cooldown = BLINK_COOLDOWN_MAX;
short frequency = 0;
unsigned long duration = 0; // blink duration in ticks
unsigned long start_time;
unsigned long last_BT_sent = 0;
int period = 0;

void setup() {
  // Serial.begin(9600);

  // Bluetooth setup
  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit AT Command Example"));
  Serial.println(F("-------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  // disable echo from console 
  ble.echo(false);
  
  ble.println("AT+GATTCLEAR");
  ble.waitForOK();  

  // define new service with UUID 0x180E
//  ble.println("AT+GATTADDSERVICE=UUID128=00-00-fc-5c-00-00-10-00-80-00-00-80-5F-9B-34-FB");
//  ble.println("AT+GATTADDSERVICE=UUID128=75-dc-fc-5c-7c-7f-49-b2-b9-2f-c4-b5-af-81-85-40");
  ble.println("AT+GATTADDSERVICE=UUID=0x180E");
  ble.waitForOK();  

  // define new characteristic with UUID 0x2A46
  ble.println("AT+GATTADDCHAR=UUID=0x2A46,PROPERTIES=0x02,MIN_LEN=1,MAX_LEN=20,VALUE=65");
  ble.waitForOK(); 
  
//  ble.println("AT+GATTADDCHAR=UUID=0x2A47,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=20,VALUE=65");
//  ble.waitForOK(); 

  ble.println("ATZ");
  ble.waitForOK();

  for (int i = 0; i < DATAPOINTS; i++) {
    avgs[i] = 0;
    blinks[i] = 0;
  }
  for (int i = 0; i < MA_SAMPLE_SIZE; i++) {
    data[i] = 0;
  }
  n = 0;
  b = 0;
  c = 0;
  s = 0;
  start_time = millis();
}

void loop() {
  int sensorValue = analogRead(A0); // get reading from IR sensor

  c = (c + 1) % N;
  s += sensorValue;

  if (c == 0) {
    // removing datapoint
    if (blinks[n % DATAPOINTS] >= 1) {
      duration -= period;
      if (blinks[n % DATAPOINTS] >= 2) {
        frequency--;
      }
    }

    // adding new datapoint
    sensorValue = s / N;
    // sensorValue = sample[n % 95];
    s = 0;
  
    data[n % MA_SAMPLE_SIZE] = (short) sensorValue;
    if (n >= MA_SAMPLE_SIZE) {
      avgs[n % DATAPOINTS] = (short) running_average(data, MA_SAMPLE_SIZE, n % MA_SAMPLE_SIZE, MA_SAMPLE_SIZE);
    } else {
      avgs[n] = data[n];
    }
    // Serial.printf("%d\n", avgs[n % DATAPOINTS]);

    stds[n % STD_MA_SIZE] = standard_dev(n % DATAPOINTS, n >= STD_SIZE ? STD_SIZE : n);

    std_ma[n % STD_MA_SIZE] = std_weighted_ma(n >= STD_MA_SIZE ? STD_MA_SIZE : n);
    
    // Serial.printf("%d: %.2f\n", n % DATAPOINTS, avgs[n % DATAPOINTS]);
    blinks[n % DATAPOINTS] = 2 * check_blink_edge(n % DATAPOINTS);
    blinks[n % DATAPOINTS] += check_if_blinking();

    Serial.print("blink:");
    if (blinks[n % DATAPOINTS] >= 1) {
      duration += millis() - start_time;
      if (blinks[n % DATAPOINTS] >= 2) {
        frequency++;
        Serial.print("500\t");
//        Serial.print("400\t");
      } else {
        Serial.print("400\t");
      }
    } else {
      Serial.print("300\t");
    }
  
    period = millis() - start_time;
    int blink_count = (((float) frequency) / DATAPOINTS) / (period / 1000.0) * 60; // blinks per minute
    float dur_avg = duration / 1000.0 / ((float) frequency);

    start_time = millis();
  
    display_serial();

    if ((int) (start_time / 1000) > (int) (last_BT_sent / 1000)) {
      send_via_BT(blink_count, dur_avg);
      last_BT_sent = start_time;
    }
    
    n++;
  }
}

// calculate 'num_points' elements average
float running_average(short* data_array, int array_size, int index, int num_points) {
  float sum = 0;
  for (int i = 0; i < num_points; i++) {
    sum += (float) data_array[INDEX_SUBTRACT(index, i, array_size)];
  }
  // Serial.printf("sum: %f\n", sum);
  return sum / (float) num_points;
}

float standard_dev(int index, int num_points) {
  float avg = running_average(avgs, DATAPOINTS, index, num_points);
  float sum = 0;
  for (int i = 0; i < num_points; i++) {
    sum += (avgs[INDEX_SUBTRACT(index, i, DATAPOINTS)] - avg) * (avgs[INDEX_SUBTRACT(index, i, DATAPOINTS)] - avg);
  }
  return sqrt(sum / (float) num_points);
}

float std_weighted_ma(int num_points) {
  float sum = 0, divisor = 0;
  for (int i = 1; i < num_points; i++) {
    if (stds[INDEX_SUBTRACT(n % STD_MA_SIZE, i, STD_MA_SIZE)] < STD_THRESHOLD) {
      sum += avgs[INDEX_SUBTRACT(n % DATAPOINTS, i, DATAPOINTS)];
      divisor++;
    }
  }
  if (sum == 0) {
    return (float) avgs[n % DATAPOINTS];
  } else {
    return sum / divisor;
  }
}

char check_blink_edge(int index) {
  if (blink_cooldown > 0) {
    blink_cooldown--;
    return 0;
  }
  // Serial.printf("%d\n", abs(avgs[index] - avgs[INDEX_SUBTRACT(index, 1, DATAPOINTS)]));
  if (abs(avgs[index] - avgs[INDEX_SUBTRACT(index, 1, DATAPOINTS)]) > BLINK_SLOPE) {
    blink_cooldown = BLINK_COOLDOWN_MAX;
    return 1;
  }
  return 0;
}

char check_if_blinking() {
  if (n < STD_MA_SIZE) {
    return 0;
  }
  if (abs(avgs[n % DATAPOINTS] - std_ma[n % STD_MA_SIZE]) > 1.5 * BLINK_DIFF_THRESHOLD) {
    return 1;
  }
  for (int i = 0; i < STD_MA_SIZE; i++) {
    if (abs(avgs[n % DATAPOINTS] - std_ma[n % STD_MA_SIZE]) < BLINK_DIFF_THRESHOLD) {
      return 0;
    }
  }
  return 1;
}

void send_via_BT(int i, float f) {
  // Serial.printf("sending via bluetooth: %d,%.2f\n", i, f);
  ble.printf("AT+GATTCHAR=1,%d/%.2f\n", i, f);
  ble.waitForOK();
}

void display_serial() {
  Serial.print("min:200\tmax:500\t");
  Serial.print("data:");
  Serial.print(avgs[n % DATAPOINTS]);
  // Serial.print("\tblink:");
  // Serial.print(blinks[n % DATAPOINTS]);
  Serial.print("\tstd:");
  Serial.print(stds[n % STD_MA_SIZE]);
  Serial.print("\tstd_ma:");
  Serial.print(std_ma[n % STD_MA_SIZE]);
  Serial.println();
}