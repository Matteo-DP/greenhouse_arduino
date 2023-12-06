import serial
import mysql.connector
from db_connector import con # Import connection, not included due to security reasons
import datetime
import time
from flask import Flask
import threading

app = Flask(__name__)

keepRunning = True
ser = None

def open_serial_connection():
    global ser
    while keepRunning:
        try:
            ser = serial.Serial('/dev/ttyACM0', 9600)
            ser.reset_input_buffer()
            print("Serial connection established.")
            return True
        except serial.SerialException as e:
            print(f"Failed to open serial port: {e}")
            time.sleep(5)  # Wait for 5 seconds before retrying

def close_serial_connection():
    global ser
    if ser is not None and ser.is_open:
        ser.close()
        print("Serial connection closed.")

@app.route('/')
def home():
    try:
        sendLamp()
        return "Lamps updated!", 200
    except Exception as e:
        return "Failed to send lamps: " + str(e), 500

def sendLamp():
    try:
        cursor = con.cursor()
        cursor.execute("SELECT * FROM `config`")
        result = cursor.fetchall()
        cursor.close()
        print(result)
        if len(result) > 0:
            for row in result:
                if(str(row[0]).startswith("USE_")):
                    message = f'<LAMP: {row[0]} {row[1]}>'
                    print('Writing message: ' + message)
                    ser.write(message.encode('utf-8'))
                    time.sleep(2) # 2 second delay to prevent overflowing serial buffer
        else:
            print("This should never happen, table config not found")
    except Exception as e:
        print("Failed to send lamps: " + str(e))

def saveToSensorDb(type, value):
    try:
        cursor = con.cursor()
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        cursor.execute("INSERT INTO `sensor_staging` (`Type`, `Value`, `datetime`) VALUES (%s, %s, %s)", (type, value, now))
        con.commit()
        cursor.close()
        print("Saved to sensor DB: " + type + " " + value)
    except Exception as e:
        print("Failed to save to sensor DB: " + str(e))

def saveToLogDb(type, message):
    try:
        cursor = con.cursor()
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        cursor.execute("INSERT INTO `log_staging` (`type`, `message`, `datetime`) VALUES (%s, %s, %s)", (logStringToInt(type), message, now))
        con.commit()
        cursor.close()
        print("Saved to log DB: " + type + " " + message)
    except Exception as e:
        print("Failed to save to log DB: " + str(e))

def logStringToInt(string):
    # Fix this
    return 0

def runSerialLoop():
    global ser
    if open_serial_connection():
        while keepRunning:
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
                if(line == "ASK: LAMPS"):
                    sendLamp()

# Start the serial loop in a separate thread
serial_thread = threading.Thread(target=runSerialLoop)
serial_thread.start()

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')