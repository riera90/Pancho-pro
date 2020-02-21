from alarm_clock import Ical
from alarm_clock import mqtt
from alarm_clock import aemet
import os
from datetime import datetime, timedelta


def get_event():
    global event
    return event


def get_transport():
    global transport
    return transport


def reset_alarm_ttl():
    global alarm_ttl
    alarm_ttl = datetime.now() + timedelta(minutes=int(os.getenv('ARLARM_TTL')))


def check_alarm_ttl():
    if alarm_ttl > datetime.now():
        return True
    return False


def reset_snooze_ttl():
    global snooze_ttl
    snooze_ttl = datetime.now() + timedelta(minutes=int(os.getenv('SNOOZE_TTL')))


def check_snooze_ttl():
    if snooze_ttl > datetime.now():
        return True
    else:
        return False


def check_ical_ttl():
    global event_ical
    global transport_ical
    global alarm_ttl
    global snooze_ttl

    if not event_ical.ttl_is_valid():
        while True:
            event_ical = Ical.Ical(str(os.getenv('EVENTS_ICAL_URL')))
            if event_ical.is_valid():
                print('Event ical has been actualized\nnext event is:')
                event_ical.get_next_event().print_event()
                break
            else:
                print('error fetching the ical')

    if not transport_ical.ttl_is_valid():
        while True:
            transport_ical = Ical.Ical(str(os.getenv('TRANSPORT_ICAL_URL')))
            if transport_ical.is_valid():
                print('Transport ical has been actualized')
                event_ical.get_next_event().print_event()
                break
            else:
                print('error fetching the ical')


def check_alarm_status():
    global alarm_ttl
    global snooze_ttl
    global event
    global transport
    global active_alarm
    global first_alarm_of_the_day_has_rang

    event = event_ical.get_next_event()
    minimum_departure_time = event.get_dt_start() - timedelta(minutes=int(os.getenv('TIME_FROM_TRANSPORT_TO_EVENT')))
    transport = transport_ical.get_previous_event(minimum_departure_time)
    delay_from_start_of_alarm_to_transport = timedelta(minutes=int(os.getenv('TIME_TO_WAKE_UP')))
    delay_from_start_of_alarm_to_transport += timedelta(minutes=int(os.getenv('TIME_TO_GET_READY')))
    delay_from_start_of_alarm_to_transport += timedelta(minutes=int(os.getenv('TIME_TO_STATION')))
    start_of_alarm_time = transport.get_dt_start() - delay_from_start_of_alarm_to_transport

    if str(os.getenv('ONE_ALARM_PER_DAY') == 'YES'):
        if first_alarm_of_the_day_has_rang == False and datetime.now().hour == int(os.getenv('ALARM_RESET_HOUR')):
            first_alarm_of_the_day_has_rang = False

        if first_alarm_of_the_day_has_rang == False and start_of_alarm_time < datetime.now():
            print('activating alarm!')
            reset_alarm_ttl()
            active_alarm = True
            first_alarm_of_the_day_has_rang = True

    else:
        if start_of_alarm_time < datetime.now():
            print('activating alarm!')
            reset_alarm_ttl()
            active_alarm = True


def ring():
    print('ring rang rung!!')
    for light in str(os.getenv('MQTT_TOPIC_LIGHTS')).split(','):
        mqtt.mqtt.send_message(light, str(os.getenv('LIGHTS_INTENSITY_RINGING')))
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_SPEAKERS')), 'play '+str(os.getenv('MQTT_SPEAKER_SONG')))


def get_formated_event_for_display():
    global event
    global transport
    formated_event = event.get_summary()
    formated_event += ' ' + str(event.get_dt_start().hour) + ':' + str(event.get_dt_start().minute)
    formated_event += ' ' + str(event.get_location())
    formated_event += '\n'
    formated_event += 'train at ' + str(transport.get_dt_start().hour) + ':' + str(transport.get_dt_start().minute)
    return formated_event


def stop_alarm():
    global active_alarm
    active_alarm = False
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_SPEAKERS')), 'stop')
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_BED_DISPLAY')), get_formated_event_for_display())
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_BED_DISPLAY')), aemet.get_meteo_report())
    print('stoping alarm')


def snooze_alarm():
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_SPEAKERS')), 'stop')
    for light in str(os.getenv('MQTT_TOPIC_LIGHTS')).split(','):
        mqtt.mqtt.send_message(light, str(os.getenv('LIGHTS_INTENSITY_SNOOZED')))
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_BED_DISPLAY')), get_formated_event_for_display())
    mqtt.mqtt.send_message(str(os.getenv('MQTT_TOPIC_BED_DISPLAY')), aemet.get_meteo_report())
    print('snoozing alarm')


def init():
    global event_ical
    global transport_ical
    global alarm_ttl
    global snooze_ttl
    global event
    global transport
    global first_alarm_of_the_day_has_rang

    first_alarm_of_the_day_has_rang = False

    minimum_alarm_ttl = int(os.getenv('TIME_TO_STATION'))\
                        + int(os.getenv('TIME_TO_GET_READY'))\
                        + int(os.getenv('TIME_TO_WAKE_UP'))\
                        + int(os.getenv('TIME_FROM_TRANSPORT_TO_EVENT'))

    if int(os.getenv('ARLARM_TTL')) <= minimum_alarm_ttl:
        print('ALARM_TTL is not big enougth, this can cause silpiples alarm triggers by one single event, setting to a bigger value:', minimum_alarm_ttl + 1)
        os.putenv('ARLARM_TTL', str(minimum_alarm_ttl + 1))


    alarm_ttl = datetime.now()
    snooze_ttl = datetime.now()
    event = None
    transport = None

    while True:
        event_ical = Ical.Ical(str(os.getenv('EVENTS_ICAL_URL')))
        transport_ical = Ical.Ical(str(os.getenv('TRANSPORT_ICAL_URL')))
        if event_ical.is_valid() and transport_ical.is_valid():
            print('ical fetched')
            break
        else:
            print('error fetching the ical, trying again')

    print("\n\n\nevent")
    event_ical.print_ical()
    print("\n\n\ntransport")
    transport_ical.print_ical()
    mqtt.mqtt.mqtt_init()

def loop():
    global event_ical
    global transport_ical
    global active_alarm

    mqtt.mqtt.loop()

    if check_alarm_ttl() == False: # alarm is not active
        check_ical_ttl() # checks and actualizes the ical on ttl
        check_alarm_status() # checks and starts the alarm on the correct time

    elif active_alarm == True: # alarm is active
        if check_snooze_ttl() == False: # alarm should ring
            reset_snooze_ttl()
            ring()

    else: # alarm is in deactivated state but ttl has not been reached, do nothing
        pass



