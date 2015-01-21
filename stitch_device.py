import os
import netaddr

class StitchDevice:

    def __init__(self, ip_addr
            , prefix_length
            , device_name
            , device_model
            , stitch_switch
            , device_id
            ):
        self.ip = netaddr.IPNetwork(ip_addr + '/' + str(prefix_length))
        self.device_name = device_name 
        self.device_model = device_model
        self.device_id = device_id
        self.stitch_switch = stitch_switch


