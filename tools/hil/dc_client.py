"""Devconsole client for the PRC152 dev firmware (serial or WiFi)."""
import json
import time

MATRIX_KEYS = {"0": 0, "1": 1, "2": 2, "3": 3, "4": 4, "5": 5, "6": 6,
               "7": 7, "8": 8, "9": 9, "CLR": 10, "ENT": 11, "UP": 12,
               "DOWN": 13, "LEFT": 14, "RIGHT": 15}


class SerialTransport:
    def __init__(self, port, baud=115200, timeout=3.0):
        import serial
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.timeout = timeout

    def cmd(self, line):
        self.ser.reset_input_buffer()
        self.ser.write(b">" + line.encode() + b"\n")
        deadline = time.monotonic() + self.timeout
        while time.monotonic() < deadline:
            raw = self.ser.readline()
            if raw.startswith(b"##"):
                return json.loads(raw[2:].decode(errors="replace"))
        raise TimeoutError(f"no ## response to {line!r}")


class HttpTransport:
    def __init__(self, host, timeout=5.0):
        import requests
        self.requests = requests
        self.url = f"http://{host}/dev"
        self.timeout = timeout

    def cmd(self, line):
        r = self.requests.post(self.url, data={"c": line}, timeout=self.timeout)
        r.raise_for_status()
        return r.json()


class DevConsole:
    def __init__(self, transport):
        self.t = transport

    def cmd(self, line):
        resp = self.t.cmd(line)
        assert resp.get("ok") == 1, f"{line!r} failed: {resp}"
        return resp

    def ping(self):
        return self.cmd("ping")

    def get(self, name):
        return self.cmd(f"get {name}")["val"]

    def set(self, name, value):
        return self.cmd(f"set {name} {value}")

    def dump(self, what):
        return self.cmd(f"dump {what}")

    def key(self, code):
        code = MATRIX_KEYS.get(code, code)
        r = self.cmd(f"key {code}")
        time.sleep(0.15)  # let the super-loop consume the event
        return r

    def enc(self, event):
        r = self.cmd(f"enc {event}")
        time.sleep(0.15)
        return r

    def screen(self):
        return bytes.fromhex(self.cmd("dump screen")["hex"])


def render_screen(data, pages=8, cols=128):
    """Render page-major GRAM bytes as ASCII art (rows = pages*8 pixels)."""
    lines = []
    for page in range(pages):
        for bit in range(8):
            row = data[page * cols:(page + 1) * cols]
            lines.append("".join("#" if b & (1 << bit) else "." for b in row))
    return "\n".join(lines)
