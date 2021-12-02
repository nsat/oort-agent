**Version:** 1.0

# Data Structures

## TfrsResponse

TFRS reading.

### Members


| Name | Type | Description |
| ---- | ---- | ----------- |
| age | int | Time in seconds since the last reading | 
| utc_time | int | Unix epoch timestamp |
| ecef_pos_x | int | ECEF X Position |
| ecef_pos_y | int | ECEF Y Position |
| ecef_pos_z | int | ECEF Z Position |
| ecef_vel_x | int | ECEF X Velocity |
| ecef_vel_y | int | ECEF Y Velocity |
| ecef_vel_z | int | ECEF Z Velocity |

## AdcsResponse

ADCS Reading

### Members

| Name | Type | Description |
| ---- | ---- | ----------- |
| mode | [string](#acsmode) | Current ACS mode |
| age | int | Time in seconds since the last reading |
| controller | string | Controlling payload |
| hk | [AdcsHk](#adcshk) | Detailed attitude information |

## AdcsCommandRequest

An ADCS command to request a particular attitude.

### Members

| Name | Type | Description |
| ---- | ---- | ----------- |
| command | [string](#adcscommandmode) | The command to send.  Required. |
| aperture | [string](#aperture) | The aperture to base the command on.  Required for `TRACK` and `NADIR` commands.|
| target | [AdcsTarget](#adcstarget) | The target to use for the command.  Required for `TRACK` command. |

## AdcsCommandResponse

The response received from a ADCS command.

### Members

| Name | Type | Description |
| ---- | ---- | ----------- |
| status | string | Status of the request (OK or FAIL) |
| reason | string | Reason why a request failed. |
| mode | [string](#acsmode) | Updated current AcsMode |

## AdcsTarget

A target.

### Members

| Name | Type | Description |
| ---- | ---- | ----------- |
| lat | number | Latitude in degrees |
| lon | number | Longitude in degrees |

## AdcsCommandMode

A command mode to be sent to ADCS.

| Type | Notes |
| ---- | ----- |
| string | Supported commands are IDLE, NADIR, and TRACK |

## AcsMode

A command mode received from ADCS via feedback. 

| Type | Notes |
| ---- | ----- |
| string | The expected values include `NADIRPOINTYAW`, `LATLONTRACK`, and `NOOP`.  Other values are possible, but anything other than these indicate an ADCS control mode not relevant to payload  operations. |

## Aperture

Aperture (imager, antenna, etc) name to use in ADCS pointing requests

| Type | Notes |
| ---- | ----- |
| string | Max length = 24 characters |

## AdcsHk

Detailed ADCS reading.

### Members

| Name | Type | Description |
| ---- | ---- | ----------- |
| control_error_angle_deg | number | Absolute control error angle |
| acs_mode_active | [string](#acsmode) | Active ACS mode |
| euler_angles | [Adcs_euler_t](#adcs_euler_t) | Current attitude in the LVLH (orbit) frame in degrees for Roll, Pitch, Yaw |
| control_error_q  | [Adcs_quat_t](#adcs_quat_t) | Feedback on quaternion error |
| lat_deg | number | Subsatellite latitude, in degrees |
| lon_deg | number | Subsatellite longitude, in degrees |
| q_bo_est | [Adcs_quat_t](#adcs_quat_t) | Estimated spacecraft attitude quaternion in the orbit (lvlh) frame |
| latlontrack_lat | number | Latitude used for ground target tracking |
| latlontrack_lon | number | Longitude used for ground target tracking |
| lease_active | number | Flag if a lease is currently active |
| eclipse_flag | number | Sunlit/eclipse status of spacecraft |
| q_bi_est | [Adcs_quat_t](#adcs_quat_t) | Estimated spacecraft attitude quaternion in the inertial coordinate frame |
| r_eci | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Estimated spacecraft position in ECI frame |
| altitude | number | Estimated altitude of satellite in meters |
| latlontrack_body_vector | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Body vector used to point at ground targets |
| omega_bo_est | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Body rate estimate in orbit frame |
| acs_mode_cmd | [string](#acsmode) | Commanded ACS mode |
| v_eci | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Estimated spacecraft velocity in ECI frame |
| qcf | [Adcs_quat_t](#adcs_quat_t) | Control frame quaternion |
| lease_time_remaining | integer | Time remaining in current ADCS lease |
| unix_timestamp | number | Unix epoch time |
| omega_bi_est | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Body rate estimate in inertial frame |
| control_error_omega | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Feedback on rate error |
| r_ecef | [Adcs_xyz_float_t](#adcs_xyz_float_t) | Estimated spacecraft position in ECEF frame |

## Adcs_xyz_float_t

A 3-element (x, y, z) vector

### Members

| Name | Type |
| ---- | ---- |
| x | number |
| y | number |
| z | number |

## Adcs_quat_t

ADCS quaternion

### Members

| Name | Type |
| ---- | ---- |
| q1 | number |
| q2 | number |
| q3 | number |
| q4 | number |

## Adcs_euler_t

ADCS Euler angles -- roll, pitch, yaw

### Members

| Name | Type |
| ---- | ---- |
| roll | number |
| pitch | number |
| yaw | number |
