#! /usr/bin/python3

import logging
import asyncio

from aiocoap import *

logging.basicConfig(level=logging.INFO)

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
        print('Result: %s\n%r'%(response.code, response.payload))
        resources = response.payload.decode("UTF-8").split(",")
        for resource in resources:
            print(resource)

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