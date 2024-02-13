import asyncio
import websockets as ws
from .client_events_handler import ClientEventsHandler


class TrainingServer:
    def __init__(self, events_handler: ClientEventsHandler):
        self.__events_handler: ClientEventsHandler = events_handler
        self.__server: ws.WebSocketServer = None


    def close(self):
        if self.__server: self.__server.close()


    async def run(self, host: str, port: int):
        async with ws.serve(ws_handler=self.__handle_connection, host=host, port=port) as self.__server:
            print(f'Server is running on: ws://{host}:{port}')
            await asyncio.Future()  # run forever


    async def __handle_connection(self, websocket: ws.WebSocketServerProtocol, path: str):
        await self.__events_handler.on_connect(websocket, path)
        try:
            async for message in websocket:
                await self.__events_handler.on_message(message, websocket, path)
        finally:
            await self.__events_handler.on_disconnect(websocket, path)
