#ifndef trajectory_generation_TYPES_HPP
#define trajectory_generation_TYPES_HPP

#include <base/commands/Joints.hpp>

namespace trajectory_generation {

/** Behavior at positional limits. Only reflexxes type IV*/
enum PositionalLimitsBehavior{
    POSITIONAL_LIMITS_IGNORE,          /** Positional limits are completely ignored*/
    POSITIONAL_LIMITS_ERROR_MSG_ONLY,  /** Component will go into error state if target is out of bounds*/
    POSITIONAL_LIMITS_ACTIVELY_PREVENT /** Reflexxes will make a smooth transition at the bounds and prevent exceededing them*/
};

/** Motion constraints to define the dynamic behavior of the trajectories generated by this component*/
struct MotionConstraint{

    struct upperLimit{
        double position;     /** Maximum joint or Cartesian position. In rad or m. (only reflexxes TypeIV, has to be > min. position!) */
        double speed;        /** Maximum joint or Cartesian velocity. In rad/s or m/s. (has to be > 0) */
        double acceleration; /** Maximum joint or Cartesian acceleration. In rad/ss or m/ss. (has to be > 0) */
    };
    struct lowerLimit{
        double position;     /** Minimum joint or Cartesian position. In rad or m. (only reflexxes TypeIV, has to be < max. position!) */
    };
    double max_jerk;         /** Maximum joint or Cartesian jerk (derivative of acceleration). In rad/sss or m/sss. (has to be > 0) */
    upperLimit max;
    lowerLimit min;

    MotionConstraint(){
        max.position      = base::unset<float>();
        min.position      = base::unset<float>();
        max.speed         = base::unset<float>();
        max.acceleration  = base::unset<float>();
        max_jerk          = base::unset<float>();
    }

    bool hasMaxPosition() const {return !base::isUnset(max.position);}
    bool hasMinPosition() const {return !base::isUnset(min.position);}
    bool hasMaxVelocity() const {return !base::isUnset(max.speed);}
    bool hasMaxAcceleration() const {return !base::isUnset(max.acceleration);}
    bool hasMaxJerk() const {return !base::isUnset(max_jerk);}

