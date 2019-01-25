//
// You received this file as part of Finroc
// A framework for intelligent robot control
//
// Copyright (C) Patrick Wolf
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    projects/smart_home/heat_control/mBMP180.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief Contains mBMP180
 *
 * \b mBMP180
 *
 * MBP180 air pressure and temperature sensor via I2C
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__shared__mBMP180_h__
#define __projects__smart_home__shared__mBMP180_h__

#include "plugins/structure/tModule.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#ifdef _LIB_WIRING_PI_PRESENT_
#include "rrlib/gpio/tI2C.h"
#endif

#include "rrlib/si_units/si_units.h"

#include <chrono>
#include <thread>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace shared
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
enum tBMP180OSSMode
{
  tBMP180_OSS_ULTRA_LOW_POWER = 0x00,
  tBMP180_OSS_STANDARD = 0x01,
  tBMP180_OSS_HIGH_RESOLUTION = 0x02,
  tBMP180_OSS_ULTRA_HIGH_RESOLUTION = 0x03
};

static constexpr uint8_t cBMP180_I2C_ADDRESS = 0x77;
static constexpr uint8_t cBMP180_ID = 0xD0;
static constexpr uint8_t cBMP180_CTRL_MEAS = 0xF4;
static constexpr uint8_t cBMP180_OUT_MSB = 0xF6;
static constexpr uint8_t cBMP180_OUT_LSB = 0xF7;
static constexpr uint8_t cBMP180_OUT_XLSB = 0xF8;
static constexpr uint8_t cBMP180_SOFT_RESET = 0xE0;

static constexpr uint8_t cBMP180_REGISTER_TEMPERATURE = 0x2E;
static constexpr uint8_t cBMP180_REGISTER_AIR_PRESSURE = 0x34;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * module that provides a hardware interface, a/d conversion and is capable of setting pumps.
 */
class mBMP180: public structure::tModule
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tOutput<rrlib::si_units::tCelsius<double>> out_temperature;
  tOutput<int32_t> out_raw_temperature;
  tOutput<rrlib::si_units::tPressure<double>> out_air_pressure;
  tOutput<rrlib::si_units::tLength<double>> out_altitude;
  tOutput<int32_t> out_raw_pressure;

  tParameter<tBMP180OSSMode> par_operation_mode;
  tParameter<rrlib::si_units::tPressure<double>> par_pressure_sea_level;
  tParameter<rrlib::si_units::tCelsius<double>> par_temperature_offset;


//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mBMP180(core::tFrameworkElement *parent, const std::string &name = "BMP180") :
    tModule(parent, name),
    par_operation_mode(tBMP180OSSMode::tBMP180_OSS_STANDARD),
	par_pressure_sea_level(101325.0)
#ifdef _LIB_WIRING_PI_PRESENT_
    , i2c_device_(cBMP180_I2C_ADDRESS)
