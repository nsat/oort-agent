import time
import uavcan
import predict
import logging

logging.basicConfig()

if __name__ == "__main__":

    # FM146
    tle = '''LEMUR-2-NOOBNOOB        
1 47538U 21006DY  21305.61575955  .00004814  00000-0  27164-3 0  9990
2 47538  97.4809   4.5190 0010822 358.0760   2.0428 15.13575263 42793'''

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
    # AcsMode = uavcan.thirdparty.ussp.payload.AcsMode

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
            # print("adcs broadcast")
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
        print("adcs command")
        # error on any command other than IDLE
        req = AdcsCommandService.Request()
        rsp = AdcsCommandService.Response()
        if e.request.adcs_command != req._constants['ADCS_COMMAND_IDLE']:
            rsp.status = rsp._constants['STATUS_FAIL']
            rsp.reason = "Command not supported"
        else:
            rsp.status = rsp._constants['STATUS_OK']
            rsp.mode.mode = rsp.mode._constants['NOOP']

        print("adcs command response")
        print(rsp)
        return rsp


    adcs_feeder = adcs_feed(node)
    tfrs_feeder = tfrs_feed(node)

    node.periodic(1, lambda: next(adcs_feeder))
    node.periodic(1, lambda: next(tfrs_feeder))
    node.add_handler(AdcsCommandService, AdcsCommandHandler)

    node.spin()
