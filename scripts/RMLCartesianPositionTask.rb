require 'orocos'
require 'readline'

Orocos.initialize
Orocos.conf.load_dir('config')

Orocos.run "trajectory_generation::RMLCartesianPositionTask" => "interpolator" do

    interpolator = Orocos::TaskContext.get "interpolator"
    Orocos.conf.apply(interpolator, ["default"], true)
    interpolator.configure
    interpolator.start

    cartesian_state             = Types::Base::Samples::RigidBodyState.new
    cartesian_state.position    = Types::Base::Vector3d.new(0,0,0)
    cartesian_state.orientation = Types::Base::Quaterniond.from_euler(Types::Base::Vector3d.new(-Math::PI,-Math::PI/2,Math::PI/2),2,1,0)
    cartesian_state.time        = Types::Base::Time.now
    cartesian_state.sourceFrame = "current_state_tip"
    cartesian_state.targetFrame = "current_state_root"

    Readline.readline("Press Enter to start")

    cartesian_state_writer = interpolator.cartesian_state.writer
    cartesian_state_writer.write(cartesian_state)

    Readline.readline("Press Enter to send target")

    target             = Types::Base::Samples::RigidBodyState.new
    target.position    = Types::Base::Vector3d.new(1,2,3)
    target.orientation = Types::Base::Quaterniond.from_euler(Types::Base::Vector3d.new(Math::PI,Math::PI/2,-Math::PI/2),2,1,0)
    target.time        = Types::Base::Time.now
    target.sourceFrame = "target_tip"
    target.targetFrame = "target_root"

    target_writer = interpolator.target.writer
    target_writer.write(target)

    command_reader = interpolator.command.reader
    while true
        command = command_reader.read
        if command
            puts "Target position: "    + target.position[0].to_s  + " " + target.position[1].to_s  + " " + target.position[2].to_s
            puts "Commanded position: " + command.position[0].to_s + " " + command.position[1].to_s + " " + command.position[2].to_s
            euler = target.orientation.to_euler
            puts "Target orientation: "    + euler[0].to_s  + " " + euler[1].to_s  + " " + euler[2].to_s
            euler = command.orientation.to_euler
            puts "Commanded orientation: " + euler[0].to_s  + " " + euler[1].to_s  + " " + euler[2].to_s
            puts "---------------------------------------------------"
        end
        sleep 0.01
    end

    Readline.readline("Press Enter to stop")
end
