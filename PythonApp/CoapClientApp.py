#! /usr/bin/python3

import logging
import asyncio
import json
import re

from requests import request

from aiocoap import *

logging.basicConfig(level=logging.INFO)

async def get_endpoints(protocol):
    request = Message(code=GET, uri='coap://localhost/endpoint-lookup/')
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
        return []
    else:
        endpoints = str(response.payload)
        endpoints = re.findall(r"[^[]*\[([^]]*)\]", endpoints)
        return endpoints

async def get_resources(protocol):
    request = Message(code=GET, uri='coap://localhost/resource-lookup/')
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
        return []
    else:
        resource_list = []
        resources = response.payload.decode("UTF-8")
        resources = resources.replace("<", "").replace(">", "").split(",")
        for resource in resources:
            resource_list.append(resource.split(']')[1])
        
        return list(dict.fromkeys(resource_list))

async def get_buttons2_status(protocol, host, buttons2_url):
    request = Message(code=GET, uri='coap://[' + host + ']'+ buttons2_url)
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
        return []
    else:
        buttons2_state = json.loads(response.payload.decode("utf-8").replace("\x00", ""))
        return buttons2_state["d"]

async def get_current_accelerometer_values(protocol, host, accel_url):
    # print("URL of Accel: %s"%(accel_url))
    request = Message(code=GET, uri='coap://[' + host + ']'+ accel_url)
    response = await protocol.request(request).response
    # b'{"d":[0.010,0.027,1.050],"u":"g"}\x00'
    binary_output = response.payload.decode("UTF-8").replace('\x00', '')
    # b'{"d":[0.010,0.027,1.050],"u":"g"}'
    return json.loads(binary_output)

async def set_led_blue(protocol,value,host,blue_led_url):
    request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[' + host + ']'+ blue_led_url)
    await protocol.request(request).response

async def set_led_green(protocol,value,host,green_led_url):
    request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[' + host + ']'+ green_led_url)
    await protocol.request(request).response

async def set_led_red(protocol,value,host,red_led_url):
    request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[' + host + ']'+ red_led_url)
    await protocol.request(request).response

async def main():
    protocol = await Context.create_client_context()
    await asyncio.sleep(1)
    hosts = await get_endpoints(protocol)
    print("Hosts: %s\n"%(hosts))
    await asyncio.sleep(1)
    resources = await get_resources(protocol)
    # print("Resources: %s\n"%(resources))
    await asyncio.sleep(1)
    accel_url = [url for url in resources if "SENSE_ACCEL" in url]
    # print('Accelerometer Endpoint: %s\n'%(accel_url))
    buttons2_url = [url for url in resources if "(SW0)" in url]
    # print('Button S2 Endpoint: %s\n'%(buttons2_url))
    led_urls = [url for url in resources if "LED" in url]
    # print('LED URL: %s\n'%(led_urls))
    led_blue = [url for url in led_urls if "blue" in url]
    # print('LED BLUE: %s\n'%(led_blue))
    led_green = [url for url in led_urls if "green" in url]
    # print('LED GREEN: %s\n'%(led_green))
    led_red = [url for url in led_urls if "red" in url]
    # print('LED RED: %s\n'%(led_red))

    sensor = -1
    
    # Kontrolllogik, um zu unterscheide, wer der Sensor (Accelerometer Messung) ist
    
    while(sensor == -1):
        for host in hosts:
            if await get_buttons2_status(protocol,host,buttons2_url[0]):
                sensor = host
                print("Sensor wurde ausgewählt\n")
                await set_led_green(protocol, 0, host, led_green[0])
                await asyncio.sleep(2)
                await set_led_red(protocol, 0, host, led_red[0])
                await asyncio.sleep(2)
                await set_led_blue(protocol,0,host,led_blue[0])
                await asyncio.sleep(2)
            else:
                
                await set_led_green(protocol, 0, host, led_green[0])
                await asyncio.sleep(2)
                await set_led_red(protocol, 0, host, led_red[0])
                await asyncio.sleep(2)
                await set_led_blue(protocol,1,host,led_blue[0])
                await asyncio.sleep(2)
                print("Sensor wurde nicht ausgewählt\n")
                print("Bitte drücken Sie den S2 Button, um den Sensor auszuwaehlen!\n")
                await asyncio.sleep(2)
    print('Sensor: %s'%(sensor))
    
    # Alle anderen Hosts sind dann Aktoren (LEDs)
    
    for host in hosts:
        if sensor != host:
            await set_led_blue(protocol,0,host,led_blue[0])
    
    hosts.remove(sensor) # Remove Sensor from Hosts
    # Kontrollogik vom Sensor
    
    while(sensor != -1):
        for host in hosts:
            accel_values = await get_current_accelerometer_values(protocol,sensor,accel_url[0])
            # print(accel_values)
            await asyncio.sleep(2)
            x_axis = accel_values['d'][0]
            # print ('X-Axis:%f\n'%(x_axis))
            y_axis = accel_values['d'][1]
            print ('Y-Axis:%f\n'%(y_axis))
            z_axis = accel_values['d'][2]
            print ('Z-Axis:%f\n'%(z_axis))
            if -1.1 <= z_axis and z_axis <= -0.95:
                # Back
                print('Turn all LED on!')
                # await turn_all_leds(protocol,led_urls, 1)
                await set_led_blue(protocol, 1, host, led_blue[0])
                await asyncio.sleep(2)
                await set_led_green(protocol, 1, host, led_green[0])
                await asyncio.sleep(2)
                await set_led_red(protocol, 1, host, led_red[0])
                await asyncio.sleep(2)
            elif -1.1 <= y_axis and y_axis <= -0.95 and abs(z_axis) >= 0.0:
                # Potrait Up
                print('Turn green LED on!')
                await set_led_blue(protocol, 0, host, led_blue[0])
                await asyncio.sleep(2)
                await set_led_green(protocol, 1, host, led_green[0])
                await asyncio.sleep(2)
                await set_led_red(protocol, 0, host, led_red[0])
            elif y_axis > -0.95 and abs(z_axis) >= 0.0:
                print('Turn red LED on!')
                await set_led_blue(protocol, 0, host, led_blue[0])
                await asyncio.sleep(2)
                await set_led_green(protocol, 0, host, led_green[0])
                await asyncio.sleep(2)
                await set_led_red(protocol, 1, host, led_red[0])
                await asyncio.sleep(2)
            else:
                '''
                print('Turn all LED off!')
                await set_led_blue(protocol, 0, host, led_blue[0])
                await set_led_green(protocol, 0, host, led_green[0])
                await set_led_red(protocol, 0, host, led_red[0])
                '''
   
# main-Function
if __name__ == "__main__":
    # asyncio.run(main())
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)

    task = loop.create_task(main())
    try:
         loop.run_forever()
    except KeyboardInterrupt:
         print()