    void validate() const{
#ifdef USING_REFLEXXES_TYPE_IV
        if(!hasMaxPosition())
            throw std::invalid_argument("Motion Constraints: Maximum position is invalid");
        if(!hasMinPosition())
            throw std::invalid_argument("Motion Constraints: Minimum position is invalid");
        if(max.position <= min.position)
            throw std::invalid_argument("Motion Constraints: Max. position has to be > min. position");
#endif
        if(!hasMaxVelocity())
            throw std::invalid_argument("Motion Constraints: Maximum speed is invalid");
        if(!hasMaxAcceleration())
            throw std::invalid_argument("Motion Constraints: Maximum acceleration is invalid");
        if(!hasMaxJerk())
            throw std::invalid_argument("Motion Constraints: Maximum jerk is invalid");
        if(max.speed <= 0)
            throw std::invalid_argument("Motion Constraints: Max. speed has to be > 0");
        if(max.acceleration <= 0)
            throw std::invalid_argument("Motion Constraints: Max. acceleration has to be > 0");
        if(max_jerk <= 0)
            throw std::invalid_argument("Motion Constraints: Max. jerk has to be > 0");
    }
    void applyDefaultIfUnset(const MotionConstraint& default_constraints){
        if(!hasMaxPosition())
            max.position = default_constraints.max.position;
        if(!hasMinPosition())
            min.position = default_constraints.min.position;
        if(!hasMaxVelocity())
            max.speed = default_constraints.max.speed;
        if(!hasMaxAcceleration())
            max.acceleration = default_constraints.max.acceleration;
        if(!hasMaxJerk())
            max_jerk = default_constraints.max_jerk;
    }
};

/** Named vector of MotionConstraints, i.e. motion constraints for all the joints of a robot*/
struct MotionConstraints : base::NamedVector<MotionConstraint>{
};

/** Named vector of Joints command with motion constraints, i.e. constrained commands for all the joints of a robot*/
struct ConstrainedJointsCmd : public base::commands::Joints{
    std::vector<MotionConstraint> motion_constraints;
    void validate() const{

        if(names.size() != size())
            throw std::invalid_argument("ConstrainedJointCmd: Size of name vector should be same as element vector");

        if(!motion_constraints.empty() && motion_constraints.size() != size())
            throw std::invalid_argument("ConstrainedJointCmd: Motion constraints need to have same size as joint command or empty");
    }
};

/** Result values of the Online Trajectory Generation algorithm. See reflexxes/ReflexxesAPI.h for further details*/
enum ReflexxesResultValue{
    RML_WORKING	                            =  0,   /** The Online Trajectory Generation algorithm is working; the final state of motion has not been reached yet.*/
    RML_FINAL_STATE_REACHED                 =  1,   /** The desired final state of motion has been reached.*/
    RML_NO_ERROR                            =  2,   /** This value is returned by the method ReflexxesAPI::SetupOverrideFilter() if executed successfully.*/
    RML_ERROR                               = -1,   /** Initialization value of TypeIVRMLPosition::ReturnValue. Should never be returned.*/
    RML_ERROR_INVALID_INPUT_VALUES          = -100, /** The applied input values are invalid*/
    RML_ERROR_EXECUTION_TIME_CALCULATION    = -101, /** An error occurred during the first step of the algorithm (i.e., during the calculation of the synchronization time).*/
    RML_ERROR_SYNCHRONIZATION               = -102, /** An error occurred during the second step of the algorithm (i.e., during the synchronization of the trajectory*/
    RML_ERROR_NUMBER_OF_DOFS                = -103, /** DOF of input/output parameters and given dof do not match*/
    RML_ERROR_NO_PHASE_SYNCHRONIZATION      = -104, /** Phase synchronization error / snychronization physically or mathematically not possible*/
    RML_ERROR_NULL_POINTER                  = -105, /** Null pointer passed to call RMLPosition/RMLVelocity*/
    RML_ERROR_EXECUTION_TIME_TOO_BIG        = -106, /** RML_MAX_EXECUTION_TIME*/
    RML_ERROR_USER_TIME_OUT_OF_RANGE        = -107, /** User selected RML_MAX_EXECUTION_TIME exceeded */
    RML_ERROR_POSITIONAL_LIMITS	            = -108, /** Only TypeIV: Positional limits are exceeded*/
    RML_ERROR_OVERRIDE_OUT_OF_RANGE         = -109, /** Only TypeIV: This is return value indicates that the specified override value or filter time value is out of range.*/
    RML_NOT_INITIALIZED                     = -200  /** RML has never been called*/
};

/** Debug: Input parameters of the reflexxes OTG algorithm*/
struct ReflexxesInputParameters{
    ReflexxesInputParameters(){}
    ReflexxesInputParameters(const size_t dof){
        selection_vector.resize(dof, base::NaN<u_int8_t>());
        current_position_vector.resize(dof, base::NaN<double>());
        current_velocity_vector.resize(dof, base::NaN<double>());
        current_acceleration_vector.resize(dof, base::NaN<double>());
        max_position_vector.resize(dof, base::NaN<double>());
        min_position_vector.resize(dof, base::NaN<double>());
        max_velocity_vector.resize(dof, base::NaN<double>());
        max_acceleration_vector.resize(dof, base::NaN<double>());
        max_jerk_vector.resize(dof, base::NaN<double>());
        target_position_vector.resize(dof, base::NaN<double>());
        target_velocity_vector.resize(dof, base::NaN<double>());
        min_synchronization_time = base::NaN<double>();
        override_value = base::NaN<double>();
    }
    std::vector<u_int8_t> selection_vector;
    std::vector<double> current_position_vector;
    std::vector<double> current_velocity_vector;
    std::vector<double> current_acceleration_vector;
    std::vector<double> max_position_vector;
    std::vector<double> min_position_vector;
    std::vector<double> max_velocity_vector;     /** Only RMLPosition*/
    std::vector<double> max_acceleration_vector;
    std::vector<double> max_jerk_vector;
    std::vector<double> target_position_vector;  /** Only RMLPosition*/
    std::vector<double> target_velocity_vector;
    double min_synchronization_time;
    double override_value; /** only reflexxes typeIV*/
};

/** Debug: Output parameters of the reflexxes OTG algorithm*/
struct ReflexxesOutputParameters{
    ReflexxesOutputParameters(){}
    ReflexxesOutputParameters(const size_t dof){
        new_position_vector.resize(dof, base::NaN<int>());
        new_velocity_vector.resize(dof, base::NaN<double>());
        new_acceleration_vector.resize(dof, base::NaN<double>());
        execution_times.resize(dof, base::NaN<double>());
        position_values_at_target_velocity.resize(dof, base::NaN<double>());
        dof_with_greatest_execution_time = base::NaN<int>();
        synchronization_time = base::NaN<double>();
        current_override_value = base::NaN<double>();
        a_new_calculation_was_performed = false;
        trajectory_is_phase_synchronized = false;
        override_filter_is_active = false;
        trajectory_exceeds_target_position = false;
    }
    std::vector<double> new_position_vector;
    std::vector<double> new_velocity_vector;
    std::vector<double> new_acceleration_vector;
    std::vector<double> execution_times;
    std::vector<double> position_values_at_target_velocity; /** Only RMLVelocity*/
    bool a_new_calculation_was_performed;
    bool trajectory_is_phase_synchronized;
    bool override_filter_is_active;   /** only reflexxes typeIV*/
    int dof_with_greatest_execution_time;
    double synchronization_time;
    double current_override_value;   /** only reflexxes typeIV*/
    bool trajectory_exceeds_target_position; /** Only RMLPosition, only reflexxes typeIV*/
};

}

#endif

