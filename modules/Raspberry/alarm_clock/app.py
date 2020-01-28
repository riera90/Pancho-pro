#!/usr/bin/python3
# -*- coding: utf-8 -*-

from dotenv import load_dotenv
from pathlib import Path
import os, signal, sys, time
from alarm_clock import mqtt, main


def signal_handler(sig, frame):
    mqtt.mqtt_disconnect()
    time.sleep(3)
    sys.exit(0)

load_dotenv(dotenv_path=Path('.env'))

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    main.init()
    while True:
        main.loop()

        try:
            pass
        except Exception as e:
            print(e)
        time.sleep(1)

