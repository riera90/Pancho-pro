import requests
import copy
from datetime import datetime, timedelta
from . import IcalEvent

def test():
    print('test')


class Ical():
    def __init__(self, ical_url=''):
        self.__ical_url = ical_url
        self.__events = self.__get_parsed_ical()
        self.__fetched_at = datetime.now()
        self.__ttl = timedelta(minutes=30)


    def get_event(self, index):
        if len(self.__events) < index:
            raise Exception('Index out of bounds of the calendar')

        return copy.deepcopy(self.__events[index])


    def get_next_event(self):
        if len(self.__events) == 0:
            raise Exception('No events in the calendar')

        next_event = self.__events[0]
        for event in self.__events:
            if next_event.get_dt_start() > event.get_dt_start():
                next_event = event

        return next_event


    def get_ttl_left(self):
        return self.__fetched_at + self.__ttl - datetime.now()


    def ttl_is_valid(self):
        return self.get_ttl_left() > timedelta(0)


    def __get_ical(self):
        r = requests.get(self.__ical_url)
        return r.text


    def __get_parsed_ical(self):
        rawIcal = self.__get_ical()
        events = []
        for event in rawIcal.split('BEGIN:VEVENT')[1::]:
            events.append(event.split('END:VEVENT')[0])

        icalEvents = []
        for event in events:
            icalEvent = IcalEvent.IcalEvent(event=event)
            if icalEvent.get_dt_start() is not None and (datetime.now() - icalEvent.get_dt_start()) < timedelta(hours=1) and 'CONFIRMED' in icalEvent.get_status():
                icalEvents.append(icalEvent)

        return icalEvents


    def print_ical(self):
        for event in self.__events:
            print('-----------------------------------')
            event.print_event()
