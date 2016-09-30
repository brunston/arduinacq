/* Arduino I2cMaster Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino I2cMaster Library
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
 * along with the Arduino I2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef I2C_MASTER_H
#define I2C_MASTER_H
/**
 * \file
 * \brief Software I2C and hardware TWI library
 */
#if ARDUINO < 100
#error Requires Arduino 1.0 or greater.
#else  // ARDUINO
#include <Arduino.h>
#endif  // ARDUINO
//------------------------------------------------------------------------------
/** I2cMaster version YYYYMMDD */
#define I2C_MASTER_VERSION 20120716
//------------------------------------------------------------------------------
/** Default hardware I2C clock in Hz */
uint32_t const F_TWI = 400000L;

/** Delay used for software I2C */
uint8_t const I2C_DELAY_USEC = 4;

/** Bit to or with address for read start and read restart */
uint8_t const I2C_READ = 1;

/** Bit to or with address for write start and write restart */
uint8_t const I2C_WRITE = 0;
//------------------------------------------------------------------------------
// Status codes in TWSR - names are from Atmel TWSR.h with TWSR_ added

/** start condition transmitted */
uint8_t const TWSR_START = 0x08;

/** repeated start condition transmitted */
uint8_t const TWSR_REP_START = 0x10;

/** slave address plus write bit transmitted, ACK received */
uint8_t const TWSR_MTX_ADR_ACK = 0x18;

/** data transmitted, ACK received */
uint8_t const TWSR_MTX_DATA_ACK = 0x28;

/** slave address plus read bit transmitted, ACK received */
uint8_t const TWSR_MRX_ADR_ACK = 0x40;

//==============================================================================
/**
 * \class I2cMasterBase
 * \brief Base class for FastI2cMaster, SoftI2cMaster, and TwiMaster
 */
class I2cMasterBase {
 public:
  /** Read a byte
   * \param[in] last send Ack if last is false else Nak to terminate read
   * \return byte read from I2C bus
   */
  virtual uint8_t read(uint8_t last) = 0;
  /** Send new address and read/write bit without sending a stop.
   * \param[in] addressRW i2c address with read/write bit
   * \return true for success false for failure
   */
  virtual bool restart(uint8_t addressRW) = 0;
  /** Issue a start condition
   * \param[in] addressRW i2c address with read/write bit
   * \return true for success false for failure
   */
  virtual bool start(uint8_t addressRW) = 0;
  /** Issue a stop condition. */
  virtual void stop(void) = 0;
  /** Write a byte
   * \param[in] data byte to write
   * \return true for Ack or false for Nak */
  virtual bool write(uint8_t data) = 0;
};
//==============================================================================
/**
 * \class SoftI2cMaster
 * \brief Software I2C master class
 */
class SoftI2cMaster : public I2cMasterBase {
 public:
  SoftI2cMaster(uint8_t sdaPin, uint8_t sclPin, bool enablePullup = true);
  uint8_t read(uint8_t last);
  bool restart(uint8_t addressRW);
  bool start(uint8_t addressRW);
  void stop(void);
  bool write(uint8_t b);
 private:
  SoftI2cMaster() {}
  bool enablePullup_;
  uint8_t sdaPin_;
  uint8_t sclPin_;
};
//==============================================================================
/**
 * \class TwiMaster
 * \brief Hardware I2C master class
 *
 * Uses ATmega TWI hardware port
 */
class TwiMaster : public I2cMasterBase {
 public:
  explicit TwiMaster(bool enablePullup);
  uint8_t read(uint8_t last);
  bool restart(uint8_t addressRW);
  void speed(bool fast);
  bool start(uint8_t addressRW);
  /** \return status from last TWI command - useful for library debug */
  uint8_t status(void) {return status_;}
  void stop(void);
  bool write(uint8_t data);
 private:
  TwiMaster() {}
  uint8_t status_;
  void execCmd(uint8_t cmdReg);
};
//==============================================================================
// experimental template based fast software I2C
#ifndef USE_FAST_I2C_MASTER
/** disable experimental fast software I2C */
#define USE_FAST_I2C_MASTER 0
#endif  // USE_FAST_I2C_MASTER
#if USE_FAST_I2C_MASTER  || DOXYGEN
#include <util/delay_basic.h>
#include <DigitalPin.h>
//------------------------------------------------------------------------------
/**
 * \class FastI2cMaster
 * \brief Fast software I2C master class
 */
