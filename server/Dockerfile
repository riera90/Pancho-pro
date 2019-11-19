FROM eclipse-mosquitto:latest

COPY ./mosquitto.conf /mosquitto/config/mosquitto.conf

COPY ./passwd /mosquitto/passwd

RUN mosquitto_passwd -U /mosquitto/passwd

VOLUME "/mosquitto/log"

CMD mosquitto -c /mosquitto/config/mosquitto.conf

