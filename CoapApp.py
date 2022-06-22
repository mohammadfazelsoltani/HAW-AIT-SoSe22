#! /usr/bin/python3

# Imports
import asyncio
import logging

from aiocoap import *
# Functions

logging.basicConfig(level=logging.INFO)

async def getWellKnownCore():
    pass

async def main():
    protocol = await Context.create_client_context()

    request = Message(code=GET, uri='coap://[::1]/.well-known/core')
    
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response.code, response.payload))
    pass

# main-Function
if __name__ == "__main__":
    asyncio.run(main())