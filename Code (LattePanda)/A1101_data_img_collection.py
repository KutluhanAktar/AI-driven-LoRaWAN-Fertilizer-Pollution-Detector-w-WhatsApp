# AI-driven LoRaWAN Fertilizer Pollution Detector w/ WhatsApp
#
# LattePanda 3 Delta
#
# Collect data to train a NN to detect fertilizer contamination.   
# Then, get informed of the results via WhatsApp over LoRaWAN.
#
# By Kutluhan Aktar

import cv2
import serial
from threading import Thread
from time import sleep
import os
from PIL import Image
from io import BytesIO
import usb1
import numpy as np
import datetime

class A1101_data_img_collection():
    def __init__(self):
        # Define the required settings to obtain information from SenseCAP A1101 LoRaWAN Vision AI Sensor.
        self.WEBUSB_JPEG = (0x2B2D2B2D)
        self.WEBUSB_TEXT = 0x0F100E12
        self.VendorId = 0x2886
        self.ProductId = [0x8060, 0x8061]
        self.img_buff = bytearray()
        self.buff_size = 0
        self.time_out = 1000
        # Get the connected USB device context.       
        self.context = usb1.USBContext()
        # Initialize serial communication with Arduino Nano to obtain the given commands.
        self.arduino_nano = serial.Serial("COM7", 115200, timeout=2000)
        # Initialize and test SenseCAP A1101 connection.
        self.get_rlease_device(False)
        self.disconnect()  

    def read_data(self):
        # If SenseCAP A1101 is accessible:
        with self.handle.claimInterface(2):
            # Get the data endpoints.
            self.handle.setInterfaceAltSetting(2, 0)
            self.handle.controlRead(0x01 << 5, request=0x22, value=0x01, index=2, length=2048, timeout=self.time_out)
            # Get all transferred data objects.
            transfer_list = []
            for _ in range(1):
                transfer = self.handle.getTransfer()
                transfer.setBulk(usb1.ENDPOINT_IN | 2, 2048, callback=self.processReceivedData, timeout=self.time_out)
                transfer.submit()
                transfer_list.append(transfer)
            # Check for any submitted data object in the received data packet.
            while any(x.isSubmitted() for x in transfer_list):
                self.context.handleEvents()

    def processReceivedData(self, transfer):
        # If SenseCAP A1101 generates a data packet successfully, process the received information.
        if transfer.getStatus() != usb1.TRANSFER_COMPLETED:
            # transfer.close()
            return
        # Extract the captured image from the processed data packet.
        data = transfer.getBuffer()[:transfer.getActualLength()]
        self.convert_and_show_img(data)
        # Resubmit the data packet after processing to avoid errors.
        transfer.submit()

    def convert_and_show_img(self, data: bytearray):
        # Convert the received data packet.
        if (len(data) == 8) and (int.from_bytes(bytes(data[:4]), 'big') == self.WEBUSB_JPEG):
            self.buff_size = int.from_bytes(bytes(data[4:]), 'big')
            self.img_buff = bytearray()
        elif (len(data) == 8) and (int.from_bytes(bytes(data[:4]), 'big') == self.WEBUSB_TEXT):
            self.buff_size = int.from_bytes(bytes(data[4:]), 'big')
            self.img_buff = bytearray()
        else:
            self.img_buff = self.img_buff + data
        # If the received data packet is converted to an image buffer successfully, display the generated image on the screen.
        if self.buff_size == len(self.img_buff):
            try:
                img = Image.open(BytesIO(self.img_buff))
                img = np.array(img)
                cv2.imshow('A1101_data_img_collection', cv2.cvtColor(img,cv2.COLOR_RGB2BGR))
                # Stop the video stream if requested.
                if cv2.waitKey(1) != -1:
                    cv2.destroyAllWindows()
                    print("\nCamera Feed Stopped!")
                # Store the latest frame captured by SenseCAP A1101.
                self.latest_frame = img
            except:
                self.img_buff = bytearray()
                return

    def connect(self):
        # Connect to SenseCAP A1101.
        self.handle = self.get_rlease_device(True)
        if self.handle is None:
            print('\nSenseCAP A1101 not detected!')
            return False
        with self.handle.claimInterface(2):
            self.handle.setInterfaceAltSetting(2, 0)
            self.handle.controlRead(0x01 << 5, request=0x22, value=0x01, index=2, length=2048, timeout=self.time_out)
            print('\nSenseCAP A1101 detected successfully!')
        return True

    def disconnect(self):
        # Reset the SenseCAP A1101 connection.
        try:
            print('Resetting the device connection... ')
            with usb1.USBContext() as context:
                handle = context.getByVendorIDAndProductID(self.VendorId, self.d_ProductId, skip_on_error=False).open()
                handle.controlRead(0x01 << 5, request=0x22, value=0x00, index=2, length=2048, timeout=self.time_out)
                handle.close()
                print('Device connection has been reset successfully!')
            return True
        except:
            return False

    def get_rlease_device(self, get=True):
        # Establish the SenseCAP A1101 connection.
        print('*' * 50)
        print('Establishing connection...')
        # Get the device information. 
        for device in self.context.getDeviceIterator(skip_on_error=True):
            product_id = device.getProductID()
            vendor_id = device.getVendorID()
            device_addr = device.getDeviceAddress()
            bus = '->'.join(str(x) for x in ['Bus %03i' % (device.getBusNumber(),)] + device.getPortNumberList())
            # Check if there is a connected device.
            if(vendor_id == self.VendorId) and (product_id in self.ProductId):
                self.d_ProductId = product_id
                print('\r' + f'\033[4;31mID {vendor_id:04x}:{product_id:04x} {bus} Device {device_addr} \033[0m', end='')
                # Turn on or off SenseCAP A1101.
                if get:
                    return device.open()
                else:
                    device.close()
                    print('\r' + f'\033[4;31mID {vendor_id:04x}:{product_id:04x} {bus} Device {device_addr} CLOSED\033[0m', flush=True)
                    
    def collect_data(self):
        while True:
            if not self.connect():
                continue
            self.read_data()
            del self.handle
            self.disconnect()

    def get_transferred_data_packets(self):
        # Obtain the transferred commands from Arduino Nano via serial communication, including fertilizer hazard classes (labels).
        if self.arduino_nano.in_waiting > 0:
            command = self.arduino_nano.readline().decode("utf-8", "ignore").rstrip()
            if(command.find("Enriched") >= 0):
                print("\nCapturing an image! Label: Enriched")
                self.save_img_sample("Enriched")
            if(command.find("Unsafe") >= 0):
                print("\nCapturing an image! Label: Unsafe")
                self.save_img_sample("Unsafe")
            if(command.find("Toxic") >= 0):
                print("\nCapturing an image! Label: Toxic")
                self.save_img_sample("Toxic")
        sleep(1)
        
    def save_img_sample(self, _class):    
        date = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = './samples/{}_IMG_{}.jpg'.format(_class, date)
        # If requested, save the recently captured image (latest frame) as a sample.
        cv2.imwrite(filename, self.latest_frame)
        print("\nSample Saved Successfully: " + filename)
        
        
# Define the soil object.
soil = A1101_data_img_collection()

# Define and initialize threads.
def A1101_camera_feed():
    soil.collect_data()
        
def activate_received_commands():
    while True:
        soil.get_transferred_data_packets()

Thread(target=A1101_camera_feed).start()
Thread(target=activate_received_commands).start()

