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
/*!\file    projects/smart_home/heat_control/gHeatControl.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/gHeatControl.h"

//----------------------------------------------------------------------
// External includes
//----------------------------------------------------------------------
#include <cassert>

#ifdef _LIB_WIRING_PI_PRESENT_
#include "libraries/gpio_raspberry_pi/mRaspberryIO.h"
#endif

#include "libraries/signal_filters/mExponentialFilter.h"

//----------------------------------------------------------------------
// Internal includes
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mController.h"
#include "projects/smart_home/heat_control/mPumpInterface.h"

#include "projects/smart_home/shared/mMCP3008.h"
#include "projects/smart_home/shared/mPT.h"

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
namespace heat_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
enum tMCP3008Output
{
	ePT1000_SOLAR = 0,
	ePT1000_ROOM,
	ePT1000_BOILER,
	ePT1000_GROUND,
	eCOUNT
};
//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<gHeatControl> cCREATE_ACTION_FOR_G_RASPBERRYPIHEATINGCONTROL("HeatControl");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// gHeatControl constructor
//----------------------------------------------------------------------
gHeatControl::gHeatControl(core::tFrameworkElement *parent, const std::string &name,
                           const std::string &structure_config_file) :
  tSenseControlGroup(parent, name, structure_config_file, true), // change to 'true' to make group's ports shared (so that ports in other processes can connect to its output and/or input ports)
  ci_control_mode(tControlModeType::eAUTOMATIC),
  ci_manual_pump_online_solar(false),
  ci_manual_pump_online_room(false),
  ci_manual_pump_online_ground(false),
  ci_disable_pump_room(false),
  ci_disable_pump_ground(false)
{
  auto controller = new mController(this, "Controller");
  controller->par_temperature_set_point_room.Set(22.5);

  this->ci_control_mode.ConnectTo(controller->ci_control_mode);

  this->ci_manual_pump_online_ground.ConnectTo(controller->ci_manual_pump_online_ground);
  this->ci_manual_pump_online_room.ConnectTo(controller->ci_manual_pump_online_room);
  this->ci_manual_pump_online_solar.ConnectTo(controller->ci_manual_pump_online_solar);
  this->ci_disable_pump_ground.ConnectTo(controller->ci_disable_pump_ground);
  this->ci_disable_pump_room.ConnectTo(controller->ci_disable_pump_room);

  this->so_led_online_green.ConnectTo(controller->co_led_online_green);
  this->so_led_online_yellow.ConnectTo(controller->co_led_online_yellow);
  this->so_led_online_red.ConnectTo(controller->co_led_online_red);
  this->so_pump_online_ground.ConnectTo(controller->co_pump_online_ground);
  this->so_pump_online_room.ConnectTo(controller->co_pump_online_room);
  this->so_pump_online_solar.ConnectTo(controller->co_pump_online_solar);

  this->co_control_mode.ConnectTo(controller->co_control_mode);
  this->co_heating_state.ConnectTo(controller->co_heating_state);

  auto pump_interface = new mPumpInterface(this, "Pump Interface");
  pump_interface->in_pump_online_ground.ConnectTo(controller->co_pump_online_ground);
  pump_interface->in_pump_online_room.ConnectTo(controller->co_pump_online_room);
  pump_interface->in_pump_online_solar.ConnectTo(controller->co_pump_online_solar);

#ifdef _LIB_WIRING_PI_PRESENT_
  auto gpio_interface = new finroc::gpio_raspberry_pi::mRaspberryIO(this, "Raspberry Pi GPIO Interface");
  gpio_interface->par_configuration_file.Set("$FINROC_PROJECT_HOME/etc/heat_control_gpio_config.xml");
  gpio_interface->Init();
  gpio_interface->GetInputs().ConnectByName(pump_interface->GetOutputs(), true);
  gpio_interface->GetOutputs().ConnectByName(pump_interface->GetInputs(), true);
#endif

  auto mcp_3008 = new shared::mMCP3008<tMCP3008Output::eCOUNT>(this, "MCP3008");
  mcp_3008->par_reference_voltage.Set(5.0);

  auto pt1000_room = new shared::mPT1000(this, "PT1000 Room");
  pt1000_room->par_pre_resistance.Set(1950);
  pt1000_room->par_reference_voltage.Set(5.0);
  pt1000_room->par_supply_voltage.Set(5.0);
  pt1000_room->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_ROOM));

  auto pt1000_boiler = new shared::mPT1000(this, "PT1000 Boiler");
  pt1000_boiler->par_pre_resistance.Set(1949);
  pt1000_boiler->par_reference_voltage.Set(5.0);
  pt1000_boiler->par_supply_voltage.Set(5.0);
  pt1000_boiler->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_BOILER));

  auto pt1000_solar = new shared::mPT1000(this, "PT1000 Solar");
  pt1000_solar->par_pre_resistance.Set(1987);
  pt1000_solar->par_reference_voltage.Set(5.0);
  pt1000_solar->par_supply_voltage.Set(5.0);
  pt1000_solar->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_SOLAR));

  auto pt1000_ground = new shared::mPT1000(this, "PT1000 Ground");
  pt1000_ground->par_pre_resistance.Set(1922);
  pt1000_ground->par_reference_voltage.Set(5.0);
  pt1000_ground->par_supply_voltage.Set(5.0);
  pt1000_ground->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_GROUND));

  auto exponential_filter_room = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Room");
  exponential_filter_room->par_number_of_ports.Set(1);
  exponential_filter_room->Init();
  exponential_filter_room->par_weight.Set(0.00035);
  exponential_filter_room->in_input_values.at(0).ConnectTo(pt1000_room->out_temperature);
  exponential_filter_room->out_filtered_values.at(0).ConnectTo(controller->si_temperature_room);

  auto exponential_filter_ground = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Ground");
  exponential_filter_ground->par_number_of_ports.Set(1);
  exponential_filter_ground->Init();
  exponential_filter_ground->par_weight.Set(0.0025);
  exponential_filter_ground->in_input_values.at(0).ConnectTo(pt1000_ground->out_temperature);
  exponential_filter_ground->out_filtered_values.at(0).ConnectTo(controller->si_temperature_ground);

  auto exponential_filter_solar = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Solar");
  exponential_filter_solar->par_number_of_ports.Set(1);
  exponential_filter_solar->Init();
  exponential_filter_solar->par_weight.Set(0.0025);
  exponential_filter_solar->in_input_values.at(0).ConnectTo(pt1000_solar->out_temperature);
  exponential_filter_solar->out_filtered_values.at(0).ConnectTo(controller->si_temperature_solar);

  auto exponential_filter_boiler = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Boiler");
  exponential_filter_boiler->par_number_of_ports.Set(1);
  exponential_filter_boiler->Init();
  exponential_filter_boiler->par_weight.Set(0.0025);
  exponential_filter_boiler->in_input_values.at(0).ConnectTo(pt1000_boiler->out_temperature);
  exponential_filter_boiler->out_filtered_values.at(0).ConnectTo(controller->si_temperature_boiler);

  this->so_temperature_room.ConnectTo(exponential_filter_room->out_filtered_values.at(0));
  this->so_temperature_ground.ConnectTo(exponential_filter_ground->out_filtered_values.at(0));
  this->so_temperature_solar.ConnectTo(exponential_filter_solar->out_filtered_values.at(0));
  this->so_temperature_boiler.ConnectTo(exponential_filter_boiler->out_filtered_values.at(0));

}

//----------------------------------------------------------------------
// gHeatControl destructor
//----------------------------------------------------------------------
gHeatControl::~gHeatControl()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
