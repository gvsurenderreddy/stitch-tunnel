import os
import requests
HTTP_CONTENT_TYPE_JSON = 'application/json'

def main():
    STITCH_WEB_SERVICE = 'https://djangovm:8191/stitch_login/'
    server_uuid = raw_input("Enter server UUID for registration:")
    print "Registering server with UUID:%s" % (server_uuid)
    s = requests.Session()
    s.verify = "./stitch_root.crt"
    params = {}
    params['server_uuid'] = server_uuid
    resp = s.post(STITCH_WEB_SERVICE+'server_add/', data= params)
    if (resp.headers['content-type'] == HTTP_CONTENT_TYPE_JSON):
        json_resp = resp.json()
        if ('result' in json_resp.keys()):
            if (json_resp['result'] == 'OK'):
                print 'Registered the server successfully.. '
                print 'Getting stitch parameters .....'
                stitch_resp = s.get(STITCH_WEB_SERVICE+'device_detail_api/?format=json')
                if (stitch_resp.headers['content-type'] == HTTP_CONTENT_TYPE_JSON):
                    json_stitch_resp = stitch_resp.json()
                    print 'IP:%s/%d' % (json_stitch_resp['ip_addr'], json_stitch_resp['ip_prefix_length'])
                    print 'Device name:%s' % (json_stitch_resp['device_name'])
                    print 'Device model:%s' % (json_stitch_resp['device_model'])
                    print 'Network:%d' % (json_stitch_resp['network'])
                else:
                    stitch_resp.text
        else:
            print 'Error!! Got a JSON response but without the result set.'
    else:
        print "Error!! Did not get a JSON response..."
        print resp.text




if __name__ == "__main__":
    print "*******Welcome to stitch network ***********"
    main()
