# ml_server.py
import asyncio, struct, msgpack
import torch, torch.nn as nn, torch.optim as optim

# ----- Model definition (simple example) -----
class TradingModel(nn.Module):
    def __init__(self, input_size=8, output_size=3):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(input_size, 64),
            nn.ReLU(),
            nn.Linear(64, output_size)
        )
    def forward(self, x):
        return self.net(x)

model = TradingModel()
optimizer = optim.Adam(model.parameters(), lr=1e-3)
loss_fn = nn.MSELoss()

# ----- Helpers -----
def pack(obj: dict) -> bytes:
    payload = msgpack.packb(obj, use_bin_type=True)
    return struct.pack("<I", len(payload)) + payload  # 4-byte little-endian length prefix

async def read_exact(reader: asyncio.StreamReader, n: int) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = await reader.read(n - len(buf))
        if not chunk:
            raise ConnectionError("Client disconnected")
        buf += chunk
    return buf

async def recv_msg(reader: asyncio.StreamReader) -> dict:
    hdr = await read_exact(reader, 4)
    (length,) = struct.unpack("<I", hdr)
    data = await read_exact(reader, length)
    return msgpack.unpackb(data, raw=False)

def to_tensor(batch_floats):
    # batch_floats: list[list[float]] with shape [B, input_size]
    return torch.tensor(batch_floats, dtype=torch.float32)

# ----- Command handlers -----
def handle_predict(payload):
    # payload: {"inputs": [[...], [...], ...]}
    x = to_tensor(payload["inputs"])
    with torch.no_grad():
        y = model(x).cpu().numpy().tolist()
    return {"ok": True, "outputs": y}

def handle_train_step(payload):
    # payload: {"inputs": [[...]], "targets": [[...]]}
    x = to_tensor(payload["inputs"])
    t = to_tensor(payload["targets"])
    optimizer.zero_grad()
    y = model(x)
    loss = loss_fn(y, t)
    loss.backward()
    optimizer.step()
    return {"ok": True, "loss": float(loss.item())}

def handle_save(payload):
    path = payload.get("path", "model.pt")
    torch.save(model.state_dict(), path)
    return {"ok": True}

def handle_load(payload):
    path = payload.get("path", "model.pt")
    sd = torch.load(path, map_location="cpu")
    model.load_state_dict(sd)
    return {"ok": True}

HANDLERS = {
    "predict": handle_predict,
    "train_step": handle_train_step,
    "save_model": handle_save,
    "load_model": handle_load,
    "ping": lambda payload: {"ok": True, "msg": "pong"},
    # "shutdown" will be handled in the loop
}

async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    try:
        while True:
            msg = await recv_msg(reader)
            cmd = msg.get("cmd")
            payload = msg.get("payload", {})

            if cmd == "shutdown":
                writer.write(pack({"ok": True}))
                await writer.drain()
                writer.close()
                await writer.wait_closed()
                # Stop server
                asyncio.get_event_loop().call_soon(asyncio.get_event_loop().stop)
                return

            handler = HANDLERS.get(cmd)
            if handler is None:
                resp = {"ok": False, "error": f"unknown cmd: {cmd}"}
            else:
                try:
                    resp = handler(payload)
                except Exception as e:
                    resp = {"ok": False, "error": repr(e)}
            writer.write(pack(resp))
            await writer.drain()
    except ConnectionError:
        pass
    finally:
        try:
            writer.close()
            await writer.wait_closed()
        except Exception:
            pass

async def main(port=5555):
    server = await asyncio.start_server(handle_client, host="127.0.0.1", port=port)
    addrs = ", ".join(str(sock.getsockname()) for sock in server.sockets)
    print(f"[ml_server] listening on {addrs}", flush=True)
    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    # pip install torch msgpack
    asyncio.run(main(5555))
