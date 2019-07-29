--------EVT2ROOT ENCORE EDITION--------

For using with the FSU ENCORE detector data. 

First the data must be converted from its inital nscldaq10 format to nscldaq11 format.
This is done using the following commands in the terminal:

cd /usr/opt/nscldaq/11.0/bin/
./convert10to11 <fullpath_name_of_nscl10_file.evt> fullpath_name_of_nscl11_file.evt-11
(Check out http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x7466.html for more info if you want)

Where here the fullpath name implies that you specify as /home/music/stagearea/complete/yourfile.evt
The converted file should be placed in /home/music/evt2root/converted_evt/yourfile.evt-11
But in principle you can do whatever you want as long as you keep track of it. 

Before running the ROOT converter you must add the file(s) you want to convert to the evt_files.lst. To do this 
open the evt_files.lst and add your file with the following syntax:

file:///fullpath_name_of_nscl11_file.evt-11

Where again here the fullpath name implies the whole /home/music/etc. If you follow the format outlined above this will look like:

file:///home/music/evt2root/converted_evt/yourfile.evt-11

To convert from nscldaq11 data to a ROOT file run the following command from this directory:

./evt2root rootfiles/yourfile.root

Where here the name yourfile.root is whatever you want to name the converted file. Here its recommended to place your files into the rootfile folder in the 
evt2root directory. The program will then prompt you to enter the name of the list file. This will be evt_files.lst or whatever else you called the file 
that contains the list of all your files to be converted. 

The program will then attempt to create a data pipe to the file you are converting. Errors will be printed out to the terminal as they occur. If there is an error 
that will cause a fatal crash, the program will stop the conversion and exit safely. Non-fatal (usually unpacker confusion) will not cause an exit, but indicate that the
converted ROOT file may have improperly intepreted data. This generally doesn't mean that the converter failed; usually a confused unpacker comes from a data stream error
when the data was originally taken. So as long as the program doesn't terminate before the end of a run, don't toss the rootfile just because there were a few complaints, 
check and see if the file makes sense first.  

After conversion is complete, rootfiles should be moved to where ever the next analysis stage will take place. DELETE YOUR ROOTFILES FROM THIS COMPUTER ONCE YOU MOVE THEM!!!!!
Leaving too many rootfiles lying around here will cause us to run out storage really quickly.