#endif
  {
#ifdef _LIB_WIRING_PI_PRESENT_
    RRLIB_LOG_PRINT(DEBUG, "Found device with ID: ", (int)i2c_device_.ReadByte(cBMP180_ID), " at I2C address ", (int)i2c_device_.GetDeviceAddress(), ".");

    // load calibration
    RRLIB_LOG_PRINT(DEBUG, "Loading calibration...");
    ac1_ = (i2c_device_.ReadByte(0xAA) << 8) + (i2c_device_.ReadByte(0xAB));
    RRLIB_LOG_PRINT(DEBUG, "AC1 :", ac1_);
    ac2_ = (i2c_device_.ReadByte(0xAC) << 8) + (i2c_device_.ReadByte(0xAD));
    RRLIB_LOG_PRINT(DEBUG, "AC2 :", ac2_);
    ac3_ = (i2c_device_.ReadByte(0xAE) << 8) + (i2c_device_.ReadByte(0xAF));
    RRLIB_LOG_PRINT(DEBUG, "AC3 :", ac3_);
    ac4_ = (i2c_device_.ReadByte(0xB0) << 8) + (i2c_device_.ReadByte(0xB1));
    RRLIB_LOG_PRINT(DEBUG, "AC4 :", ac4_);
    ac5_ = (i2c_device_.ReadByte(0xB2) << 8) + (i2c_device_.ReadByte(0xB3));
    RRLIB_LOG_PRINT(DEBUG, "AC5 :", ac5_);
    ac6_ = (i2c_device_.ReadByte(0xB4) << 8) + (i2c_device_.ReadByte(0xB5));
    RRLIB_LOG_PRINT(DEBUG, "AC6 :", ac6_);
    b1_ = (i2c_device_.ReadByte(0xB6) << 8) + (i2c_device_.ReadByte(0xB7));
    RRLIB_LOG_PRINT(DEBUG, "B1 :", b1_);
    b2_ = (i2c_device_.ReadByte(0xB8) << 8) + (i2c_device_.ReadByte(0xB9));
    RRLIB_LOG_PRINT(DEBUG, "B2 :", b2_);
    mb_ = (i2c_device_.ReadByte(0xBA) << 8) + (i2c_device_.ReadByte(0xBB));
    RRLIB_LOG_PRINT(DEBUG, "MB :", mb_);
    mc_ = (i2c_device_.ReadByte(0xBC) << 8) + (i2c_device_.ReadByte(0xBD));
    RRLIB_LOG_PRINT(DEBUG, "MC :", mc_);
    md_ = (i2c_device_.ReadByte(0xBE) << 8) + (i2c_device_.ReadByte(0xBF));
    RRLIB_LOG_PRINT(DEBUG, "MD :", md_);
#endif
  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of modules is declared protected to avoid accidental deletion. Deleting
   * modules is already handled by the framework.
   */
  ~mBMP180() {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

#ifdef _LIB_WIRING_PI_PRESENT_
  rrlib::gpio::tI2C i2c_device_;
#endif
  int16_t ac1_;
  int16_t ac2_;
  int16_t ac3_;
  uint16_t ac4_;
  uint16_t ac5_;
  uint16_t ac6_;
  int16_t b1_;
  int16_t b2_;
  int16_t mb_;
  int16_t mc_;
  int16_t md_;

  inline virtual void Update() override
  {
#ifdef _LIB_WIRING_PI_PRESENT_

    // start temperature measurement
    i2c_device_.WriteByte(cBMP180_CTRL_MEAS, cBMP180_REGISTER_TEMPERATURE);
    std::this_thread::sleep_for(std::chrono::microseconds(4500));
    int16_t raw_temperature = (int16_t)((i2c_device_.ReadByte(cBMP180_OUT_MSB) << 8) + (i2c_device_.ReadByte(cBMP180_OUT_LSB)));
    auto temperature_time = rrlib::time::Now();
    out_raw_temperature.Publish(raw_temperature, temperature_time);

    // start air pressure measurement
    i2c_device_.WriteByte(cBMP180_CTRL_MEAS, cBMP180_REGISTER_AIR_PRESSURE + (static_cast<uint8_t>(par_operation_mode.Get()) << 6));
    switch (par_operation_mode.Get())
    {
    case tBMP180OSSMode::tBMP180_OSS_ULTRA_LOW_POWER:
      std::this_thread::sleep_for(std::chrono::microseconds(4500));
      break;
    case tBMP180OSSMode::tBMP180_OSS_STANDARD:
      std::this_thread::sleep_for(std::chrono::microseconds(7500));
      break;
    case tBMP180OSSMode::tBMP180_OSS_HIGH_RESOLUTION:
      std::this_thread::sleep_for(std::chrono::microseconds(13500));
      break;
    case tBMP180OSSMode::tBMP180_OSS_ULTRA_HIGH_RESOLUTION:
      std::this_thread::sleep_for(std::chrono::microseconds(25500));
      break;
    default:
        std::this_thread::sleep_for(std::chrono::microseconds(25500));
    };
    int32_t raw_pressure = ((i2c_device_.ReadByte(cBMP180_OUT_MSB) << 16) +
    		(i2c_device_.ReadByte(cBMP180_OUT_LSB) << 8) +
			i2c_device_.ReadByte(cBMP180_OUT_XLSB)) >> (8 - par_operation_mode.Get());
    auto pressure_time = rrlib::time::Now();
    out_raw_pressure.Publish(raw_pressure, pressure_time);

    // compute temperature
    int32_t x1 = (raw_temperature - ac6_) * ac5_ / std::pow(2, 15);
    int32_t x2 = mc_ * std::pow(2,11) / (x1 + md_);
    int32_t b5 = x1 + x2;
    int32_t temperature = (b5 + 8) / 16;
    RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "x1: ", x1, " x2: ", x2, " b5: ", b5);
    out_temperature.Publish(static_cast<rrlib::si_units::tCelsius<double>>((static_cast<double>(temperature) * 0.1) + par_temperature_offset.Get().ValueFactored()), temperature_time);

    // compute air pressure
    // TODO: replace by algorithm from data sheet using the calibration
    int32_t air_pressure = raw_pressure;
    out_air_pressure.Publish(static_cast<rrlib::si_units::tPressure<double>>(air_pressure), pressure_time);

    // compute altitude
    out_altitude.Publish(static_cast<rrlib::si_units::tLength<double>>(44330.0 * (1.0 - std::pow(static_cast<double>(air_pressure) / par_pressure_sea_level.Get().Value(), 1 / 5.255))), pressure_time);
#endif
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
