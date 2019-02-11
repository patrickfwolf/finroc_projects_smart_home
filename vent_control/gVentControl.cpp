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
/*!\file    projects/smart_home/vent_control/gVentControl.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/vent_control/gVentControl.h"

//----------------------------------------------------------------------
// External includes
//----------------------------------------------------------------------
#ifdef _LIB_WIRING_PI_PRESENT_
#include "libraries/gpio_raspberry_pi/mRaspberryIO.h"
#endif

#include "libraries/signal_filters/mExponentialFilter.h"
#include "libraries/structure_elements/mMean.h"

#include <cassert>

//----------------------------------------------------------------------
// Internal includes
//----------------------------------------------------------------------
#include "projects/smart_home/vent_control/mController.h"

#include "projects/smart_home/shared/mBMP180.h"
#include "projects/smart_home/shared/mMCP3008.h"
#include "projects/smart_home/shared/mPT.h"
#include "projects/smart_home/shared/mMQ9.h"

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace vent_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
enum tMCP3008Output
{
  ePT100 = 0,
  eMQ9,
  eCOUNT
};


//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<gVentControl> cCREATE_ACTION_FOR_G_RASPBERRYPIVENTILATIONCONTROL("VentControl");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// gVentControl constructor
//----------------------------------------------------------------------
gVentControl::gVentControl(core::tFrameworkElement *parent, const std::string &name,
                           const std::string &structure_config_file) :
  tGroup(parent, name, structure_config_file, true)
{

#ifdef _LIB_WIRING_PI_PRESENT_
  auto gpio_interface = new gpio_raspberry_pi::mRaspberryIO(this, "Raspberry Pi GPIO Interface", true, 500000);
  gpio_interface->par_configuration_file.Set("$FINROC_PROJECT_HOME/etc/vent_control_gpio_config.xml");
  gpio_interface->Init();
#endif

  auto bmp180 = new shared::mBMP180(this, "BMP180");
  bmp180->par_temperature_offset.Set(rrlib::si_units::tCelsius<double>(-3.5));
  bmp180->out_air_pressure.ConnectTo(this->out_air_pressure_room);
  bmp180->out_temperature.ConnectTo(this->out_bmp180_temperature_room);

  auto mcp3008 = new shared::mMCP3008<tMCP3008Output::eCOUNT>(this, "MCP3008");
  mcp3008->par_reference_voltage.Set(5.0);
#ifdef _LIB_WIRING_PI_PRESENT_
  mcp3008->in_voltage_raw.at(tMCP3008Output::ePT100).ConnectTo("/Main Thread/VentControl/Raspberry Pi GPIO Interface/Output/MCP3008 AD Voltage PT100");
  mcp3008->in_voltage_raw.at(tMCP3008Output::eMQ9).ConnectTo("/Main Thread/VentControl/Raspberry Pi GPIO Interface/Output/MCP3008 AD Voltage MQ9");
#endif

  auto pt100 = new shared::mPT100(this, "PT100");
  pt100->par_pre_resistance.Set(93.5);
  pt100->par_supply_voltage.Set(5.0);
  pt100->par_reference_voltage.Set(5.0);
  pt100->in_voltage.ConnectTo(mcp3008->out_voltage.at(tMCP3008Output::ePT100));

  auto pt100_filter = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT100 Filter");
  pt100_filter->par_number_of_ports.Set(1);
  pt100_filter->Init();
  pt100_filter->par_weight.Set(0.01);
  pt100_filter->in_input_values.at(0).ConnectTo(pt100->out_temperature);
  pt100_filter->out_filtered_values.at(0).ConnectTo(this->out_pt100_temperature_room);

  auto average_temperature_room = new structure_elements::mMean<rrlib::si_units::tCelsius<double>, double>(this, "Average Temperature Room");
  average_temperature_room->par_number_of_values.Set(2);
  average_temperature_room->Init();
  average_temperature_room->in_signals.at(0).ConnectTo(pt100_filter->out_filtered_values.at(0));
  average_temperature_room->in_signals.at(1).ConnectTo(bmp180->out_temperature);
  average_temperature_room->out_signal.ConnectTo(this->out_average_temperature_room);

  auto mq9 = new shared::mMQ9(this, "MQ9");
  mq9->in_voltage.ConnectTo(mcp3008->out_voltage.at(tMCP3008Output::eMQ9));
  mq9->out_carbon_monoxid.ConnectTo(this->out_carbon_monoxid_room);
#ifdef _LIB_WIRING_PI_PRESENT_
  this->out_carbon_monoxid_threshold_room.ConnectTo("/Main Thread/VentControl/Raspberry Pi GPIO Interface/Output/GPIO MQ9");
#endif

  auto controller = new mController(this, "Controller");
  controller->co_ventilation.ConnectTo(this->out_ventilation);
#ifdef _LIB_WIRING_PI_PRESENT_
  controller->co_gpio_ventilation.ConnectTo("/Main Thread/VentControl/Raspberry Pi GPIO Interface/Input/GPIO Ventilation");
#endif

}

//----------------------------------------------------------------------
// gVentControl destructor
//----------------------------------------------------------------------
gVentControl::~gVentControl()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
