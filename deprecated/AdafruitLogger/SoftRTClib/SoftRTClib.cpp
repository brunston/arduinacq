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

#include <avr/pgmspace.h>
#include <SoftRTClib.h>
#define EPOCH_YEAR DateTime::FIRST_YEAR
#include <utility/InlineDateAlgorithms.h>
//------------------------------------------------------------------------------
inline uint8_t bcd2bin (uint8_t val) {return val - 6 * (val >> 4);}
//------------------------------------------------------------------------------
inline uint8_t bin2bcd (uint8_t val) {return val + 6 * (val / 10);}

//------------------------------------------------------------------------------
// parse two digit field
static uint8_t c2b(const char *s) {
  uint8_t b = '0' <= s[0] && s[0] <= '9' ? s[0] - '0' : 0;
  return 10*b + s[1] - '0';
}
//------------------------------------------------------------------------------
// used by c2m and printMmm
static char Mmm[] PROGMEM = "JanFebMarAprMayJunJulAugSepOctNovDec";
//------------------------------------------------------------------------------
// convert Mmm string to [0,12]
static uint8_t c2m(const char* s) {
  uint8_t m;
  for (m = 0; m < 12; m ++) {
    if (!strncmp_P(s, &Mmm[3*m], 3)) return m + 1;
  }
  return 0;
}
//------------------------------------------------------------------------------
// print two digit field with zero fill
static void print2d(Print* pr, uint8_t n) {
  if (n < 10) pr->write('0');
  pr->print(n, DEC);
}
//==============================================================================
// DateTime member functions
//------------------------------------------------------------------------------
/** \return Day of Week (Sunday == 0) */
int DateTime::dayOfWeek() const {
  uint16_t eday = daysSinceEpoch(year_, month_, day_);
  return epochDayToDayOfWeek(eday);
}
//------------------------------------------------------------------------------
/** \return Day of Year [0, 365] */
int DateTime::dayOfYear() const {
  return daysBeforeMonth(year_, month_) + day_ - 1;
}
//------------------------------------------------------------------------------
/** Print day with zero fill
 * \param[in] pr Print stream.
 */
void DateTime::printDD(Print* pr) const {
  print2d(pr, day());
}
//------------------------------------------------------------------------------
/** Print date in DD Mmm YYYY format
 * \param[in] pr Print stream.
 */
void DateTime::printDate(Print* pr) const {
  print2d(pr, day());
  pr->write(' ');
  printMmm(pr);
  pr->write(' ');
  pr->print(year());
}
//------------------------------------------------------------------------------
/** Print date in DD Mmm YYYY format
 * \param[in] pr Print stream.
 */
void DateTime::printDateTime(Print* pr) const {
  printDate(pr);
  pr->write(' ');
  printIsoTime(pr);
}
//------------------------------------------------------------------------------
/** Print three character day of week
 * \param[in] pr Print stream.
 */
void DateTime::printDdd(Print* pr) const {
  static char Ddd[] PROGMEM = "SunMonTueWedThuFriSat";
  char buf[4];
  uint8_t w = dayOfWeek();
  strncpy_P(buf, &Ddd[3*w], 3);
  buf[3] = 0;
  pr->write(buf);
}
//------------------------------------------------------------------------------
/** Print date in ISO YYY-MM-DD format.
 * \param[in] pr Print stream.
 */
void DateTime::printIsoDate(Print* pr) const {
  pr->print(year());
  pr->write('-');
  print2d(pr, month());
  pr->write('-');
  print2d(pr, day());
}
//------------------------------------------------------------------------------
/** Print date/time in ISO YYY-MM-DD hh:mm:ss format.
 * \param[in] pr Print stream.
 */
void DateTime::printIsoDateTime(Print* pr) const {
  printIsoDate(pr);
  pr->write(' ');
  printIsoTime(pr);
}
//------------------------------------------------------------------------------
/** Print time in ISO hh:mm:ss format.
 * \param[in] pr Print stream.
 */
void DateTime::printIsoTime(Print* pr)const {
  print2d(pr, hour());
  pr->write(':');
  print2d(pr, minute());
  pr->write(':');
  print2d(pr, second());
}
//------------------------------------------------------------------------------
/** Print month with zero fill
 * \param[in] pr Print stream.
 */
void DateTime::printMM(Print* pr) const {
  print2d(pr, month());
}
//------------------------------------------------------------------------------
/** Print three character month
 * \param[in] pr Print stream.
 */
void DateTime::printMmm(Print* pr) const {
  char buf[4];
  strncpy_P(buf, &Mmm[3*month() - 3], 3);
  buf[3] = 0;
  pr->write(buf);
}
//------------------------------------------------------------------------------
/** Print date in USA MM/DD/YYYY format
 * \param[in] pr Print stream .
 */
