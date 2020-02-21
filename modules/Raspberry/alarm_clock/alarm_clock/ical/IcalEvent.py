from datetime import datetime, timedelta, timezone
import icalendar
from pprint import pprint

class IcalEvent():
    def __init__(self, event:icalendar.Calendar=None):
        self.__dtStart = None
        self.__dtEnd = None
        self.__description = None
        self.__summary = None
        self.__location = None
        self.__status = None
        self.__rrule = None
        self.__valid = False

        if event.name == "VEVENT":
            self.__valid = True
            self.__dtStart = datetime.strptime(str(event.decoded('dtstart')).replace(':',''), '%Y-%m-%d %H%M%S%z')
            self.__dtEnd = datetime.strptime(str(event.decoded('dtend')).replace(':',''), '%Y-%m-%d %H%M%S%z')
            self.__description = event.get('description')
            self.__summary = event.get('summary')
            self.__location = event.get('location')
            self.__status = event.get('status')
            self.__rrule = event.get('rrule')

        if self.__dtEnd is None:
            self.__dtEnd = self.__dtStart


    def get_dt_start(self):
        return self.__dtStart


    def get_dt_end(self):
        return self.__dtEnd


    def get_description(self):
        return self.__description


    def get_summary(self):
        return self.__summary


    def get_location(self):
        return self.__location


    def get_status(self):
        return self.__status


    def get_rrule(self):
        return self.__rrule


    def event_is_active(self):
        now = datetime.now(timezone.utc)
        return now > self.__dtStart and now < self.__dtEnd


    def print_event(self):
        print('dtStart ---->', self.__dtStart)
        print('dtEnd ------>', self.__dtEnd)
        print('description >', self.__description)
        print('summary ---->', self.__summary)
        print('location --->', self.__location)
        print('status ----->', self.__status)
        print('rrule ------>', self.__rrule)
        print('active ----->', self.event_is_active())


    def is_valid(self):
        return self.__valid and self.__dtStart is not None