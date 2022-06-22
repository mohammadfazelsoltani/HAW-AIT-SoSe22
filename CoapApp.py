#! /usr/bin/python3

# Imports
import asyncio
import logging

from aiocoap import *
# Functions

logging.basicConfig(level=logging.INFO)

def getWellKnownCore():
    return Message(code=GET, uri='coap://[::1]/.well-known/core')

def getResourceLookup():
    return Message(code=GET, uri='coap://[::1]/resource-lookup/')

async def main():
    protocol = await Context.create_client_context()

    request = getResourceLookup()
    
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response.code, response.payload))
        resources = response.payload.split(",")
        for resource in resources:
            print(resource)
    pass

# main-Function
if __name__ == "__main__":
    asyncio.run(main())