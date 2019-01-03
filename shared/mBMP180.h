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
#include <wiringPiI2C.h>
#endif

#include "rrlib/si_units/si_units.h"

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
	tBMP180_OSS_ULTRA_LOW_POWER= 0x00,
	tBMP180_OSS_STANDARD = 0x01,
	tBMP180_OSS_HIGH_RESOLUTION = 0x02,
	tBMP180_OSS_ULTRA_HIGH_RESOLUTION = 0x03
};

static constexpr unsigned short cBMP180_I2C_ADDRESS = 0x77;
static constexpr unsigned short cBMP180_ID = 0xD0;
static constexpr unsigned short cBMP180_CTRL_MEAS = 0xF4;
static constexpr unsigned short cBMP180_OUT_MSB = 0xF6;
static constexpr unsigned short cBMP180_OUT_LSB = 0xF7;
static constexpr unsigned short cBMP180_OUT_XLSB = 0xF8;
static constexpr unsigned short cBMP180_SOFT_RESET = 0xE0;

static constexpr unsigned short cBMP180_REGISTER_TEMPERATURE = 0x2E;
static constexpr unsigned short cBMP180_REGISTER_AIR_PRESSURE = 0x34;

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
  tOutput<rrlib::si_units::tPressure<double>> out_air_pressure;

  tParameter<tBMP180OSSMode> par_operation_mode;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mBMP180(core::tFrameworkElement *parent, const std::string &name = "PT") :
    tModule(parent, name),
	par_operation_mode(tBMP180OSSMode::tBMP180_OSS_STANDARD),
	i2c_setup_success_(false)
  {
#ifdef _LIB_WIRING_PI_PRESENT_
	  i2c_setup_success_ = wiringPiI2CSetup(cBMP180_I2C_ADDRESS);
#endif
	  if(not i2c_setup_success_)
	  {
		  RRLIB_LOG_PRINT(ERROR, "I2C setup of BMP180 failed");
	  }
#ifdef _LIB_WIRING_PI_PRESENT_
	  else
	  {
		  wiringPiI2CWrite (cBMP180_I2C_ADDRESS, cBMP180_ID);
		  auto id = wiringPiI2CRead (cBMP180_I2C_ADDRESS);
		  RRLIB_LOG_PRINT(DEBUG, "Found device with ID: ", id, " at I2C address ", cBMP180_I2C_ADDRESS, ".");
	  }
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

  bool i2c_setup_success_;

  inline virtual void Update() override
  {
#ifdef _LIB_WIRING_PI_PRESENT_
	  auto time = rrlib::time::Now();

	  if(i2c_setup_success_)
	  {
		  int temperature = wiringPiI2CRead(cBMP180_REGISTER_TEMPERATURE);
		  out_temperature.Publish(static_cast<rrlib::si_units::tCelsius<double>>(temperature) * 0.1, time);

		  int air_pressure = wiringPiI2CRead(cBMP180_REGISTER_AIR_PRESSURE + (par_operation_mode.Get() << 6));
		  out_air_pressure.Publish(static_cast<rrlib::si_units::tPressure<double>>(air_pressure), time);
	  }
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
