
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
            res = str(response.payload)
            
            #resources = re.findall(r'<(.*?)>', res)
            allhosts = re.findall(r"[^[]*\[([^]]*)\]", res)#.groups()[0]
        
            hosts = list(dict.fromkeys(allhosts))
            
            print(hosts)
            
            if(len(hosts) == 0):
                print("no resources found")


            
            for host in hosts:
                #print(res)
                #time.sleep(1)
                #request = Message(code=GET, uri=res)
                #print(host)
                #print('coap://[' + host + ']/sense/temp')
                try:

                    request = Message(code=GET, uri='coap://[' + host + ']/sense/temp')
                except:
                    print("failed with ")
                    print(host)
                    
                    try:
                        response = await protocol.request(request).response
                        
                    except Exception as e:
                        print('Failed to fetch resource:')
                        print(e)
                    else:
                        print("host: " +  host)
                        response
                        print(json.loads(response.payload.decode("utf-8").replace("\x00", "")))
            
                    
            
            #print("resources found")
            #print(resources)

        #time.sleep(3)



if __name__ == "__main__":
    asyncio.run(main())
