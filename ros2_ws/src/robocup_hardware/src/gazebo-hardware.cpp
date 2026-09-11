#include "../include/robocup_hardware/gazebo-hardware.hpp"
#include <gz/sim/Types.hh>
#include <gz/sim/components/JointVelocity.hh>
#include <hardware_interface/handle.hpp>
#include <hardware_interface/types/hardware_interface_return_values.hpp>
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <pluginlib/class_list_macros.hpp>
#include <vector>

GazeboHardware::CallbackReturn
GazeboHardware::on_init(const hardware_interface::HardwareInfo &hardware_info) {
  if (hardware_interface::SystemInterface::on_init(hardware_info) !=
      CallbackReturn::SUCCESS) {
    return CallbackReturn::ERROR;
  }

  return CallbackReturn::SUCCESS;
};

GazeboHardware::CallbackReturn
GazeboHardware::on_configure(const rclcpp_lifecycle::State &) {
  return CallbackReturn::SUCCESS;
};

GazeboHardware::CallbackReturn
GazeboHardware::on_activate(const rclcpp_lifecycle::State &) {
  return CallbackReturn::SUCCESS;
};

GazeboHardware::CallbackReturn
GazeboHardware::on_deactivate(const rclcpp_lifecycle::State &) {
  return CallbackReturn::SUCCESS;
};

bool GazeboHardware::initSim(
    rclcpp::Node::SharedPtr &model_nh,
    std::map<std::string, sim::Entity> &joints_map,
    const hardware_interface::HardwareInfo &info,
    sim::EntityComponentManager &entity_component_manager, int &_ur) {
  nh_ = model_nh;

  info_ = info;
  ecm_ = &entity_component_manager;
  update_rate = _ur;

  for (const auto &joint_info : info_.joints) {
    auto it = joints_map.find(joint_info.name);

    if (it == joints_map.end()) {
      return false;
    }

    wheel_entities_.emplace_back(it->second);
  }

  auto entity = wheel_entities_[0];

  if (!ecm_->Component<sim::components::JointPosition>(entity)) {
    ecm_->CreateComponent(entity, sim::components::JointPosition({0.0}));
  }

  if (!ecm_->Component<sim::components::JointVelocity>(entity)) {
    ecm_->CreateComponent(entity, sim::components::JointVelocity({0.0}));
  }
  ecm_->CreateComponent(wheel_entities_[0],
                        sim::components::JointVelocityCmd({0.0}));

  velocity_command_.resize(1, 0.0);
  velocity_state_.resize(1, 0.0);
  position_state_.resize(1, 0.0);

  return true;
};

std::vector<hardware_interface::CommandInterface>
GazeboHardware::export_command_interfaces() {
  std::vector<hardware_interface::CommandInterface> commands;
  commands.emplace_back(info_.joints[0].name,
                        hardware_interface::HW_IF_VELOCITY,
                        &velocity_command_[0]);
  return commands;
};

std::vector<hardware_interface::StateInterface>
GazeboHardware::export_state_interfaces() {
  std::vector<hardware_interface::StateInterface> states;
  states.emplace_back(info_.joints[0].name, hardware_interface::HW_IF_VELOCITY,
                      &velocity_state_[0]);
  states.emplace_back(info_.joints[0].name, hardware_interface::HW_IF_POSITION,
                      &position_state_[0]);
  return states;
};

hardware_interface::return_type
GazeboHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {

  auto entity = wheel_entities_[0];

  auto pos_component = ecm_->Component<sim::components::JointPosition>(entity);

  auto vel_component = ecm_->Component<sim::components::JointVelocity>(entity);

  if (pos_component->Data().empty() || vel_component->Data().empty()) {
    return hardware_interface::return_type::OK;
  }
  position_state_[0] = pos_component->Data()[0];
  velocity_state_[0] = vel_component->Data()[0];

  return hardware_interface::return_type::OK;
};

hardware_interface::return_type
GazeboHardware::write(const rclcpp::Time &time,
                      const rclcpp::Duration &period) {

  auto vel_component =
      ecm_->Component<sim::components::JointVelocityCmd>(wheel_entities_[0]);

  std::vector<double> command = {velocity_command_[0]};

  if (!vel_component->Data().empty()) {
    vel_component->Data()[0] = velocity_command_[0];
  }

  return hardware_interface::return_type::OK;
};

PLUGINLIB_EXPORT_CLASS(GazeboHardware,
                       gz_ros2_control::GazeboSimSystemInterface);