void DateTime::printUsaDate(Print* pr) const {
  print2d(pr, month());
  pr->write('/');
  print2d(pr, day());
  pr->write('/');
  pr->print(year());
}
//------------------------------------------------------------------------------
/** Print date/time in USA MM/DD/YYYY hh:mm:ss format
 * \param[in] pr Print stream .
 */
void DateTime::printUsaDateTime(Print* pr) const{
  printUsaDate(pr);
  pr->write(' ');
  printIsoTime(pr);
}
//------------------------------------------------------------------------------
/** set time to epoch, 1/1/1970 00:00:00 */
void DateTime::settime() {
  year_ = FIRST_YEAR;
  month_ = 1;
  day_ = 1;
  hour_ = 0;
  minute_ = 0;
  second_ = 0;
}
//------------------------------------------------------------------------------
/** Convert posix seconds to broken-down time.
 * \param[in] t Posix time in seconds since epoch.
 * \return true for success else false
 */
bool DateTime::settime(time_t t) {
  if (t < 0) return false;

  int32_t tmp = t;
  t /= 60L;
  second_ = tmp - 60L * t;
  tmp = t;
  t /= 60L;
  minute_ = tmp - 60L * t;
  uint16_t days = t / 24L;
  hour_ = t - 24L * days;

  year_ = epochDayToYear(days);
  days -= daysBeforeYear(year_);
  month_ = dayOfYearToMonth(year_, days);
  day_ = 1 + days - daysBeforeMonth(year_, month_);
  return true;
}
//------------------------------------------------------------------------------
/**  Set date time
 * \param[in] year 1970 <= year <= 2037.
 * \param[in] month 1 <= month <= 12.
 * \param[in] day 1 <= day <= (days in month).
 * \param[in] hour 0 <= hour <= 23.
 * \param[in] minute 0 <= minute <= 59.
 * \param[in] second 0 <= second <= 59.
 * \return true for success or false for failure.  Fails if invalid time/date.
 */
bool DateTime::settime(uint16_t year, uint8_t month, uint8_t day,
  uint8_t hour, uint8_t minute, uint8_t second) {
  if (hour > 23 || minute > 59 || second > 59
    || year < FIRST_YEAR || year > LAST_YEAR
    || month < 1 || month > 12
    || day < 1 || day > daysInMonth(year, month)) {
      return false;
    }
    year_ = year;
    month_ = month;
    day_ = day;
    hour_ = hour;
    minute_ = minute;
    second_ = second;
    return true;
}
//------------------------------------------------------------------------------
/** Set date time using __DATE__ and __TIME__ strings
 *
 * \param[in] date string in __DATE__ format.
 * \param[in] time string in __TIME__ format.
 * \return true for success or false for failure.  Fails if invalid time/date.
 */
bool DateTime::settime(const char* date, const char* time) {
  return settime(2000 + c2b(date + 9), c2m(date), c2b(date + 4),
    c2b(time), c2b(time + 3), c2b(time + 6));
}
//------------------------------------------------------------------------------
/** \return Posix time in seconds since 1 Jan 1970 00:00:00 */
time_t DateTime::time() const {
  uint16_t days = daysSinceEpoch(year_, month_, day_);
  return second_ + 60L * (minute_ + 60L * (hour_ + 24L * days));
}
//==============================================================================
/** read date time
 *
 * \param[out] dt destination for date time
 *
 * \return true if read is successful else false
 */
bool RTC_DS1307::getTime(DateTime* dt) {
  uint8_t r[7];
  if (!readTime(r)) return false;
  dt->year_ = 2000 + r[6];
  dt->month_ = r[5];
  dt->day_ = r[4];
  dt->hour_ = r[2];
  dt->minute_ = r[1];
  dt->second_ = r[0];
  return true;
}
//------------------------------------------------------------------------------
/** \return true if no I/O error and DS1307 is running */
bool RTC_DS1307::isrunning(void) {
  uint8_t r;
  if (!read(0, &r, 1)) return false;
  return !(r & 0x80);
}
//------------------------------------------------------------------------------
/** \return current DS1307 date and time */
DateTime RTC_DS1307::now() {
  uint8_t r[7];
  readTime(r);
  return DateTime(2000 + r[6], r[5], r[4], r[2], r[1], r[0]);
}
//------------------------------------------------------------------------------
/**
 * Read RTC time registers
 * \param[out] r Location for return of seven RTC time registers.
 * Values are converted from BCD to binary.
 * \return true for success or false for failure.
 */
bool RTC_DS1307::readTime(uint8_t *r) {
  if (!read(0, r, 7)) return false;
  r[0] &= 0X7F;
  for (uint8_t i = 0; i < 7; i++) r[i] = bcd2bin(r[i]);
  return true;
}
//------------------------------------------------------------------------------
/** set mode for SQW/OUT pin
 *
 * \param[in] sqwMode DS1307_SQW_LOW, DS1307_SQW_HIGH, DS1307_SQW_1_HZ,
 * DS1307_SQW_4096_HZ, DS1307_SQW_8192_HZ, or DS1307_SQW_32768_HZ
 * \return true for success or false for failure.
 */
