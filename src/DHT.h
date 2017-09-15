/**
 * @file DHT.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2017, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef DHT_H
#define DHT_H

#include "GPIO.h"

/**
 * DHT Humidity & Temperature Sensor device driver template class.
 * @param[in] DATA_PIN board pin for data signal.
 * @param[in] DEVICE type number (11, 21 or 22).
 * @section Circuit
 * @code
 *                           DHTXX
 *                       +------------+
 * (VCC)---------------1-|VCC  ====== |
 * (DATA_PIN)----------2-|DATA ====== |
 *                     3-|     ====== |
 * (GND)---------------4-|GND  ====== |
 *                       +------------+
 * @endcode
 * Connect DHT to data pin, VCC and ground. A pullup resistor from
 * the pin to VCC should be used. Most DHT modules have a built-in
 * pullup resistor.
 */
template<BOARD::pin_t DATA_PIN, uint8_t DEVICE>
class DHT {
public:
  /**
   * Initiate device communication pin.
   */
  DHT() :
    m_humidity(0),
    m_temperature(0)
  {
    m_data.open_collector();
  }

  /**
   * Return latest humidity reading.
   * @return humidity.
   */
  float humidity()
  {
    return (m_humidity);
  }

  /**
   * Return latest temperature reading.
   * @return temperature.
   */
  float temperature()
  {
    return (m_temperature);
  }

  /**
   * Read humidity and temperature from device. Returns number of changed
   * values (0-2) otherwise negative error code (-1: check sum error,
   * -2: response pulse error)
   */
  int read(float& humidity, float& temperature)
  {
    // Issue start signal and wait for device to respond
    m_data.output();
    delay(START_SIGNAL);
    m_data.input();
    int retry = 16;
    do {
      delayMicroseconds(PULLUP);
    } while (m_data && --retry);
    if (retry == 0 || m_data.pulse() < THRESHOLD) return (-2);

    // Read data from the device. Each bit is pulse width coded;
    // low for 50 us, and high 26-28 us for zero(0), and 70 us for one(1).
    uint8_t chsum = 0;
    uint8_t d[5];
    for (int i = 0; i < 5; i++) {
      uint8_t v = 0;
      for (int j = 0; j < 8; j++) {
	uint8_t width = m_data.pulse();
	v = (v << 1) | (width > THRESHOLD);
      }
      d[i] = v;
      if (i < 4) chsum += v;
    }

    // Validate check sum
    if (chsum != d[4]) return (-1);

    // Check type of device and convert data
    if (DEVICE == 11) {
      humidity = d[0];
      temperature = d[2];
    }
    else {
      humidity = ((d[0] << 8) | d[1]) / 10.0;
      temperature = (((d[2] & 0x7f) << 8) | d[3]) / 10.0;
      if (d[2] & 0x80) temperature *= -1;
    }

    // Check for value changes
    int res = (humidity != m_humidity) + (temperature != m_temperature);
    if (res > 0) {
      m_humidity = humidity;
      m_temperature = temperature;
    }

    // Return number of changed values: 0, 1 och 2
    return (res);
  }

protected:
  /** Start signal pulse width in milliseconds. */
  static const uint16_t START_SIGNAL = 18;

  /** Pullup delay in microseconds. */
  static const uint16_t PULLUP = 4;

  /** Pulse threshold in microseconds. */
  static const int THRESHOLD = 60;

  /** Board pin for data communication. */
  GPIO<DATA_PIN> m_data;

  /** Latest humidity reading. */
  float m_humidity;

  /** Latest temperature reading. */
  float m_temperature;
};

/**
 * DHT11 Humidity & Temperature Sensor device driver template class.
 * @param[in] DATA_PIN board pin for data signal.
 */
template<BOARD::pin_t DATA_PIN> class DHT11 : public DHT<DATA_PIN, 11> {};

/**
 * DHT21 Humidity & Temperature Sensor device driver template class.
 * @param[in] DATA_PIN board pin for data signal.
 */
template<BOARD::pin_t DATA_PIN> class DHT21 : public DHT<DATA_PIN, 21> {};

/**
 * DHT22 Humidity & Temperature Sensor device driver template class.
 * @param[in] DATA_PIN board pin for data signal.
 */
template<BOARD::pin_t DATA_PIN> class DHT22 : public DHT<DATA_PIN, 22> {};
#endif
