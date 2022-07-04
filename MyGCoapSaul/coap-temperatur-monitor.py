
from multiprocessing import connection
import sys
import json
import time
import re

import python_weather
import asyncio
import requests

from aiocoap import *

#temperature in hamburg
currentTemp = 0

#master mode
masterNode = -1
masterTemp = 0

#ressource directory host
rdhost = ""

#connection stuff
connection
protocol



async def getweather():
    global currentTemp
    print("getweather")

    # declare the client. format defaults to metric system (celcius, km/h, etc.)
    client = python_weather.Client(format=python_weather.METRIC)

    # fetch a weather forecast from a city
    weather = await client.find("Hamburg, Germany")

    # returns the current day's forecast temperature (int)
    #print(weather.current.temperature)
    currentTemp = float(weather.current.temperature)
    print("current temp is:")
    print(currentTemp)

    # close the wrapper once done
    await client.close()

    await asyncio.sleep(10)
    await getweather()


async def getAndUpdateNodes():
    global currentTemp, masterNode, masterTemp, connection, protocol
    print("getAndUpdateNodes")
    nodeTemp = 0

    #debug
    protocol = await Context.create_client_context()
    request = Message(code=GET, uri='coap://' + rdhost + '/resource-lookup/')

    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        res = str(response.payload)


    allhosts = re.findall(r"[^[]*\[([^]]*)\]", res)

    hosts = list(dict.fromkeys(allhosts))
    
    print(hosts)

    #get all node values

    '''
    </saul/Button(CS0)/SENSE_BTN>
    </saul/Button(SW0)/SENSE_BTN>
    </saul/LED(blue)/ACT_SWITCH>
    </saul/LED(green)/ACT_SWITCH>
    </saul/LED(red)/ACT_SWITCH>
    </saul/hdc1000/SENSE_HUM>
    </saul/hdc1000/SENSE_TEMP>
    </saul/mag3110/SENSE_MAG>
    </saul/mma8x5x/SENSE_ACCEL>
    </saul/mpl3115a2/SENSE_PRESS>
    </saul/mpl3115a2/SENSE_TEMP>
    </saul/tcs37727/SENSE_COLOR>
    </saul/tmp00x/SENSE_OBJTEMP>
    </cli/stats>;ct=0;rt="count";obs,</riot/board>
    '''



    # 
    masterNode = -1
    
    #loop through nodes and check for master button
    for host in hosts:
        
        request = Message(code=GET, uri='coap://[' + host + ']/saul/Button(SW0)/SENSE_BTN')
        try:
            response = await protocol.request(request).response
        except Exception as e:
            print('Failed to fetch resource:')
            print(e)
        else:
            res = str(response.payload)
            buttonState =  json.loads(response.payload.decode("utf-8").replace("\x00", ""))
            print(buttonState["d"])
            if(buttonState["d"]):
                masterNode = host
            
    masterTemp = 0
    if(masterNode != -1):
        request = Message(code=GET, uri='coap://[' + masterNode + ']/saul/hdc1000/SENSE_TEMP')
        try:
            response = await protocol.request(request).response
        except Exception as e:
            print('Failed to fetch resource:')
            print(e)
        else:
            res = str(response.payload)
            masterTemp =  json.loads(response.payload.decode("utf-8").replace("\x00", ""))["d"]


    print("Master")
    print(masterNode)
    print("Master Temp")
    print(masterTemp)

    #loop through nodes and set led
    for setHost in hosts:

        request = Message(code=GET, uri='coap://[' + setHost + ']/saul/hdc1000/SENSE_TEMP')
        try:
            response = await protocol.request(request).response
        except Exception as e:
            print('Failed to fetch resource:')
            print(e)
        else:
            res = str(response.payload)
            nodeTemp =  json.loads(response.payload.decode("utf-8").replace("\x00", ""))["d"]

        if(masterNode != -1):
            if(setHost != masterNode):
                if(nodeTemp >= masterTemp):
                    await setNodeLed(setHost, [0,1,1])
                else:
                    await setNodeLed(setHost, [1,0,0])
            else:
                await setNodeLed(setHost, [1,0,1])
            
        else:
            if(nodeTemp >= currentTemp):
                #green
                await setNodeLed(setHost, [0,1,0])
            else:
                #red
                await setNodeLed(setHost, [1,0,0])

    await asyncio.sleep(2)
    await getAndUpdateNodes()


async def setNodeLed(host, leds):
    global connection
    request = Message(code=PUT, uri='coap://[' + host + ']/saul/LED(red)/ACT_SWITCH', payload=str.encode(str(leds[0])))
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    request = Message(code=PUT, uri='coap://[' + host + ']/saul/LED(green)/ACT_SWITCH', payload=str.encode(str(leds[1])))
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    request = Message(code=PUT, uri='coap://[' + host + ']/saul/LED(blue)/ACT_SWITCH', payload=str.encode(str(leds[2])))
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    return


def main():
    global rdhost

    print("Weather finder started")
    if(len(sys.argv) != 2):
        print("app needs host address")
        exit()
    else:
        rdhost = str(sys.argv[1])


    loop = asyncio.get_event_loop()
    loop.create_task(getweather())
    loop.create_task(getAndUpdateNodes())
    loop.run_forever()


if __name__ == "__main__":
    main()


