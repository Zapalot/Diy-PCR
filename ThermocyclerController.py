#########################################################################
# Thermocycler Software for usage with an Arduino based Peltier Control. By Felix Bonowski (2017)
# !Use only under constant supervision - your device _WILL_ cause a fire if it malfunctions!
#
# Works onyl with Python 3.x!
# -> type "python3 ThermocyclerController.py" in command line to run
#
# Depends on pyserial library
#  -> type "pip3 install pyserial" to install
#
# Uses the following Serial commands:
# - set temperature to ###.### (enables peltier):  "t###.###\n"
#
# See the Arduino code in the same Repository for syntax of status messages
#
# Licensed as Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0) 
#
#########################################################################

import serial
import time

###!!!Choose the same Port as you would in the Arduino IDE here!!!
# arduinoport = '/dev/cu.usbmodem621'
arduinoport = 'COM5'
pcrSerial = serial.Serial(arduinoport, 115200)


tempTolerance=1.5 # when temperature is in this range from setpoint, start counting time

# initial cycle that can be different to the following ones
startTemps=[94] #melt the dna double strands completely
startTimes=[3*60]

# repeated cycles
cycleTemps=[94.0, 48.0, 68.0] # melt, let primers bind, then let the polymerase work
cycleTimes=[30, 30, 45]
nCycles=30

#what to do when finished
endTemps=[68.0,10.0] # last "strand finishing" @ 68°C to eliminate single strands, than cool for preservation
endTimes=[5*60, 5*60*60]


##glue all the steps together
allTemps=startTemps+cycleTemps*nCycles+endTemps
allTimes=startTimes+cycleTimes*nCycles+endTimes
    
#wait for pcr Machine to start up and give us a few status messages
print ("Waiting for pcr Machine to answer...")
for i in range(3):
    pcrString=pcrSerial.readline()


# start the PCR: go to one temperature after another and hold them for the specified time
for stepIndex,(currentTime, currentTemp) in enumerate(zip(allTimes,allTemps)):
    print ("Setting temperature to "+ str(currentTemp))
    
    #tell the Arduino to heat/cool to the current temperature
    pcrSerial.write(("t"+str(currentTemp)+"\n").encode())
    pcrSerial.flush()
    
    #now wait until the temperature has been reached
    temperatureReached=False
    while (temperatureReached==False):
        #read temperature from PCR-Machine:
        pcrString=str(pcrSerial.readline(),'utf-8').strip()
        parts=pcrString.split("\t")
        #messages with 6 tab separated parts contain the current temp in part 3
        if(len(parts)==8):
            measuredTemp=float(parts[3])
            print("Step("+str(stepIndex+1)+"/"+str(len(allTemps))+"): Heating/cooling - "+pcrString)
            if(abs(measuredTemp-currentTemp)<tempTolerance):
                temperatureReached=True
        else:
            print ("Status Message from pcr: "+pcrString)
            
    # now that we are on temperature, wait for the specified amount of time
    timeWhenTempWasReached=time.time() # when was the target temperature of the current step reached?
    while((time.time()-timeWhenTempWasReached)<currentTime):
        pcrString=str(pcrSerial.readline(),'utf-8').strip()
        parts=pcrString.split("\t")
        if(len(parts)==8):
            measuredTemp=float(parts[3])
            print("Step("+str(stepIndex+1)+"/"+str(len(allTemps))+"): Waiting..." + str(round(time.time()-timeWhenTempWasReached,1)) +"s of "+ str(currentTime)+"s. "+pcrString)
        else:
            print ("Status Message from pcr:"+pcrString)

pcrSerial.close() 