import time
import uavcan
import predict
import logging
import signal
import math
import sys

logging.basicConfig(level=logging.INFO)

log = logging.getLogger(__name__)

if __name__ == "__main__":

    if len(sys.argv) > 1:
        rate = float(sys.argv[1])
    else:
        rate = 1

    # FM146
    tle = '''LEMUR-2-NOOBNOOB        
1 47538U 21006DY  21305.61575955  .00004814  00000-0  27164-3 0  9990
2 47538  97.4809   4.5190 0010822 358.0760   2.0428 15.13575263 42793'''

    # FM102
    tle = '''0 LEMUR 2 VICTOR-ANDREW
1 44087U 19018K   21316.40927568  .00006438  00000-0  23916-3 0  9991
2 44087  97.3488  16.2685 0012746 155.5993 204.5852 15.27821745145603'''

    # some random geostationary satellite
    # tle='''NILESAT 201             
# 1 36830U 10037A   21305.79265616 -.00000049  00000-0  00000+0 0  9997
# 2 36830   0.0548 144.3680 0003727  59.9489 115.3637  1.00271893 41369'''

    uavcan.load_dsdl('ussp')
    # payload shim would be node 50, but use 51 to not conflict.  
    # agent under test needs to be aware of this difference for services.
    node = uavcan.make_node('can0', node_id=51)

    TfrsFeedPublisher = uavcan.thirdparty.ussp.tfrs.ReceiverNavigationState
    AdcsFeedPublisher = uavcan.thirdparty.ussp.payload.PayloadAdcsFeed
    AdcsCommandService = uavcan.thirdparty.ussp.payload.PayloadAdcsCommand
    AcsMode = uavcan.thirdparty.ussp.payload.AcsMode

    # this is complete fantasy
    acs_constants = {c.name: c.value for c in AcsMode.constants}
    pointing_status = {
        "mode": AcsMode(mode=acs_constants['NOOP']),
        "error": 0,
    }

    def adcs_feed(node):
        msg = AdcsFeedPublisher()
        while True:
            thetime = time.time()
            result = predict.observe(tle, (0,0,0), thetime)
            msg.unix_timestamp = thetime
            # predict gives results in mm, we want meters.
            msg.r_eci.x = result["eci_x"]*1000
            msg.r_eci.y = result["eci_y"]*1000
            msg.r_eci.z = result["eci_z"]*1000
            msg.v_eci.x = result["eci_vx"]*1000
            msg.v_eci.y = result["eci_vy"]*1000
            msg.v_eci.z = result["eci_vz"]*1000
            msg.acs_mode_active = pointing_status['mode']
            # whee
            if pointing_status['mode'].mode != acs_constants['NOOP']:
                log.info("adjusting pointing error to %s", pointing_status['error'])
                pointing_status['error'] -= min(pointing_status['error'] / 2.0, 3)
                pointing_status['error'] = abs(pointing_status['error'])
            msg.control_error_q.q1 = 0
            msg.control_error_q.q2 = 0
            msg.control_error_q.q3 = 0
            msg.control_error_q.q4 = math.cos(pointing_status['error'] / 2 / 180 * math.pi)
            node.broadcast(msg)
            yield

    # exact same position info as adcs 
    # unfortunately its completely wrong.
    def tfrs_feed(node):
        msg = TfrsFeedPublisher()
        while True:
            thetime = time.time()
            result = predict.observe(tle, (0,0,0), thetime)
            msg.utc_time = int(thetime)
            # predict gives results in mm, we want meters.
            msg.ecef_pos_x = result["eci_x"]*1000
            msg.ecef_pos_y = result["eci_y"]*1000
            msg.ecef_pos_z = result["eci_z"]*1000
            msg.ecef_vel_x = result["eci_vx"]*1000
            msg.ecef_vel_y = result["eci_vy"]*1000
            msg.ecef_vel_z = result["eci_vz"]*1000
            # print("tfrs broadcast")
            node.broadcast(msg)
            yield

    def AdcsCommandHandler(e):
        log.info("adcs command: %s", uavcan.to_yaml(e))
        # error on any command other than IDLE
        req = AdcsCommandService.Request()
        rsp = AdcsCommandService.Response()
        if e.request.adcs_command == req._constants['ADCS_COMMAND_IDLE']:
            rsp.status = rsp._constants['STATUS_OK']
            pointing_status["mode"] = AcsMode(mode=acs_constants['NOOP'])
        elif e.request.adcs_command == req._constants['ADCS_COMMAND_NADIR']:
            if e.request.aperture == "GOPRO":
                rsp.status = rsp._constants['STATUS_OK']
                pointing_status["mode"] = AcsMode(mode=acs_constants['NADIRPOINTYAW'])
                rsp.mode = pointing_status["mode"]
                pointing_status["error"] = 25.3
            else:
                rsp.status = rsp._constants['STATUS_FAIL']
                rsp.reason = "NADIR pointing not supported for " + e.request.aperture
        elif e.request.adcs_command == req._constants['ADCS_COMMAND_TRACK']:
            rsp.status = rsp._constants['STATUS_FAIL']
            rsp.reason = "Target tracking not supported"
        else:
            rsp.status = rsp._constants['STATUS_FAIL']
            rsp.reason = "Can't happen"

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
