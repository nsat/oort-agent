# unit_shim.py - uavcan shim for unit tests
# feeds send constant data
# service reflects inputs

import time
import uavcan
import logging
import signal
import math
import sys

logging.basicConfig(level=logging.INFO)

log = logging.getLogger(__name__)

if len(sys.argv) > 1:
    rate = float(sys.argv[1])
else:
    rate = 1

uavcan.load_dsdl('ussp')
# payload shim would be node 50, but use 51 to not conflict.  
# agent under test needs to be aware of this difference for services.
node = uavcan.make_node('can0', node_id=51)

TfrsFeedPublisher = uavcan.thirdparty.ussp.tfrs.ReceiverNavigationState
AdcsFeedPublisher = uavcan.thirdparty.ussp.payload.PayloadAdcsFeed
AdcsCommandService = uavcan.thirdparty.ussp.payload.PayloadAdcsCommand
AcsMode = uavcan.thirdparty.ussp.payload.AcsMode
acs_constants = {c.name: c.value for c in AcsMode.constants}
TargetT = uavcan.thirdparty.ussp.payload.TargetT
QuatT = uavcan.thirdparty.ussp.payload.QuatT
XyzFloatT = uavcan.thirdparty.ussp.payload.XyzFloatT

def adcs_feed(node):
    msg = AdcsFeedPublisher()
    while True:
        msg.acs_mode_active.mode = msg.acs_mode_active._constants['SUNSPIN']

        msg.control_error_q.q1 = 1
        msg.control_error_q.q2 = 2
        msg.control_error_q.q3 = 3
        msg.control_error_q.q4 = 4

        msg.q_bo_est.q1 = 5
        msg.q_bo_est.q2 = 6
        msg.q_bo_est.q3 = 7
        msg.q_bo_est.q4 = 8

        msg.latlontrack_lat = 99
        msg.latlontrack_lon = 88

        msg.lease_active = True
        msg.eclipse_flag = True

        msg.q_bi_est.q1 = 9
        msg.q_bi_est.q2 = 10
        msg.q_bi_est.q3 = 11
        msg.q_bi_est.q4 = 12

        msg.r_eci.x = -1
        msg.r_eci.y = -2
        msg.r_eci.z = -3

        msg.latlontrack_body_vector.x = -4
        msg.latlontrack_body_vector.y = -5
        msg.latlontrack_body_vector.z = -6

        msg.omega_bo_est.x = -7
        msg.omega_bo_est.y = -8
        msg.omega_bo_est.z = -9

        msg.acs_mode_cmd.mode = msg.acs_mode_cmd._constants["NADIRPOINTYAW"]

        msg.v_eci.x = -11
        msg.v_eci.y = -12
        msg.v_eci.z = -13

        msg.qcf.q1 = 21
        msg.qcf.q2 = 22
        msg.qcf.q3 = 23
        msg.qcf.q4 = 24
        
        msg.lease_time_remaining = 3600

        msg.unix_timestamp = 1234.5

        msg.omega_bi_est.x = -31
        msg.omega_bi_est.y = -32
        msg.omega_bi_est.z = -33

        msg.control_error_omega.x = -41
        msg.control_error_omega.y = -42
        msg.control_error_omega.z = -43

        log.info("broadcast ADCS")
        node.broadcast(msg)
        yield


def tfrs_feed(node):
    msg = TfrsFeedPublisher()
    while True:
        # uint8 iod
        msg.iod = 1
        # uint16 receiver_week
        msg.receiver_week = 2
        # uint32 receiver_time_of_week
        msg.receiver_time_of_week = 3
        # uint16 measurement_period
        msg.measurement_period = 4
        # bool data_good
        msg.data_good = True
        # bool dual_receiver
        msg.dual_receiver = True
        # uint3 navigation_state
        msg.navigation_state = 2
        # uint16 gps_week
        msg.gps_week = 10
        # float64 gps_time_of_week
        msg.gps_time_of_week = 1.5
        # float64 ecef_pos_x
        msg.ecef_pos_x = 2.5
        # float64 ecef_pos_y
        msg.ecef_pos_y = 3.5
        # float64 ecef_pos_z
        msg.ecef_pos_z = 4.5
        # float32 ecef_vel_x
        msg.ecef_vel_x = 5.5
        # float32 ecef_vel_y
        msg.ecef_vel_y = 6.5
        # float32 ecef_vel_z
        msg.ecef_vel_z = 7.5
        # float64 clock_bias
        msg.clock_bias = 8.5
        # float32 clock_drift
        msg.clock_drift = 1.25
        # float32 gdop
        msg.gdop = 2.25
        # float32 pdop
        msg.pdop = 3.25
        # float32 hdop
        msg.hdop = 4.25
        # float32 vdop
        msg.vdop = 5.25
        # float32 tdop
        msg.tdop = 6.25
        # int8 gps_leap_seconds
        msg.gps_leap_seconds = 127
        # uint32 utc_time
        msg.utc_time = 1234
        # int8 temperature_C
        msg.temperature_C = 37
        log.info("broadcast TFRS")
        node.broadcast(msg)
        yield

def AdcsCommandHandler(e):
    req = e.request
    rsp = AdcsCommandService.Response()

    log.info("adcs command: %s", uavcan.to_yaml(e))
    log.info("Command request is %s", req)

    rsp.status = rsp._constants['STATUS_FAIL']
    rsp.reason = "Aperture {}".format(req.aperture)

    if e.request.adcs_command == req._constants['ADCS_COMMAND_IDLE']:
        rsp.mode = AcsMode(mode=acs_constants['SUNSPIN'])
    else:
        rsp.mode = AcsMode(mode=acs_constants['NOOP'])

    if len(req.target) > 0:
        log.info("request has target")
        target = TargetT()
        target.lat = req.target[0].lat + 1
        target.lon = req.target[0].lon + req.angle[0]
        rsp.target.append(target)

    if len(req.quat) > 0:
        log.info("request has quat")
        quat = QuatT()
        quat.q1 = req.quat[0].q1 + 1
        quat.q2 = req.quat[0].q2 + 2
        quat.q3 = req.quat[0].q3 + 3
        quat.q4 = req.quat[0].q4 + 4
        rsp.quat.append(quat)

    if len(req.vector) > 0:
        log.info("request has vector")
        vector = XyzFloatT()
        vector.x = req.vector[0].x - 1
        vector.y = req.vector[0].y - 2
        vector.z = req.vector[0].z - 3
        rsp.vector.append(vector)

    log.info("adcs command response %s", rsp)
    return rsp


adcs_feeder = adcs_feed(node)
tfrs_feeder = tfrs_feed(node)

node.periodic(rate, lambda: next(adcs_feeder))
node.periodic(rate, lambda: next(tfrs_feeder))
node.add_handler(AdcsCommandService, AdcsCommandHandler)

def die(sig, info):
    print("I am terminated!")
    exit(1)

signal.signal(signal.SIGINT, die)
signal.signal(signal.SIGTERM, die)

node.spin()
