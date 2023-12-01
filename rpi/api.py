from flask import Flask
from arduino_serial import sendLamp

app = Flask(__name__)

@app.route('/updateLamps')
def home():
    sendLamp()
    return "Lamps updated!", 200

if __name__ == '__main__':
    app.run(debug=True)