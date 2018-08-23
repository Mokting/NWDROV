/**
 * KKMulticopterFlashTool, a avrdude GUI for flashing KK boards and other
 *   equipment.
 *   Copyright (C) 2011 Christian Moll
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package de.lazyzero.kkMulticopterFlashTool.gui;

import static lu.tudor.santec.i18n.Translatrix._;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Vector;
import java.util.logging.Logger;

import javax.swing.JOptionPane;

import avr8_burn_o_mat.AVR;
import avr8_burn_o_mat.AvrdudeProgrammer;
import avr8_burn_o_mat.InterfaceTextOutput;
import de.lazyzero.kkMulticopterFlashTool.KKMulticopterFlashTool;
import de.lazyzero.kkMulticopterFlashTool.utils.ArduinoUpload;
import de.lazyzero.kkMulticopterFlashTool.utils.Firmware;
import de.lazyzero.kkMulticopterFlashTool.utils.PortScanner;

public class ArduinoUSBLinkerUploader {
	public static final String ARDUINO = "arduino";
	private static final String ATMEGA328p = "ATmega328p";
	private static final String ATMEGA168 = "atmega168";
	
	private static final String ATMEGA328p_CAPTION = "m328p";
	private static final String ATMEGA168_CAPTION = "m168";
	
	private static Logger logger = KKMulticopterFlashTool.getLogger();
			
	private  LinkedHashMap<String, ArduinoUpload> arduinos = new LinkedHashMap<String, ArduinoUpload>();
	private String arduinoPort = "";
	private Firmware firmware;
	private KKMulticopterFlashTool kkflashtool;
	private InterfaceTextOutput textOutput;
	
	public ArduinoUSBLinkerUploader(KKMulticopterFlashTool parent) {
		this.kkflashtool = parent;
		this.textOutput = parent;
 		init();
	}

	private void init() {
		for (Firmware firmware : kkflashtool.getFirmwares()) {
			if (firmware.getController().equals(ArduinoUSBLinkerUploader.ARDUINO.toLowerCase())) {
				this.firmware = firmware;
				break;
			}
		}
		arduinos.put("Arduino Uno", new ArduinoUpload("Arduino Uno", new AVR(ARDUINO, ATMEGA328p_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 115200));
		arduinos.put("Arduino Duemilanove w/ ATmega328", new ArduinoUpload("Arduino Duemilanove w/ ATmega328", new AVR(ARDUINO, ATMEGA328p_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 57600));
		arduinos.put("Arduino Diecimila or Duemilanove w/ ATmega168", new ArduinoUpload("Arduino Diecimila or Duemilanove w/ ATmega168", new AVR(ATMEGA168, ATMEGA168_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 19200));
		arduinos.put("Arduino Nano w/ ATmega328", new ArduinoUpload("Arduino Nano w/ ATmega328", new AVR(ATMEGA328p, ATMEGA328p_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 57600));
		arduinos.put("Arduino Nano w/ ATmega168", new ArduinoUpload("Arduino Nano w/ ATmega168", new AVR(ATMEGA168, ATMEGA168_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 19200));
		arduinos.put("Arduino Mini w/ ATmega328", new ArduinoUpload("Arduino Mini w/ ATmega328", new AVR(ATMEGA328p, ATMEGA328p_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 115200));
		arduinos.put("Arduino Mini w/ ATmega168", new ArduinoUpload("Arduino Mini w/ ATmega168", new AVR(ATMEGA168, ATMEGA168_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 19200));
		arduinos.put("Arduino Pro or Pro Mini w/ ATmega328", new ArduinoUpload("Arduino Pro or Pro Mini w/ ATmega328", new AVR(ATMEGA328p, ATMEGA328p_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 115200));
		arduinos.put("Arduino Pro or Pro Mini w/ ATmega168", new ArduinoUpload("Arduino Pro or Pro Mini w/ ATmega168", new AVR(ATMEGA168, ATMEGA168_CAPTION), new AvrdudeProgrammer(ARDUINO, ARDUINO, ""), 19200));
	}
	
	public Iterator<String> getArduinos() {
		return arduinos.keySet().iterator();
	}

	public void upload(String name) {
		ArduinoUpload uploader = arduinos.get(name);
		logger.info("Flash ArduinoUSBLinker to " + name);
		logger.info(uploader.toString());
		textOutput.clearText();
		if (detectArduino()) {
			logger.info("now flash the ArduinoUSBLinker hex to the detected Arduino.");
			logger.info("firmware" + firmware);
			textOutput.println(_("arduinousblinker.logAutodetect"));
			textOutput.println(_("port") + ": " + this.arduinoPort);
			textOutput.print(_("firmware") + ": " + this.firmware.getName() + " " + _("by") + " ");
			textOutput.println(_("author") + ": " + this.firmware.getAuthor());
			
			kkflashtool.flashArduino(arduinos.get(name), firmware, arduinoPort);
		} else {
			logger.warning("No Arduino connected during search phase.");
			textOutput.err(_("arduinousblinker.LogNoArduinoConnected"));
			JOptionPane.showMessageDialog(null, _("arduinousblinker.noArduinoConnected"));
		}
	}

	private boolean detectArduino() {
		boolean arduinoDetected = false;
		Vector<String> availablePorts = PortScanner.listSerialPorts();
		
		JOptionPane.showMessageDialog(null, _("arduinousblinker.connectArduino"));
		Vector<String> newPorts = PortScanner.listSerialPorts();
		for (String port : newPorts) {
			if (!availablePorts.contains(port)) {
				JOptionPane.showMessageDialog(null, _("arduinousblinker.arduinoDetected")+" "+port);
				this.arduinoPort  = port;
				arduinoDetected = true;
				return arduinoDetected;
			}
		}
		return arduinoDetected;
	}
}

/*

diecimila.name=Arduino Diecimila or Duemilanove w/ ATmega168

diecimila.upload.protocol=arduino
diecimila.upload.maximum_size=14336
diecimila.upload.speed=19200

diecimila.bootloader.low_fuses=0xff
diecimila.bootloader.high_fuses=0xdd
diecimila.bootloader.extended_fuses=0x00
diecimila.bootloader.path=atmega
diecimila.bootloader.file=ATmegaBOOT_168_diecimila.hex
diecimila.bootloader.unlock_bits=0x3F
diecimila.bootloader.lock_bits=0x0F

diecimila.build.mcu=atmega168
diecimila.build.f_cpu=16000000L
diecimila.build.core=arduino
diecimila.build.variant=standard

##############################################################

nano328.name=Arduino Nano w/ ATmega328

nano328.upload.protocol=arduino
nano328.upload.maximum_size=30720
nano328.upload.speed=57600

nano328.bootloader.low_fuses=0xFF
nano328.bootloader.high_fuses=0xDA
nano328.bootloader.extended_fuses=0x05
nano328.bootloader.path=atmega
nano328.bootloader.file=ATmegaBOOT_168_atmega328.hex
nano328.bootloader.unlock_bits=0x3F
nano328.bootloader.lock_bits=0x0F

nano328.build.mcu=atmega328p
nano328.build.f_cpu=16000000L
nano328.build.core=arduino
nano328.build.variant=eightanaloginputs

##############################################################

nano.name=Arduino Nano w/ ATmega168

nano.upload.protocol=arduino
nano.upload.maximum_size=14336
nano.upload.speed=19200

nano.bootloader.low_fuses=0xff
nano.bootloader.high_fuses=0xdd
nano.bootloader.extended_fuses=0x00
nano.bootloader.path=atmega
nano.bootloader.file=ATmegaBOOT_168_diecimila.hex
nano.bootloader.unlock_bits=0x3F
nano.bootloader.lock_bits=0x0F

nano.build.mcu=atmega168
nano.build.f_cpu=16000000L
nano.build.core=arduino
nano.build.variant=eightanaloginputs

##############################################################

mega2560.name=Arduino Mega 2560 or Mega ADK

mega2560.upload.protocol=stk500v2
mega2560.upload.maximum_size=258048
mega2560.upload.speed=115200

mega2560.bootloader.low_fuses=0xFF
mega2560.bootloader.high_fuses=0xD8
mega2560.bootloader.extended_fuses=0xFD
mega2560.bootloader.path=stk500v2
mega2560.bootloader.file=stk500boot_v2_mega2560.hex
mega2560.bootloader.unlock_bits=0x3F
mega2560.bootloader.lock_bits=0x0F

mega2560.build.mcu=atmega2560
mega2560.build.f_cpu=16000000L
mega2560.build.core=arduino
mega2560.build.variant=mega

##############################################################

mega.name=Arduino Mega (ATmega1280)

mega.upload.protocol=arduino
mega.upload.maximum_size=126976
mega.upload.speed=57600

mega.bootloader.low_fuses=0xFF
mega.bootloader.high_fuses=0xDA
mega.bootloader.extended_fuses=0xF5
mega.bootloader.path=atmega
mega.bootloader.file=ATmegaBOOT_168_atmega1280.hex
mega.bootloader.unlock_bits=0x3F
mega.bootloader.lock_bits=0x0F

mega.build.mcu=atmega1280
mega.build.f_cpu=16000000L
mega.build.core=arduino
mega.build.variant=mega

##############################################################

leonardo.name=Arduino Leonardo
leonardo.upload.protocol=avr109
leonardo.upload.maximum_size=28672
leonardo.upload.speed=57600
leonardo.upload.disable_flushing=true
leonardo.bootloader.low_fuses=0xff
leonardo.bootloader.high_fuses=0xd8
leonardo.bootloader.extended_fuses=0xcb
leonardo.bootloader.path=caterina
leonardo.bootloader.file=Caterina-Leonardo.hex
leonardo.bootloader.unlock_bits=0x3F
leonardo.bootloader.lock_bits=0x2F
leonardo.build.mcu=atmega32u4
leonardo.build.f_cpu=16000000L
leonardo.build.vid=0x2341
leonardo.build.pid=0x8036
leonardo.build.core=arduino
leonardo.build.variant=leonardo

##############################################################

mini328.name=Arduino Mini w/ ATmega328

mini328.upload.protocol=arduino
mini328.upload.maximum_size=28672
mini328.upload.speed=115200

mini328.bootloader.low_fuses=0xff
mini328.bootloader.high_fuses=0xd8
mini328.bootloader.extended_fuses=0x05
mini328.bootloader.path=optiboot
mini328.bootloader.file=optiboot_atmega328-Mini.hex
mini328.bootloader.unlock_bits=0x3F
mini328.bootloader.lock_bits=0x0F

mini328.build.mcu=atmega328p
mini328.build.f_cpu=16000000L
mini328.build.core=arduino
mini328.build.variant=eightanaloginputs

##############################################################

mini.name=Arduino Mini w/ ATmega168

mini.upload.protocol=arduino
mini.upload.maximum_size=14336
mini.upload.speed=19200

mini.bootloader.low_fuses=0xff
mini.bootloader.high_fuses=0xdd
mini.bootloader.extended_fuses=0x00
mini.bootloader.path=atmega
mini.bootloader.file=ATmegaBOOT_168_ng.hex
mini.bootloader.unlock_bits=0x3F
mini.bootloader.lock_bits=0x0F

mini.build.mcu=atmega168
mini.build.f_cpu=16000000L
mini.build.core=arduino
mini.build.variant=eightanaloginputs

##############################################################

ethernet.name=Arduino Ethernet

ethernet.upload.protocol=arduino
ethernet.upload.maximum_size=32256
ethernet.upload.speed=115200

ethernet.bootloader.low_fuses=0xff
ethernet.bootloader.high_fuses=0xde
ethernet.bootloader.extended_fuses=0x05
ethernet.bootloader.path=optiboot
ethernet.bootloader.file=optiboot_atmega328.hex
ethernet.bootloader.unlock_bits=0x3F
ethernet.bootloader.lock_bits=0x0F

ethernet.build.variant=standard
ethernet.build.mcu=atmega328p
ethernet.build.f_cpu=16000000L
ethernet.build.core=arduino

##############################################################

fio.name=Arduino Fio

fio.upload.protocol=arduino
fio.upload.maximum_size=30720
fio.upload.speed=57600

fio.bootloader.low_fuses=0xFF
fio.bootloader.high_fuses=0xDA
fio.bootloader.extended_fuses=0x05
fio.bootloader.path=arduino:atmega
fio.bootloader.file=ATmegaBOOT_168_atmega328_pro_8MHz.hex
fio.bootloader.unlock_bits=0x3F
fio.bootloader.lock_bits=0x0F

fio.build.mcu=atmega328p
fio.build.f_cpu=8000000L
fio.build.core=arduino
fio.build.variant=eightanaloginputs

##############################################################

bt328.name=Arduino BT w/ ATmega328

bt328.upload.protocol=arduino
bt328.upload.maximum_size=28672
bt328.upload.speed=19200
bt328.upload.disable_flushing=true

bt328.bootloader.low_fuses=0xff
bt328.bootloader.high_fuses=0xd8
bt328.bootloader.extended_fuses=0x05
bt328.bootloader.path=bt
bt328.bootloader.file=ATmegaBOOT_168_atmega328_bt.hex
bt328.bootloader.unlock_bits=0x3F
bt328.bootloader.lock_bits=0x0F

bt328.build.mcu=atmega328p
bt328.build.f_cpu=16000000L
bt328.build.core=arduino
bt328.build.variant=eightanaloginputs

##############################################################

bt.name=Arduino BT w/ ATmega168

bt.upload.protocol=arduino
bt.upload.maximum_size=14336
bt.upload.speed=19200
bt.upload.disable_flushing=true

bt.bootloader.low_fuses=0xff
bt.bootloader.high_fuses=0xdd
bt.bootloader.extended_fuses=0x00
bt.bootloader.path=bt
bt.bootloader.file=ATmegaBOOT_168.hex
bt.bootloader.unlock_bits=0x3F
bt.bootloader.lock_bits=0x0F

bt.build.mcu=atmega168
bt.build.f_cpu=16000000L
bt.build.core=arduino
bt.build.variant=eightanaloginputs

##############################################################

lilypad328.name=LilyPad Arduino w/ ATmega328

lilypad328.upload.protocol=arduino
lilypad328.upload.maximum_size=30720
lilypad328.upload.speed=57600

lilypad328.bootloader.low_fuses=0xFF
lilypad328.bootloader.high_fuses=0xDA
lilypad328.bootloader.extended_fuses=0x05
lilypad328.bootloader.path=atmega
lilypad328.bootloader.file=ATmegaBOOT_168_atmega328_pro_8MHz.hex
lilypad328.bootloader.unlock_bits=0x3F
lilypad328.bootloader.lock_bits=0x0F

lilypad328.build.mcu=atmega328p
lilypad328.build.f_cpu=8000000L
lilypad328.build.core=arduino
lilypad328.build.variant=standard

##############################################################

lilypad.name=LilyPad Arduino w/ ATmega168

lilypad.upload.protocol=arduino
lilypad.upload.maximum_size=14336
lilypad.upload.speed=19200

lilypad.bootloader.low_fuses=0xe2
lilypad.bootloader.high_fuses=0xdd
lilypad.bootloader.extended_fuses=0x00
lilypad.bootloader.path=lilypad
lilypad.bootloader.file=LilyPadBOOT_168.hex
lilypad.bootloader.unlock_bits=0x3F
lilypad.bootloader.lock_bits=0x0F

lilypad.build.mcu=atmega168
lilypad.build.f_cpu=8000000L
lilypad.build.core=arduino
lilypad.build.variant=standard

##############################################################

pro5v328.name=Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega328

pro5v328.upload.protocol=arduino
pro5v328.upload.maximum_size=30720
pro5v328.upload.speed=57600

pro5v328.bootloader.low_fuses=0xFF
pro5v328.bootloader.high_fuses=0xDA
pro5v328.bootloader.extended_fuses=0x05
pro5v328.bootloader.path=atmega
pro5v328.bootloader.file=ATmegaBOOT_168_atmega328.hex
pro5v328.bootloader.unlock_bits=0x3F
pro5v328.bootloader.lock_bits=0x0F

pro5v328.build.mcu=atmega328p
pro5v328.build.f_cpu=16000000L
pro5v328.build.core=arduino
pro5v328.build.variant=standard

##############################################################

pro5v.name=Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega168

pro5v.upload.protocol=arduino
pro5v.upload.maximum_size=14336
pro5v.upload.speed=19200

pro5v.bootloader.low_fuses=0xff
pro5v.bootloader.high_fuses=0xdd
pro5v.bootloader.extended_fuses=0x00
pro5v.bootloader.path=atmega
pro5v.bootloader.file=ATmegaBOOT_168_diecimila.hex
pro5v.bootloader.unlock_bits=0x3F
pro5v.bootloader.lock_bits=0x0F

pro5v.build.mcu=atmega168
pro5v.build.f_cpu=16000000L
pro5v.build.core=arduino
pro5v.build.variant=standard

##############################################################

pro328.name=Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328

pro328.upload.protocol=arduino
pro328.upload.maximum_size=30720
pro328.upload.speed=57600

pro328.bootloader.low_fuses=0xFF
pro328.bootloader.high_fuses=0xDA
pro328.bootloader.extended_fuses=0x05
pro328.bootloader.path=atmega
pro328.bootloader.file=ATmegaBOOT_168_atmega328_pro_8MHz.hex
pro328.bootloader.unlock_bits=0x3F
pro328.bootloader.lock_bits=0x0F

pro328.build.mcu=atmega328p
pro328.build.f_cpu=8000000L
pro328.build.core=arduino
pro328.build.variant=standard

##############################################################

pro.name=Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega168

pro.upload.protocol=arduino
pro.upload.maximum_size=14336
pro.upload.speed=19200

pro.bootloader.low_fuses=0xc6
pro.bootloader.high_fuses=0xdd
pro.bootloader.extended_fuses=0x00
pro.bootloader.path=atmega
pro.bootloader.file=ATmegaBOOT_168_pro_8MHz.hex
pro.bootloader.unlock_bits=0x3F
pro.bootloader.lock_bits=0x0F

pro.build.mcu=atmega168
pro.build.f_cpu=8000000L
pro.build.core=arduino
pro.build.variant=standard

##############################################################

atmega168.name=Arduino NG or older w/ ATmega168

atmega168.upload.protocol=arduino
atmega168.upload.maximum_size=14336
atmega168.upload.speed=19200

atmega168.bootloader.low_fuses=0xff
atmega168.bootloader.high_fuses=0xdd
atmega168.bootloader.extended_fuses=0x00
atmega168.bootloader.path=atmega
atmega168.bootloader.file=ATmegaBOOT_168_ng.hex
atmega168.bootloader.unlock_bits=0x3F
atmega168.bootloader.lock_bits=0x0F

atmega168.build.mcu=atmega168
atmega168.build.f_cpu=16000000L
atmega168.build.core=arduino
atmega168.build.variant=standard

##############################################################

atmega8.name=Arduino NG or older w/ ATmega8

atmega8.upload.protocol=arduino
atmega8.upload.maximum_size=7168
atmega8.upload.speed=19200

atmega8.bootloader.low_fuses=0xdf
atmega8.bootloader.high_fuses=0xca
atmega8.bootloader.path=atmega8
atmega8.bootloader.file=ATmegaBOOT.hex
atmega8.bootloader.unlock_bits=0x3F
atmega8.bootloader.lock_bits=0x0F

atmega8.build.mcu=atmega8
atmega8.build.f_cpu=16000000L
atmega8.build.core=arduino
atmega8.build.variant=standard

*/