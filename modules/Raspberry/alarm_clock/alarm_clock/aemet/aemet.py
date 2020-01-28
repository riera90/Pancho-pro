import os
import requests
from datetime import datetime

def get_meteo_report():
    '''
    Gets the meteorologic report from AEMET (spanish weather report agency)
    and returns a string to be displayed at the nightstand display
    (if available).
    This function output is in spanish, becouse it uses the AEMET api
    (witch data is in spanish)
    :return: string
    '''

    if os.getenv('AEMET_MUNICIPE_CODE') is None or os.getenv('AEMET_API_KEY') is None:
        raise Exception('"AEMET_MUNICIPE_CODE" or "AEMET_API_KEY" not found in ".env" file')

    aemet_timeperiods = { # period end time (hour) : index in aemet api
        6: 3,
        12: 4,
        18: 5,
        24: 6,
    }

    aemet_timeperiod = 3
    for time_period in aemet_timeperiods.keys():
        if time_period > datetime.now().hour:
            aemet_timeperiod = aemet_timeperiods[time_period]
            break


    aemet_url = "https://opendata.aemet.es/opendata/api/prediccion/especifica/municipio/diaria/" + str(os.getenv('AEMET_MUNICIPE_CODE')) + "/?api_key=" + str(os.getenv('AEMET_API_KEY'))
    try:
        response = requests.get(aemet_url)
        aemet_url = response.json()["datos"]
    except Exception as e:
        print(e)
        return 'Weather error: api fetch'

    try:
        response = requests.get(aemet_url)
        response = response.json()
    except Exception as e:
        print(e)
        return 'Weather error: data fetch'

    try:
        weather = response[0]['prediccion']['dia'][0]['estadoCielo'][aemet_timeperiod]['descripcion']

        if weather == '': # in the ae + emet api, a blank field in the sky description is sunny
            weather = "soleado"

        rain = str(response[0]['prediccion']['dia'][0]['probPrecipitacion'][aemet_timeperiod]['value'])

        temp_min = str(response[0]['prediccion']['dia'][0]['temperatura']['minima'])
        temp_max = str(response[0]['prediccion']['dia'][0]['temperatura']['maxima'])

        return weather + ", " + rain + "%\ntemp: " + temp_min + "/" + temp_max + " C"
    except Exception as e:
        print(e)
        return 'Weather error: json parsing'
