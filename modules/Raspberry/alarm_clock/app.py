#!/usr/bin/python3
# -*- coding: utf-8 -*-

from dotenv import load_dotenv
from pathlib import Path
import os, signal, sys, time
from alarm_clock import mqtt, main
from alarm_clock import Ical

def signal_handler(sig, frame):
    mqtt.mqtt_disconnect()
    time.sleep(3)
    sys.exit(0)

load_dotenv(dotenv_path=Path('.env'))

if __name__ == '__main__':
    ical = Ical.Ical(ical_url=os.getenv('EVENTS_ICAL_URL'))
    ical.print_ical()
    os._exit(0)
    signal.signal(signal.SIGINT, signal_handler)
    main.init()
    while True:
        try:
            main.loop()
        except Exception as e:
            print(e)
        time.sleep(1)

