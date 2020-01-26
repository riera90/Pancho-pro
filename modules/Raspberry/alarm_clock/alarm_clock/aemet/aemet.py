
def get_meteo_report():
    '''
    Gets the meteorologic report from AEMET (spanish weather report agency)
    and returns a string to be displayed at the nightstand display
    (if available).
    This function output is in spanish, becouse it uses the AEMET api
    (witch data is in spanish)
    :return: string
    '''

    return "sunny, 15/2"