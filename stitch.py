import os
import requests
import cookielib
from stitch_device import StitchDevice
import subprocess

HTTP_CONTENT_TYPE_JSON = 'application/json'
COOKIEJAR_PERSISTENT_FILE = '.cookies'

def main():
    STITCH_WEB_SERVICE = 'https://djangovm:8191/stitch_login/'
    #get the key from user.
    server_uuid = raw_input("Enter server UUID for registration:")
    print "Registering server with UUID:%s" % (server_uuid)
    cookiejar = cookielib.LWPCookieJar(filename=COOKIEJAR_PERSISTENT_FILE)
    try:
        cookiejar.load(filename=COOKIEJAR_PERSISTENT_FILE)
    except IOError as io:
        print "No cookie JAR to read from, using an empty cookie JAR"
        pass
    s = requests.Session()
    s.cookies = cookiejar
    #check if there is a CSRF token in the cookiejar
    for cookie in cookiejar:
        if (cookie.name == "csrftoken"):
            print "Found the CSRF token:%s" % (cookie.value)
            csrftoken = cookie.value
    s.verify = "./stitch_root.crt"
    params = {}
    params['server_uuid'] = server_uuid
    params['csrfmiddlewaretoken'] = csrftoken
    headers = {}
    headers['referer'] = STITCH_WEB_SERVICE+'server_add/'
    s.headers = headers
    resp = s.post(STITCH_WEB_SERVICE+'server_add/', data= params)
    #store the cookies in persistent storage
    s.cookies.save(filename=COOKIEJAR_PERSISTENT_FILE )
    if (resp.headers['content-type'] == HTTP_CONTENT_TYPE_JSON):
        json_resp = resp.json()
        if ('result' in json_resp.keys()):
            if (json_resp['result'] == 'OK'):
                print 'Registered the server successfully.. '
                print 'Getting stitch parameters .....'
                stitch_resp = s.get(STITCH_WEB_SERVICE+'device_detail_api/?format=json')
                s.cookies.save(filename=COOKIEJAR_PERSISTENT_FILE )
                if (stitch_resp.headers['content-type'] == HTTP_CONTENT_TYPE_JSON):
                    json_stitch_resp = stitch_resp.json()
                    print 'IP:%s/%d' % (json_stitch_resp['ip_addr'], json_stitch_resp['ip_prefix_length'])
                    print 'Device name:%s' % (json_stitch_resp['device_name'])
                    print 'Device model:%s' % (json_stitch_resp['device_model'])
                    print 'Network:%d' % (json_stitch_resp['network'])
                    device = StitchDevice(json_stitch_resp['ip_addr'] 
                    , json_stitch_resp['ip_prefix_length']
                    , json_stitch_resp['device_name']
                    , json_stitch_resp['device_model']
                    , json_stitch_resp['network']
                    , json_stitch_resp['device_id'])

                    try:
                        ret = subprocess.check_output(['./tun/stitch_tun','-i',str(device.ip.ip),'-p',str(device.ip.prefixlen),'-s','avi-mac'])
                    except subprocess.CalledProcessError as e:
                        print "Could not create the tunnel e"

                else:
                    stitch_resp.text
        else:
            print 'Error!! Got a JSON response but without the result set.'
            print json_resp
    else:
        print "Error!! Did not get a JSON response..."
        print resp.text




if __name__ == "__main__":
    print "*******Welcome to stitch network ***********"
    main()
