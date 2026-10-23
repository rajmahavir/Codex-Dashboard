"""Enter the ESP32-S3 ROM loader using Arduino's native CDC 1200-baud reset."""
import glob
import os
import termios
import time


def enter_loader(port):
    before = set(glob.glob('/dev/cu.usbmodem*'))
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    try:
        cfg = termios.tcgetattr(fd)
        cfg[4] = cfg[5] = termios.B1200
        termios.tcsetattr(fd, termios.TCSANOW, cfg)
    finally:
        os.close(fd)
    end = time.monotonic() + 12
    while time.monotonic() < end:
        new = set(glob.glob('/dev/cu.usbmodem*')) - before
        if len(new) == 1:
            return new.pop()
        time.sleep(.2)
    raise RuntimeError('Loader port did not appear. Use BOOT + RESET and supply its port without --enter-loader.')
