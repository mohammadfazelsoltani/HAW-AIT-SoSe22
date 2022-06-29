#! /usr/bin/python3

import logging
import asyncio

from aiocoap import *

logging.basicConfig(level=logging.INFO)

def get_current_accelerometer_status():
    return Message(code=GET, uri='coap://[::1]/saul/mma8x5x/SENSE_ACCEL')

def get_led_blue():
    return Message(code=GET, uri='coap://[::1]/saul/LED(blue)/ACT_SWITCH')

def set_led_blue(payload):
    return Message(code=PUT, payload=payload, uri="coap://[::1]/saul/LED(blue)/ACT_SWITCH")

def get_led_green():
    return Message(code=GET, uri='coap://[::1]/saul/LED(green)/ACT_SWITCH')

def set_led_green(payload):
    return Message(code=PUT, payload=payload, uri="coap://[::1]/saul/LED(green)/ACT_SWITCH")

def get_led_red():
    return Message(code=GET, uri='coap://[::1]/saul/LED(red)/ACT_SWITCH')

def set_led_red():
    return Message(code=PUT, payload=payload, uri="coap://[::1]/saul/LED(red)/ACT_SWITCH")

async def main():
    protocol = await Context.create_client_context()
    # PUT Example
    #await asyncio.sleep(2)
    # payload = b"The quick brown fox jumps over the lazy dog.\n" * 30
    # request = Message(code=PUT, payload=payload, uri="coap://localhost/other/block")
    # GET Example
    request = Message(code=GET, uri='coap://[::1]/.well-known/core')
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        # print('Result: %s\n%r'%(response.code, response.payload))
        resources = response.payload.decode("UTF-8").split(",")
        for resource in resources:
            print(resource)
    ###
    await asyncio.sleep(2)
    protocol2 = await Context.create_client_context()
    request2 = set_led_blue(1)
    try:
        response2 = await protocol2.request(request2).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response2.code, response2.payload))
        #resources2 = response2.payload.decode("UTF-8").split(",")
        #for resource2 in resources2:
        #    print(resource2)
    ###
    await asyncio.sleep(2)
    protocol3 = await Context.create_client_context()
    request3 = set_led_blue(0)
    try:
        response3 = await protocol3.request(request3).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response3.code, response3.payload))

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