template<uint8_t sdaPin, uint8_t sclPin, bool enablePullups = true>
class FastI2cMaster : public I2cMasterBase {
 public:
  FastI2cMaster() {
    fastPinMode(sdaPin, OUTPUT);
    fastDigitalWrite(sdaPin, HIGH);
    fastPinMode(sclPin, OUTPUT);
    fastDigitalWrite(sclPin, HIGH);
  }
  //----------------------------------------------------------------------------
  uint8_t read(uint8_t last) {
    uint8_t data = 0;
    // make sure pull-up enabled
    fastPinMode(sdaPin, INPUT);
    fastDigitalWrite(sdaPin, enablePullups);
    readBit(7, &data);
    readBit(6, &data);
    readBit(5, &data);
    readBit(4, &data);
    readBit(3, &data);
    readBit(2, &data);
    readBit(1, &data);
    readBit(0, &data);
    // send Ack or Nak
    fastPinMode(sdaPin, OUTPUT);
    fastDigitalWrite(sdaPin, last);
    fastDigitalWrite(sclPin, HIGH);
    sclDelay(3);
    fastDigitalWrite(sclPin, LOW);
    fastDigitalWrite(sdaPin, LOW);
    return data;
  }
  //----------------------------------------------------------------------------
  bool restart(uint8_t addressRW) {
    fastDigitalWrite(sdaPin, HIGH);
    sclDelay(8);
    fastDigitalWrite(sclPin, HIGH);
    sclDelay(8);
    return start(addressRW);
  }
  //----------------------------------------------------------------------------
  bool start(uint8_t addressRW) {
    fastDigitalWrite(sdaPin, LOW);
    sclDelay(8);
    fastDigitalWrite(sclPin, LOW);
    sclDelay(8);
    return write(addressRW);
  }
  //----------------------------------------------------------------------------
  void stop(void) {
    fastDigitalWrite(sdaPin, LOW);
    sclDelay(8);
    fastDigitalWrite(sclPin, HIGH);
    sclDelay(8);
    fastDigitalWrite(sdaPin, HIGH);
    sclDelay(8);
  }
  //----------------------------------------------------------------------------
  bool write(uint8_t data) {
    // write byte
    writeBit(7, data);
    writeBit(6, data);
    writeBit(5, data);
    writeBit(4, data);
    writeBit(3, data);
    writeBit(2, data);
    writeBit(1, data);
    writeBit(0, data);

    // get Ack or Nak
    fastPinMode(sdaPin, INPUT);
    // enable pullup
    fastDigitalWrite(sdaPin, HIGH);

    fastDigitalWrite(sclPin, HIGH);
    sclDelay(3);
    uint8_t rtn = fastDigitalRead(sdaPin);
    fastDigitalWrite(sclPin, LOW);
    fastPinMode(sdaPin, OUTPUT);
    fastDigitalWrite(sdaPin, LOW);
    return rtn == 0;
  }
  //----------------------------------------------------------------------------
 private:
  inline __attribute__((always_inline))
  void readBit(uint8_t bit, uint8_t* data) {
    fastDigitalWrite(sclPin, HIGH);
    sclDelay(3);
    if (fastDigitalRead(sdaPin)) *data |= 1 << bit;
    fastDigitalWrite(sclPin, LOW);
    sclDelay(7);
  }
  //----------------------------------------------------------------------------
  void sclDelay(uint8_t n) {
     _delay_loop_1(n);
  }
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void writeBit(uint8_t bit, uint8_t data) {
    fastDigitalWrite(sdaPin, data & (1 << bit));
    fastDigitalWrite(sclPin, HIGH);
    sclDelay(4);
    fastDigitalWrite(sclPin, LOW);
    sclDelay(6);
  }
};
#endif  // USE_FAST_I2C_MASTER
#endif  // I2C_MASTER_H