bool RTC_DS1307::setSQW(uint8_t sqwMode) {
  return write(DS1307_CONTROL_ADDRESS, &sqwMode, 1);
}
//------------------------------------------------------------------------------
/** set date and time
 * \param[in] dt date and time to be used.
 * \return true for success or false for failure.
 */
bool RTC_DS1307::setTime(const DateTime* dt) {
  uint8_t r[7];

  r[0] = dt->second();
  r[1] = dt->minute();
  r[2] = dt->hour();
#if SET_RTC_DAY_OF_WEEK
  r[3] = dt->isoDayOfWeek();
#else  // SET_RTC_DAY_OF_WEEK
  r[3] = 0;
#endif
  r[4] = dt->day();
  r[5] = dt->month();
  r[6] = dt->year() % 100;
  for (uint8_t i = 0; i < 7; i++) {
   r[i] = bin2bcd(r[i]);
  }
  return write(0, r, 7);
}
//==============================================================================
#if DS_RTC_USE_WIRE
//------------------------------------------------------------------------------
/**
 * Read data from the RTC.
 *
 * \param[in] address starting address.
 * \param[out] buf Location for data.
 * \param[in] count Number of bytes to write.
 * \return The value true, 1, for success or false, 0, for failure.
 */
bool RTC_DS1307::read(uint8_t address, uint8_t *buf, uint8_t count) {
  Wire.beginTransmission(DS_RTC_I2C_ADD/2);
#if ARDUINO < 100
  Wire.send(0);
#else // ARDUINO < 100
  Wire.write(0);
#endif // ARDUINO < 100
  if (Wire.endTransmission()) return false;
  Wire.requestFrom((uint8_t)(DS_RTC_I2C_ADD/2), count);
  if (Wire.available() != count) return false;
  for (uint8_t i = 0; i < count; i++) {
#if ARDUINO < 100
    buf[i] = Wire.receive();
#else // ARDUINO < 100
    buf[i] = Wire.read();
#endif // ARDUINO < 100
  }
  return true;
}
//------------------------------------------------------------------------------
/**
 * Write data to the RTC.
 *
 * \param[in] address Starting address.
 * \param[in] buf Location of data.
 * \param[in] count Number of bytes to write.
 * \return The value true, 1, for success or false, 0, for failure.
 */
bool RTC_DS1307::write(uint8_t address, uint8_t *buf, uint8_t count) {
  Wire.beginTransmission(DS_RTC_I2C_ADD/2);
#if ARDUINO < 100
  Wire.send(0);
  Wire.send(buf, count);
#else // ARDUINO < 100
  Wire.write(0);
  Wire.write(buf, count);
#endif // ARDUINO < 100
  return Wire.endTransmission() ? false : true;
}
#else  // DS_RTC_USE_WIRE
//------------------------------------------------------------------------------
/**
 * Read data from the RTC.
 *
 * \param[in] address starting address.
 * \param[out] buf Location for data.
 * \param[in] count Number of bytes to write.
 * \return The value true, 1, for success or false, 0, for failure.
 */
bool RTC_DS1307::read(uint8_t address, uint8_t *buf, uint8_t count) {
  // issue a start condition, send device address and write direction bit
  if (!i2cBus_->start(DS_RTC_I2C_ADD | I2C_WRITE)) goto fail;

  // send address
  if (!i2cBus_->write(address)) goto fail;

  // issue a repeated start condition, send device address and direction bit
  if (!i2cBus_->restart(DS_RTC_I2C_ADD | I2C_READ)) goto fail;

  // read data
  for (uint8_t i = 0; i < count; i++) {
    // send Ack until last byte then send Nak
    buf[i] = i2cBus_->read(i == (count - 1));
  }
  // issue a stop condition
  i2cBus_->stop();
  return true;

 fail:
  i2cBus_->stop();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Write data to the RTC.
 *
 * \param[in] address Starting address.
 * \param[in] buf Location of data.
 * \param[in] count Number of bytes to write.
 * \return The value true, 1, for success or false, 0, for failure.
 */
bool RTC_DS1307::write(uint8_t address, uint8_t *buf, uint8_t count) {
  // issue a start condition, send device address and write direction bit
  if (!i2cBus_->start(DS_RTC_I2C_ADD | I2C_WRITE)) goto fail;

  // send the address
  if (!i2cBus_->write(address)) goto fail;

  // send data
  for (uint8_t i = 0; i < count; i++) {
    if (!i2cBus_->write(buf[i])) goto fail;
  }
  // issue a stop condition
  i2cBus_->stop();
  return true;

 fail:
  i2cBus_->stop();
  return false;
}
#endif  // DS_RTC_USE_WIRE