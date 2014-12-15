import os
import netaddr

class StitchDevice:

    def __init__(self, ip_addr
            , prefix_length
            , device_name
            , device_model
            , stitch_network
            , device_id):
        self.ip = netaddr.IPNetwork(ip_addr + '/' + str(prefix_length))
        self.device_name = device_name 
        self.device_model = device_model
        self.network = stitch_network
        self.device_id = device_id


