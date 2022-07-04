#! /usr/bin/python3

import logging
import asyncio
import json

from requests import request

from aiocoap import *

logging.basicConfig(level=logging.INFO)

def get_well_known_core():
    return Message(code=GET, uri='coap://[::1]/.well-known/core')

async def get_resource_lookup(protocol):
    request = Message(code=GET, uri='coap://[::1]/resource-lookup/')
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
        return []
    else:
        resources = response.payload.decode("UTF-8")
        resources = resources.replace("<", "").replace(">", "").split(",")
        return resources

async def get_current_accelerometer_values(protocol, accel_url):
    # request = Message(code=GET, uri=accel_url)
    request = Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/mma8x5x/SENSE_ACCEL')
    response = await protocol.request(request).response
    # b'{"d":[0.010,0.027,1.050],"u":"g"}\x00'
    binary_output = response.payload.decode("UTF-8").replace('\x00', '')
    # b'{"d":[0.010,0.027,1.050],"u":"g"}'
    return json.loads(binary_output)

async def turn_all_leds(protocol, led_urls, value):
    #
    # turns all leds from led_urls to the given value
    # value 1 is on , value 0 is off
    # 
    for led_url in led_urls:
        # print("Put LED: ", led_urls.index(led_url), " to ", value)
        led_url = led_url.replace("(", "%28").replace(")", "%29")
        request = Message(code=Code.PUT, payload=str.encode(str(value)), uri=led_url)
        # await asyncio.sleep(2)
        await protocol.request(request).response

def get_led_blue():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/LED(blue)/ACT_SWITCH')

async def set_led_blue(protocol,url,value):
    # print('Given URL: %s\n'%(url))
    # blue_led = url.replace("(", "%28").replace(")", "%29")
    # print('Set: %s\n'%(blue_led))
    # request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/LED(blue)/ACT_SWITCH')
    request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[2001:67c:254:b0b2:affe:3056:b773:a2de]/saul/LED(blue)/ACT_SWITCH')
    await protocol.request(request).response

def get_led_green():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/LED(green)/ACT_SWITCH')

async def set_led_green(protocol,url,value):
    # green_led = url.replace("(", "%28").replace(")", "%29")
    # print('Set: %s\n'%(green_led))
    # request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/LED(green)/ACT_SWITCH')
    request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[2001:67c:254:b0b2:affe:3056:b773:a2de]/saul/LED(green)/ACT_SWITCH')
    await protocol.request(request).response

def get_led_red():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/LED(red)/ACT_SWITCH')

async def set_led_red(protocol,url,value):
    # red_led = url.replace("(", "%28").replace(")", "%29")
    # print('Set: %s\n'%(red_led))
    # request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/saul/LED(red)/ACT_SWITCH')
    request = Message(code=PUT, payload=str.encode(str(value)), uri='coap://[2001:67c:254:b0b2:affe:3056:b773:a2de]/saul/LED(red)/ACT_SWITCH')
    await protocol.request(request).response

async def main():
    protocol = await Context.create_client_context()
    # PUT Example
    #await asyncio.sleep(2)
    # payload = b"The quick brown fox jumps over the lazy dog.\n" * 30
    # request = Message(code=PUT, payload=payload, uri="coap://localhost/other/block")
    # GET Example
    # request = Message(code=GET, uri='coap://[::1]/.well-known/core')
    # request = get_current_accelerometer_status()
    
    await asyncio.sleep(2)
    
    resources = await get_resource_lookup(protocol)
    
    accel_url = [url for url in resources if "SENSE_ACCEL" in url]
    print('Accelerometer URL: %s\n'%(accel_url))
    
    led_urls = [url for url in resources if "LED" in url]
    print('LED URL: %s\n'%(led_urls))
    
    led_blue = [url for url in led_urls if "blue" in url]
    # print('LED BLUE: %s\n'%(led_blue))
    
    led_green = [url for url in led_urls if "green" in url]
    # print('LED GREEN: %s\n'%(led_green))
    
    led_red = [url for url in led_urls if "red" in url]
    # print('LED RED: %s\n'%(led_red))
    
    while(1):
    # while loop to get periodically current values of accelerometer
        accel_values = await get_current_accelerometer_values(protocol,accel_url)
        # await asyncio.sleep(1)
        # print(accel_values)
        x_axis = accel_values['d'][0]
        print ('X-Axis:%d\n'%(x_axis))
        y_axis = accel_values['d'][1]
        print ('Y-Axis:%d\n'%(y_axis))
        z_axis = accel_values['d'][2]
        print ('Y-Axis:%d\n'%(z_axis))
        if abs(z_axis) <= 1.1 and abs(z_axis) >= 0.9:
            # Back
            print('Turn all LED on!')
            await turn_all_leds(protocol,led_urls, 1)
        #elif accel_values['d'][1] <= 0.0 and accel_values['d'][2] <= -1.1 and accel_values['d'][2] < 0.0:
            # Front
        #    print('Turn all LED off!')
        #    await turn_all_leds(protocol,led_urls, 0)
        elif abs(y_axis) < 1.1 and abs(y_axis) > 0.91 and abs(z_axis) >= 0.0:
            # Potrait Up
            print('Turn green LED on!')
            await set_led_blue(protocol,led_blue,0)
            await asyncio.sleep(1)
            await set_led_red(protocol,led_red,0)
            await asyncio.sleep(1)
            await set_led_green(protocol,led_green,1)
        elif abs(y_axis) <= 0.9 and abs(y_axis) > 0.0 and z_axis >= 0.0:
            print('Turn red LED on!')
            await set_led_blue(protocol,led_blue,0)
            await asyncio.sleep(1)
            await set_led_green(protocol,led_green,0)
            await asyncio.sleep(1)
            await set_led_red(protocol,led_red,1)
            await asyncio.sleep(1)
        else:
            print('Do nothing.')
            await asyncio.sleep(1)
   
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