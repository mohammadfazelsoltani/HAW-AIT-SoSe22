
import asyncio

import sys
import json
import time
import re

from aiocoap import *


async def main():
    print("coap ressource directory finder started")
    if(len(sys.argv) != 2):
        print("please enter host address")
        exit()
    else:
        host = str(sys.argv[1])

    protocol = await Context.create_client_context()

    


    while(1):

        request = Message(code=GET, uri='coap://' + host + '/resource-lookup/')

        try:
            response = await protocol.request(request).response
        except Exception as e:
            print('Failed to fetch resource:')
            print(e)
        else:
            #print('Result: %s\n%r'%(response.code, response.payload))
            res = str(response.payload)
            #res = '<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/0>;rt="ACT_SWITCH";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/1>;rt="ACT_SWITCH";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/10>;rt="SENSE_PRESS";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/11>;rt="SENSE_COLOR";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/12>;rt="SENSE_OBJTEMP";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/2>;rt="ACT_SWITCH";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/3>;rt="SENSE_BTN";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/4>;rt="SENSE_BTN";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/5>;rt="SENSE_TEMP";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/6>;rt="SENSE_HUM";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/7>;rt="SENSE_MAG";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/8>;rt="SENSE_ACCEL";if="saul",<coap://[2001:67c:254:b0b2:affe:1896:134b:16e6]/saul/9>;rt="SENSE_TEMP";if="saul"'
            #print(res)
            resources = re.findall(r'<(.*?)>', res)
        
            if(len(resources) == 0):
                print("no resources found")

            for res in resources:
                #print(res)
                time.sleep(1)
                request = Message(code=GET, uri=res)

                try:
                    response = await protocol.request(request).response
                    print(res)
                    print(response.payload)
                except Exception as e:
                    print('Failed to fetch resource:')
                    print(e)
            
            #print("resources found")
            #print(resources)

        time.sleep(3)



if __name__ == "__main__":
    asyncio.run(main())
