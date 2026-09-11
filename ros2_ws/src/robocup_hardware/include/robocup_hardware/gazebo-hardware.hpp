#include <gz/sim/components/JointPosition.hh>
#include <gz/sim/components/JointVelocity.hh>
#include <gz/sim/components/JointVelocityCmd.hh>
#include <gz_ros2_control/gz_system_interface.hpp>

class GazeboHardware : public gz_ros2_control::GazeboSimSystemInterface {

public:
  using CallbackReturn =
      rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  CallbackReturn
  on_init(const hardware_interface::HardwareInfo &hardware_info) override;

  CallbackReturn
  on_configure(const rclcpp_lifecycle::State &previous_state) override;

  CallbackReturn
  on_activate(const rclcpp_lifecycle::State &previous_state) override;

  CallbackReturn
  on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  bool initSim(rclcpp::Node::SharedPtr &model_nh,
               std::map<std::string, sim::Entity> &joints,
               const hardware_interface::HardwareInfo &hardware_info,
               sim::EntityComponentManager &_ecm, int &update_rate) override;

  std::vector<hardware_interface::StateInterface>
  export_state_interfaces() override;

  std::vector<hardware_interface::CommandInterface>
  export_command_interfaces() override;

  hardware_interface::return_type read(const rclcpp::Time &time,
                                       const rclcpp::Duration &period) override;

  hardware_interface::return_type
  write(const rclcpp::Time &time, const rclcpp::Duration &period) override;

private:
  std::vector<sim::Entity> wheel_entities_;
  sim::EntityComponentManager *ecm_ = nullptr;
  int update_rate;

  std::vector<double> velocity_command_;
  std::vector<double> velocity_state_;
  std::vector<double> position_state_;
};