#!/usr/bin/python3
# -*- coding: utf-8 -*-

from alarm_clock import Ical, aemet
from dotenv import load_dotenv
from pathlib import Path
import os

load_dotenv(dotenv_path=Path('.env'))

if __name__ == '__main__':
    ical = Ical.Ical(ical_url=os.getenv('ICAL_URL'))
    ical.print_ical()
    print(aemet.get_meteo_report())

