from datetime import datetime, timedelta


class IcalEvent():
    def __init__(self, event=''):
        self.__dtStart = None
        self.__dtEnd = None
        self.__description = ''
        self.__summary = ''
        self.__location = ''
        self.__status = ''

        for line in event.split('\n'):
            if 'DTSTART' in line:
                self.__dtStart = self.__get_datetime_from_register(line)
            elif 'DTEND' in line:
                self.__dtEnd = self.__get_datetime_from_register(line)
            elif 'DESCRIPTION' in line:
                self.__description = line.split(':', 1)[1]
            elif 'SUMMARY' in line:
                self.__summary = line.split(':', 1)[1]
            elif 'LOCATION' in line:
                self.__location = line.split(':', 1)[1]
            elif 'STATUS' in line:
                self.__status = line.split(':', 1)[1]

        if self.__dtEnd is None:
            self.__dtEnd = self.__dtStart


    def get_dt_start(self):
        return self.__dtStart


    def get_dt_end(self):
        return self.__dtEnd


    def get_description(self):
        return self.__description.replace('\r', '')


    def get_summary(self):
        return self.__summary.replace('\r', '')


    def get_location(self):
        return self.__location.replace('\r', '')


    def get_status(self):
        return self.__status.replace('\r', '')


    def event_is_active(self):
        return datetime.now() > self.__dtStart and datetime.now() < self.__dtEnd



    def print_event(self):
        print('dtStart ---->', self.__dtStart)
        print('dtEnd ------>', self.__dtEnd)
        print('description >', self.__description)
        print('summary ---->', self.__summary)
        print('location --->', self.__location)
        print('status ----->', self.__status)
        print('active ----->', self.event_is_active())


    def __get_datetime_from_register(self, register):
        dtString = register.split(':', 1)[1]
        dt = None

        if 'TZID=Europe/Madrid' in register:
            dt = datetime.strptime(dtString, '%Y%m%dT%H%M%S\r')
            dt = dt - timedelta(hours=-1) # adust for your timezone

        elif 'TZID=x/x' in register:
            dt = datetime.strptime(dtString, '%Y%m%dT%H%M%S\r')
            dt = dt - timedelta(hours=5)

        elif 'VALUE=DATE' in register:
            pass

        else:
            dt = datetime.strptime(dtString, '%Y%m%dT%H%M%SZ\r')
            dt = dt - timedelta(hours=0)


        return dt