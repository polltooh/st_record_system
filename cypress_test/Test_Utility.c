/*
 * test utility
 * copyright (c) 2013 cypress semiconductor
 *
 * this library is free software; you can redistribute it and/or
 * modify it under the terms of the gnu lesser general public
 * license as published by the free software foundation; either
 * version 2.1 of the license, or (at your option) any later version.
 *
 * this library is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the gnu
 * lesser general public license for more details.
 *
 * you should have received a copy of the gnu lesser general public
 * license along with this library; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma 02110-1301 usa
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>

#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>

#include "cyusbserial.h"
#define cy_max_devices 30
#define cy_max_interfaces 4

void printlistofdevices (bool isprint);

typedef struct _cy_device_struct {
    int devicenumber;
    int interfacefunctionality[cy_max_interfaces];
    bool isi2c;
    bool isspi;
    int numinterface; 
}cy_device_struct;

cy_device_struct *gldevice;
int i2cdeviceindex[cy_max_devices][cy_max_interfaces];
unsigned char *devicenumber = null;
int cydevices, i2cdevices = 0, numdevices = 0;
int selecteddevicenum = -1, selectedinterfacenum = -1;
bool exitapp = false;
unsigned short pageaddress = -1;
short readwritelength = -1;
bool deviceaddedremoved = false;
int getuserinput()
{
    char userinput[6], x;
    int output,i = 0;
    bool isdigit = true;
    x = getchar();
    while (x != '\n'){
        if (i < 5){
            userinput[i] = x;
            i++;
        }
        if (!isdigit(x))
            isdigit = false;

        x = getchar();
    }
    userinput[i] = '\0';
    if (isdigit == false)
        return -1;
    output = atoi(userinput);
    return output;
}
void devicehotplug () {
    
    cy_return_status rstatus;
    deviceaddedremoved = true;
    selecteddevicenum = -1;
    selectedinterfacenum = -1;
    printf ("device of interest removed/added \n");
    rstatus = cygetlistofdevices (&numdevices);
    if (rstatus != cy_success) {
        printf ("cy:error in getting list of devices: error no:<%d> \n", rstatus);
        //return rstatus;
    }
    printlistofdevices (false);
}

int main (int argc, char **agrv)
{
    int index = 0, i, j, userinput;
    int tempselecteddevicenum, tempselectedinterfacenum, temppageaddress, templength;
    cy_return_status rstatus;
    signal (sigusr1, devicehotplug);
    gldevice = (cy_device_struct *)malloc (cy_max_devices *sizeof (cy_device_struct));
    if (gldevice == null){
        printf ("memory allocation failed ...!! \n");
        return -1;
    }
    rstatus = cylibraryinit ();
    if (rstatus != cy_success) {
        printf ("cy:error in doing library init error no:<%d> \n", rstatus);
        return rstatus;
    }
    rstatus = cygetlistofdevices (&numdevices);
    if (rstatus != cy_success) {
        printf ("cy:error in getting list of devices: error no:<%d> \n", rstatus);
        return rstatus;
    }
    printlistofdevices(true);
    do {

        printf ("-------------------------------------------------------------------\n");
        printf ("1: print list of devices \n");
        if (selecteddevicenum != -1 && selectedinterfacenum != -1){
            printf ("2: change device selection--selected device: [device number %d] : [interface no %d]",\
                    selecteddevicenum, selectedinterfacenum);
            if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == cy_type_i2c)
                printf (" : i2c\n");
            else if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == cy_type_spi)
                printf (" : spi\n");
            else
                printf (" : na\n");

        }
        else    
            printf ("2: select device...no device selected !!\n");

        if (readwritelength != -1 && pageaddress != -1){
            printf ("3: change flash page address and length ...entered is page address %d and length %d\n", pageaddress, readwritelength);
            printf ("4: verify data\n5: exit\n");
        }
        else
            printf ("3: enter i2c/spi flash page address and length to write/read.\n4: verify data\n5: exit\n");
        printf ("-------------------------------------------------------------------\n");

        userinput = getuserinput();
        if (userinput < 1 || userinput > 5){
            printf ("wrong selection code ... enter again \n");
            continue;
        }

        switch (userinput){

            case 1:
                printlistofdevices(true);
                break;
            case 2:
                if (cydevices == 0) {
                    printf ("no device of interest connected ...!!\n");
                    continue;
                }
                printf ("enter device number to be selected.. \n");
                tempselecteddevicenum = getuserinput();
                //printf ("selected device number is %d \n",tempselecteddevicenum);
                if (tempselecteddevicenum >= cydevices || tempselecteddevicenum == -1){
                    printf ("wrong device selection \n");
                    continue;
                }
                printf ("enter interface number..\n");
                tempselectedinterfacenum = getuserinput();
                //printf ("selected device number is %d %d\n",tempselectedinterfacenum, gldevice[tempselecteddevicenum].numinterface);

                if (tempselectedinterfacenum >= gldevice[tempselecteddevicenum].numinterface || 
                        tempselectedinterfacenum == -1) {
                    printf ("wrong interface selection selection \n");
                    continue;
                }
                if ((gldevice[tempselecteddevicenum].interfacefunctionality[tempselectedinterfacenum] == -1)){
                    printf ("selected device does not have i2c or spi !!! \n");
                    continue;
                }
                if (deviceaddedremoved == true) {
                    printf ("device of interest was added/removed .... print and select from new list\n");
                    continue;
                }
                selecteddevicenum = tempselecteddevicenum;
                selectedinterfacenum = tempselectedinterfacenum;
                //pageaddress = -1;
                //readwritelength = -1;    
                break;
            case 3:
                if (selecteddevicenum == -1) {
                    printf ("select proper device!!! \n");
                    continue;
                }
                if (selecteddevicenum >= cydevices){
                    printf ("select proper device!!! \n");
                    continue;
                }
                if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == cy_type_i2c){
                    printf ("enter page address... (less than 256)\n");
                    temppageaddress = getuserinput();
                    printf ("enter length to read/write...(less than 64)\n");
                    templength = getuserinput();
                    if (deviceaddedremoved == true) {
                        printf ("device of interest was added/removed .... print and select from new list\n");
                        continue;
                    }
                    if (temppageaddress > 256){
                        printf ("invalid page address ..enter page address less than 256 (1 bytes len)\n");
                        continue;
                    }
                    if (templength < 0 || templength > 64){
                        printf ("invalid length ..enter length less than 64 bytes\n");
                        continue;
                    }
                    pageaddress = temppageaddress;
                    readwritelength = templength;
                    break;
                }
                if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == cy_type_spi){
                    printf ("enter page address... (less than 65536)\n");
                    temppageaddress = getuserinput();
                    printf ("enter length to read/write...(less than 256)\n");
                    templength = getuserinput();
                    if (deviceaddedremoved == true) {
                        printf ("device of interest was added/removed .... print and select from new list\n");
                        continue;
                    }
                    if (temppageaddress > 65536){
                        printf ("invalid page address ..enter page address less than 65536 (1 bytes len)\n");
                        continue;
                    }
                    if (templength < 0 || templength > 256){
                        printf ("invalid length ..enter length less than 256 bytes\n");
                        continue;
                    }
                    pageaddress = temppageaddress;
                    readwritelength = templength;
                    break;
                }
            case 4:
                if (selecteddevicenum == -1) {
                    printf ("select proper device!!! \n");
                    continue;
                }
                if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == -1){
                    printf ("selected device does not have i2c or spi !!! \n");
                    continue;
                }
                if ((pageaddress == -1) || (readwritelength == -1)){
                    printf ("select proper page address and length !!! \n");
                    continue;
                }
                if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == cy_type_i2c){
                    if (pageaddress > 256){
                        printf ("invalid page address for i2c .. need to be less than 256\n");
                        continue;
                    }
                    if (readwritelength > 64){
                        printf ("invalid read write length for i2c .. need to be less than 64\n");
                        continue;
                    }
                    printf ("calling i2c \n");
                    i2cverifydata(gldevice[selecteddevicenum].devicenumber, selectedinterfacenum);
                }
                if (gldevice[selecteddevicenum].interfacefunctionality[selectedinterfacenum] == cy_type_spi){
                    spiverifydata(gldevice[selecteddevicenum].devicenumber, selectedinterfacenum);
                }
                break;

            case 5:
                exitapp = true;
                cylibraryexit ();
                break;    
        }
    }while (exitapp == false);
    free (gldevice);
}
int spiwriteenable (cy_handle handle)
{
    unsigned char wr_data,rd_data;
    cy_return_status status = cy_success;
    cy_data_buffer writebuf;
    cy_data_buffer readbuf;

    writebuf.buffer = &wr_data;
    writebuf.length = 1;

    readbuf.buffer = &rd_data;
    readbuf.length = 1;

    wr_data = 0x06; /* write enable */
    status = cyspireadwrite (handle, &readbuf, &writebuf, 5000);
    if (status != cy_success)
    {
        return status;
    }
    return status;
}
//helper functions for doing data transfer to/from spi flash
int spiwaitforidle (cy_handle handle)
{
    char rd_data[2], wr_data[2];
    cy_data_buffer writebuf, readbuf;
    writebuf.length = 2;
    writebuf.buffer = (unsigned char *)wr_data;
    int timeout = 0xffff;
    cy_return_status status;

    readbuf.length = 2;
    readbuf.buffer = (unsigned char *)rd_data;
    do
    {
        wr_data[0] = 0x05; /* status */
        status = cyspireadwrite (handle, &readbuf, &writebuf, 5000);
        if (status != cy_success)
        {
            break;
        }
        timeout--;
        if (timeout == 0)
        {
            status = cy_error_io_timeout;
            return status;
        }
    } while (rd_data[1] & 0x01);
    return status;
}

