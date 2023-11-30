import serial
import mysql.connector
from db_connector import con
import datetime

ser = serial.Serial('/dev/ttyACM0', 9600)
ser.reset_input_buffer()

def saveToSensorDb(type, value):
    cursor = con.cursor()
    now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    cursor.execute("INSERT INTO `sensor_staging` (`Type`, `Value`, `datetime`) VALUES (%s, %s, %s)", (type, value, now))
    con.commit()
    cursor.close()
    print("Saved to sensor DB: " + type + " " + value)

def saveToLogDb(type, message):
    cursor = con.cursor()
    now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    cursor.execute("INSERT INTO `log_staging` (`type`, `message`, `datetime`) VALUES (%s, %s, %s)", (logStringToInt(type), message, now))
    con.commit()
    cursor.close()
    print("Saved to log DB: " + type + " " + message)

def logStringToInt(string):
    # Fix this
    return 0

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').rstrip()
        print(line)
        if line.startswith("RPI: "):
            split = line.split(" ")
            type = split[1]
            value = split[2]
            saveToSensorDb(type, value)
        if line.startswith("LOG: "):
            split = line.split(" ")
            type = split[2]
            message = " ".join(split[2:])
            saveToLogDb(type, message)
