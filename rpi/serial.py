import serial
import mysql.connector
from db_connector import con

# To be tested

ser = serial.Serial('/dev/ttyACM0', 9600) # Check if the port is correct
ser.reset_input_buffer()

def saveToSensorDb(type, value):
    cursor = con.cursor()
    cursor.execute("INSERT INTO `sensor_staging` (`Type`, `Value`) VALUES (?, ?, ?)", (type, value))
    con.commit()
    cursor.close()
    print("Saved to sensor DB: " + type + " " + value)

def saveToLogDb(type, message):
    cursor = con.cursor()
    cursor.execute("INSERT INTO `log_staging` (`type`, `message`) VALUES (?, ?, ?)", (logStringToInt(type), message))
    con.commit()
    cursor.close()
    print("Saved to log DB: " + type + " " + message)

def logStringToInt(string):
    map = {
        "S": 4,
        "D": 3,
        "*": 2,
        "X": 1
    }
    return map[string]

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
            # Format: LOG: [type] message
            # LOG:, [, type, ], message
            split = line.split(" ")
            type = split[2]
            message = " ".join(split[3:])
            saveToLogDb(type, message)
