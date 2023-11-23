from flask import Flask

# Possible methods:
# Turn on pump for x millis
# Toggle lamps

app = Flask(__name__)

@app.route('/')
def home():
    return "Hello, World!"

if __name__ == '__main__':
    app.run(debug=True)