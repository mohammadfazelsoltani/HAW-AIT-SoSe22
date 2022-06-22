#! /usr/bin/python3

# Imports
import asyncio
import logging

from aiocoap import *
# Functions

logging.basicConfig(level=logging.INFO)


def getWellKnownCore():
    return Message(code=GET, uri='coap://[::1]/.well-known/core')

def getResourceLookupReq():
    return Message(code=GET, uri='coap://[::1]/resource-lookup/')

def get_blue_led_req():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/blue')

def put_blue_led_on_req():
    return Message(code=PUT, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/blue --payload=1')

def put_blue_led_off_req():
    return Message(code=PUT, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/blue --payload=0')

def get_green_led_req():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/green')

def put_green_led_on_req():
    return Message(code=PUT, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/green --payload=1')

def put_green_led_off_req():
    return Message(code=PUT, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/green --payload=0')

def get_red_led_req():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/red')

def put_red_led_on_req():
    return Message(code=PUT, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/red --payload=1')

def put_red_led_off_req():
    return Message(code=PUT, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/led/red --payload=0')

def get_temperature_req():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/sense/temp')

def get_accel_req():
    return Message(code=GET, uri='coap://[2001:67c:254:b0b2:affe:3060:6eff:7da6]/sense/accel')

async def main():
    protocol = await Context.create_client_context()

    request = get_accel_req()
    
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response.code, response.payload))
        resources = response.payload.decode("UTF-8").split(",")
        for resource in resources:
            print(resource)
    pass

# main-Function
if __name__ == "__main__":
    asyncio.run(main())