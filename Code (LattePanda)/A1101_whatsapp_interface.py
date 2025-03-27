# AI-driven LoRaWAN Fertilizer Pollution Detector w/ WhatsApp
#
# LattePanda 3 Delta
#
# Collect data to train a NN to detect fertilizer contamination.   
# Then, get informed of the results via WhatsApp over LoRaWAN.
#
# By Kutluhan Aktar

from twilio.rest import Client
import requests
import json
from time import sleep

# Define the Twilio account settings and the client object.
twilio_account_sid = '<_SID_>'
twilio_auth_token = '<_TOKEN_>'
twilio_client = Client(twilio_account_sid, twilio_auth_token)

# Define the API ID and Access API key variables to connect to the SenseCAP Portal.
API_ID = '<_ID_>'
API_key = '<_KEY_>'

# Define the required device information of SenseCAP A1101 LoRaWAN Vision AI Sensor.
device_eui = "2CF7F1C04340004A"
measurement_id = "4175"
channel_index = "1"

# Define the host of the SenseCAP HTTP API. 
host = "https://sensecap.seeed.cc/openapi/"

# Define the URL endpoint to obtain model detection results via the SenseCAP HTTP API.
get_latest_result = "view_latest_telemetry_data?device_eui={}&measurement_id={}&channel_index={}".format(device_eui, measurement_id, channel_index)

def send_WhatsApp_message(_from, _to, _message):
    # Send the given message via WhatsApp to inform the user of the model detection results.
    twilio_message = twilio_client.messages.create(
      from_ = 'whatsapp:'+_from,
      body = _message,
      media_url = 'https://media-cdn.seeedstudio.com/media/catalog/product/cache/bb49d3ec4ee05b6f018e93f896b8a25d/1/0/101990962-a1101-first-new-10.17.jpg',
      to = 'whatsapp:'+_to
    )
    print("\nWhatsApp Message Sent: "+twilio_message.sid)

def transfer_latest_result():
    # Obtain the latest model detection result via the SenseCAP HTTP API and notify the user of the received information through WhatsApp.
    url = host + get_latest_result
    # Make an HTTP GET request to the SenseCAP Portal by utilizing the provided HTTP authentication credentials (username and password).
    res = requests.get(url, auth = (API_ID, API_key))
    # Decode the received JSON object.
    res = json.loads(res.text)
    detection_digit = res["data"][0]["points"][0]["measurement_value"]
    date = res["data"][0]["points"][0]["time"]
    # Convert the obtained result digits to the detected class and the precision score.
    detected_class = "Nothing!"
    precision = 0
    if(detection_digit > 0 and detection_digit < 1):
        detected_class = "Enriched"
        precision = detection_digit
    if(detection_digit > 1 and detection_digit < 2):
        detected_class = "Toxic"
        precision = detection_digit-1
    if(detection_digit > 2):
        detected_class = "Unsafe"
        precision = detection_digit-2
    # Create a WhatsApp message from the retrieved information.
    message = "📌 Latest Model Detection Result\n\n🕒 {}\n🌱 Class: {}\n💯 Precision: {}".format(date, detected_class, round(precision, 2))
    print(message)
    # Transmit the generated message to the user via WhatsApp.
    send_WhatsApp_message('+_____________', '+_____________', message)
    
    
while True:
    # Notify the user of the latest model detection result every 10 minutes.
    transfer_latest_result()
    sleep(60*10)