int spiverifydata (int devicenumber, int interfacenum)
{
    cy_data_buffer databufferwrite,databufferread;
    cy_handle handle;
    bool isverify = true;
    unsigned char wbuffer[256 + 4], rbuffer[256 + 4];
    int rstatus, length;

    memset (rbuffer, 0x00, 256);
    memset (wbuffer, 0x00, 256);

    rstatus = cyopen (devicenumber, interfacenum, &handle);
    if (rstatus != cy_success){
        printf ("cy_spi: open failed \n");
        return rstatus;
    }
    databufferwrite.buffer = wbuffer;
    databufferread.buffer = rbuffer;

    rstatus = spiwaitforidle (handle); 
    if (rstatus){
        printf("error in waiting for eepom active state %d \n", rstatus);
        cyclose (handle);

        return cy_error_request_failed;
    }
    printf ("calling spi write enable \n");
    rstatus = spiwriteenable (handle);
    if (rstatus){
        printf("error in setting write enable %d \n", rstatus);
        cyclose (handle);
        return cy_error_request_failed;
    }
    //write spi write command
    wbuffer[0] = 0x02;
    //spi flash address
    wbuffer[1] = (pageaddress >> 8);
    wbuffer[2] = (pageaddress) & 0x00ff;
    wbuffer[3] = 0;

    printf ("the data written is ...\n");
    printf ("\n--------------------------------------------------------------------\n");
    for (rstatus = 4; rstatus < (readwritelength + 4); rstatus++){
        wbuffer[rstatus] = rand() % 256;
        printf ("%x ",wbuffer[rstatus]);
    }
    printf ("\n--------------------------------------------------------------------\n");

    databufferread.length = (4 + readwritelength);
    databufferwrite.length = (4 + readwritelength);

    rstatus = cyspireadwrite (handle , &databufferread, &databufferwrite, 5000);
    if (rstatus != cy_success){
        cyclose (handle);
        printf ("error in doing spi data write data write is %d data read is %d\n" , databufferwrite.transfercount,databufferread.transfercount);
        return cy_error_request_failed;    
    }

    spiwaitforidle (handle);
    //write spi read command
    wbuffer[0] = 0x03;
    databufferread.length =  (4 + readwritelength);
    databufferwrite.length = (4 + readwritelength);

    rstatus = cyspireadwrite (handle, &databufferread, &databufferwrite, 5000);
    if (rstatus != cy_success){
        cyclose (handle);
        printf ("the error is %d \n", rstatus);
        printf ("error in doing spi data write data write is %d data read is %d\n" , databufferwrite.transfercount,databufferread.transfercount);
        return cy_error_request_failed;    
    }
    printf ("data read back is \n");
    printf ("\n---------------------------------------------------------------------\n");
    for (rstatus = 4; rstatus < (readwritelength + 4); rstatus++){
        printf ("%x ",rbuffer[rstatus]);
        if (rbuffer[rstatus] != wbuffer[rstatus]){
            isverify = false;
        }
    }
    printf ("\n---------------------------------------------------------------------\n");
    if (isverify)
        printf ("data verified successfully \n");
    else
        printf ("data corruption occured!!\n");

    cyclose (handle);
    return cy_success;
}

