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
/*!\file    projects/smart_home/heat_control/mController.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mController.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/util/fileio.h"
#include <string>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

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

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<mController> cCREATE_ACTION_FOR_M_CONTROLLER("Controller");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mController constructor
//----------------------------------------------------------------------
mController::mController(core::tFrameworkElement *parent, const std::string &name) :
  tSenseControlModule(parent, name, true),
  ci_control_mode(tControlModeType::eAUTOMATIC),
  par_temperature_set_point_room("Temperature Set Point Room", this, 23.0, "temperature_set_point_room"),
  par_max_update_duration("Max Temperature Update Duration", this, std::chrono::seconds(10), "max_update_duration"),
  par_max_pump_update_duration("Max Pump Duration", this, std::chrono::seconds(45), "max_pump_duration"),
  par_temperature_log_interval("Temperature Log Interval", this, std::chrono::hours(1), "temperature_log_interval"),
  par_temperature_error_log_interval("Temperature Error Log Interval", this, std::chrono::hours(1), "temperature_outdated_error_log_interval"),
  control_state_(nullptr),
  set_point_(23.0),
  error_(tErrorState::eNO_ERROR),
  error_condition_(false),
  last_temperature_logging_time_(rrlib::time::cNO_TIME),
  last_temperature_outdated_logging_time_(rrlib::time::cNO_TIME),
  last_temperature_implausible_logging_time_(rrlib::time::cNO_TIME)
{
  control_state_ = std::unique_ptr<heat_control_states::tState>(new heat_control_states::tReady());

  ci_increase_set_point_temperature.ResetChanged();
  ci_decrease_set_point_temperature.ResetChanged();
  ci_reset_set_point_temperature.ResetChanged();

  // start logging
  std::string temperature_filename = rrlib::util::fileio::ShellExpandFilename("$HOME/temperatures_" + rrlib::time::ToFilenameCompatibleString(rrlib::time::Now()) + ".txt");
  temperature_log_file_.open(temperature_filename, std::fstream::in | std::fstream::out | std::fstream::app);

  // check if opening the file was successful
  if (temperature_log_file_.fail() and not temperature_filename.empty())
  {
    RRLIB_LOG_PRINT(ERROR, "Failed to open file: ", temperature_filename);
  }
  if (temperature_log_file_.good())
  {
    RRLIB_LOG_PRINT(DEBUG, "Start logging temperatures: ", temperature_filename);
    temperature_log_file_ << "Temperaturen (" << rrlib::time::ToFilenameCompatibleString(rrlib::time::Now()) << ")\n";
    temperature_log_file_ << "-----------------------------------------------------\n";
    temperature_log_file_ << "Zeit, ";
    temperature_log_file_ << "Speicher (unten), ";
    temperature_log_file_ << "Speicher (mitte), ";
    temperature_log_file_ << "Speicher (oben), ";
    temperature_log_file_ << "Ofen, ";
    temperature_log_file_ << "Garage, ";
    temperature_log_file_ << "Bodenplatte, ";
    temperature_log_file_ << "Raum, ";
    temperature_log_file_ << "Solar\n";
    temperature_log_file_ << "-----------------------------------------------------\n";
  }

  std::string event_filename = rrlib::util::fileio::ShellExpandFilename("$HOME/events_" + rrlib::time::ToFilenameCompatibleString(rrlib::time::Now()) + ".txt");
  event_log_file_.open(event_filename, std::fstream::in | std::fstream::out | std::fstream::app);

  // check if opening the file was successful
  if (event_log_file_.fail() and not event_filename.empty())
  {
    RRLIB_LOG_PRINT(ERROR, "Failed to open file: ", event_filename);
  }
  if (event_log_file_.good())
  {
    RRLIB_LOG_PRINT(DEBUG, "Start logging events: ", event_filename);
    event_log_file_ << "Systemereignisse (" << rrlib::time::ToFilenameCompatibleString(rrlib::time::Now()) << ")\n";
    event_log_file_ << "-----------------------------------------------------\n";
    event_log_file_ << "Zeit, Beschreibung\n";
    event_log_file_ << "-----------------------------------------------------\n";
    event_log_file_ << rrlib::time::Now() << " Start der Steuerung\n";

  }
}

//----------------------------------------------------------------------
// mController destructor
//----------------------------------------------------------------------
mController::~mController()
{
  if (temperature_log_file_.is_open())
  {
    temperature_log_file_.close();
  }
  if (event_log_file_.is_open())
  {
    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Herunterfahren der Steuerung\n";
    }
    event_log_file_.close();
  }
}

//----------------------------------------------------------------------
// mController OnParameterChange
//----------------------------------------------------------------------
void mController::OnParameterChange()
{
  if (par_temperature_set_point_room.HasChanged())
  {
    this->set_point_ = rrlib::si_units::tCelsius<double>(par_temperature_set_point_room.Get());
    co_set_point_temperature.Publish(set_point_, rrlib::time::Now());
  }
}

//----------------------------------------------------------------------
// mController Sense
//----------------------------------------------------------------------
void mController::Sense()
{
  bool outdated_temperature = false;
  bool previous_outdated_temperature = std::all_of(temperature_update_error_condition_.begin(), temperature_update_error_condition_.end(), [](bool i)
  {
    return i;
  });

  auto current_time = rrlib::time::Now();

  bool sensor_value_outdated = false;
  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_boiler_bottom.GetTimestamp()) ? true : false;
  so_outdated_temperature_boiler_bottom.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eBOILER_BOTTOM_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_boiler_top.GetTimestamp()) ? true : false;
  so_outdated_temperature_boiler_top.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eBOILER_TOP_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_boiler_middle.GetTimestamp()) ? true : false;
  so_outdated_temperature_boiler_middle.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eBOILER_MIDDLE_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_furnace.GetTimestamp()) ? true : false;
  so_outdated_temperature_furnace.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eFURNACE_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_garage.GetTimestamp()) ? true : false;
  so_outdated_temperature_garage.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eGARAGE_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_ground.GetTimestamp()) ? true : false;
  so_outdated_temperature_ground.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eGROUND_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_room.GetTimestamp()) ? true : false;
  so_outdated_temperature_room.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eROOM_SENSOR) = sensor_value_outdated;

  sensor_value_outdated = (current_time - par_max_update_duration.Get() > si_temperature_solar.GetTimestamp()) ? true : false;
  so_outdated_temperature_solar.Publish(sensor_value_outdated, current_time);
  outdated_temperature = outdated_temperature or sensor_value_outdated;
  temperature_update_error_condition_.at(tTemperatureSensors::eSOLAR_SENSOR) = sensor_value_outdated;

  bool external_outdated = (current_time - par_max_update_duration.Get() > si_temperature_room_external.GetTimestamp()) ? true : false;
  so_outdated_temperature_room_external.Publish(external_outdated, current_time);

  // log the error event
  if (event_log_file_.good() and outdated_temperature)
  {
    // log after a duration or new failure
    if ((last_temperature_outdated_logging_time_ + par_temperature_error_log_interval.Get() < current_time))
    {
      event_log_file_ << rrlib::time::Now() << " Fehlerzustand: Temperaturdaten veraltet (";
      for (size_t i = 0; i < temperature_update_error_condition_.size(); i++)
      {
        if (temperature_update_error_condition_.at(i))
        {
          event_log_file_ << make_builder::GetEnumString(static_cast<tTemperatureSensors>(i)) << "; ";
        }
      }
      event_log_file_ << ")\n";

      last_temperature_outdated_logging_time_ = current_time;
    }
  }
  if (event_log_file_.good() and previous_outdated_temperature and not outdated_temperature)
  {
    event_log_file_ << rrlib::time::Now() << " Zustand: Alle Temperaturdaten sind wieder aktuell.\n";
  }

  bool implausible_temperature = false;

  if (this->SensorInputChanged())
  {
    // check plausibility of temperatures
    bool sensor_value_implausible = false;
    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_room.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_room.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eROOM_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_solar.Get(), rrlib::si_units::tCelsius<double>(150.0), rrlib::si_units::tCelsius<double>(-40.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_solar.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eSOLAR_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_ground.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_ground.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eGROUND_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_garage.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_garage.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eGARAGE_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_furnace.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_furnace.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eFURNACE_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_boiler_bottom.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_boiler_bottom.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eBOILER_BOTTOM_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_boiler_top.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_boiler_top.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eBOILER_TOP_SENSOR) = sensor_value_implausible;

    sensor_value_implausible = not IsTemperatureInBounds(si_temperature_boiler_middle.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or sensor_value_implausible;
    so_implausible_temperature_boiler_middle.Publish(sensor_value_implausible, current_time);
    this->temperature_plausibility_error_condition_.at(tTemperatureSensors::eBOILER_MIDDLE_SENSOR) = sensor_value_implausible;

    // logging of wrong temperature values
    if (event_log_file_.good() and implausible_temperature)
    {
      if (last_temperature_implausible_logging_time_ + par_temperature_error_log_interval.Get() < current_time)
      {
        event_log_file_ << rrlib::time::Now() << " Fehlerzustand: Temperaturdaten sind nicht plausibel (";
        event_log_file_ << "Raum " << si_temperature_room.Get().ValueFactored() << "; ";
        event_log_file_ << "Solar " << si_temperature_solar.Get().ValueFactored() << "; ";
        event_log_file_ << "Bodenplatte " << si_temperature_ground.Get().ValueFactored() << "; ";
        event_log_file_ << "Ofen " << si_temperature_furnace.Get().ValueFactored() << "; ";
        event_log_file_ << "Garage " << si_temperature_garage.Get().ValueFactored() << "; ";
        event_log_file_ << "Speicher (oben) " << si_temperature_boiler_top.Get().ValueFactored() << "; ";
        event_log_file_ << "Speicher (mitte) " << si_temperature_boiler_middle.Get().ValueFactored() << "; ";
        event_log_file_ << "Speicher (unten) " << si_temperature_boiler_bottom.Get().ValueFactored() << ")\n";

        last_temperature_implausible_logging_time_ = current_time;
      }
    }

    if (event_log_file_.good() and
        (this->error_ == tErrorState::eIMPLAUSIBLE_TEMPERATURE or
         this->error_ == tErrorState::eIMPLAUSIBLE_OUTDATED_TEMPERATURE)
        and not implausible_temperature)
    {
      event_log_file_ << rrlib::time::Now() << " Zustand: Alle Temperaturdaten sind wieder plausibel.\n";
    }

    bool external_implausible = not IsTemperatureInBounds(si_temperature_room_external.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    so_implausible_temperature_room_external.Publish(external_implausible, current_time);

    // integrate external room temperature if value is available
    auto temperature_room = si_temperature_room.Get();
    if (not external_outdated and not external_implausible)
    {
      temperature_room += si_temperature_room_external.Get();
      temperature_room /= 2.0;
      so_temperature_room_combined.Publish(temperature_room, si_temperature_room.GetTimestamp());
    }

    temperatures_ = shared::tTemperatures
    {
      si_temperature_boiler_middle.Get(),
      temperature_room,
      si_temperature_solar.Get(),
      si_temperature_ground.Get(),
      set_point_
    };

    // log temperatures
    if (temperature_log_file_.good())
    {
      if (last_temperature_logging_time_ + par_temperature_log_interval.Get() < current_time)
      {
        temperature_log_file_ << rrlib::time::Now() << ",";
        temperature_log_file_ << si_temperature_boiler_bottom.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_boiler_middle.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_boiler_top.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_furnace.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_garage.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_ground.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_room.Get().ValueFactored() << ", ";
        temperature_log_file_ << si_temperature_solar.Get().ValueFactored() << "\n";

        last_temperature_logging_time_ = current_time;
        temperature_log_file_.flush();
      }
    }
  }

  // log error condition recovery
  if (event_log_file_.good() and this->error_condition_
      and not implausible_temperature
      and not outdated_temperature)
  {
    event_log_file_ << rrlib::time::Now() << " Zustand: Steuerung ist wieder im fehlerfreien Zustand.\n";
  }

  // flush to drive
  event_log_file_.flush();


  // determine error state
  if (implausible_temperature)
  {
    if (outdated_temperature)
    {
      this->error_ = tErrorState::eIMPLAUSIBLE_OUTDATED_TEMPERATURE;
    }
    else
    {
      this->error_ = tErrorState::eIMPLAUSIBLE_TEMPERATURE;
    }
    this->error_condition_ = true;
  }
  else
  {
    if (outdated_temperature)
    {
      this->error_ = tErrorState::eOUTDATED_TEMPERATURE;
      this->error_condition_ = true;
    }
    else
    {
      this->error_ = tErrorState::eNO_ERROR;
    }
  }
  this->so_error_state.Publish(error_, current_time);
  this->so_error_condition.Publish(error_condition_, current_time);
  if (error_condition_)
  {
    this->so_last_error_time.Publish(current_time, current_time);
  }
}

//----------------------------------------------------------------------
// mController Control
//----------------------------------------------------------------------
void mController::Control()
{
  // reset if control mode changes
  if (ci_control_mode.HasChanged())
  {
    co_pump_online_ground.Publish(false, rrlib::time::Now());
    co_pump_online_room.Publish(false, rrlib::time::Now());
    co_pump_online_solar.Publish(false, rrlib::time::Now());
    if (control_state_ != nullptr)
    {
      control_state_ = std::unique_ptr<heat_control_states::tState>(new heat_control_states::tReady());
    }
    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Zustand: Neuer Heizungskontrollzustand <" << make_builder::GetEnumString(ci_control_mode.Get()) << ">\n";
    }

    co_control_mode.Publish(ci_control_mode.Get(), ci_control_mode.GetTimestamp());
  }

  // handle set point
  if (ci_increase_set_point_temperature.HasChanged())
  {
    set_point_ += rrlib::si_units::tTemperature<double>(0.5);
    co_set_point_temperature.Publish(set_point_, ci_increase_set_point_temperature.GetTimestamp());

    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Erhöhung der Solltemperatur auf " << set_point_ << "\n";
    }
  }
  if (ci_decrease_set_point_temperature.HasChanged())
  {
    set_point_ -= rrlib::si_units::tTemperature<double>(0.5);
    co_set_point_temperature.Publish(set_point_, ci_decrease_set_point_temperature.GetTimestamp());

    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Reduzierung der Solltemperatur auf " << set_point_ << "\n";
    }
  }
  if (ci_reset_set_point_temperature.HasChanged())
  {
    set_point_ = par_temperature_set_point_room.Get();
    co_set_point_temperature.Publish(set_point_, ci_reset_set_point_temperature.GetTimestamp());

    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Zurücksetzung der Solltemperatur auf " << set_point_ << "\n";
    }
  }

  // error condition: determine how the issue affects the pumps
  bool pump_ground_error = false;
  bool pump_solar_error = false;
  bool pump_room_error = false;

  if ((this->error_ != tErrorState::eNO_ERROR))
  {
    bool boiler_sensor_failure_ =
      temperature_plausibility_error_condition_.at(tTemperatureSensors::eBOILER_MIDDLE_SENSOR) or
      temperature_update_error_condition_.at(tTemperatureSensors::eBOILER_MIDDLE_SENSOR);
    bool room_sensor_failure_ =
      temperature_plausibility_error_condition_.at(tTemperatureSensors::eROOM_SENSOR) or
      temperature_update_error_condition_.at(tTemperatureSensors::eROOM_SENSOR);
    bool solar_sensor_failure_ =
      temperature_plausibility_error_condition_.at(tTemperatureSensors::eSOLAR_SENSOR) or
      temperature_update_error_condition_.at(tTemperatureSensors::eSOLAR_SENSOR);
    bool ground_sensor_failure_ =
      temperature_plausibility_error_condition_.at(tTemperatureSensors::eGROUND_SENSOR) or
      temperature_update_error_condition_.at(tTemperatureSensors::eGROUND_SENSOR);

    // error affects room pump -> boiler temperature outdated or implausible | room temperature outdated or implausible
    if (boiler_sensor_failure_ or room_sensor_failure_)
    {
      pump_room_error = true;
      co_pump_online_room.Publish(false, rrlib::time::Now());
    }

    // error affects solar pump -> boiler temperature outdated or implausible | solar temperature outdated or implausible
    if (boiler_sensor_failure_ or solar_sensor_failure_)
    {
      pump_solar_error = true;
      co_pump_online_solar.Publish(false, rrlib::time::Now());
    }

    // error affects ground pump -> boiler temperature outdated or implausible | ground temperature outdated or implausible
    if (boiler_sensor_failure_ or ground_sensor_failure_)
    {
      pump_ground_error = true;
      co_pump_online_ground.Publish(false, rrlib::time::Now());
    }
  }

  co_pump_error_ground.Publish(pump_ground_error, rrlib::time::Now());
  co_pump_error_room.Publish(pump_room_error, rrlib::time::Now());
  co_pump_error_solar.Publish(pump_solar_error, rrlib::time::Now());

  co_pump_working_ground.Publish(not pump_ground_error, rrlib::time::Now());
  co_pump_working_room.Publish(not pump_room_error, rrlib::time::Now());
  co_pump_working_solar.Publish(not pump_solar_error, rrlib::time::Now());

  // determine state
  bool state_changed = false;
  std::unique_ptr<heat_control_states::tState> next_state;
  control_state_->ComputeControlState(next_state, temperatures_);
  state_changed = control_state_->HasChanged();
  if (state_changed)
  {
    control_state_ = std::move(next_state);
    co_heating_state.Publish(control_state_->GetCurrentState(), rrlib::time::Now());

    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: <" << make_builder::GetEnumString(control_state_->GetCurrentState());
      event_log_file_ << ">   (Raum " << si_temperature_room.Get().ValueFactored() << "; ";
      event_log_file_ << "Solar " << si_temperature_solar.Get().ValueFactored() << "; ";
      event_log_file_ << "Bodenplatte " << si_temperature_ground.Get().ValueFactored() << "; ";
      event_log_file_ << "Ofen " << si_temperature_furnace.Get().ValueFactored() << "; ";
      event_log_file_ << "Garage " << si_temperature_garage.Get().ValueFactored() << "; ";
      event_log_file_ << "Speicher (oben) " << si_temperature_boiler_top.Get().ValueFactored() << "; ";
      event_log_file_ << "Speicher (mitte) " << si_temperature_boiler_middle.Get().ValueFactored() << "; ";
      event_log_file_ << "Speicher (unten) " << si_temperature_boiler_bottom.Get().ValueFactored() << ")\n";
    }
  }

  // reactivate pump state in case of error recovery
  if (error_condition_ and error_ == tErrorState::eNO_ERROR)
  {
    error_condition_ = false;
    state_changed = true;

    if (event_log_file_.good())
    {
      event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Wiederherstellung des Zustands nach einem Fehler.\n";
    }
  }

  // control mode
  switch (ci_control_mode.Get())
  {
  case tControlModeType::eAUTOMATIC:
  {
    // get pump settings
    auto pumps = control_state_->GetPumpSettings();

    // check if state differs from last state and update time window is valid
    if (this->pump_last_state_.at(tPumps::eGROUND) != pumps.IsGroundOnline() and
        this->pump_switch_time_.at(tPumps::eGROUND) + par_max_pump_update_duration.Get() < rrlib::time::Now())
    {
      // no error condition
      if (not pump_room_error)
      {
        co_pump_online_ground.Publish(pumps.IsGroundOnline(), rrlib::time::Now());
        this->pump_last_state_.at(tPumps::eGROUND) = pumps.IsGroundOnline();
        this->pump_switch_time_.at(tPumps::eGROUND) = rrlib::time::Now();

        if (event_log_file_.good())
        {
          if (pumps.IsGroundOnline())
          {
            event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Aktiviere Pumpe Bodenplatte.\n";
          }
          else
          {
            event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Deaktiviere Pumpe Bodenplatte.\n";
          }
        }
      }
      // error condition
      else
      {
        co_pump_online_ground.Publish(false, rrlib::time::Now());
        if (event_log_file_.good())
        {
          event_log_file_ << rrlib::time::Now() << " Fehlerzustand: Zustandswechsel von Pumpe Boden verhindert. Pumpe deaktiviert.\n";
        }
      }
    }

    // check if state differs from last state and update time window is valid
    if (this->pump_last_state_.at(tPumps::eROOM) != pumps.IsRoomOnline() and
        this->pump_switch_time_.at(tPumps::eROOM) + par_max_pump_update_duration.Get() < rrlib::time::Now())
    {
      // no error condition
      if (not pump_room_error)
      {
        co_pump_online_room.Publish(pumps.IsRoomOnline(), rrlib::time::Now());
        this->pump_last_state_.at(tPumps::eROOM) = pumps.IsRoomOnline();
        this->pump_switch_time_.at(tPumps::eROOM) = rrlib::time::Now();

        if (event_log_file_.good())
        {
          if (pumps.IsRoomOnline())
          {
            event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Aktiviere Pumpe Raum.\n";
          }
          else
          {
            event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Deaktiviere Pumpe Raum.\n";
          }
        }
      }
      // error condition
      else
      {
        co_pump_online_room.Publish(false, rrlib::time::Now());
        if (event_log_file_.good())
        {
          event_log_file_ << rrlib::time::Now() << " Fehlerzustand: Zustandswechsel von Pumpe Raum verhindert. Pumpe deaktiviert.\n";
        }
      }
    }

    // solar pump cannot be disabled
    // check if state differs from last state and update time window is valid
    if (this->pump_last_state_.at(tPumps::eSOLAR) != pumps.IsSolarOnline() and
        this->pump_switch_time_.at(tPumps::eSOLAR) + par_max_pump_update_duration.Get() < rrlib::time::Now())
    {
      // bo error condition
      if (not pump_solar_error)
      {
        co_pump_online_solar.Publish(pumps.IsSolarOnline(), rrlib::time::Now());

        this->pump_last_state_.at(tPumps::eSOLAR) = pumps.IsSolarOnline();
        this->pump_switch_time_.at(tPumps::eSOLAR) = rrlib::time::Now();

        if (event_log_file_.good())
        {
          if (pumps.IsSolarOnline())
          {
            event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Aktiviere Pumpe Solar.\n";
          }
          else
          {
            event_log_file_ << rrlib::time::Now() << " Automatischer Zustandswechsel: Deaktiviere Pumpe Solar.\n";
          }
        }
      }
      // error condition
      else
      {
        co_pump_online_solar.Publish(false, rrlib::time::Now());
        if (event_log_file_.good())
        {
          event_log_file_ << rrlib::time::Now() << " Fehlerzustand: Zustandswechsel von Pumpe Solar verhindert. Pumpe deaktiviert.\n";
        }
      }

    }
  }
  break;
  case tControlModeType::eSTOP:
    if (ci_control_mode.HasChanged())
    {
      co_pump_online_ground.Publish(false, rrlib::time::Now());
      co_pump_online_room.Publish(false, rrlib::time::Now());
      co_pump_online_solar.Publish(false, rrlib::time::Now());
    }
    break;
  case tControlModeType::eMANUAL:
  {
    if (ci_manual_pump_online_ground.HasChanged())
    {
      co_pump_online_ground.Publish(ci_manual_pump_online_ground.Get());

      if (event_log_file_.good())
      {
        if (ci_manual_pump_online_ground.Get())
        {
          event_log_file_ << rrlib::time::Now() << " Manueller Zustandswechsel: Aktiviere Pumpe Bodenplatte.\n";
        }
        else
        {
          event_log_file_ << rrlib::time::Now() << " Manueller Zustandswechsel: Deaktiviere Pumpe Bodenplatte.\n";
        }
      }
    }
    if (ci_manual_pump_online_room.HasChanged())
    {
      co_pump_online_room.Publish(ci_manual_pump_online_room.Get());

      if (event_log_file_.good())
      {
        if (ci_manual_pump_online_room.Get())
        {
          event_log_file_ << rrlib::time::Now() << " Manueller Zustandswechsel: Aktiviere Pumpe Raum.\n";
        }
        else
        {
          event_log_file_ << rrlib::time::Now() << " Manueller Zustandswechsel: Deaktiviere Pumpe Raum.\n";
        }
      }
    }
    if (ci_manual_pump_online_solar.HasChanged())
    {
      co_pump_online_solar.Publish(ci_manual_pump_online_solar.Get());

      if (event_log_file_.good())
      {
        if (ci_manual_pump_online_room.Get())
        {
          event_log_file_ << rrlib::time::Now() << " Manueller Zustandswechsel: Aktiviere Pumpe Solar.\n";
        }
        else
        {
          event_log_file_ << rrlib::time::Now() << " Manueller Zustandswechsel: Deaktiviere Pumpe Solar.\n";
        }
      }
    }
  }
  break;
  default:
    co_pump_online_ground.Publish(false, rrlib::time::Now());
    co_pump_online_room.Publish(false, rrlib::time::Now());
    co_pump_online_solar.Publish(false, rrlib::time::Now());
  };

  // led control
  auto boiler = si_temperature_boiler_middle.Get();
  if (boiler <= rrlib::si_units::tCelsius<double>(25.0))
  {
    co_led_online_green.Publish(false, rrlib::time::Now());
    co_led_online_yellow.Publish(false, rrlib::time::Now());
    co_led_online_red.Publish(true, rrlib::time::Now());
  }
  else if (boiler > rrlib::si_units::tCelsius<double>(25.0) && boiler <= rrlib::si_units::tCelsius<double>(30.0))
  {
    co_led_online_green.Publish(false, rrlib::time::Now());
    co_led_online_yellow.Publish(true, rrlib::time::Now());
    co_led_online_red.Publish(true, rrlib::time::Now());
  }
  else if (boiler > rrlib::si_units::tCelsius<double>(30.0) && boiler <= rrlib::si_units::tCelsius<double>(40.0))
  {
    co_led_online_green.Publish(false, rrlib::time::Now());
    co_led_online_yellow.Publish(true, rrlib::time::Now());
    co_led_online_red.Publish(false, rrlib::time::Now());
  }
  else if (boiler > rrlib::si_units::tCelsius<double>(40.0) && boiler <= rrlib::si_units::tCelsius<double>(45.0))
  {
    co_led_online_green.Publish(true, rrlib::time::Now());
    co_led_online_yellow.Publish(true, rrlib::time::Now());
    co_led_online_red.Publish(false, rrlib::time::Now());
  }
  else if (boiler > rrlib::si_units::tCelsius<double>(45.0))
  {
    co_led_online_green.Publish(true, rrlib::time::Now());
    co_led_online_yellow.Publish(false, rrlib::time::Now());
    co_led_online_red.Publish(false, rrlib::time::Now());
  }

}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
