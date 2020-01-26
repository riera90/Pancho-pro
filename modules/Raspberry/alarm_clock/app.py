#!/usr/bin/python3
# -*- coding: utf-8 -*-

from alarm_clock import Ical
from dotenv import load_dotenv
from pathlib import Path
import os

if __name__ == '__main__':
    load_dotenv(dotenv_path=Path('.env'))
    ical = Ical.Ical(ical_url=os.getenv('ICAL_URL'))
    ical.print_ical()