int i2cverifydata (int devicenumber, int interfacenum)
{
    cy_data_buffer databufferwrite, databufferread;
    cy_handle handle;
    int length = 0;
    bool isverify = true;
    int loopcount = 100, i;
    cy_return_status rstatus;
    unsigned char bytespending = 0, address[2], wbuffer[66], rbuffer[66];
    cy_i2c_data_config i2cdataconfig;

    memset (wbuffer, 0, 66);
    memset (rbuffer, 0, 66);

    i2cdataconfig.isstopbit = true;
    i2cdataconfig.slaveaddress = 0x51;

    rstatus = cyopen (devicenumber, interfacenum, &handle);
    if (rstatus != cy_success){
        printf("cy_i2c: open failed \n");
        return rstatus;
   }
    loopcount = 100;
    length = readwritelength;
    wbuffer[0]= pageaddress;
    wbuffer[1] = 0;
    databufferwrite.buffer = wbuffer;
    i2cdataconfig.isstopbit = true;
    databufferwrite.length = (length + 2);
    printf ("\n data that is written on to i2c ...\n");
    printf ("\n-----------------------------------------------------------------\n");
    for (i = 2; i < (length +2); i++){
        wbuffer[i] = rand() % 256;
        printf ("%x ", wbuffer[i]);
    }
    printf ("\n-----------------------------------------------------------------\n");
    rstatus = cyi2cwrite (handle, &i2cdataconfig, &databufferwrite, 5000);
    if (rstatus != cy_success){
        printf ("error in doing i2c write \n");
        cyclose (handle);
        return -1;
    }
    //we encountered a error in i2c read repeat the procedure again       
    //loop here untill read vendor command passes
    i2cdataconfig.isstopbit = false;
    databufferwrite.length = 2;

    do {
        rstatus = cyi2cwrite (handle, &i2cdataconfig, &databufferwrite, 5000); 
        loopcount--;
    }while (rstatus != cy_success && loopcount != 0);
    
    if (loopcount == 0 && rstatus != cy_success){
        printf ("error in sending read command \n");
        cyclose (handle);
        return -1;
    }

    databufferread.buffer = rbuffer;
    rbuffer[0]= address[0];
    rbuffer[1] = 0;
    i2cdataconfig.isstopbit = true; 
    i2cdataconfig.isnakbit = true;
    databufferread.length = length; 
    databufferread.buffer = rbuffer;

    memset (rbuffer, 0, 64);

    rstatus = cyi2cread (handle, &i2cdataconfig, &databufferread, 5000); 
    if (rstatus != cy_success){
        printf ("error in doing i2c read ... error is %d \n", rstatus);
        cyclose (handle);
        return -1;
    }

    printf ("\n data that is read from i2c ...\n");
    printf ("\n-----------------------------------------------------------------\n");
    for (rstatus = 0; rstatus < length; rstatus++){
        printf ("%x ", rbuffer[rstatus]);
        if (rbuffer[rstatus] != wbuffer[rstatus + 2]){
            isverify = false;
        }
    }
    printf ("\n-----------------------------------------------------------------\n");
    if (!isverify)
        printf ("data corruption occured ..!!!\n");
    else
        printf ("data verified successfully \n");
    
    cyclose (handle); 
}
bool iscypressdevice (int devicenum) {
    cy_handle handle;
    unsigned char interfacenum = 0;
    unsigned char sig[6];
    cy_return_status rstatus;
    rstatus = cyopen (devicenum, interfacenum, &handle);
    if (rstatus == cy_success){
        rstatus = cygetsignature (handle, sig);
        if (rstatus == cy_success){
            cyclose (handle);
            return true;
        }
        else {
            cyclose (handle);
            return false;
        }
    }
    else 
        return false;
}
void printlistofdevices (bool isprint)
{
    int  index_i = 0, index_j = 0, i, j, countofdevice = 0, devnum;
    int length, index = 0, numinterfaces, interfacenum;
    bool set1 = false;

    unsigned char deviceid[cy_max_devices];
    unsigned char functionality[64];
    cy_device_info deviceinfo;
    cy_device_class deviceclass[cy_max_interfaces];
    cy_device_type  devicetype[cy_max_interfaces];
    cy_return_status rstatus;

    deviceaddedremoved = false; 
    cygetlistofdevices (&numdevices);
    //printf ("the number of devices is %d \n", numdevices); 
    for (i = 0; i < numdevices; i++){
        for (j = 0; j< cy_max_interfaces; j++)
            gldevice[i].interfacefunctionality[j] = -1;
    }
    if (isprint){
        printf ("\n\n---------------------------------------------------------------------------------\n");
        printf ("device number | vid | pid | interface number | functionality \n");
        printf ("---------------------------------------------------------------------------------\n");
    }
    cydevices = 0;
    for (devnum = 0; devnum < numdevices; devnum++){
        rstatus = cygetdeviceinfo (devnum, &deviceinfo);
        interfacenum = 0;
        if (!rstatus)
        {
            if (!iscypressdevice (devnum)){
                continue;
            }
	printf ("---------------------------------------------------------------------------------\n");
	printf("device num is %d \n", devnum);
	printf ("---------------------------------------------------------------------------------\n");
            strcpy (functionality, "na");
            numinterfaces = deviceinfo.numinterfaces;
            gldevice[index].numinterface = numinterfaces;
            cydevices++;
            
            while (numinterfaces){
                if (deviceinfo.deviceclass[interfacenum] == cy_class_vendor)
                {
                    gldevice[index].devicenumber = devnum;
                    switch (deviceinfo.devicetype[interfacenum]){
                        case cy_type_i2c:
                            gldevice[index].interfacefunctionality[interfacenum] = cy_type_i2c;
                            strcpy (functionality, "vendor_i2c");
                            gldevice[index].isi2c = true;
                            break;
                        case cy_type_spi:
                            gldevice[index].interfacefunctionality[interfacenum] = cy_type_spi;
                            strcpy (functionality, "vendor_spi");
                            gldevice[index].isspi = true;
                            break;
                        default:
                            strcpy (functionality, "na");
                            break;    
                    }
                }
                else if (deviceinfo.deviceclass[interfacenum] == cy_class_cdc){
                    strcpy (functionality, "na");
                }
                if (isprint) {
                    printf ("%d             |%x  |%x    | %d     | %s\n", \                
                            index, \
                            deviceinfo.vidpid.vid, \
                            deviceinfo.vidpid.pid,  \
                            interfacenum, \
                            functionality \
                            );
                }
                interfacenum++;
                numinterfaces--;
            }
            index++;
        }
    }
    if (isprint){
        printf ("---------------------------------------------------------------------------------\n\n");
    }
}

