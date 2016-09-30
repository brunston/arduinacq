/* Arduino SoftRTClib Library
 * Copyright (C) 2011 by William Greiman
 *
 * This file is part of the Arduino SoftRTClib Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SoftRTClib Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
 /*
  * The API for this library was designed for easy use with programs
  * that use the JeeLabs RTClib.
  * http://news.jeelabs.org/code/
  */
/**
 * \file
 * \brief SoftRTClib header
 */
#ifndef SoftRTClib_h
#define SoftRTClib_h
// set nonzero to use Wire library
#define DS_RTC_USE_WIRE 0
#if DS_RTC_USE_WIRE
#include <Wire.h>
#else  // DS_RTC_USE_WIRE
#include <I2cMaster.h>
#endif  // DS_RTC_USE_WIRE
//------------------------------------------------------------------------------
/**
 * Set nonzero to set RTC internal day-of-week register
 */
#define SET_RTC_DAY_OF_WEEK 1
//------------------------------------------------------------------------------
#include <Print.h>

/** i2c 8-bit address for RTC. low bit is read/write */
uint8_t const  DS_RTC_I2C_ADD = 0XD0;
/*
The DS1307 control register is used to control the operation of the SQW/OUT pin.

The SQW/OUT pin is open drain and requires an external pullup resistor. SQW/OUT
operates with either VCC or VBAT applied. The pullup voltage can be up to 5.5V
regardless of the voltage on VCC. If not used, this pin can be left floating.

07h  DS1307 Control
+---+---+---+----+---+---+---+---+
|OUT| 0 | 0 |SQWE| 0 | 0 |RS1|RS2|
| 0 | 0 | 0 |  0 | 0 | 0 | 1 | 1 |
+---+---+---+----+---+---+---+---+
OUT SQWE RS1 RS2  SQW Pin
 x    1   0   0   1 Hz
 x    1   0   1   4096 Hz
 x    1   1   0   8196 Hz
 x    1   1   1   32768 Hz
 1    0   x   x   high
 0    0   x   x   low
*/
/** DS1307 control register address. */
uint8_t const DS1307_CONTROL_ADDRESS = 0X07;
/** DS1307 control register value for SQW pin low. */
uint8_t const DS1307_SQW_LOW         = 0X00;
/** DS1307 control register value for SQW pin high. */
uint8_t const DS1307_SQW_HIGH        = 0X80;
/** DS1307 control register value for 1 Hz square-wave on SQW pin. */
uint8_t const DS1307_SQW_1_HZ        = 0X10;
/** DS1307 control register value for 4096 Hz square-wave on SQW pin. */
uint8_t const DS1307_SQW_4096_HZ     = 0X11;
/** DS1307 control register value for 8192 Hz square-wave on SQW pin. */
uint8_t const DS1307_SQW_8192_HZ     = 0X12;
/** DS1307 control register value for 32768 Hz square-wave on SQW pin. */
uint8_t const DS1307_SQW_32768_HZ    = 0X13;
//------------------------------------------------------------------------------
/** typedef for seconds since epoch */
typedef int32_t time_t;
//------------------------------------------------------------------------------
/**
 * \class DateTime
 * \brief Class for date/time objects
 */
class DateTime {
 public:
  /** Year of posix epoch */
  static const uint16_t FIRST_YEAR = 1970;
  /** last year representable by int32_t */
  static const uint16_t LAST_YEAR = FIRST_YEAR + 67;
  //----------------------------------------------------------------------------
  /** constructor */
  DateTime() {settime();}
  /** constructor
   *
   * \param[in] t Posix time in seconds since epoch.
   */
  explicit DateTime(time_t t) {if (!settime(t)) settime();}
  /** constructor
   *
   * \param[in] year [1970, 2037]
   * \param[in] month [1, 12]
   * \param[in] day [1, last day in month]
   * \param[in] hour [0, 23]
   * \param[in] minute [0, 59]
   * \param[in] second [0, 59]
   */
  DateTime(uint16_t year, uint8_t month, uint8_t day,
    uint8_t hour, uint8_t minute, uint8_t second) {
    if (!settime(year, month, day, hour, minute, second)) settime();
  }
  /** constructor
   *
   * \param[in] date string in __DATE__ format.
   * \param[in] time string in __TIME__ format.
   */
  DateTime(const char* date, const char* time) {
    if (!settime(date, time)) settime();
  }
  //----------------------------------------------------------------------------
  // inline functions
  /** \return year */
  int year() const {return year_;}
  /** \return month [1, 12] */
  int month() const {return month_;}
  /** \return day [1, 31] */
  int day() const {return day_;}
  /** \return hour [0, 23] */
  int hour() const {return hour_;}
  /** \return hour 12-hour clock [1, 12] */
  int hour12() const {return (hour_ + 11) % 12 + 1;}
  /** \return ISO day of week [1, 7] Sunday = 1 */
  int isoDayOfWeek() const {return (dayOfWeek() + 6) % 7 + 1;}
  /** \return ISO day of year [1, 366]; first day of year is 001.  */
  int isoDayOfYear() const {return dayOfYear() + 1;}
  /** \return minute [0, 59] */
  int minute() const {return minute_;}
  /** \return second [0, 59]*/
  int second() const {return second_;}
  /** \return posix seconds since 1/1/1970 00:00:00 */
  time_t unixtime() const {return time();}
  //----------------------------------------------------------------------------
  int dayOfWeek() const;
  int dayOfYear() const;
  void printDate(Print* pr) const;
  void printDateTime(Print* pr) const;
  void printDD(Print* pr) const;
  void printDdd(Print* pr) const;
  void printIsoDate(Print* pr) const;
  void printIsoDateTime(Print* pr) const;
  void printIsoTime(Print* pr)const ;
  void printUsaDate(Print* pr) const;
  void printMM(Print* pr) const;
  void printMmm(Print* pr) const;
  void printUsaDateTime(Print* pr) const;
  void settime();
  bool settime(time_t t);
  bool settime(uint16_t year, uint8_t month, uint8_t day,
    uint8_t hour, uint8_t minute, uint8_t second);
  bool settime(const char* date, const char* time);
  time_t time() const;
 private:
  friend class RTC_DS1307;
  uint16_t year_;
  uint8_t month_;
  uint8_t day_;
  uint8_t hour_;
  uint8_t minute_;
  uint8_t second_;
};
//------------------------------------------------------------------------------
/**
 * \class RTC_DS1307
 * \brief Class for DS1307 access
 */
class RTC_DS1307 {
 public:
 /** \return true for RTClib compatibility */
  uint8_t begin() {return 1;};
  /** set time
   * \param[in] dt new date and time
   */
  void adjust(const DateTime& dt) {setTime(&dt);}
  bool getTime(DateTime* dt);
  bool isrunning();
  DateTime now();
  bool read(uint8_t address, uint8_t *buf, uint8_t count);
  bool readTime(uint8_t *r);
  bool setSQW(uint8_t sqwMode);
  bool setTime(const DateTime* dt);
  bool write(uint8_t address, uint8_t *buf, uint8_t count);
#if DS_RTC_USE_WIRE
  /** Constructor */
  RTC_DS1307() {}
#else  // DS_RTC_USE_WIRE
/** Constructor
 * \param[in] i2cBus I2C bus for this RTC.
 */
  explicit RTC_DS1307(I2cMasterBase* i2cBus) : i2cBus_(i2cBus) {}
private:
  RTC_DS1307() {}
  I2cMasterBase *i2cBus_;
#endif  // DS_RTC_USE_WIRE
};
#endif  // SoftRTClib_h